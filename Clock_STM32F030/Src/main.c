/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MAX7219\font.h"
#include "MAX7219\f6x8m.h"
#include "MAX7219\Max72xx.h"
#include "DHT\dhtConf.h"
#include "DHT\dht.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BitSet(xByte,yBit) (xByte|=(1<<yBit))
#define BitClear(xByte,yBit) (xByte&= ~(1<<yBit))
#define BitToggle(xByte,yBit) (xByte^=(1<<yBit))
//#define BitFlip(xByte,yBit) (xByte^=(1<<yBit))
#define BitTest(xByte,yBit)  (xByte&(1<<yBit))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sData;
uint8_t needSendData = 0;
DHT_t dht11;
char ScrollBuff[800] = {0};
char WeatherBuffer[100] = {0};
char WeatherTomorrowBuffer[100] = {0};
char WeatherAfterTomorrowBuffer[100] = {0};
char UARTReceiveBuffer[100] = {0};
uint8_t rx_data = 0;
uint16_t rx_index = 0;
uint8_t UARTFlags = 0;
enum E_UARTFlags
{
	E_UARTFlags_UART_RX_COMPLEATE = 0,
	E_UARTFlags_MESSAGE_TOO_LONG,
};
enum E_MyErrore
{	
	E_MyErrore_FAILED_TO_ACCEPT_HTTP_REQUEST = 1,
	E_MyErrore_FAILED_TO_CONNECT_TO_ACCESS_POINT,
	E_MyErrore_TIMED_OUT_WHEN_RECEIVING_DATA_FROM_THE_UART,
	E_MyErrore_NOT_DESCRIBED_ERROR,
};	
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM14_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
//{
//	needSendData = 1;
//    //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
//}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == dht11.pin)
  {
    DHT_pinChangeCallBack(&dht11);
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		if(rx_index == 0)
		{
			for(uint16_t i = 0; i < sizeof(UARTReceiveBuffer); i++)
			{
				UARTReceiveBuffer[i] = 0;
			}
		}
		if(rx_data != 13)
		{
			if(sizeof(UARTReceiveBuffer) -1 == rx_index)
			{
				BitSet(UARTFlags, E_UARTFlags_MESSAGE_TOO_LONG);
				BitSet(UARTFlags, E_UARTFlags_UART_RX_COMPLEATE);
			}
			else
			{
				UARTReceiveBuffer[rx_index++] = rx_data;
				HAL_UART_Receive_IT(&huart1, &rx_data, 1);
			}
		}
		else if(rx_data == 13)
		{
			BitSet(UARTFlags, E_UARTFlags_UART_RX_COMPLEATE);
			//uartMessage_rx_compleate = 1;			
		}
		
	}
}
uint8_t UARTHandleRx()
{
	if(BitTest(UARTFlags, E_UARTFlags_MESSAGE_TOO_LONG))
	{
		BitClear(UARTFlags, E_UARTFlags_MESSAGE_TOO_LONG);
		BitClear(UARTFlags, E_UARTFlags_UART_RX_COMPLEATE);
		rx_index = 0;
		return 0;
	}
	else
	{
		BitClear(UARTFlags, E_UARTFlags_UART_RX_COMPLEATE);
		rx_index = 0;
		return 1;
	}
	//return 0;
}

uint8_t UARTcheckMessageForErrors()
{
	if(strstr(UARTReceiveBuffer, "Errore") != NULL)// my errore
	{
		return *strstr(UARTReceiveBuffer, "Errore")+7;
	}
	else if(atoi(UARTReceiveBuffer) < 0)//httpClient errore
	{
		return 255 + atoi(UARTReceiveBuffer);
	}
//#define HTTPC_ERROR_CONNECTION_FAILED   (-1)
//#define HTTPC_ERROR_SEND_HEADER_FAILED  (-2)
//#define HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
//#define HTTPC_ERROR_NOT_CONNECTED       (-4)
//#define HTTPC_ERROR_CONNECTION_LOST     (-5)
//#define HTTPC_ERROR_NO_STREAM           (-6)
//#define HTTPC_ERROR_NO_HTTP_SERVER      (-7)
//#define HTTPC_ERROR_TOO_LESS_RAM        (-8)
//#define HTTPC_ERROR_ENCODING            (-9)
//#define HTTPC_ERROR_STREAM_WRITE        (-10)
//#define HTTPC_ERROR_READ_TIMEOUT        (-11)
	return 0;
}

uint8_t UARTReadLine(UART_HandleTypeDef *huart, uint32_t rxTimeout)
{
	HAL_UART_Receive_IT(&huart1,&rx_data, 1);
	uint32_t timeOut = 0;
	while(!BitTest(UARTFlags, E_UARTFlags_UART_RX_COMPLEATE))
	{
		HAL_Delay(1);
		if(timeOut++ > rxTimeout)
		{	
			return 3;
		}
	}
	if(UARTHandleRx())
	{
		uint8_t temp = UARTcheckMessageForErrors();
		if(temp == 0)
		{
			return 0;
		}		
		else
			return temp;
	}
	else 
	{
		return 4;
	}
//	return 0;
}

uint8_t UARTSendDataAndReadLine(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t txSize, 
uint32_t txTimeout, uint32_t rxTimeout)
{
	HAL_UART_Transmit(huart, pData, txSize, txTimeout);
	//HAL_Delay(100);
	return UARTReadLine(huart, rxTimeout);
}


uint8_t init_Time(uint32_t TimeOut)
{
	//char tempBuf[40]={0};
//		HAL_UART_Transmit(&huart1, (uint8_t *) "getDateTime", 30, 500);
//		//HAL_Delay(100);
//		HAL_UART_Receive_IT(&huart1,&rx_data, 1);
//		uint8_t timeOut = 0;
//		while(!BitTest(UARTFlags, E_UARTFlags_UART_RX_COMPLEATE))
//		{
//			HAL_Delay(50);
//			if(timeOut++ > 20)
//				break;
//		}
		uint8_t val = UARTSendDataAndReadLine(&huart1, (uint8_t *) "getDateTime", 16, 500, TimeOut);
		if((val >= 244))//get httpClient errore. need read line
		{	
			val = UARTReadLine(&huart1, TimeOut);
		}
		if(val == 0)
		{			
			char* index;
			index	= (char *) strstr(UARTReceiveBuffer, "tm");
			if(index != NULL)
			//if(strstr(UARTReceiveBuffer, "tm") != NULL)
			{				
				//dt 14, 15, 16
				sTime.Hours = (uint8_t) UARTReceiveBuffer[35] - 30;
				sTime.Minutes = (uint8_t) UARTReceiveBuffer[36] - 30;
				sTime.Seconds = (uint8_t) UARTReceiveBuffer[37] - 30;
//				index += 3;
//				sData.Year = (uint8_t) UARTReceiveBuffer[14] - 30;
//				sData.Month = (uint8_t) UARTReceiveBuffer[15] - 30;
//				sData.Date = (uint8_t) UARTReceiveBuffer[16] - 30;
				HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				return 0;
			}
			else 
				return E_MyErrore_NOT_DESCRIBED_ERROR;
		}
		else
			return val;
}

uint8_t getWeather(uint32_t timeOut)
{
	uint8_t j = 0;
	for(uint8_t n = 1; n < 4; n++)
	{
		char tempBuf[16];// = "getWeather_";
		sprintf(tempBuf, "getWeather_%.u    ", n);
//		HAL_UART_Transmit(&huart1, (uint8_t *) tempBuf , 30, 500);
//		//HAL_Delay(100);
//		HAL_UART_Receive_IT(&huart1,&rx_data, 1);
//		uint8_t _timeOut = 0;
//		while(!BitTest(UARTFlags, E_UARTFlags_UART_RX_COMPLEATE))
//		{
//			HAL_Delay(1);
//			if(_timeOut++ > timeOut)
//				break;
//		}
		uint8_t val = UARTSendDataAndReadLine(&huart1, (uint8_t *) tempBuf, 16, 500, timeOut);
		if((val >= 244))//get httpClient errore. need read line
		{	
			val = UARTReadLine(&huart1, timeOut);
		}
		if(val == 0)
		{						
			for(uint16_t i = 0; i < sizeof(WeatherBuffer); i++)
			{
				switch (n)
				{
					case 1:
						WeatherBuffer[i] = UARTReceiveBuffer[i];
						break;
					case 2:
						WeatherTomorrowBuffer[i] = UARTReceiveBuffer[i];
						break;
					case 3:
						WeatherAfterTomorrowBuffer[i] = UARTReceiveBuffer[i];
						break;
				}				
			}
			j++;		  
		}
		else
			return val;
	}
	if(j == 3)
		return 0;
	return E_MyErrore_NOT_DESCRIBED_ERROR;
}

void printTime(void)
{
	char tempBuf[6]={0};
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		if((sTime.Hours < 10)&&(sTime.Minutes < 10))
		{
			sprintf(tempBuf, "0%.u:0%.u", sTime.Hours, sTime.Minutes);
		}
		else if((sTime.Hours < 10))
		{
			sprintf(tempBuf, "0%.u:%.u", sTime.Hours, sTime.Minutes);
		}
		else if((sTime.Minutes < 10))
		{
			sprintf(tempBuf, "%.u:0%.u", sTime.Hours, sTime.Minutes);
		}
		else
		{
			sprintf(tempBuf, "%.u:%.u", sTime.Hours, sTime.Minutes);
		}		
		font_DrawString((uint8_t *) tempBuf, (uint8_t *) ledmatrix_screenbuff, MAX7219_NUM*8);
	  ledmatrix_update_from_buff();
		//demo_ScrollBuff((uint8_t *) ScrollBuff, ScrollLines, 1);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_TIM14_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	ledmatrix_init();
  max7219_set_testmode_onoff(MAX7219_ALL_IDX, 1);
  HAL_Delay(500);
  max7219_set_testmode_onoff(MAX7219_ALL_IDX, 0);
  max7219_set_intensity(MAX7219_ALL_IDX, 5);
  DHT_init(&dht11, DHT_Type_DHT11, &htim14, 48, GPIOA, GPIO_PIN_4);
  __HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	HAL_Delay(1000);
	//uint8_t _init_Time = 0;
	uint16_t ScrollLines = 0;
	while(init_Time(6500) != 0)
	{
		//_init_Time = init_Time();
		//uint16_t ScrollLines = 0;
		ScrollLines = font_DrawString((uint8_t *)"Init time", (uint8_t *) ScrollBuff, 60);
	  demo_ScrollBuff((uint8_t *) ScrollBuff, ScrollLines, 1, 30);
		
	}
	while(getWeather(9000) != 0)
	{
		//_init_Time = init_Time();
		//uint16_t ScrollLines = 0;
		ScrollLines = font_DrawString((uint8_t *)"getWeather", (uint8_t *) ScrollBuff, 60);
	  demo_ScrollBuff((uint8_t *) ScrollBuff, ScrollLines, 1, 30);		
	}
	float temperature, humidity;
	DHT_readData(&dht11, &temperature, &humidity);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		char tempBuf[40]={0};
		
		
	  if (DHT_readData(&dht11, &temperature, &humidity))
	  {		  
		  //sprintf(ScrollBuff,"%.2ft",temperature);//float convert to string
		  sprintf(tempBuf,"%.1ft %.1fh", temperature, humidity);//float convert to string
		  ScrollLines = font_DrawString((uint8_t *) tempBuf, (uint8_t *)ScrollBuff, 100);
		  //ledmatrix_update_from_buff();
		  demo_ScrollBuff((uint8_t *) ScrollBuff, ScrollLines, 1, 18);
		  //HAL_Delay(2000);
		  //sprintf(ScrollBuff,"%.2fh", humidity);//float convert to string
		  //font_DrawString((uint8_t *) ScrollBuff, ledmatrix_screenbuff, MAX7219_NUM * 8);
		  //ledmatrix_update_from_buff();
		  //HAL_Delay(2000);
	  }
		printTime();
		HAL_Delay(10000);
		ScrollLines = font_DrawString((uint8_t *) WeatherBuffer, (uint8_t *) ScrollBuff, 799);
	  demo_ScrollBuff((uint8_t *) ScrollBuff, ScrollLines, 1, 25);
		printTime();
		HAL_Delay(10000);
		ScrollLines = font_DrawString((uint8_t *) WeatherTomorrowBuffer, (uint8_t *) ScrollBuff, 799);
	  demo_ScrollBuff((uint8_t *) ScrollBuff, ScrollLines, 1, 25);
		printTime();
		HAL_Delay(10000);
		ScrollLines = font_DrawString((uint8_t *) WeatherAfterTomorrowBuffer, (uint8_t *) ScrollBuff, 799);
	  demo_ScrollBuff((uint8_t *) ScrollBuff, ScrollLines, 1, 25);
		//showBuffer((uint8_t *) ScrollBuff);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
    
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date 
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 0;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 0;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
