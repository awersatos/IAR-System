//******************************************************************************
//              ОСНОВНОЙ ЗАГОЛОВОЧНЫЙ ФАЙЛ ПРОЕКТА
//******************************************************************************

//**************Подключаемые файлы**********************************************
#include "stm32f10x.h"

//**************Макроопределения************************************************
#define red GPIO_Pin_0
#define green GPIO_Pin_1
#define BUZZER_PIN_1 GPIO_Pin_8
#define BUZZER_PIN_2 GPIO_Pin_9
#define BUTTON_1 GPIO_Pin_8
#define BUTTON_2 GPIO_Pin_9

#define OPEN 'O'   // Команды управления
#define CLOSE 'C'
#define ALARM 'A'
//**************Определение собственных типов***********************************

typedef enum  //Определение цвета светодиода
{
 OFF,
 GREEN,
 RED,
 ORANGE 
}Color_TypeDef;

typedef enum //Управление пищалкой
{
  Long_Bip_1, 
  Medium_Bip_2,
  Medium_Bip_3,
  Short_Bip_15,
  
}Bip_TypeDef;


//**************Глобальные переменные доступные из других файлов****************
extern FunctionalState delay_EnableStatus; //Статус активности программы задержки
extern uint32_t delay_Counter; //Интервал программы задержки
extern FunctionalState Transceiver_Status; //Статус активности трансивера
extern uint8_t KEY[]; //Ключ шифрования 
extern uint8_t BUFER[16]; //Массив шифрования
extern FunctionalState CRYPT; //Статус шифрования

//*************Объявление испльзуемых функций***********************************
void delay_ms(uint16_t msec); //Задержка в милисекундах
void RCC_Configuration(void); //Тактирование и настройка периферийных устройств
void GPIO_Configuration(void); //Настройка параметров портов ввода вывода
void EXTI_Configuration(void); //Инициализация контроллера внешних прерываний
void NVIC_Configuration(void); //Инициализация и настройка прерываний
void SPI_Configuration(void); //Инициализация SPI
void TIMER_Configuration(void); //Инициализация таймера
void LED(Color_TypeDef COLOR); //Функция управления светодиодом
void BUZZER(Bip_TypeDef BIP); //Функция управления пьезодинамиком
/***************************КОНЕЦ ФАЙЛА****************************************/
