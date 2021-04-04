
#ifndef __MAX72XX
#define __MAX72XX
//#include "types.h"
#include "stm32f0xx_hal.h"

//=============================================
//Work with matrix low leavel
//=============================================

// Расположение пина управления сигналом CS у max7219
#define MAX7219_CS_Port         GPIOA
#define MAX7219_CS_Pin          GPIO_Pin_4

// Кол-во MAX7219 в цепочке
#define MAX7219_NUM             4


// Коды команд MAX7219
#define MAX7219_CMD_NO_OP       0x0
#define MAX7219_CMD_DIGIT_0     0x1
#define MAX7219_CMD_DIGIT_1     0x2
#define MAX7219_CMD_DIGIT_2     0x3
#define MAX7219_CMD_DIGIT_3     0x4
#define MAX7219_CMD_DIGIT_4     0x5
#define MAX7219_CMD_DIGIT_5     0x6
#define MAX7219_CMD_DIGIT_6     0x7
#define MAX7219_CMD_DIGIT_7     0x8
#define MAX7219_CMD_DECODE_MODE 0x9
#define MAX7219_CMD_INTENSITY   0xA
#define MAX7219_CMD_SCAN_LIMIT  0xB
#define MAX7219_CMD_SHUTDOWN    0xC
#define MAX7219_CMD_DISP_TEST   0xF


// Варианты декодирования цифр для команды DECODE MODE (используется с 7-сегментными индикаторами)
#define MAX7219_NO_DECODE       0x00
#define MAX7219_0_FOR_71        0x01
#define MAX7219_30_FOR_74       0x0F
#define MAX7219_DECODE_FOR_70   0xFF

// Индекс микросхемы max7219, указывающий что команду+данные нужно записать во все микросхемы цепочки
#define MAX7219_ALL_IDX         0xFF


// Процедура инициализирует ножки, используемые интерфейсом обмена с max7219
void max7219_GPIO_init(void);
// Процедура инициализирует SPI
//void max7219_SPI_init(void);
// Процедура инициализирует интерфейс обмена с цепочкой max7219
void max7219_init(void);
// Процедура отправляет команду с данным в один или во все max7219 в цепочке
void max7219_send(uint8_t MAX_Idx, uint8_t Cmd, uint8_t Data);
// Процедура отправляет массив команд в max7219
void max7219_sendarray(uint16_t *pArray);
// Процедура отправляет 16-битное слово по SPI
void max7219_send16bit(uint16_t Word);
// Процедура устанавливает режим декодирования символов в 1 или во всех max7219
void max7219_set_decodemode(uint8_t MAX_Idx, uint8_t DecodeMode);
// Процедура устанавливает яркость в 1 или во всех max7219
void max7219_set_intensity(uint8_t MAX_Idx, uint8_t Intensity);
// Процедура устанавливает кол-во знаков/строк в 1 или во всех max7219
void max7219_set_scan_limit(uint8_t MAX_Idx, uint8_t Limit);
// Процедура включает/выключает max7219. После подачи питания он выключен
void max7219_set_run_onoff(uint8_t MAX_Idx, uint8_t On);
// Процедура включает/выключает тестовый режим max7219 (горят все индикаторы)
void max7219_set_testmode_onoff(uint8_t MAX_Idx, uint8_t On);

//=============================================
//
//=============================================

extern uint8_t ledmatrix_screenbuff[];


// Процедура инициализирует матрицу
void ledmatrix_init(void);
// Процедура управляет режимом Test матрицы
void ledmatrix_testmatrix(uint8_t TestOn);
// Процедура устанавливает яркость матрицы. Диапазон 0-15
void ledmatrix_set_brightness(uint8_t Value);
// Процедура заполняет буфер кадра значением FillValue
void ledmatrix_fill_screenbuff(uint8_t FillValue);
// Процедура сдвигает содержимое буфера кадра ВЛЕВО
// Самый правый столбец сохраняет при этом своё старое значение
void ledmatrix_ScrollLeft(void);
// Процедура сдвигает содержимое буфера кадра ВПРАВО
// Самый левый столбец сохраняет при этом своё старое значение
void ledmatrix_ScrollRight(void);
// Процедура обновляет состояние индикаторов в соответствии с буфером экрана ledmatrix_screenbuff
void ledmatrix_update_from_buff(void);

//=============================================
//work with matrix high leavel
//=============================================

// Процедура прокручивает на матрице буфер pBuff (ВЛЕВО/ВПРАВО)
void demo_ScrollBuff(uint8_t *pBuff, uint16_t ScrollLines, uint8_t RightToLeft, uint8_t speed);
// Процедура показывает по частям на матрице буфер pBuff (ВЛЕВО/ВПРАВО)
void showBuffer(uint8_t *pBuff);

#endif
