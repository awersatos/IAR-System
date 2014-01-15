/*******************************************************************************
********************************************************************************
**                                                                            **
**          БИБЛИОТЕКА ФУНКЦИЙ МОДУЛЯ GSM WISMO 228                           **
**                                                                            **
********************************************************************************
*******************************************************************************/

//******************Подключаемые файлы******************************************
#include "GSM.h"
#include "main.h"
#include "navigation.h"
#include <string.h>
#include <stdio.h>
#include "Alarm.h"
//*************Инициализация глобальных переменных******************************
const char PHONE_NUMBER[] = "+79619918106";
const char CLIENT_PHONE_NUMBER[] = "+79134725888"; //Номер клиента

                                  /*+79139207097*/ //Дмитрий
const char SERVER[]= "\"bb1.avtoblackbox.com\""; //Сервер
const char PORT[]= ",80\r\n"; //Порт
char IMEI[] = "123456789012345"; //Массив для IMEI

//uint8_t call_cont = 0;

//***************Прототипы внутренних функций***********************************
static FunctionalState START_TCP_IP(void); //Функция запуска стека TCP/IP
//*****************Функции для работы с GSM  ***********************************


void GSM_Configuration(void) //Инициализация GSM
{
 FunctionalState stack = DISABLE; 
 uint8_t nr_sim=1;
 uint8_t x; //Счетчик
 char *im; //Ссылочная переменная для IMEI
 IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
 do {
  Reset_rxDMA_ClearBufer(GSM); //Сброс буфера 

  do{
  if(GPIO_ReadInputDataBit(GSM_MOD,GSM_READY)==0)
 {
  delay_ms(200); 
 GPIO_ResetBits(GSM_MOD , GSM_ON); //Включаем модем
 delay_ms(800);
  SIM(nr_sim); //Выбор сим карты
 
 GPIO_SetBits(GSM_MOD , GSM_ON);
 }
 
 else
 {
  GPIO_ResetBits(GSM_MOD , GSM_RESET); //Перезагружаем  модем
   SIM(nr_sim); //Выбор сим карты
 delay_ms(100);
 GPIO_SetBits(GSM_MOD , GSM_RESET); //Устанавливакм бит сброса
   
 }
  delay_ms(3000);
  }while(GPIO_ReadInputDataBit(GSM_MOD,GSM_READY)==0); //Ожидание готовности модема
 IWDG_ReloadCounter(); 
  delay_ms(1000);
  
  SendString_InUnit("AT+GSN\r\n" , GSM);  //Запрос IMEI  
  delay_ms(100);
  im=strstr(GSM_RxBuffer , "\r\n\r\n");
   if (im !=NULL)
   {
    im=im+4;
    for(x=0;x<15;x++) IMEI[x]=*im++; //Копирование IMEI в переменную
    Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
   }
  
  SendString_InUnit("AT+CLIP=1\r\n" , GSM); //Включение определителя номера
  delay_ms(100);
  Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
  
  SendString_InUnit("AT+CPIN?\r\n" , GSM);
  delay_ms(500);
  
  IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
  if (strstr(GSM_RxBuffer , "READY") !=NULL) //Если СИМ карта опознана
   {
     Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
     for(x=0;x<10;x++) //Проверка регистрации в сети
     {
      if(REG_NET()=='R') //Если регистрация успешна
      {
        STATUS.GSM_Status=ACTIVE;
        STATUS.SIM_Card=nr_sim;
        SendString_InUnit("AT+COPS?\r\n" , GSM); //Определение оператора сотовой связи
        delay_ms(100);
        if (strstr(GSM_RxBuffer , "Beeline") !=NULL) STATUS.OPERATOR=Beeline_OP;
        if (strstr(GSM_RxBuffer , "MTS") !=NULL) STATUS.OPERATOR=MTS_OP;
        if (strstr(GSM_RxBuffer , "MegaFon") !=NULL) STATUS.OPERATOR=Megafon_OP;
        Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
        
   for(x=0;x<3;x++) //Запуск TCP/IP стека
  {
   stack =  START_TCP_IP();  
   if(stack == ENABLE) break;
   delay_ms(3000);
   IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
  }
        
        break;
      }
       else 
       {
        STATUS.GSM_Status=INACTIVE;
        delay_ms(2000);
       }
     }
    
   }
  
 
  
   nr_sim++;
   if(nr_sim>3) nr_sim=1; 
   IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
 }while((STATUS.GSM_Status==INACTIVE) && (stack == DISABLE));  
 
 SendString_InUnit("AT+CMGF=1\r\n" , GSM); //Переключение в режим текстовых сообщений
 delay_ms(100);
 
Reset_rxDMA_ClearBufer(GSM); //Сброс буфера

}

//===============================================================================

void SIM(uint8_t sim) //Функция переключения СИМ карт
{
  switch(sim)
  {
  case 1: 
    {
      GPIO_ResetBits(GSM_MOD , SIM2);
      GPIO_ResetBits(GSM_MOD , SIM3); 
      GPIO_SetBits(GSM_MOD , SIM1);
      break;
    }
    
  case 2:
    {
      GPIO_ResetBits(GSM_MOD , SIM1);
      GPIO_ResetBits(GSM_MOD , SIM3);
      GPIO_SetBits(GSM_MOD , SIM2);
      break;
    }
    
    case 3:
    {
      GPIO_ResetBits(GSM_MOD , SIM1);
      GPIO_ResetBits(GSM_MOD , SIM2);
      GPIO_SetBits(GSM_MOD , SIM3);
      break;
    }
  }
  
}
//===============================================================================
char REG_NET(void)  //Проверка регистации в сети
{ char *d;
  Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
  SendString_InUnit("AT+CREG?\r\n" , GSM);
  delay_ms(100);
  d=strstr(GSM_RxBuffer , "+CREG:");
   if(d!=NULL)
   {
     d=d+9;
     if((*d=='1')|(*d=='5')) 
     { Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
       return 'R';}    
   }
    Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
   return NULL; 
}
//===============================================================================  
void SendData_onServer(uint16_t state, uint8_t rmc_buf)  //Функция отправки данных на сервер

{
 
  FunctionalState execute = DISABLE; //Статус обмена данными с сервером
  uint8_t cntr; //Счетчик
  char out[]="ffff"; 


  // Если выбрана не 1 сим карта или нет регистрации в сети то перезапуск модема и поиск доступной сети
  if((STATUS.SIM_Card!=1) || (REG_NET()!='R')) GSM_Configuration();  
  Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
  
    SendString_InUnit("AT+WIPCREATE=2,1," , GSM); //Открытие сокета 
    SendString_InUnit(SERVER , GSM);
    SendString_InUnit(PORT , GSM);
    
     for (cntr=0;cntr<250;cntr++)
    {
      if(strstr(GSM_RxBuffer , "+WIPREADY: 2,1") !=NULL) //Если сокет открылся 
      {
       SendString_InUnit("AT+WIPDATA=2,1,1\r\n" , GSM); //Пререход в режим передачи данных
       delay_ms(100);
       if(strstr(GSM_RxBuffer , "CONNECT") !=NULL) //Если связь установлена 
       {  
         STATUS.GSM_DataMode = ENABLE;
        Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
        
/***************************Передача информации на сервер*************************************/
        
        SendString_InUnit("POST /bb/status HTTP/1.1\r\n" , GSM); //Заголовок POST запроса
        SendString_InUnit("Host: avtoblackbox.com\r\n" , GSM);
        SendString_InUnit("Content-Type: application/x-www-form-urlencoded\r\n" , GSM);
        
        if(STATUS.CoordinatesStatus == 'A')
        { 
        if(Speed[rmc_buf][3] == 0x00) SendString_InUnit("Content-Length: 136\r\n" , GSM);
        else{
             if(Speed[rmc_buf][4] == 0x00) SendString_InUnit("Content-Length: 137\r\n" , GSM);
             else SendString_InUnit("Content-Length: 138\r\n" , GSM);             
            }
        Navi_ResetCounter++;
        }
        else 
        {
          period = 30;
          SendString_InUnit("Content-Length: 86\r\n" , GSM);
          Navi_ResetCounter += 3;  
        }
        
        SendString_InUnit("\r\n" , GSM); //Конец строки
                    /*********/
        
        SendString_InUnit("utc_time=" , GSM); //Передача времени
        SendString_InUnit(&(Timestamp[rmc_buf][0]) , GSM);
        
        if(STATUS.CoordinatesStatus == 'A')
        { SendString_InUnit("&lat=" , GSM); //Передача широты
        SendString_InUnit(&(Latitude[rmc_buf][0]) , GSM); }
        
        if(STATUS.CoordinatesStatus == 'A')
        {SendString_InUnit("&lon=" , GSM); //Передача долготы
        SendString_InUnit(&(Longitude[rmc_buf][0]) , GSM);}
        
        SendString_InUnit("&imei=" , GSM); //Передача IMEI
        SendString_InUnit(IMEI , GSM);
        
        SendString_InUnit("&command=" , GSM); //Передача команды
        sprintf(out , "%04d", state);
        SendString_InUnit(out ,GSM); 
        
        SendString_InUnit("&s_n=" , GSM); //Передача серийного номера
        SendString_InUnit(SERIAL_NUMBER , GSM);
        
        SendString_InUnit("&phone=" , GSM); //Передача телефонного  номера
        SendString_InUnit(PHONE_NUMBER , GSM);
        
        if(STATUS.CoordinatesStatus == 'A')
        {SendString_InUnit("&speed=" , GSM); //Передача скорости  
        SendString_InUnit(&(Speed[rmc_buf][0]) , GSM);}
        
        if(STATUS.CoordinatesStatus == 'A')
        {SendString_InUnit("&date=" , GSM); //Передача даты  
        SendString_InUnit(&(Date[rmc_buf][0]) , GSM);}
        
        SendString_InUnit("\r\n" , GSM); //Конец строки
        SendString_InUnit("\r\n" , GSM); //Конец строки
/*************************Ожидание ответа от сервера***************************/       
         for (cntr=0;cntr<13;cntr++)
         {
           
          if(strstr(GSM_RxBuffer , "OK*") !=NULL)
          {
            if(strstr(GSM_RxBuffer , "*00#") !=NULL) STATUS.COMMAND=00;
            else
            {
            if(strstr(GSM_RxBuffer , "*10#") !=NULL) STATUS.COMMAND=10;  //Открыть машину снять с охраны
            if(strstr(GSM_RxBuffer , "*11#") !=NULL) STATUS.COMMAND=11;  //Закрыть машину/поставить на охрану
            if(strstr(GSM_RxBuffer , "*13#") !=NULL) STATUS.COMMAND=13;  //Запустить двигатель 
            if(strstr(GSM_RxBuffer , "*14#") !=NULL) STATUS.COMMAND=14;  //Заглушить двигатель
            if(strstr(GSM_RxBuffer , "*15#") !=NULL) STATUS.COMMAND=15;  //Активировать режим автопрогрева 5/30
            if(strstr(GSM_RxBuffer , "*16#") !=NULL) STATUS.COMMAND=16;  ////Активировать режим автопрогрева 10/30
            if(strstr(GSM_RxBuffer , "*17#") !=NULL) STATUS.COMMAND=17;  ////Активировать режим автопрогрева 30/30
            if(strstr(GSM_RxBuffer , "*18#") !=NULL) STATUS.COMMAND=18;  ////Активировать режим автопрогрева 30/20
            if(strstr(GSM_RxBuffer , "*19#") !=NULL) STATUS.COMMAND=19;  //Отключить режим автопрогрева
            if(strstr(GSM_RxBuffer , "*20#") !=NULL) STATUS.COMMAND=20;  //Найти машину
           
            }
           execute = ENABLE; 
           break;
          }
          delay_ms(1000);
          IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
         }
 /***********************Закрытие сокета****************************************/
        delay_ms(100);   
        Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
        
        SendString_InUnit("+++", GSM);
        
        for(uint8_t i=0;i<10; i++)
        {
         delay_ms(100);
         if(strstr(GSM_RxBuffer , "OK") !=NULL)
         {
           SendString_InUnit("AT+WIPCLOSE=2,1\r\n", GSM);
           delay_ms(100);
           STATUS.GSM_DataMode = DISABLE;
           break; 
         }
        }
        
       }
       break;
      }
      if(strstr(GSM_RxBuffer , "ERROR") !=NULL)
      {
 
       break;
      }  
      delay_ms(100);
      IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
    }
    
 Reset_rxDMA_ClearBufer(GSM); //Сброс буфера 
 /*********************************/
 if((execute == DISABLE) && (rmc_buf == 0))
 {
 delay_ms(1000);  
 if(state != 0)
 {
 STATUS.EVENT_BUF[2] = STATUS.EVENT_BUF[1];
 STATUS.EVENT_BUF[1] = STATUS.EVENT_BUF[0];
 STATUS.EVENT_BUF[0] = state;
 }
 /****************Запись данных в резервный буфер******************************/
 else if(STATUS.CoordinatesStatus == 'A')
 {
  if(Timestamp[1][0] == 0)
  {
    for(uint8_t i=0;i<11;i++) 
    {
    Timestamp[1][i] = Timestamp[0][i];
    if(i<9) Latitude[1][i] = Latitude[0][i];
    if(i<10) Longitude[1][i] = Longitude[0][i];
    if(i<6) Speed[1][i] = Speed[0][i];
    if(i<7) Date[1][i] = Date[0][i];
    }
  }
  else 
  {
    if(Timestamp[2][0] == 0)
  {
    for(uint8_t i=0;i<11;i++) 
    {
    Timestamp[2][i] = Timestamp[0][i];
    if(i<9) Latitude[2][i] = Latitude[0][i];
    if(i<10) Longitude[2][i] = Longitude[0][i];
    if(i<6) Speed[2][i] = Speed[0][i];
    if(i<7) Date[2][i] = Date[0][i];
    }
  }
    else
    {
      if(Timestamp[3][0] == 0)
  {
    for(uint8_t i=0;i<11;i++) 
    {
    Timestamp[3][i] = Timestamp[0][i];
    if(i<9) Latitude[3][i] = Latitude[0][i];
    if(i<10) Longitude[3][i] = Longitude[0][i];
    if(i<6) Speed[3][i] = Speed[0][i];
    if(i<7) Date[3][i] = Date[0][i];
    }
  }
      else
      {
        if(Timestamp[4][0] == 0)
  {
    for(uint8_t i=0;i<11;i++) 
    {
    Timestamp[4][i] = Timestamp[0][i];
    if(i<9) Latitude[4][i] = Latitude[0][i];
    if(i<10) Longitude[4][i] = Longitude[0][i];
    if(i<6) Speed[4][i] = Speed[0][i];
    if(i<7) Date[4][i] = Date[0][i];
    }
  }
        else if(Timestamp[5][0] == 0)
  {
    for(uint8_t i=0;i<11;i++) 
    {
    Timestamp[5][i] = Timestamp[0][i];
    if(i<9) Latitude[5][i] = Latitude[0][i];
    if(i<10) Longitude[5][i] = Longitude[0][i];
    if(i<6) Speed[5][i] = Speed[0][i];
    if(i<7) Date[5][i] = Date[0][i];
    }
  }
      }
    }
  }
 }
 GSM_Configuration();  
   
 }
 
 //Очистка резервного буфера
 else if(rmc_buf !=0)
 {
  switch(rmc_buf)
  {
  case 1:
    
  
    for(uint8_t i=0;i<11;i++) 
    {
    Timestamp[1][i] = 0x00;
    if(i<9) Latitude[1][i] = 0x00;
    if(i<10) Longitude[1][i] = 0x00;
    if(i<6) Speed[1][i] = 0x00;
    if(i<7) Date[1][i] = 0x00;
    }
  
    break;
    
  case 2:
    for(uint8_t i=0;i<11;i++) 
    {
    Timestamp[2][i] = 0x00;
    if(i<9) Latitude[2][i] = 0x00;
    if(i<10) Longitude[2][i] = 0x00;
    if(i<6) Speed[2][i] = 0x00;
    if(i<7) Date[2][i] = 0x00;
    }
    break; 
        
  case 3:
    for(uint8_t i=0;i<11;i++) 
    {
    Timestamp[3][i] = 0x00;
    if(i<9) Latitude[3][i] = 0x00;
    if(i<10) Longitude[3][i] = 0x00;
    if(i<6) Speed[3][i] = 0x00;
    if(i<7) Date[3][i] = 0x00;
    }
    break;  
    
  case 4:
    for(uint8_t i=0;i<11;i++) 
    {
    Timestamp[4][i] = 0x00;
    if(i<9) Latitude[4][i] = 0x00;
    if(i<10) Longitude[4][i] = 0x00;
    if(i<6) Speed[4][i] = 0x00;
    if(i<7) Date[4][i] = 0x00;
    }
    break; 
        
  case 5:
    for(uint8_t i=0;i<11;i++) 
    {
    Timestamp[5][i] = 0x00;
    if(i<9) Latitude[5][i] = 0x00;
    if(i<10) Longitude[5][i] = 0x00;
    if(i<6) Speed[5][i] = 0x00;
    if(i<7) Date[5][i] = 0x00;
    }
    break;  
    
    
  }
   
   
 }
 STATUS.GSM_DataMode = DISABLE;
}

//===============================================================================

void ANSWER_CALL(void) //Функция ответа на входящий звонок
{
if(strstr(GSM_RxBuffer , "RING") !=NULL)
{
if((strstr(GSM_RxBuffer , CLIENT_PHONE_NUMBER) !=NULL)/* &&  (call_cont == 0)*/ )
{
 SendString_InUnit("ATH\r\n" , GSM); 
 Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
 delay_ms(500);
 IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
 
 if(STATUS.AUTOSTART==ENABLE) 
 { /*
   AUTOSTART(DISABLE);
   SEND_SMS(MOTOR_OFF);
   */
   STATUS.COMMAND = 14;
 }
 else
 {
   /*
   AUTOSTART(ENABLE);
   delay_ms(3000);
   if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) SEND_SMS(MOTOR_ON); 
   else SEND_SMS(MOTOR_START_ERROR);
   */
   STATUS.COMMAND = 13;
 }
 //call_cont = 10;
}
SendString_InUnit("ATH\r\n" , GSM); 

  
}
//if(call_cont != 0) call_cont--;
Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
}

void RECEIVE_SMS(void) //Функция получения СМС сообщения
{
  char *start;
  char  *end; 
  char *data;
  uint16_t CODE;
  char index;
  char out[] = "AT+CMGD=x\r\n" ;
  
  
  Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
  
 SendString_InUnit("AT+CMGL=\"ALL\"\r\n" , GSM);  //команда чтения всех сообщений
 delay_ms(100);
 
 start = strstr(GSM_RxBuffer , "+CMGL: "); //Определение наличия нового сообщения
 
 if(start!=NULL)
 {
  start = start+7; 
  end = strstr(start , "\r\n");
  data = strstr(GSM_RxBuffer , CLIENT_PHONE_NUMBER); //Определение что текущее сообщение с зарегистрированного номера
  
  if(data>start && data<end)
  {
   end = end+5; //Определение границ командного кода
   data = end-3;
   if((*data == '0' ) && (*end == '#'))
   {
   CODE = (uint16_t)data[2] | ((uint16_t)data[1]<<8); //Выделение командного кода
   
   switch(CODE) //Дешифрация командного кода
   {
   case 0x3130:
     STATUS.COMMAND = 10; //открыть машину/снять с охраны
     break;
     
   case 0x3131:
     STATUS.COMMAND = 11; //Закрытьмашину/поставить на охрану
     break;
   
   case 0x3133:
     STATUS.COMMAND = 13; //Запустить двигатель
     break;  
     
   case 0x3134:
     STATUS.COMMAND = 14; //заглушить двигатель
     break;  
     
   case 0x3135:
     STATUS.COMMAND = 15; //Активировать режим автопрогрева 5/30
     break;
     
  case 0x3136:
     STATUS.COMMAND = 16; //Активировать режим автопрогрева 10/30 
     break; 
     
  case 0x3137:
     STATUS.COMMAND = 17; //Активировать режим автопрогрева 30/30 
     break;
     
  case 0x3138:
     STATUS.COMMAND = 18; //Активировать режим автопрогрева 30/20 
     break;
     
  case 0x3139:
     STATUS.COMMAND = 19; //Отключить режим автопрогрева  
     break;   
     
  case 0x3230:
     STATUS.COMMAND = 20; //Найти машину
     break;
   }
   
   }
  }
 index = *start; 
 //index = index-30; 
 sprintf(out , "AT+CMGD=%c\r\n", index); //Удаление текущего сообщения
 SendString_InUnit(out , GSM); 
 delay_ms(100); 
 SendString_InUnit("AT+CMGD=0,4\r\n" , GSM); 
 }
 
delay_ms(100);    
Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера 
}

//===============================================================================

void SEND_SMS(Answer_TypeDef answer) //Функция отправки СМС
{
 uint8_t i, j; //Счетчик 
 FunctionalState Send = DISABLE;
 char PDU_PHONE_NUMBER[] = "123456789012"; //Массив номера в PDU формате
 
 for(i=0; i<10; i+=2) PDU_PHONE_NUMBER[i] = CLIENT_PHONE_NUMBER[i+2]; //Заполнение массива PDU
 PDU_PHONE_NUMBER[10] = 'F';
 for(i=1; i<12; i+=2) PDU_PHONE_NUMBER[i] = CLIENT_PHONE_NUMBER[i];
 
 SendString_InUnit("AT+CMGF=0\r\n" , GSM); //Включение PDU Режима
 delay_ms(50);
 for(j=0;j<5;j++)
 {
 SendString_InUnit("AT+CMGS=" , GSM); //Команда отправки сообщения
 
 switch(answer) //Ввод длины сообщения
 {
 case SECURITY_OFF:
   SendString_InUnit("41\r\n" , GSM);
   break;
   
 case SECURITY_ON:
   SendString_InUnit("53\r\n" , GSM);
   break;
   
 case MOTOR_ON:
   SendString_InUnit("63\r\n" , GSM);
   break;
   
 case MOTOR_START_ERROR:
   SendString_InUnit("85\r\n" , GSM);
   break;
   
 case MOTOR_OFF:
   SendString_InUnit("49\r\n" , GSM);
   break;
   
 case AUTOHEAT_ON_0530:
   SendString_InUnit("85\r\n" , GSM);
   break;
   
 case AUTOHEAT_ON_1030:
   SendString_InUnit("85\r\n" , GSM);
   break; 
   
 case AUTOHEAT_ON_3030:
   SendString_InUnit("85\r\n" , GSM);
   break;
   
 case AUTOHEAT_ON_3020:
   SendString_InUnit("85\r\n" , GSM);
   break;  
      
   
 case AUTOHEAT_OFF:
   SendString_InUnit("67\r\n" , GSM);
   break;
   
 case CHANGE_HEAT_MODE_0530:
   SendString_InUnit("83\r\n" , GSM);
   break;   
   
case CHANGE_HEAT_MODE_1030:
   SendString_InUnit("83\r\n" , GSM);
   break;    

case CHANGE_HEAT_MODE_3030:
   SendString_InUnit("83\r\n" , GSM);
   break; 

case CHANGE_HEAT_MODE_3020:
   SendString_InUnit("83\r\n" , GSM);
   break;
   
case SHOCK_ALARM:
   SendString_InUnit("53\r\n" , GSM);
   break; 

case HOOD_ALARM:
   SendString_InUnit("55\r\n" , GSM);
   break; 
   
case DOOR_ALARM:
   SendString_InUnit("53\r\n" , GSM);
   break;   

case TRUNK_ALARM:
   SendString_InUnit("61\r\n" , GSM);
   break;   
   
   
 }
 
 delay_ms(100);
 
 if(strchr(GSM_RxBuffer , '>')!=NULL)
 {
 Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
 
 SendString_InUnit("0001000B91" , GSM); //Заголовок PDU
 SendString_InUnit(PDU_PHONE_NUMBER , GSM); //Номер в формате PDU
 SendString_InUnit("0008" , GSM); //Настройки
 
 switch(answer) //Ввод текста сообщения
  {
  case SECURITY_OFF:
    SendString_InUnit("1C0421043D044F0442043E002004410020043E044504400430043D044B" , GSM);
    break;
    
  case SECURITY_ON:
    SendString_InUnit("28041F043E0441044204300432043B0435043D043E0020043D04300020043E044504400430043D0443" , GSM);
    break;
    
  case MOTOR_ON:
    SendString_InUnit("320414043204380433043004420435043B044C002004430441043F04350448043D043E002004370430043F044304490435043D" , GSM);  
    break;
  
  case MOTOR_START_ERROR:
    SendString_InUnit("4804170430043F04430441043A00200434043204380433043004420435043B044F00200437043004320435044004480438043B0441044F0020043D0435044304340430044704350439" , GSM);
    break;
    
  case MOTOR_OFF:
    SendString_InUnit("240414043204380433043004420435043B044C0020043704300433043B044304480435043D" , GSM);
    break;
     
  case AUTOHEAT_ON_0530:
    SendString_InUnit("480410043A04420438043204380440043E04320430043D00200440043504360438043C0020043004320442043E043F0440043E04330440043504320430002000310035002F00360447" , GSM);
    break;
    
  case AUTOHEAT_ON_1030:
    SendString_InUnit("480410043A04420438043204380440043E04320430043D00200440043504360438043C0020043004320442043E043F0440043E04330440043504320430002000310035002F00340447" , GSM);
    break;  
    
  case AUTOHEAT_ON_3030:
    SendString_InUnit("480410043A04420438043204380440043E04320430043D00200440043504360438043C0020043004320442043E043F0440043E04330440043504320430002000320030002F00330447" , GSM);
    break; 
    
  case AUTOHEAT_ON_3020:
    SendString_InUnit("480410043A04420438043204380440043E04320430043D00200440043504360438043C0020043004320442043E043F0440043E04330440043504320430002000320030002F00320447" , GSM);
    break;  

  case AUTOHEAT_OFF: 
    SendString_InUnit("360420043504360438043C0020043004320442043E043F0440043E043304400435043204300020043E0442043A043B044E04470435043D" , GSM); 
    break;
    
  case CHANGE_HEAT_MODE_0530: 
    SendString_InUnit("460420043504360438043C0020043004320442043E043F0440043E04330440043504320430002004380437043C0435043D0435043D0020043D0430002000310035002F00360447" , GSM); 
    break;
    
  case CHANGE_HEAT_MODE_1030: 
    SendString_InUnit("460420043504360438043C0020043004320442043E043F0440043E04330440043504320430002004380437043C0435043D0435043D0020043D0430002000310035002F00340447" , GSM); 
    break; 
    
  case CHANGE_HEAT_MODE_3030: 
    SendString_InUnit("460420043504360438043C0020043004320442043E043F0440043E04330440043504320430002004380437043C0435043D0435043D0020043D0430002000320030002F00330447" , GSM); 
    break; 
    
  case CHANGE_HEAT_MODE_3020: 
    SendString_InUnit("460420043504360438043C0020043004320442043E043F0440043E04330440043504320430002004380437043C0435043D0435043D0020043D0430002000320030002F00320447" , GSM); 
    break;  
    
    
  case SHOCK_ALARM: 
    SendString_InUnit("2804140430044204470438043A00200443043404300440043000200442044004350432043E04330430" , GSM); 
    break; 
    
  case HOOD_ALARM: 
    SendString_InUnit("2A04140430044204470438043A0020043A0430043F043E0442043000200442044004350432043E04330430" , GSM); 
    break;
    
  case DOOR_ALARM: 
    SendString_InUnit("2804140430044204470438043A00200434043204350440043800200442044004350432043E04330430" , GSM); 
    break;
    
  case TRUNK_ALARM: 
    SendString_InUnit("3004140430044204470438043A002004310430043304300436043D0438043A043000200442044004350432043E04330430" , GSM); 
    break;      
    
  }
 SendString_InUnit("\x1A" , GSM); //Команда отправки сообщения
 
 for( i=0; i<100; i++)
 { 
 delay_ms(50);
 if(strstr(GSM_RxBuffer , "OK") != NULL) 
  {
   Send = ENABLE;
   break;
  }
 delay_ms(50);
 if(strstr(GSM_RxBuffer , "ERROR") != NULL) break;
 }

 } 
  Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
if(Send == ENABLE) break;
delay_ms(2000);
 }
 
 SendString_InUnit("AT+CMGF=1\r\n" , GSM); //Переключение в режим текстовых сообщений
  delay_ms(100);
  Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
}

//==================================================================================================
static FunctionalState START_TCP_IP(void) //Функция запуска стека TCP/IP
{
  FunctionalState br = DISABLE;
  IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
  
  delay_ms(3000);
SendString_InUnit("AT+WIPCFG=1\r\n" , GSM); //Запуск TCP/IP стека
  delay_ms(100);
   if(strstr(GSM_RxBuffer , "ERROR") !=NULL) //Если стек не запускается останавливаем его и запускаем снова
   {
     
    Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
    SendString_InUnit("AT+WIPCFG=0\r\n" , GSM); //Остановка TCP/IP стека
    delay_ms(300);
    Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
    SendString_InUnit("AT+WIPCFG=1\r\n" , GSM); //Запуск TCP/IP стека     
    delay_ms(300);
   }
  
  if(strstr(GSM_RxBuffer , "OK") !=NULL) //Если стек запустился
  {
   Reset_rxDMA_ClearBufer(GSM); //Сброс буфера 
   SendString_InUnit("AT+WIPBR=1,6\r\n" , GSM);  //Открытие GPRS барьера
   delay_ms(100);
   
   switch(STATUS.OPERATOR) //Установка параметров GPRS барьера в зависимости от оператора
   {
   case Beeline_OP:
     {
      SendString_InUnit("AT+WIPBR=2,6,11,\"internet.beeline.ru\"\r\n" , GSM); //Установка APN
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,0,\"beeline\"\r\n" , GSM); //Ввод логина 
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,1,\"beeline\"\r\n" , GSM); //Ввод пароля
      delay_ms(100);
      break;
     }
     
   case MTS_OP:
     {
      SendString_InUnit("AT+WIPBR=2,6,11,\"internet.mts.ru\"\r\n" , GSM); //Установка APN
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,0,\"mts\"\r\n" , GSM); //Ввод логина 
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,1,\"mts\"\r\n" , GSM); //Ввод пароля
      delay_ms(100);
      break;
     }  
    case Megafon_OP:
     {
      SendString_InUnit("AT+WIPBR=2,6,11,\"internet\"\r\n" , GSM); //Установка APN
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,0,\"\"\r\n" , GSM); //Ввод логина 
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,1,\"\"\r\n" , GSM); //Ввод пароля
      delay_ms(100);
      break;
     }   
   }
    Reset_rxDMA_ClearBufer(GSM); //Сброс буфера
    SendString_InUnit("AT+WIPBR=4,6,0\r\n" , GSM);
    for (uint8_t cntr=0;cntr<50;cntr++)
    {
      if(strstr(GSM_RxBuffer , "OK") !=NULL)
      {
       br = ENABLE;
       break;
      }
      if(strstr(GSM_RxBuffer , "ERROR") !=NULL)
      {
       br = DISABLE;
       break;
      }  
      delay_ms(1000);
      IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
      br = DISABLE;
    }
  }
  Reset_rxDMA_ClearBufer(GSM); //Сброс буфера  
  
  return br;
  
}

 /***************************КОНЕЦ ФАЙЛА****************************************/ 