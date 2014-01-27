//******************************************************************************
//            ЗАГОЛОВОЧНЫЙ ФАЙЛ БИБЛИОТЕКИ ФУНКЦИЙ МОДУЛЯ GSM
//******************************************************************************

//*****************Подключаемые файлы*******************************************
#include "stm32f10x.h"
//***************Макроопределения***********************************************
#define GSM_MOD GPIOC
#define GSM_ON GPIO_Pin_0
#define GSM_RESET GPIO_Pin_1
#define GSM_READY GPIO_Pin_2
#define GSM_RING GPIO_Pin_3
#define SIM1 GPIO_Pin_13
#define SIM2 GPIO_Pin_14
#define SIM3 GPIO_Pin_15
#define Beeline_OP 1
#define MTS_OP 2
#define Megafon_OP 3

//**************Определение собственных типов***********************************
typedef enum //Определение СМС ответа
{
 SECURITY_OFF,
 SECURITY_ON,
 MOTOR_ON,
 MOTOR_START_ERROR,
 MOTOR_OFF,
 AUTOHEAT_ON_0530,
 AUTOHEAT_ON_1030,
 AUTOHEAT_ON_3030,
 AUTOHEAT_ON_3020,
 AUTOHEAT_OFF,
 CHANGE_HEAT_MODE_0530, 
 CHANGE_HEAT_MODE_1030, 
 CHANGE_HEAT_MODE_3030,
 CHANGE_HEAT_MODE_3020,
 SHOCK_ALARM,
 HOOD_ALARM,
 DOOR_ALARM,
 TRUNK_ALARM
}Answer_TypeDef;

//********* Инициализация глобальных переменных ****************************
extern const char PHONE_NUMBER[];

extern char IMEI[]; //Массив для IMEI

extern uint8_t SendDataError;

//*************Объявление испльзуемых функций***********************************
void SIM(uint8_t sim); //Функция переключения СИМ карт
void GSM_Configuration(uint8_t sim); //Инициализация GSM
char REG_NET(void);  //Проверка регистации в сети
void SendData_onServer(uint16_t state, uint8_t rmc_buf);  //Функция отправки данных на сервер
void ANSWER_CALL(void); //Функция ответа на входящий звонок
void RECEIVE_SMS(void); //Функция получения СМС сообщения
void SEND_SMS(Answer_TypeDef answer); //Функция отправки СМС








/***************************КОНЕЦ ФАЙЛА****************************************/