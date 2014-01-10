/*******************************************************************************
********************************************************************************
**                                                                            **
**                  ОСНОВНОЙ ФАЙЛ ПРОЕКТА MFU v1.00                           **
**                                                                            **
********************************************************************************
*******************************************************************************/

//Корректировка  10,01,2014

//****************Подключаемые файлы********************************************
#include "stm32f10x_conf.h"
#include "eeprom.h"
#include "stm32f10x.h"
#include "main.h"
#include "navigation.h"
#include "Bluetooth.h"
#include "GSM.h"
#include "Alarm.h"
#include <string.h>
#include "Transceiver_433MHz.h"
#include "CRYPTO-AES.h"
#include "Service.h"
//************* Инициализация глобальных переменных ****************************




const char SERIAL_NUMBER[] = "1231231230" ; //Серийный номер устройства
const char BRELOK_SN[] = "BR1234567890" ; //Серийный номер брелка

const uint8_t KEY[16]=     //Ключ шифрования
{  'B', 'R', 'E', 'L',
   'O', 'K', 'C', 'O',
   'N', 'T', 'R', 'O',
   'L', '_', 'O', 'K'
  
};

uint8_t BUFER[16]; //Буффер данных шифрования
FunctionalState CRYPT = DISABLE; //Статус шифрования

StatusStruct_TypeDef STATUS; //Структурная переменная статуса устройства

FunctionalState delay_EnableStatus = DISABLE; //Статус активности программы задержки
uint32_t delay_Counter; //Интервал программы задержки

char Bluetooth_TxBuffer[TX_BufferSize+1]; //Передающий Bluetooth буфер USART1
char Bluetooth_RxBuffer[RX_BufferSize+1]; //Приемный Bluetooth буфер USART1
char GSM_TxBuffer[TX_BufferSize+1];       //Передающий GSM буфер USART2
char GSM_RxBuffer[RX_BufferSize+1];       //Приемный GSM буфер USART2
char Navi_TxBuffer[TX_BufferSize+1];      //Передающий Navigation буфер USART3
char Navi_RxBuffer[RX_BufferSize+1];      //Приемный Navigation буфер USART3

uint16_t sec_cnt=0;
 
//******************************************************************************

void main()  //Основная фукция программы
{
          /*ИНИЦИАЛИЗАЦИЯ ПЕРЕФЕРИЙНЫХ УСТРОЙСТВ КОНТРОЛЛЕРА*/
//SystemInit(); //Инициализация префирии с дефолтными настройками
FLASH_Unlock(); //Разблокировка контроллера флеш памяти
EE_Init(); //Инициализация виртуального EEPROM
RCC_Configuration(); //Включение тактироания и настройка перриферийных устройств
BKP_Configuration(); //Конфигурация резервного домена питпания
RTC_Configuration(); //Конфигурацмя часов реального времени
GPIO_Configuration(); //Инициализация портов ввода вывода
EXTI_Configuration(); //Инициализация контроллера внешних прерываний
NVIC_Configuration();//Инициализация прерываний
ADC_Configuration();//Инициализация АЦП
TIMER_Configuration();//Инициализация таймеров
DMA_Configuration(); //Инициализация каналов DMA
UART_Configuration();//Инициализация USART
SPI_Configuration();//Инициализация SPI
SysTick_Config(2*SystemCoreClock/1000); //Преиод счета системного таймера 1 мС
NVIC_SetPriority(SysTick_IRQn,0); //Приоритетпрерывания системного таймера наивысший



Write_Default_Setting();


while(STATUS.MainPower == DISABLE);

IWDG_Configuration();//Инициализация сторожевого таймера

         /*ИНИЦИАЛИЗАЦИЯ ВНЕШНИХ МОДУЛЕЙ*/

Transceiver_Configuration(); //Инициализация трансивера
NAVI_Configuration(); //Инициализация навигационного приемника
Bluetooth_Configuration(); //Инициализация Bluetooth
GSM_Configuration(); //Инициализация GSM




delay_ms(500);
SIREN_and_LIGHTS(3); //Тройное мигание фарами и звуковой сигнал

 


SendString_InUnit("AT+CMGD=0,4\r\n" , GSM); 
delay_ms(500);
SendData_onServer(0,0);



/******************************************************************************/

 while(1) //Основной цикл программы
 {
  
   
  IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
  delay_ms(1000); 


  
  Bluetooth_Read();
  SendString_InUnit("\r\nSTATUS:NORMAL\r\n" , Bluetooth);
  //ANSWER_CALL(); //Функция ответа на входящий звонок
  RECEIVE_SMS(); //Функция получения СМС сообщения
  COMAND_EXEC();  //Исполнитель команд
  
  if(STATUS.EVENT_BUF[0] !=0)
  {
   SendData_onServer(STATUS.EVENT_BUF[0], 0); 
   STATUS.EVENT_BUF[0] = STATUS.EVENT_BUF[1];
   STATUS.EVENT_BUF[1] = STATUS.EVENT_BUF[2]; 
   STATUS.EVENT_BUF[2] = 0; 
  }
  
   
   if(sec_cnt>300)
   {
    sec_cnt=0;
  Transceiver_Configuration(); //Инициализация трансивера
    if(STATUS.MainPower==ENABLE) SendData_onServer(0,0);
    else SendData_onServer((1200+STATUS.BatteryCharge), 0);
         /*Проверка резервных буферов*/
     if(Timestamp[1][0]  != 0) SendData_onServer(0,1);
     if(Timestamp[2][0]  != 0) SendData_onServer(0,2);
     if(Timestamp[3][0]  != 0) SendData_onServer(0,3);
     if(Timestamp[4][0]  != 0) SendData_onServer(0,4);
     if(Timestamp[5][0]  != 0) SendData_onServer(0,5);
   }
 }
 
}
//==============================================================================
             /*КОНЕЦ ОСНОВНОЙ ФУНКЦИИ ПРОГРАММЫ*/
//==============================================================================
void RCC_Configuration(void)
{
 //Включение тактирования портов A,B,C,D 
RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD), ENABLE); 
RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO , ENABLE); //Включение тактирования альтернативных функций GPIO
RCC_ADCCLKConfig(RCC_PCLK2_Div4); //Установка предделителя тактового сигнала АЦП
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_ADC2, ENABLE); //Включение тактирования АЦП1-2
RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM6 | RCC_APB1Periph_TIM7 , ENABLE ); //Включение тактирования таймеров 3,6,7
RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 , ENABLE); //Включение тактирования USART1
RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB1Periph_USART3 , ENABLE ); //Включение тактирования USART2-3
RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 , ENABLE); //Разрешить тактирование SPI1
RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); //Разрешить тактирование DMA1
RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//Включение тактирования домена питания и резервного домена
RCC_LSEConfig(RCC_LSE_OFF );
RCC_LSICmd(ENABLE); //Включение внутреннего RC генератора
while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET); //Ожидание готовности внутреннего RC генератора
 

}  

void BKP_Configuration(void) //Конфигурация резервного домена питания
{
PWR_BackupAccessCmd(ENABLE); //Разрешение записи в регистры с резервированием питания
BKP_DeInit();

BKP_TamperPinCmd(DISABLE); //Пин тампера отключен

  
  
}

void RTC_Configuration(void) //Конфигурацмя часов реального времени
{
  
 RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI); //Источник тактового сигнала внутренний RC генератор
 RCC_RTCCLKCmd(ENABLE); //Включение тактирования часов

 RTC_WaitForSynchro(); //Ожидание синхронизации
 RTC_WaitForLastTask(); //Ожидание окончания записи в регистры 
 RTC_SetPrescaler(40000);//Установка предделителя
 RTC_WaitForLastTask(); //Ожидание окончания записи в регистры
 
 RTC_ITConfig(RTC_IT_SEC, ENABLE); //Прерывание каждую секунду
 RTC_WaitForLastTask(); //Ожидание окончания записи в регистры
 
 RTC_ITConfig(RTC_IT_ALR, ENABLE); //Прерывание аларм
 RTC_WaitForLastTask(); //Ожидание окончания записи в регистры
}

void NVIC_Configuration(void)
{
NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//Кофигурация групп приоритетов:2 подгруппы
NVIC_InitTypeDef NVIC_InitStruct; //Объявляем структуру настройки прерываний

NVIC_InitStruct.NVIC_IRQChannel = ADC1_2_IRQn; //Прерывание от АЦП 1-2
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3; //Приоритетная подгруппа
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1; // Приоритет
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //Активируем прерывание
NVIC_Init(&NVIC_InitStruct); //Передаем структуру в функцию 

NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel3_IRQn; //Прерывание DMA1 канал 3 USART3 
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2; //Приоритетная подгруппа
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1; //Приоритет
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //Активируем прерывание
NVIC_Init(&NVIC_InitStruct); //Передаем структуру в функцию

NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn; //Прерывания внешние линии 3 входящий телефонный звонок
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //Приоритетная подгруппа
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1; //Приоритет
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //Активируем прерывание
NVIC_Init(&NVIC_InitStruct); //Передаем структуру в функцию


NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn; //Прерывания внешние линии 5-9
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3; //Приоритетная подгруппа
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2; //Приоритет
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //Активируем прерывание
NVIC_Init(&NVIC_InitStruct); //Передаем структуру в функцию

NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn; //Прерывания внешние линии 10-15
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //Приоритетная подгруппа
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2; //Приоритет
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //Активируем прерывание
NVIC_Init(&NVIC_InitStruct); //Передаем структуру в функцию

NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn; //Прерывания внешние линии 0
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2; //Приоритетная подгруппа
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3; //Приоритет
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //Активируем прерывание
NVIC_Init(&NVIC_InitStruct); //Передаем структуру в функцию

NVIC_InitStruct.NVIC_IRQChannel = TIM6_IRQn; //Прерывания таймера 6
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3; //Приоритетная подгруппа
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3; //Приоритет
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //Активируем прерывание
NVIC_Init(&NVIC_InitStruct); //Передаем структуру в функцию

NVIC_InitStruct.NVIC_IRQChannel = TIM7_IRQn; //Прерывания таймера 7
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3; //Приоритетная подгруппа
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2; //Приоритет
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //Активируем прерывание
NVIC_Init(&NVIC_InitStruct); //Передаем структуру в функцию

NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_InitStruct);




}  


void GPIO_Configuration(void)
{
GPIO_InitTypeDef GPIO_Init_struct; // Обявляем структуру инициализации портов

 //Порт А0-аналоговый вход канал Х акселерометра
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_0;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AIN; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А1-аналоговый вход канал Y акселерометра
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_1;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AIN; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А2-двухтактный альтернативный выход GSM UART2 TX
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_2;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А3- вход 3-состояние GSM UART2 RX
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_3;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IN_FLOATING; //Устанавливаем режим работы
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
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_50MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IN_FLOATING; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А7-двухтактный альтернативный выход SPI_MOSI
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_7;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_50MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А8-двухтактный  выход BLUETOOTH_RESET
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_8;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А9-двухтактный альтернативный выход BLUETOOTH UART1 TX
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_9;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_10MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А10- вход 3-состояние BLUETOOTH UART1 RX
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_10;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IN_FLOATING; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А11- вход 3-состояние TR_GP1 сервисный вход для трансивера
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_11;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А12- вход 3-состояние TR_GP2 сервисный вход для трансивера
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_12;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Блокировка кофигурации портов А0-3, A8-12
GPIO_PinLockConfig(GPIOA, (GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12));

//Порт B0-аналоговый вход контроль входного напряжения питания
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_0;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AIN; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B1-аналоговый вход контроль заряда резервной батареи
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_1;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AIN; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B2- вход с подтяжкой к земле контроль зажигания 1
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_2;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B5-двухтактный  выход включение зажигания 1
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_5;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B6- вход с подтяжкой к земле от стартера
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_6;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B7-двухтактный  выход включение стартера
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_7;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B8-двухтактный  выход включение зажигания 2
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_8;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B9-двухтактный  выход включение цепи ACC
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_9;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B10-двухтактный альтернативный выход NAVIGATION UART3 TX
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_10;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B11- вход 3-состояние NAVIGATION UART3 RX
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_11;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IN_FLOATING; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B12-двухтактный  выход standby NAVIGATION
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_12;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B13-двухтактный  выход reset NAVIGATION
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_13;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B14- вход с подтяжкой к земле от таходатчика
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_14;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPU; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B15- вход с подтяжкой к земле от контроль ручного тормоза или нейтрали
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_15;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Блокировка кофигурации портов B0-2,5-15
GPIO_PinLockConfig(GPIOB, (GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15));

//Порт C0-двухтактный  выход GSM_ON/OFF
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_0;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию
GPIO_SetBits(GPIOC, GPIO_Pin_0);          //Устанавливаем порт в 1
//Порт C1-двухтактный  выход GSM_RESET
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_1;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию
GPIO_SetBits(GPIOC, GPIO_Pin_1);          //Устанавливаем порт в 1
//Порт C2- вход с подтяжкой к земле GSM_READY
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_2;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт C3- вход с подтяжкой к земле GSM_RING
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_3;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт C4-двухтактный  выход открытия дверей
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_4;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт C5-двухтактный  выход закрытия дверей
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_5;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт C6-двухтактный  выход габаритные огни
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_6;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт C7-двухтактный  выход сирена
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_7;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт C8- вход с подтяжкой к земле датчик удара предупреждение
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_8;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт C9- вход с подтяжкой к земле датчик удара тревога
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_9;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт C10- вход с подтяжкой к земле  тригер багажника
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_10;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт C11- вход с подтяжкой к земле тригер двери
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_11;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт C12- вход с подтяжкой к плюсу тригер капота
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_12;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPU; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт C13-двухтактный  выход включение SIM1
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_13;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт C14-двухтактный  выход включение SIM2
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_14;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт C15-двухтактный  выход включение SIM3
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_15;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOC, &GPIO_Init_struct);      //Передаем структуру в функцию

//Блокировка кофигурации портов C0-15
GPIO_PinLockConfig(GPIOC, GPIO_Pin_All);

//Порт D2-двухтактный  выход блокировка двигателя
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_2;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOD, &GPIO_Init_struct);      //Передаем структуру в функцию

//Блокировка конфигурации порта D2
GPIO_PinLockConfig(GPIOC, GPIO_Pin_2); 

/* Подключение линий внешних прерываний*/

GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource3); //Линия прерывания GSM_RING
GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6); //Линия прерывания ST_IN
GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource8); //Линия прерывания SHOCK1
GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource9); //Линия прерывания SHOCK2
GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource10); //Линия прерывания TRUNK_TR
GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource11); //Линия прерывания TR_GP1 прием пакета
GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource12); //Линия прерывания HOOD_TR
GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14); //Линия прерывания MOTOR_CTRL
GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15); //Линия прерывания BRAKE_CTRL
}  

void EXTI_Configuration(void) //Инициализация контроллера внешних прерываний
{
  EXTI_InitTypeDef   EXTI_InitStructure; //Объявляем структуру для инициализации внешних прерываний
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line0; //Линия прерывания тригера двери
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //Режим прерывания
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //Прерывание по положительному фронту
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //Прерывание активно
  EXTI_Init(&EXTI_InitStructure); //Прередаем структуру в функцию
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line3; //Линия прерывания входящего звонка
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //Режим прерывания
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //Прерывание по отрицательному фронту
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //Прерывание активно
  EXTI_Init(&EXTI_InitStructure); //Прередаем структуру в функцию

  EXTI_InitStructure.EXTI_Line = EXTI_Line6; //Линия прерывания 6 ST_IN
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //Режим прерывания
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //Прерывание по положительному фронту
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //Прерывание активно
  EXTI_Init(&EXTI_InitStructure); //Прередаем структуру в функцию
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line8; //Линия прерывания 8 SHOCK1
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //Режим прерывания
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //Прерывание по положительному фронту
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //Прерывание активно
  EXTI_Init(&EXTI_InitStructure); //Прередаем структуру в функцию 

  EXTI_InitStructure.EXTI_Line = EXTI_Line9; //Линия прерывания 9 SHOCK2
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //Режим прерывания
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //Прерывание по положительному фронту
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //Прерывание активно
  EXTI_Init(&EXTI_InitStructure); //Прередаем структуру в функцию 
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line10; //Линия прерывания 10 TRUNK_TR
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //Режим прерывания
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //Прерывание по отрицательному фронту
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //Прерывание активно
  EXTI_Init(&EXTI_InitStructure); //Прередаем структуру в функцию 
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line11; //Линия прерывания 11 TR_GP1 прием пакета
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //Режим прерывания
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //Прерывание по положительному фронту
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //Прерывание активно
  EXTI_Init(&EXTI_InitStructure); //Прередаем структуру в функцию 
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line12; //Линия прерывания 12 HOOD_TR
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //Режим прерывания
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //Прерывание по отрицательному фронту
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //Прерывание активно
  EXTI_Init(&EXTI_InitStructure); //Прередаем структуру в функцию 
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line14; //Линия прерывания 14 MOTOR_CTRL
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //Режим прерывания
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //Прерывание по положительному фронту
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //Прерывание активно
  EXTI_Init(&EXTI_InitStructure); //Прередаем структуру в функцию
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line15; //Линия прерывания 15 BRAKE_CTRL
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //Режим прерывания
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //Прерывание по положительному фронту
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //Прерывание активно
  EXTI_Init(&EXTI_InitStructure); //Прередаем структуру в функцию 
 
}


void ADC_Configuration(void) //Инициализация АЦП
{                     /*АЦП1*/
ADC_InitTypeDef  ADC_InitStructure;//Объявляем структуру для настройки АЦП

ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //Независимый режим работы АЦП
ADC_InitStructure.ADC_ScanConvMode = ENABLE; // Сканирование каналов включено
ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; //Непрерывный режим работы включен
ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //Внешний запуск выключен
ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //Правое выравнивание данных
ADC_InitStructure.ADC_NbrOfChannel = 2; //Колличество каналов 2
ADC_Init(ADC1, &ADC_InitStructure); //Передаем структуру в функцию

ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5); //Настройка канала0
ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_55Cycles5); //Настройка канала1
 
ADC_AnalogWatchdogThresholdsConfig(ADC1, 0x090F, 0x07C7); //Установка порогов сработки Акселерометра +-5g
ADC_AnalogWatchdogCmd(ADC1,  ADC_AnalogWatchdog_AllRegEnable);//Проверка всех каналов регулярной группы
ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE); //Разрешить прерывание оконного компаратора

ADC_Cmd(ADC1, ENABLE); //Включение АЦП1
ADC_ResetCalibration(ADC1);//Сброс калибровки АЦП1
while(ADC_GetResetCalibrationStatus(ADC1));//Проверка окончания сброса калибровки АЦП
ADC_StartCalibration(ADC1);//Запуск каоибровки АЦП1
while(ADC_GetCalibrationStatus(ADC1));//Ожидание окончания калибровки
ADC_SoftwareStartConvCmd(ADC1, ENABLE);//Програмный запуск преобразований АЦП1

                    /*АЦП2*/

ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //Независимый режим работы АЦП
ADC_InitStructure.ADC_ScanConvMode = DISABLE; // Сканирование каналов выключено
ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //Непрерывный режим работы выключен
ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO; //Внешний запуск от выхода таймера 3
ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //Правое выравнивание данных
ADC_InitStructure.ADC_NbrOfChannel = 1; //Колличество каналов 2
ADC_Init(ADC2, &ADC_InitStructure); //Передаем структуру в функцию

ADC_RegularChannelConfig(ADC2, ADC_Channel_8, 1, ADC_SampleTime_55Cycles5); //Настройка канала 8 регулярной группы

ADC_InjectedSequencerLengthConfig(ADC1, 1); //Колличество каналов инжектированой группы 1
ADC_InjectedChannelConfig(ADC2, ADC_Channel_9, 1, ADC_SampleTime_55Cycles5); //Настройка канала 9 ижектированной группы
ADC_ExternalTrigInjectedConvConfig(ADC2, ADC_ExternalTrigInjecConv_None); //Внешнее управление запуском инжектированной группы отключено
ADC_AutoInjectedConvCmd(ADC2, ENABLE); //Включение автоматического запуска инжектированной группы

ADC_ExternalTrigConvCmd(ADC2, ENABLE); //Разрешение внешнего управления запуском АЦП2
ADC_ITConfig(ADC2, ADC_IT_JEOC, ENABLE); //Разрешение прерывания по окончанию ижектированной группы 

ADC_Cmd(ADC2, ENABLE); //Включение АЦП2
ADC_ResetCalibration(ADC2);//Сброс калибровки АЦП2
while(ADC_GetResetCalibrationStatus(ADC2));//Проверка окончания сброса калибровки АЦП
ADC_StartCalibration(ADC2);//Запуск каоибровки АЦП2
while(ADC_GetCalibrationStatus(ADC2));//Ожидание окончания калибровки
}

void TIMER_Configuration(void)   //Инициализация таймеров
{
TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure; //Объявление базовой структуры таймера
TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); //Очистка полей структуры

TIM_TimeBaseStructure.TIM_Period = 0xFFFF;   //Период счета       
TIM_TimeBaseStructure.TIM_Prescaler = 0xFF;   //Предделитель    
TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //Делитель для фильтров   
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //Режим работы таймера
TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //Передаем структуру в функцию

TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);//Выходной тригер работает по переполнению

TIM_Cmd(TIM3, ENABLE); //Запуск таймера 

TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); //Очистка полей структуры

TIM_TimeBaseStructure.TIM_Prescaler = 5000;   //Предделитель  
TIM_TimeBaseStructure.TIM_Period = 11200 ;  //Период счета  
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //Режим работы таймера
TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure); //Передаем структуру в функцию

TIM_ITConfig(TIM6 , TIM_IT_Update , ENABLE); //Запрос на прерывание по переполнению

TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); //Очистка полей структуры

TIM_TimeBaseStructure.TIM_Prescaler = 36000;   //Предделитель  
TIM_TimeBaseStructure.TIM_Period = 36000 ;  //Период счета  
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //Режим работы таймера
TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //Передаем структуру в функцию

TIM_ITConfig(TIM7 , TIM_IT_Update , ENABLE); //Запрос на прерывание по переполнению

}  


void UART_Configuration(void)    //Инициализация USART
{
 USART_InitTypeDef USART_InitStructure; //Объявление структуры инциализации
 USART_StructInit(&USART_InitStructure); //Очистка полей структуры
                   /*USART1*/
  USART_InitStructure.USART_BaudRate = 921600;  //Скорость 921600 б/с
  USART_InitStructure.USART_WordLength = USART_WordLength_8b; //Биты данных 8
  USART_InitStructure.USART_StopBits = USART_StopBits_1;  //Стоповые 1
  USART_InitStructure.USART_Parity = USART_Parity_No;     //Четность нет
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //Управляющие сигналы нет
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //Передатчик и приемник включены
  
  USART_Init(USART1, &USART_InitStructure); //Передаем структуру в функцию
  USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE); //Разрешить запрос DMA на прием и передачу
  USART_Cmd(USART1, ENABLE); //Активировать USART1
  
                 /*USART2*/
  USART_InitStructure.USART_BaudRate = 115200;  //Скорость 115200 б/с
  
  USART_Init(USART2, &USART_InitStructure); //Передаем структуру в функцию
  USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE); //Разрешить запрос DMA на прием и передачу
  USART_Cmd(USART2, ENABLE); //Активировать USART2
  
                 /*USART3*/
  USART_Init(USART3, &USART_InitStructure); //Передаем структуру в функцию
  USART_DMACmd(USART3, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE); //Разрешить запрос DMA на прием и передачу
  USART_Cmd(USART3, ENABLE); //Активировать USART3 
  
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
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; //Предделитель 8, тактовая частота SPI 7 МГц
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //Старший бит идет первым
  SPI_InitStructure.SPI_CRCPolynomial = 8; //Полином для рассчета CRC
  
  SPI_Init(SPI1, &SPI_InitStructure); //Передаем структуру в функцию

  
  
}


void DMA_Configuration(void) //Инициализация каналов DMA
{
  DMA_InitTypeDef DMA_InitStructure; //Инициализация структуры DMA
  DMA_StructInit(&DMA_InitStructure); //Очистка полей структуры
  
  /*USART1_TX-Канал 4*/
  DMA_DeInit(DMA1_Channel4);  //Сброс настроек канала 4
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804; //Базовый адрес регистра данных USART1
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Bluetooth_TxBuffer; //Передающий буфер USART1
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //Направление данных из памяти к перефирии
  DMA_InitStructure.DMA_BufferSize = TX_BufferSize; //Размер буфера
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //Инкремент перефирийного регистра запрещен
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //Инкремент памяти разрешен 
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //Размер данных в перефирии 1 байт
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //Размер данных в памяти 1 байт
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //Нормальный режим работы DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low; //Приоритет  низкий
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //Передача из памяти в память выключена
  
  DMA_Init(DMA1_Channel4 , &DMA_InitStructure); //Передаем структуру в функцию
 // DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE); //Разрешаем прерывание по окончании передачи
  
  /*USART1_RX-Канал5*/
  DMA_DeInit(DMA1_Channel5);  //Сброс настроек канала 5
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804; //Базовый адрес регистра данных USART1
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Bluetooth_RxBuffer; //Приемный буфер USART1
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //Направление данных от периферии к памяти
  DMA_InitStructure.DMA_BufferSize = RX_BufferSize; //Размер буфера
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //Циклический режим работы DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; //Приоритет  высокий
  
  DMA_Init(DMA1_Channel5 , &DMA_InitStructure); //Передаем структуру в функцию
  
  /*USART2_TX-Канал 7*/
  DMA_DeInit(DMA1_Channel7);  //Сброс настроек канала 7
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404; //Базовый адрес регистра данных USART2
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)GSM_TxBuffer; //Передающий буфер USART2
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //Направление данных из памяти к перефирии
  DMA_InitStructure.DMA_BufferSize = TX_BufferSize; //Размер буфера
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //Нормальный режим работы DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low; //Приоритет  низкий
  
  DMA_Init(DMA1_Channel7 , &DMA_InitStructure); //Передаем структуру в функцию
 // DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE); //Разрешаем прерывание по окончании передачи
  
  /*USART2_RX-Канал 6*/ 
  DMA_DeInit(DMA1_Channel6);  //Сброс настроек канала 6
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404; //Базовый адрес регистра данных USART2
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)GSM_RxBuffer; //Приемный буфер USART2
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //Направление данных от периферии к памяти
  DMA_InitStructure.DMA_BufferSize = RX_BufferSize; //Размер буфера
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //Циклический режим работы DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; //Приоритет  высокий
  
  DMA_Init(DMA1_Channel6 , &DMA_InitStructure); //Передаем структуру в функцию
  
  /*USART3_TX-Канал 2*/
  
  DMA_DeInit(DMA1_Channel2);  //Сброс настроек канала 2
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004804; //Базовый адрес регистра данных USART3
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Navi_TxBuffer; //Передающий буфер USART3
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //Направление данных из памяти к перефирии
  DMA_InitStructure.DMA_BufferSize = TX_BufferSize; //Размер буфера
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //Нормальный режим работы DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low; //Приоритет  низкий
  
  DMA_Init(DMA1_Channel2 , &DMA_InitStructure); //Передаем структуру в функцию
 // DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE); //Разрешаем прерывание по окончании передачи
  
   /*USART3_RX-Канал 3*/ 
  DMA_DeInit(DMA1_Channel3);  //Сброс настроек канала 3
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004804; //Базовый адрес регистра данных USART3
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Navi_RxBuffer; //Приемный буфер USART3
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //Направление данных от периферии к памяти
  DMA_InitStructure.DMA_BufferSize = RX_BufferSize; //Размер буфера
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //Циклический режим работы
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //Приоритет  средний
  
  DMA_Init(DMA1_Channel3 , &DMA_InitStructure); //Передаем структуру в функцию
  DMA_ITConfig(DMA1_Channel3,  DMA_IT_TC, ENABLE); //Разрешаем прерывание по половине передачи
  
  /*Активация каналов приема*/
  DMA_Cmd(DMA1_Channel5, ENABLE);//USART1_RX
  DMA_Cmd(DMA1_Channel6, ENABLE);//USART2_RX
  DMA_Cmd(DMA1_Channel3, ENABLE);//USART3_RX
  
  
} 

void delay_ms(uint16_t msec) //Задержка в милисекундах
{
  delay_Counter=msec; //Присваиваем счетчику параметр
  delay_EnableStatus = ENABLE; //Активируем задержку
  while(delay_Counter); //Цикл пока счетчик не равен 0
  delay_EnableStatus = DISABLE; //Деактивируем задержку
}  



void SendString_InUnit(const char *str , uint8_t Unit) //Функция отправки строки навнешний модуль через UART

{
 uint16_t len; //Длинна строки
 uint16_t i;   //Счетчик
 len=strlen(str); //Вычисляем длинну строки
 
 switch(Unit)  //Выбираем на какой модуль отправить
 {
 case  Bluetooth:{
  for(i=0;i<len;i++)  Bluetooth_TxBuffer[i]=*str++; //Записывем данные в буфер передачи
  DMA_Cmd(DMA1_Channel4 , DISABLE);                 //Отключаем канал DMA
  DMA_SetCurrDataCounter(DMA1_Channel4 , len);      //Устанавливаем счетчик DMA канала
  DMA_ClearFlag(DMA1_FLAG_TC4);                     //Сбрасываем флаг окончания передачи
  DMA_Cmd(DMA1_Channel4 , ENABLE );                 //Активируем DMA канал
  while(DMA_GetFlagStatus(DMA1_FLAG_TC4)==RESET);   //Ждем окончания передачи
  DMA_Cmd(DMA1_Channel4 , DISABLE);                 //Отключаем канал DMA
  ClearBufer(Bluetooth_TxBuffer);                   //Очищаем буфер 
  break; }
                 
 case  GSM:{
  for(i=0;i<len;i++)  GSM_TxBuffer[i]=*str++;       //Записывем данные в буфер передачи
  DMA_Cmd(DMA1_Channel7 , DISABLE);                 //Отключаем канал DMA
  DMA_SetCurrDataCounter(DMA1_Channel7 , len);      //Устанавливаем счетчик DMA канала
  DMA_ClearFlag(DMA1_FLAG_TC7);                     //Сбрасываем флаг окончания передачи
  DMA_Cmd(DMA1_Channel7 , ENABLE );                 //Активируем DMA канал
  while(DMA_GetFlagStatus(DMA1_FLAG_TC7)==RESET);   //Ждем окончания передачи
  DMA_Cmd(DMA1_Channel7 , DISABLE);                 //Отключаем канал DMA
  ClearBufer(GSM_TxBuffer);                         //Очищаем буфер 
  break; }
                 
 case  Navigator:{
  for(i=0;i<len;i++)  Navi_TxBuffer[i]=*str++;      //Записывем данные в буфер передачи
  DMA_Cmd(DMA1_Channel2 , DISABLE);                 //Отключаем канал DMA
  DMA_SetCurrDataCounter(DMA1_Channel2 , len);      //Устанавливаем счетчик DMA канала
  DMA_ClearFlag(DMA1_FLAG_TC2);                     //Сбрасываем флаг окончания передачи
  DMA_Cmd(DMA1_Channel2 , ENABLE );                 //Активируем DMA канал
  while(DMA_GetFlagStatus(DMA1_FLAG_TC2)==RESET);   //Ждем окончания передачи
  DMA_Cmd(DMA1_Channel2 , DISABLE);                 //Отключаем канал DMA
  ClearBufer(Navi_TxBuffer);                        //Очищаем буфер  
  break; }                
   
   
 } 
  
  
  
}  

void ClearBufer(char *buf) //Функция очистки буфера
{
 uint16_t size; //Размер буфера 
 uint16_t j; //Счетчик
 /*Определяем приемный или передающий буфер*/
 if((buf==Bluetooth_RxBuffer) || (buf==GSM_RxBuffer) || (buf==Navi_RxBuffer)) size=RX_BufferSize;
 else size=TX_BufferSize; 
 
 for(j=0;j<size;j++) 
 {
   *buf=0x00; //Присваиваем текущему элементу значение по умолчание
   buf++; //Инкрементируем ссылочную переменную
 }
}





void Reset_rxDMA_ClearBufer(uint8_t Unit) //Сброс приемного DMA канала и очистка буфера приема
{   
 switch(Unit)  //Выбираем на какой модуль отправить  
 { 
 case  Bluetooth:   
   {
     DMA_Cmd(DMA1_Channel5 , DISABLE ); //Останавливаем DMA канал
     ClearBufer(Bluetooth_RxBuffer);                  //Очищаем буфер
     DMA_SetCurrDataCounter(DMA1_Channel5 ,RX_BufferSize); //Ставим счетчик в размер буфера
     DMA_Cmd(DMA1_Channel5 , ENABLE ); //Активируем DMA канал
   break;}
 case  GSM: 
    
     {
     DMA_Cmd(DMA1_Channel6 , DISABLE ); //Останавливаем DMA канал
     ClearBufer(GSM_RxBuffer);                  //Очищаем буфер
     DMA_SetCurrDataCounter(DMA1_Channel6 , RX_BufferSize); //Ставим счетчик в размер буфера
     DMA_Cmd(DMA1_Channel6 , ENABLE ); //Активируем DMA канал
     break;}
 case  Navigator:   
    {
     DMA_Cmd(DMA1_Channel3 , DISABLE ); //Останавливаем DMA канал
     ClearBufer(Navi_RxBuffer);                  //Очищаем буфер
     DMA_SetCurrDataCounter(DMA1_Channel3 , RX_BufferSize); //Ставим счетчик в размер буфера
     DMA_ClearFlag(DMA1_FLAG_TC3);                     //Сбрасываем флаг окончания передачи
     DMA_Cmd(DMA1_Channel3 , ENABLE ); //Активируем DMA канал
   break; }
    
  }
    
}

void COMAND_EXEC(void)  //Исполнитель команд
{
  Autoheatmode_TypeDef LastMode;
  
  LastMode = STATUS.AUTOHEAT_MODE;
    
  switch(STATUS.COMMAND) //Чтение статуса команды
  {
  case 10:  //Открыть машину снять с охраны
    {
     SECURITY(DISABLE);
     SEND_SMS(SECURITY_OFF);
    break;}
    
  case 11:  //Закрыть машину/поставить на охрану
    {
     SECURITY(ENABLE);
     SEND_SMS(SECURITY_ON);
    break;} 
    
  case 13:  //Запустить двигатель 
    {
    AUTOSTART(ENABLE); 
    delay_ms(2000);
    if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) SEND_SMS(MOTOR_ON); 
    else SEND_SMS(MOTOR_START_ERROR);
    break;}
    
  case 14:  //Заглушить двигатель
    {
    AUTOSTART(DISABLE);
    SEND_SMS(MOTOR_OFF);
    break;} 
    
  case 15:  //Активировать режим автопрогрева 05/30
    {
     STATUS.AUTOHEAT_MODE = MODE_AUTOHEAT_0530;
     AUTOHEAT();
    if(STATUS.AUTOHEAT_MODE != MODE_AUTOHEAT_OFF) 
    { 
     if(LastMode == MODE_AUTOHEAT_OFF) SEND_SMS(AUTOHEAT_ON_0530);
     else SEND_SMS(CHANGE_HEAT_MODE_0530);
    }
    break;}
    
  case 16:  //Активировать режим автопрогрева 10/30
    {
      STATUS.AUTOHEAT_MODE = MODE_AUTOHEAT_1030;
       AUTOHEAT();
    if(STATUS.AUTOHEAT_MODE != MODE_AUTOHEAT_OFF) 
    { 
     if(LastMode == MODE_AUTOHEAT_OFF) SEND_SMS(AUTOHEAT_ON_1030);
     else SEND_SMS(CHANGE_HEAT_MODE_1030);
    }
    break;} 
    
  case 17:  //Активировать режим автопрогрева 30/30
    {
      STATUS.AUTOHEAT_MODE = MODE_AUTOHEAT_3030;
       AUTOHEAT();
    if(STATUS.AUTOHEAT_MODE != MODE_AUTOHEAT_OFF) 
    { 
     if(LastMode == MODE_AUTOHEAT_OFF) SEND_SMS(AUTOHEAT_ON_3030);
     else SEND_SMS(CHANGE_HEAT_MODE_3030);
    }
    break;} 
    
  case 18:  //Активировать режим автопрогрева 30/20
    {
      STATUS.AUTOHEAT_MODE = MODE_AUTOHEAT_3020;
       AUTOHEAT();
    if(STATUS.AUTOHEAT_MODE != MODE_AUTOHEAT_OFF) 
    { 
     if(LastMode == MODE_AUTOHEAT_OFF) SEND_SMS(AUTOHEAT_ON_3020);
     else SEND_SMS(CHANGE_HEAT_MODE_3020);
    }
    break;}  
    
    
 
 
  case 19:  //Отключить режим автопрогрева
    {
     STATUS.AUTOHEAT_MODE = MODE_AUTOHEAT_OFF; 
      AUTOHEAT();
    SEND_SMS(AUTOHEAT_OFF);  
    break;}
    
  case 20: //Найти машину
    {
      
    break;}
    
   
  }
  STATUS.COMMAND=0;
}
//================================================================================
void IWDG_Configuration(void)//Инициализация сторожевого таймера
{
 IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //Разрешить запись в регистры настройки 
    /*Установка перида сторожевого таймера на 26,2 секунды*/
 IWDG_SetPrescaler(IWDG_Prescaler_256); //Установка предделителя
 IWDG_SetReload(0xFFF); //Установка перезагружаемого значения
 
 IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
 
 IWDG_Enable(); //Активация системного таймера
 
 DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE); //Остановка системного таймера в режиме отладки
 
  
}


/*******************************************************************************
********************************************************************************
**                                                                            **
**                        КОНЕЦ ПРОГРАММЫ                                     **
**                                                                            **
********************************************************************************
*******************************************************************************/