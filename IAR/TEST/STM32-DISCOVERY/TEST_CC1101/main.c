/*******************************************************************************
********************************************************************************
**                                                                            **
**                  ОСНОВНОЙ ФАЙЛ ПРОЕКТА TEST_CC1101                         **
**                                                                            **
********************************************************************************
*******************************************************************************/

//****************Подключаемые файлы********************************************
#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "main.h"
#include "Transceiver_433MHz.h"
#include "STM32vldiscovery.h"
//************* Инициализация глобальных переменных ****************************
FunctionalState delay_EnableStatus = DISABLE; //Статус активности программы задержки
uint32_t delay_Counter; //Интервал программы задержки



//******************************************************************************

void main(void) //Основная функция программы
{
       /*ИНИЦИАЛИЗАЦИЯ ПЕРЕФЕРИЙНЫХ УСТРОЙСТВ КОНТРОЛЛЕРА*/
RCC_Configuration(); //Включение тактироания и настройка перриферийных устройств
GPIO_Configuration(); //Инициализация портов ввода вывода
EXTI_Configuration(); //Инициализация контроллера внешних прерываний
NVIC_Configuration();//Инициализация прерываний
SPI_Configuration();//Инициализация SPI 
SysTick_Config(SystemCoreClock/100000); //Преиод счета системного таймера 1 мкС
NVIC_SetPriority(SysTick_IRQn,0); //Приоритетпрерывания системного таймера наивысший

STM32vldiscovery_LEDInit(LED3); //Инициализация сетодиодов отладочной платы
STM32vldiscovery_LEDInit(LED4);

STM32vldiscovery_LEDOn(LED3); //Включить светодиод





              /*ИНИЦИАЛИЗАЦИЯ ВНЕШНИХ МОДУЛЕЙ*/
Transceiver_Configuration(); //Инициализация трансивера
//*****************************************************************************


//SPI_buffer[0]=STATUS_TR();
//STM32vldiscovery_LEDOn(LED4); //Включить светодиод


  
 while(1) //Основной цикл программы
 {
 
 }
 
 
}

      /*КОНЕЦ ОСНОВНОЙ ФУНКЦИИ ПРОГРАММЫ*/



void delay_ms(uint16_t msec) //Задержка в милисекундах
{
  delay_us(msec*100);
}  

void delay_us(uint32_t usec) //Задержка в микросекундах
{
  delay_Counter=usec; //Присваиваем счетчику параметр
  delay_EnableStatus = ENABLE; //Активируем задержку
  while(delay_Counter); //Цикл пока счетчик не равен 0
  delay_EnableStatus = DISABLE; //Деактивируем задержку
  
}  

void RCC_Configuration(void) //Тактирование и настройка периферийных устройств
{
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE); //Включение тактирования портов A,B
RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO , ENABLE); //Включение тактирования альтернативных функций GPIO
RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 , ENABLE); //Разрешить тактирование SPI1
  
  
}

void GPIO_Configuration(void) //Настройка параметров портов ввода вывода
{
 GPIO_InitTypeDef GPIO_Init_struct; // Обявляем структуру инициализации портов

 //Порт А0- вход с подтяжкой к - (вход WKUP)
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_0;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А2-A3- входы с подтяжкой к - (сервисные входы трансивера TR_GP1 , TR_GP2)
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_2 | GPIO_Pin_3;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А4-двухтактный  выход SPI_SS
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_4;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А5-двухтактный альтернативный выход SPI_SCK
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_5;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_50MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А6- вход 3-состояние SPI_MISO
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_6;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IN_FLOATING; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А7-двухтактный альтернативный выход SPI_MOSI
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_7;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_50MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию




  
/* Подключение линий внешних прерываний*/
GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2); //Линия прерывания TR_GP1

}

void EXTI_Configuration(void) //Инициализация контроллера внешних прерываний
{
  EXTI_InitTypeDef   EXTI_InitStructure; //Объявляем структуру для инициализации внешних прерываний

  EXTI_InitStructure.EXTI_Line = EXTI_Line2; //Линия прерывания 2 TR_GP1
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //Режим прерывания
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //Прерывание по положительному фронту
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //Прерывание активно
  EXTI_Init(&EXTI_InitStructure); //Прередаем структуру в функцию
}

void NVIC_Configuration(void) //Инициализация и настройка прерываний
{
NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//Кофигурация групп приоритетов:2 подгруппы
NVIC_InitTypeDef NVIC_InitStruct; //Объявляем структуру настройки прерываний

NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn; //Прерывания внешние линии 2 от TR_GP1
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //Приоритетная подгруппа
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; //Приоритет
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //Активируем прерывание
NVIC_Init(&NVIC_InitStruct); //Передаем структуру в функцию





}

void SPI_Configuration(void) //Инициализация SPI
{
 SPI_InitTypeDef  SPI_InitStructure; //Инициализация структуры для настройки SPI 
 SPI_StructInit(&SPI_InitStructure); //Очиятка полей структуры
 
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //Полнодуплексный режим работы SPI
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //Режим работы Мастер
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //Размерность данных 8 бит
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; //Полярность тактового сигнала отрицательная
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; //Фаза начало цикла
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; //Программное управление NSS
  SPI_InitStructure.SPI_BaudRatePrescaler =  SPI_BaudRatePrescaler_8 ; //Предделитель 8 тактовая частота 3 МГц
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //Старший бит идет первым
  SPI_InitStructure.SPI_CRCPolynomial = 8; //Полином для рассчета CRC
  
  SPI_Init(SPI1, &SPI_InitStructure); //Передаем структуру в функцию

  
  
}




/*******************************************************************************
********************************************************************************
**                                                                            **
**                        КОНЕЦ ПРОГРАММЫ                                     **
**                                                                            **
********************************************************************************
*******************************************************************************/
