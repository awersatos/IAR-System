//******************************************************************************
//              ОСНОВНОЙ ЗАГОЛОВОЧНЫЙ ФАЙЛ ПРОЕКТА
//******************************************************************************

//**************Подключаемые файлы**********************************************
#include "stm32f10x.h"
//#include "Alarm.h"
//**************Макроопределения************************************************

#define TX_BufferSize 64       //Размер буферов  передачи  байт
#define RX_BufferSize 192       //Размер буферов приема байт
#define  Bluetooth  1
#define  GSM        2
#define  Navigator  3
#define  ACTIVE ENABLE
#define  INACTIVE DISABLE

#define OPEN 'O'   // Команды управления
#define CLOSE 'C'
#define ALARM 'A'

       
//**************Определение собственных типов***********************************



typedef enum //Определение режима автопрогрева
{
MODE_AUTOHEAT_OFF,
MODE_AUTOHEAT_0530,
MODE_AUTOHEAT_1030,
MODE_AUTOHEAT_3030,
MODE_AUTOHEAT_3020
}Autoheatmode_TypeDef;



typedef struct
{
FunctionalState MainPower; //Основное питание

uint16_t BatteryCharge;  //Заряд батареи

FunctionalState NavigatorStatus; //Статус навигатора

uint8_t CoordinatesStatus; //Валидность координат

FunctionalState GSM_Status;//Статус GSM

FunctionalState GSM_DataMode; //Режим передачи данных

FunctionalState AUTOSTART; //Статус автозапуска

Autoheatmode_TypeDef AUTOHEAT_MODE; //Режим автозапуска

uint8_t SIM_Card; //Номер СИМ карты

uint8_t OPERATOR;

FunctionalState SecurityStatus; //Статус охраны

uint8_t COMMAND;//Статус команды

uint16_t EVENT_BUF[3]; //Буфер событий

FunctionalState Transceiver_Status; //Статус трансивера

FunctionalState LONG_ALARM; //Статус активности длительного звучания сирены


}StatusStruct_TypeDef; //Структура переменной статуса устройства


//**************Глобальные переменные доступные из других файлов****************
extern StatusStruct_TypeDef STATUS; //Структурная переменная статуса устройства
extern FunctionalState delay_EnableStatus; //Статус активности программы задержки
extern uint32_t delay_Counter; //Интервал программы задержки

extern const char SERIAL_NUMBER[]; //Серийный номер устройства
extern const char BRELOK_SN[]; //Серийный номер брелка

extern const uint8_t KEY[]; //Ключ шифрования 
extern uint8_t BUFER[16]; //Буфер временных данных
extern FunctionalState CRYPT; //Статус шифрования

extern char Bluetooth_TxBuffer[TX_BufferSize+1]; //Передающий Bluetooth буфер USART1
extern char Bluetooth_RxBuffer[RX_BufferSize+1]; //Приемный Bluetooth буфер USART1
extern char GSM_TxBuffer[TX_BufferSize+1];       //Передающий GSM буфер USART2
extern char GSM_RxBuffer[RX_BufferSize+1];       //Приемный GSM буфер USART2
extern char Navi_TxBuffer[TX_BufferSize+1];      //Передающий Navigation буфер USART3
extern char Navi_RxBuffer[RX_BufferSize+1];      //Приемный Navigation буфер USART3

extern uint16_t sec_cnt;
//*************Объявление испльзуемых функций***********************************
void delay_ms(uint16_t msec); //Задержка в милисекундах
void delay_us(uint32_t usec); //Задержка в микросекундах
void RCC_Configuration(void); //Тактирование и настройка периферийных устройств
void GPIO_Configuration(void); //Настройка параметров портов ввода вывода
void NVIC_Configuration(void); //Инициализация и настройка прерываний
void ADC_Configuration(void);//Инициализация АЦП
void TIMER_Configuration(void); //Инициализация таймеров
void UART_Configuration(void);//Инициализация USART
void DMA_Configuration(void); //Инициализация каналов DMA
void ClearBufer(char *buf); //Функция очистки буфера
void SendString_InUnit(const char *str , uint8_t Unit); //Функция отправки строки навнешний модуль через UART
void Reset_rxDMA_ClearBufer(uint8_t Unit); //Сбросприемного DMA канала и очистка буфера приема
void BKP_Configuration(void); //Конфигурация резервного домена питания
void COMAND_EXEC(void);  //Исполнитель команд
void EXTI_Configuration(void); //Инициализация контроллера внешних прерываний
void SPI_Configuration(void); //Инициализация SPI
void IWDG_Configuration(void);//Инициализация сторожевого таймера
void RTC_Configuration(void); //Конфигурацмя часов реального времени

/***************************КОНЕЦ ФАЙЛА****************************************/