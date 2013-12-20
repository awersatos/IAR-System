//******************************************************************************
//              ОСНОВНОЙ ЗАГОЛОВОЧНЫЙ ФАЙЛ ПРОЕКТА
//******************************************************************************

//**************Подключаемые файлы**********************************************
#include "stm32f10x.h"

//**************Макроопределения************************************************

#define TX_BufferSize 64       //Размер буферов  передачи  байт
#define RX_BufferSize 192       //Размер буферов приема байт
#define  Bluetooth  1
#define  GSM        2
#define  Navigator  3
#define  ACTIVE ENABLE
#define  INACTIVE DISABLE
//**************Определение собственных типов***********************************
typedef struct
{
FunctionalState MainPower; //Основное питание

uint16_t BatteryCharge;  //Заряд батареи

FunctionalState NavigatorStatus; //Статус навигатора

uint8_t CoordinatesStatus; //Валидность координат

FunctionalState GSM_Status;//Статус GSM

FunctionalState AUTOSTART; //Статус автозапуска

uint8_t SIM_Card; //Номер СИМ карты

uint8_t OPERATOR;

FunctionalState SecurityStatus; //Статус охраны

uint8_t COMMAND;//Статус команды


}StatusStruct_TypeDef; //Структура переменной статуса устройства


//**************Глобальные переменные доступные из других файлов****************
extern StatusStruct_TypeDef STATUS; //Структурная переменная статуса устройства
extern FunctionalState delay_EnableStatus; //Статус активности программы задержки
extern uint32_t delay_Counter; //Интервал программы задержки

extern const char SERIAL_NUMBER[]; //Серийный номер устройства

extern char Bluetooth_TxBuffer[TX_BufferSize+1]; //Передающий Bluetooth буфер USART1
extern char Bluetooth_RxBuffer[RX_BufferSize+1]; //Приемный Bluetooth буфер USART1
extern char GSM_TxBuffer[TX_BufferSize+1];       //Передающий GSM буфер USART2
extern char GSM_RxBuffer[RX_BufferSize+1];       //Приемный GSM буфер USART2
extern char Navi_TxBuffer[TX_BufferSize+1];      //Передающий Navigation буфер USART3
extern char Navi_RxBuffer[RX_BufferSize+1];      //Приемный Navigation буфер USART3
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


/***************************КОНЕЦ ФАЙЛА****************************************/