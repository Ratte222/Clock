#include "Max72xx.h"
#include "stm32f0xx_hal.h"

#define MAX7219_CS_HIGH()       HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET)
#define MAX7219_CS_LOW()        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET)

//=============================================
//Work with matrix low leavel
//=============================================


extern SPI_HandleTypeDef hspi1;
//==============================================================================
// Процедура инициализирует ножки, используемые интерфейсом обмена с max7219
//==============================================================================
void max7219_GPIO_init(void)
{
//  GPIO_InitTypeDef GPIO_InitStruct;
//
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//
//  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
//  GPIO_InitStruct.GPIO_Pin = (1 << 5) | (1 << 7);
//  GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
//  GPIO_InitStruct.GPIO_Pin = MAX7219_CS_Pin;
//  GPIO_Init(MAX7219_CS_Port, &GPIO_InitStruct);

  MAX7219_CS_HIGH();
}
//==============================================================================


//==============================================================================
// Процедура инициализирует SPI
//==============================================================================
//void max7219_SPI_init(void)
//{
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
//
//  SPI_InitTypeDef SPI_InitStruct;
//
//  SPI_InitStruct.SPI_Direction = SPI_Direction_1Line_Tx;
//  SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
//  SPI_InitStruct.SPI_DataSize = SPI_DataSize_16b;
//  SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
//  SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
//  SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
//  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//16;
//  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
//  SPI_InitStruct.SPI_CRCPolynomial = 0x7;
//
//  SPI_Init(SPI1, &SPI_InitStruct);
//  SPI_Cmd(SPI1, ENABLE);
//}
//==============================================================================


//==============================================================================
// Процедура инициализирует интерфейс обмена с цепочкой max7219
//==============================================================================
void max7219_init(void)
{
  //max7219_SPI_init();
  max7219_GPIO_init();
}
//==============================================================================


//==============================================================================
// Процедура отправляет команду с данным в один или во все max7219 в цепочке
//==============================================================================
void max7219_send(uint8_t MAX_Idx, uint8_t Cmd, uint8_t Data)
{
  uint16_t max7219_SpiBuff[MAX7219_NUM];
  uint16_t Word = Data | ((uint16_t) Cmd << 8);

  for (uint8_t i = 0; i < MAX7219_NUM; i++)
  {
    if (MAX_Idx == 0xFF)  // Нужно записать во все max7219 в цепочке
      max7219_SpiBuff[i] = Word;
    else                  // Нужно записать только в один max7219
    {
      if (i == MAX_Idx)         // Та микросхема max7219, в которую нужно записать команду/данные
        max7219_SpiBuff[i] = Word;
      else                      // max7219, которому нет данных на запись
        max7219_SpiBuff[i] = 0x00 | ((uint16_t) MAX7219_CMD_NO_OP << 8);
    }
  }

  // Пишем пачку слов во все подключенные max7219
  max7219_sendarray((uint16_t *) max7219_SpiBuff);
}
//==============================================================================


//==============================================================================
// Процедура отправляет массив команд в max7219
//==============================================================================
void max7219_sendarray(uint16_t *pArray)
{
  MAX7219_CS_LOW();

  for (uint8_t i = 0; i < MAX7219_NUM; i++)
    max7219_send16bit(*(pArray++));

  MAX7219_CS_HIGH();
}
//==============================================================================


//==============================================================================
// Процедура отправляет 16-битное слово по SPI
//==============================================================================
void max7219_send16bit(uint16_t Word)
{
  //SPI_I2S_SendData(SPI1, Word);
  //while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET)  {}
	uint8_t pData[2] = {Word>>8, Word};
	HAL_SPI_Transmit(& hspi1, (uint8_t *) pData, 2, HAL_MAX_DELAY);
	//uint8_t temp = Word << 8;
	//HAL_SPI_Transmit(&hspi1, &temp, 1, HAL_MAX_DELAY);
	//temp = Word >> 8;
	//HAL_SPI_Transmit(&hspi1, &temp, 1, HAL_MAX_DELAY);
}
//==============================================================================


//==============================================================================
// Процедура устанавливает режим декодирования символов в 1 или во всех max7219
//==============================================================================
void max7219_set_decodemode(uint8_t MAX_Idx, uint8_t DecodeMode)
{
  max7219_send(MAX_Idx, MAX7219_CMD_DECODE_MODE, DecodeMode);
}
//==============================================================================


//==============================================================================
// Процедура устанавливает яркость в 1 или во всех max7219
//==============================================================================
void max7219_set_intensity(uint8_t MAX_Idx, uint8_t Intensity)
{
  if (Intensity > 15)
    Intensity = 15;

  max7219_send(MAX_Idx, MAX7219_CMD_INTENSITY, Intensity);
}
//==============================================================================


//==============================================================================
// Процедура устанавливает кол-во знаков/строк в 1 или во всех max7219
//==============================================================================
void max7219_set_scan_limit(uint8_t MAX_Idx, uint8_t Limit)
{
  if (Limit > 7)
    Limit = 7;

  max7219_send(MAX_Idx, MAX7219_CMD_SCAN_LIMIT, Limit);
}
//==============================================================================


//==============================================================================
// Процедура включает/выключает max7219. После подачи питания он выключен
//==============================================================================
void max7219_set_run_onoff(uint8_t MAX_Idx, uint8_t On)
{
  if (On)
    On = 1;

  max7219_send(MAX_Idx, MAX7219_CMD_SHUTDOWN, On);
}
//==============================================================================


//==============================================================================
// Процедура включает/выключает тестовый режим max7219 (горят все индикаторы)
//==============================================================================
void max7219_set_testmode_onoff(uint8_t MAX_Idx, uint8_t On)
{
  if (On)
    On = 1;

  max7219_send(MAX_Idx, MAX7219_CMD_DISP_TEST, On);
}
//==============================================================================

uint8_t ledmatrix_screenbuff[MAX7219_NUM * 8];


//==============================================================================
// Процедура инициализирует матрицу
//==============================================================================
void ledmatrix_init(void)
{
  max7219_init();

  // Выводим все max7219 из спящего режима
  max7219_set_run_onoff(MAX7219_ALL_IDX, 1);
  // Устанавливаем кол-во символов (или строк), подключенных к max7219
  max7219_set_scan_limit(MAX7219_ALL_IDX, 0x07);
  // Устанавливаем режим перекодирования кодов цифр перед выводом на индикатор (для матриц =0)
  max7219_set_decodemode(MAX7219_ALL_IDX, MAX7219_NO_DECODE);
  // Устанавливаем яркость индикаторов (от 0 до 15)
  max7219_set_intensity(MAX7219_ALL_IDX, 0xF);

  // Очищаем ОЗУ микросхем max7219, потому что сейчас в них может быть мусор
  ledmatrix_fill_screenbuff(0);
  ledmatrix_update_from_buff();
}
//==============================================================================


//==============================================================================
// Процедура управляет режимом Test матрицы
//==============================================================================
void ledmatrix_testmatrix(uint8_t TestOn)
{
  max7219_set_testmode_onoff(MAX7219_ALL_IDX, TestOn);
}
//==============================================================================


//==============================================================================
// Процедура устанавливает яркость матрицы. Диапазон 0-15
//==============================================================================
void ledmatrix_set_brightness(uint8_t Value)
{
  max7219_set_intensity(MAX7219_ALL_IDX, Value);
}
//==============================================================================


//==============================================================================
// Процедура заполняет буфер кадра значением FillValue
//==============================================================================
void ledmatrix_fill_screenbuff(uint8_t FillValue)
{
  for (uint16_t i = 0; i < (8 * MAX7219_NUM); i++)
    ledmatrix_screenbuff[i] = FillValue;
}
//==============================================================================


//==============================================================================
// Процедура сдвигает содержимое буфера кадра ВЛЕВО
// Самый правый столбец сохраняет при этом своё старое значение
//==============================================================================
void ledmatrix_ScrollLeft(void)
{
  for (uint16_t col = 1; col < (MAX7219_NUM * 8); col++)
    ledmatrix_screenbuff[col - 1] = ledmatrix_screenbuff[col];
}
//==============================================================================


//==============================================================================
// Процедура сдвигает содержимое буфера кадра ВПРАВО
// Самый левый столбец сохраняет при этом своё старое значение
//==============================================================================
void ledmatrix_ScrollRight(void)
{
  for (uint16_t col = (MAX7219_NUM * 8) - 1; col; col--)
    ledmatrix_screenbuff[col] = ledmatrix_screenbuff[col - 1];
}
//==============================================================================


//==============================================================================
// Процедура обновляет состояние индикаторов в соответствии с буфером кадра ledmatrix_screenbuff
//==============================================================================
void ledmatrix_update_from_buff(void)
{
  uint16_t max7219_SpiBuff[MAX7219_NUM];

  for (uint8_t Digit = 0; Digit < 8; Digit++)
  {
    for (uint8_t i = 0; i < MAX7219_NUM; i++)
    {
      // Формирование слова для записи в max7219[i] в строку Digit
      uint8_t Data = 0;
      for (uint8_t Col = 0; Col < 8; Col++)
      {
        uint8_t bit = (ledmatrix_screenbuff[(i << 3) + Col] & (1 << Digit)) ? 1 : 0;
        Data |= (bit << (7 - Col));
      }
      max7219_SpiBuff[i] = Data | ((uint16_t) (Digit + 1) << 8);
    }

    // Записываем одну строку во все max7219
    max7219_sendarray(max7219_SpiBuff);
  }
}


//=============================================
//work with matrix high leavel
//=============================================

//==============================================================================
// Процедура прокручивает на матрице буфер pBuff (ВЛЕВО/ВПРАВО)
//==============================================================================
void demo_ScrollBuff(uint8_t *pBuff, uint16_t ScrollLines, uint8_t RightToLeft, uint8_t speed)
{
  uint16_t ScrollIdx = (RightToLeft) ? 0 : ScrollLines - 1;

  ledmatrix_fill_screenbuff(0x00);  // Очистка буфера кадра

  for (uint16_t i = 0; i < ScrollLines + (MAX7219_NUM * 8); i++)
  {
    if (RightToLeft)    // Прокрутка справа налево
    {
      ledmatrix_ScrollLeft();

      if (i < ScrollLines)
        ledmatrix_screenbuff[(MAX7219_NUM * 8) - 1] = pBuff[ScrollIdx++];
      else
        ledmatrix_screenbuff[(MAX7219_NUM * 8) - 1] = 0;
    }
    else                // Прокрутка слева направо
    {
      ledmatrix_ScrollRight();

      if (i < ScrollLines)
        ledmatrix_screenbuff[0] = pBuff[ScrollIdx--];
      else
        ledmatrix_screenbuff[0] = 0;
    }

    ledmatrix_update_from_buff();

    HAL_Delay((1000/speed));
  }
}

//==============================================================================
// Процедура показывает по частям на матрице буфер pBuff (ВЛЕВО/ВПРАВО)
//==============================================================================
void showBuffer(uint8_t *pBuff)
{
	uint16_t size_pBuff = sizeof(pBuff);
	ledmatrix_fill_screenbuff(0x00);  // Очистка буфера кадра
	for(uint16_t i = 0; i < (size_pBuff / (MAX7219_NUM * 8)); i+=(MAX7219_NUM * 8))
	{
		for(uint16_t j = 0; j < (MAX7219_NUM * 8); j++)
		{
			if(j+i < size_pBuff-1)
				ledmatrix_screenbuff[j] = pBuff[j+i];
			else
				ledmatrix_screenbuff[j] = 0;
		}
		ledmatrix_update_from_buff();
		HAL_Delay(1000);
	}
}
