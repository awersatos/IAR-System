//******************************************************************************
//            ЗАГОЛОВОЧНЫЙ ФАЙЛ БИБЛИОТЕКИ ФУНКЦИЙ МОДУЛЯ GSM
//******************************************************************************

//*****************Подключаемые файлы*******************************************
#include "stm32f10x.h"

//***************Макроопределения***********************************************

#define ALARM1 GPIOB
/******/
#define IGN1_IN     GPIO_Pin_2
#define IGN1_OUT    GPIO_Pin_5
#define ST_IN       GPIO_Pin_6
#define ST_OUT      GPIO_Pin_7
#define IGN2        GPIO_Pin_8
#define ACC         GPIO_Pin_9
#define MOTOR_CTRL  GPIO_Pin_14
#define BRAKE_CTRL  GPIO_Pin_15
//==============================================================================

#define ALARM2 GPIOC
/******/
#define  DOOR_OP    GPIO_Pin_4
#define  DOOR_CL    GPIO_Pin_5
#define  LIGHTS     GPIO_Pin_6
#define  SIREN      GPIO_Pin_7
#define  SHOCK1     GPIO_Pin_8
#define  SHOCK2     GPIO_Pin_9
#define  TRUNK_TR   GPIO_Pin_10
#define  DOOR_TR    GPIO_Pin_11
#define  HOOD_TR    GPIO_Pin_12
//==============================================================================

#define ALARM3 GPIOD
/******/
#define M_LOCK    GPIO_Pin_2



//************* Инициализация глобальных переменных ****************************

//*************Объявление испльзуемых функций***********************************
void SECURITY(FunctionalState stat); //Функция снятия/постановки на охрану и закрывания открывания дверей
void AUTOSTART(FunctionalState status); //Дистанционный запуск/остановка двигателя
void SIREN_and_LIGHTS(uint8_t flash); //Управление миганием фар и сиреной


/***************************КОНЕЦ ФАЙЛА****************************************/