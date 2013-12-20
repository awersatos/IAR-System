/*******************************************************************************
********************************************************************************
**                                                                            **
**                  ОСНОВНОЙ ФАЙЛ ПРОЕКТА Brelok_MFU v 1.00                   **
**                                                                            **
********************************************************************************
*******************************************************************************/

//****************Подключаемые файлы********************************************
#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "main.h"
#include "Transceiver_433MHz.h"
#include "CRYPTO-AES.h"
#include <string.h>
//************* Инициализация глобальных переменных ****************************
FunctionalState delay_EnableStatus = DISABLE; //Статус активности программы задержки
uint32_t delay_Counter; //Интервал программы задержки
FunctionalState Transceiver_Status = DISABLE; //Статус активности трансивера
FunctionalState CRYPT = DISABLE; //Статус шифрования
uint16_t i,j;

const char SERIAL[] = "\015BR1234567890"; //Серийный номер
/*
const char COMMAND_1[] = "\015BRELOK_TEST";
const char COMMAND_2[] = "\014TEST_COMAND";
const char COMMAND_3[] = "\013TEST_ALARM";
*/
 uint8_t KEY[16]=    //Ключ шифрования
{  'B', 'R', 'E', 'L',
   'O', 'K', 'C', 'O',
   'N', 'T', 'R', 'O',
   'L', '_', 'O', 'K'
  
};

uint8_t BUFER[16]; //Массив шифрования


//******************************************************************************

void main(void) //Основная функция программы
{
 
  
       /*ИНИЦИАЛИЗАЦИЯ ПЕРЕФЕРИЙНЫХ УСТРОЙСТВ КОНТРОЛЛЕРА*/
RCC_Configuration(); //Включение тактироания и настройка перриферийных устройств
GPIO_Configuration(); //Инициализация портов ввода вывода
EXTI_Configuration(); //Инициализация контроллера внешних прерываний
NVIC_Configuration();//Инициализация прерываний
SPI_Configuration();//Инициализация SPI 
TIMER_Configuration(); //Инициализация таймера
SysTick_Config(SystemCoreClock/1000); //Преиод счета системного таймера 1 мС
NVIC_SetPriority(SysTick_IRQn,0); //Приоритетпрерывания системного таймера наивысший
PWR_WakeUpPinCmd(ENABLE); //Активизация входа ожидания

              /*ИНИЦИАЛИЗАЦИЯ ВНЕШНИХ МОДУЛЕЙ*/
Transceiver_Configuration(); //Инициализация трансивера
//*****************************************************************************

BUZZER(Long_Bip_1); 
//delay_ms(1000);

/*Обработчик кнопки 1 открыть машину /снять с охраны*/
if(GPIO_ReadInputDataBit(GPIOA , BUTTON_1) == 1 && GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 0)
{
 LED(GREEN); 
 delay_ms(10);
  
 if(GPIO_ReadInputDataBit(GPIOA , BUTTON_1) == 1)
  {
   for(j=0;j<5;j++)
   {  
   for(i=0;i<SERIAL[0];i++) SPI_buffer[i] = SERIAL[i];
   SEND_PAKET();
   delay_ms(10);
   STROB(SRX);
   CRYPT = ENABLE; //Шифрование активно
   delay_ms(300);
   
   if(CRYPT==DISABLE)
   {
   SPI_buffer[0]=18;
   for(i=1;i<17;i++) SPI_buffer[i] = BUFER[i-1];
   SPI_buffer[17] = OPEN; //Открыть снять с охраны
   SEND_PAKET();
   delay_ms(10);
   STROB(SRX);
   }
   delay_ms(300);
   if(strstr(SPI_buffer , "EXECUTED") !=NULL) 
   {BUZZER(Medium_Bip_2);
    break;
   }
   }
   if(strstr(SPI_buffer , "EXECUTED") ==NULL) BUZZER(Long_Bip_1);
   

   while(GPIO_ReadInputDataBit(GPIOA , BUTTON_1) == 1); 
  }
}

/*Обработчик кнопки 2 закрыть машину/ поставить на охрану*/
if(GPIO_ReadInputDataBit(GPIOA , BUTTON_1) == 0 && GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1)
{
 LED(RED); 
 delay_ms(10);
  
 if(GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1)
  {
    for(j=0;j<5;j++)
  { 
   for(i=0;i<SERIAL[0];i++) SPI_buffer[i] = SERIAL[i];
   SEND_PAKET();
   delay_ms(10);
   STROB(SRX);
   CRYPT = ENABLE; //Шифрование активно
   delay_ms(300);
   
   if(CRYPT==DISABLE)
   {
   SPI_buffer[0]=18;
   for(i=1;i<17;i++) SPI_buffer[i] = BUFER[i-1];
   SPI_buffer[17] = CLOSE; //Закрыть поставить на охрану
   SEND_PAKET();
   delay_ms(10);
   STROB(SRX);
   }
   delay_ms(300);
   if(strstr(SPI_buffer , "EXECUTED") !=NULL)
    {BUZZER(Medium_Bip_3);
    break;}
  }
   if(strstr(SPI_buffer , "EXECUTED") ==NULL) BUZZER(Long_Bip_1);
   

   while(GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1); 
  }
}

/*Обработчик нажатия 2 кнопок одновременно сигнал тревоги*/
if(GPIO_ReadInputDataBit(GPIOA , BUTTON_1) == 1 && GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1)
{
 LED(ORANGE); 
 delay_ms(10000);
  
 if(GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1 && GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1)
  {
    for(j=0;j<5;j++)
   {   
    for(i=0;i<SERIAL[0];i++) SPI_buffer[i] = SERIAL[i];
   SEND_PAKET();
   delay_ms(10);
   STROB(SRX);
   CRYPT = ENABLE; //Шифрование активно
   delay_ms(300);
   
   if(CRYPT==DISABLE)
   {
   SPI_buffer[0]=18;
   for(i=1;i<17;i++) SPI_buffer[i] = BUFER[i-1];
   SPI_buffer[17] = ALARM; //Авария
   SEND_PAKET();
   delay_ms(10);
   STROB(SRX);
   }
   delay_ms(300);
   if(strstr(SPI_buffer , "EXECUTED") !=NULL) 
    { BUZZER(Short_Bip_15);
      break;
    }
   }
   if(strstr(SPI_buffer , "EXECUTED") ==NULL) BUZZER(Long_Bip_1);
   
   while(GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1 && GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1); 
  }
}





 
 while(1) //Основной цикл программы
 {
   /*
 LED(RED);
 delay_ms(1000);  
 LED(GREEN);
 
 */
 STROB(SIDLE);
 delay_ms(1000);
 LED(OFF);
 
 DBGMCU_Config(DBGMCU_STANDBY , ENABLE); //Разрешить отладку в режиме ожидания
 
 STROB(SPWD); //Перевод транчивера в спящий режим 
 delay_ms(100);
 PWR_EnterSTANDBYMode(); //Перевод контроллера в режим ожидания
 
 }
 
 
}

      /*КОНЕЦ ОСНОВНОЙ ФУНКЦИИ ПРОГРАММЫ*/



void delay_ms(uint16_t msec) //Задержка в милисекундах
{
  delay_Counter=msec; //Присваиваем счетчику параметр
  delay_EnableStatus = ENABLE; //Активируем задержку
  while(delay_Counter); //Цикл пока счетчик не равен 0
  delay_EnableStatus = DISABLE; //Деактивируем задержку
}  


void RCC_Configuration(void) //Тактирование и настройка периферийных устройств
{
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE); //Включение тактирования портов A,B
RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO , ENABLE); //Включение тактирования альтернативных функций GPIO
RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 , ENABLE); //Разрешить тактирование SPI1
RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17 , ENABLE);  //Разрешить тактирование Таймера 17 
RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  //Включение тактирования домена питания 
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
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
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
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт А8-A9  входы с подтяжкой к - (кнопки S1,S2)
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_8 | GPIO_Pin_9;     //Указываем пин
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //Устанавливаем режим работы
GPIO_Init(GPIOA, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B0-B1 -двухтактный  выходы (светодиод)
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin= GPIO_Pin_0 | GPIO_Pin_1;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию

//Порт B8-B9 -двухтактный  выходы (пьезодинамик)
GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
GPIO_Init_struct.GPIO_Pin= GPIO_Pin_8 | GPIO_Pin_9;     //Указываем пин
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //Устанавливаем частоту
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы
GPIO_Init(GPIOB, &GPIO_Init_struct);      //Передаем структуру в функцию
  
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

NVIC_InitStruct.NVIC_IRQChannel = TIM1_TRG_COM_TIM17_IRQn; //Прерывания внешние линии 2 от Таймера 17
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //Приоритетная подгруппа
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1; //Приоритет
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
  SPI_InitStructure.SPI_BaudRatePrescaler =  SPI_BaudRatePrescaler_8 ; //Предделитель 8 тактовая частота 1 МГц
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //Старший бит идет первым
  SPI_InitStructure.SPI_CRCPolynomial = 8; //Полином для рассчета CRC
  
  SPI_Init(SPI1, &SPI_InitStructure); //Передаем структуру в функцию

  
  
}

void TIMER_Configuration(void)   //Инициализация таймера
{
uint16_t Period = 690;
TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure; //Объявление базовой структуры таймера
TIM_OCInitTypeDef  TIM_OCInitStructure; //Объявление структуры настройки сравнения  

TIM_OCStructInit(&TIM_OCInitStructure); //Очистка полей структуры
TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); //Очистка полей структуры

TIM_TimeBaseStructure.TIM_Period = Period;   //Период счета       
TIM_TimeBaseStructure.TIM_Prescaler = 0;   //Предделитель    
TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //Делитель для фильтров   
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //Режим работы таймера

TIM_TimeBaseInit(TIM17, &TIM_TimeBaseStructure); //Передаем структуру в функцию

TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;  //Режим переключения
TIM_OCInitStructure.TIM_Pulse = (Period/2); //значение регистра равнения
TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable; //Основной канал отключен
TIM_OCInitStructure.TIM_OutputNState = TIM_OutputState_Disable; //Комплиментарный канал отключен

TIM_OC1Init(TIM17, &TIM_OCInitStructure); //Передаем структуру в функцию

TIM_OC1PreloadConfig(TIM17, TIM_OCPreload_Disable); //Перезагрузка региста сравнения отключена

TIM_ITConfig(TIM17, TIM_IT_CC1 | TIM_IT_Update , ENABLE);//Прерывания по переполнению счетчика и по совпадению активны

 
}

void LED(Color_TypeDef COLOR) //Функция управления светодиодом
{
  switch(COLOR)
  {
  case OFF: //Светодиод отключен
    {
      GPIO_ResetBits(GPIOB , red);
      GPIO_ResetBits(GPIOB , green);
      break;
    }
    
  case GREEN: //Зеленый
    {
      GPIO_ResetBits(GPIOB , red);
      GPIO_SetBits(GPIOB , green);
      break;
    } 
    
  case RED:  //Красный
    {
      GPIO_ResetBits(GPIOB , green);
      GPIO_SetBits(GPIOB , red);
      break;
    }
    
  case ORANGE: //Оранжевый
    {
      GPIO_SetBits(GPIOB , green);
      GPIO_SetBits(GPIOB , red);
      break;
    }  
  }
  
}

void BUZZER(Bip_TypeDef BIP) //Функция управления пьезодинамиком
{ 
  uint8_t buz_cnt; //Счетчик
  
  switch(BIP)
  {
  case Long_Bip_1: //Одиночный длинный сигнал
    {
     TIM_Cmd(TIM17, ENABLE); //Запуск таймера
     delay_ms(1000); 
     TIM_Cmd(TIM17, DISABLE); //Остановка таймера
     break;  
    }
    
  case Medium_Bip_2: //2 средних сигнала
    {
     for(buz_cnt=0;buz_cnt<2;buz_cnt++)
     {
     TIM_Cmd(TIM17, ENABLE); //Запуск таймера
     delay_ms(300); 
     TIM_Cmd(TIM17, DISABLE); //Остановка таймера
     delay_ms(200);
     }
     break;  
    }
    
  case Medium_Bip_3:  //3 средних сигнала
    {
      for(buz_cnt=0;buz_cnt<3;buz_cnt++)
     {
     TIM_Cmd(TIM17, ENABLE); //Запуск таймера
     delay_ms(300); 
     TIM_Cmd(TIM17, DISABLE); //Остановка таймера
     delay_ms(200);
     }
     break; 
    }
    
  case Short_Bip_15: //15 кортоких сигналов
    {
      
      for(buz_cnt=0;buz_cnt<15;buz_cnt++)
     {
     TIM_Cmd(TIM17, ENABLE); //Запуск таймера
     delay_ms(200); 
     TIM_Cmd(TIM17, DISABLE); //Остановка таймера
     delay_ms(150);
     }
     break; 
    }
    
  }
    
  
  GPIO_ResetBits(GPIOB , BUZZER_PIN_1); //Сброс в 0 обоих выводов динамика
  GPIO_ResetBits(GPIOB , BUZZER_PIN_1);
}



/*******************************************************************************
********************************************************************************
**                                                                            **
**                        КОНЕЦ ПРОГРАММЫ                                     **
**                                                                            **
********************************************************************************
*******************************************************************************/