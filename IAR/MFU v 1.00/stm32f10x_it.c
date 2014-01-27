/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "main.h"
#include "navigation.h" 
#include "Alarm.h"
#include "GSM.h"
#include "Bluetooth.h"  
#include "Transceiver_433MHz.h"
#include "CRYPTO-AES.h"    
#include <string.h> 
#include <stdio.h>
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
FunctionalState Door_alarm_state = DISABLE;

uint32_t MotorControlTime , Last_MotorControlTime;



/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{ 
    
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
// GPIO_SetBits(GPIOA , GPIO_Pin_11|GPIO_Pin_12);
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void) //Прерывание системного таймера итервал 1мкС
{
 if((delay_EnableStatus == ENABLE)&&(delay_Counter!=0)) delay_Counter--;//Если активна подпрограмма задержки декремент счетчика

 
 if((GPIO_ReadInputDataBit(ALARM2, DOOR_TR)==1)&&(STATUS.SecurityStatus==ENABLE)) //Проверка тригера двери DOOR_TR
 {
   if (Door_alarm_state==DISABLE)
   {
     Door_alarm_state = ENABLE;
     EXTI_GenerateSWInterrupt(EXTI_Line0);//Генерация прерывания
   }
 }
 else Door_alarm_state = DISABLE; 
 
 if (MotorCtrlMode == TAHOMETR) MotorControlTime++;
 else
 {
   if(GPIO_ReadInputDataBit(ALARM1, MOTOR_CTRL ) == 1) STATUS.MOTOR_Status = ENABLE;
   else STATUS.MOTOR_Status = DISABLE;
 }
}

                      /*ПРЕРЫВАНИЯ ОТ ПЕРЕФИРИИ*/
void ADC1_2_IRQHandler(void)        //Обработчик прерывания от АЦП1-2
{
  uint16_t BAT;//Заряд батареи
  uint8_t alarm_cnt; //Счетчик
  char second[] = "FF"; //Буфер секунд
  
 if(ADC_GetITStatus(ADC1 , ADC_IT_AWD)!=RESET) 
 {      
                  /*Обработчик события аварии*/
   for(alarm_cnt=30 ; alarm_cnt>0 ; alarm_cnt--)
   {
     SendString_InUnit("STATUS:ALARM" , Bluetooth); //Отправка статуса аварии
     sprintf(second , "%02d", alarm_cnt); //Преобразование числа в строку
     SendString_InUnit(second , Bluetooth); //Отправка секунд
     SendString_InUnit("\r\n" , Bluetooth); //Символы конца строки
     
     if (Bluetooth_Parser(Bluetooth_Answer2) != NULL) 
    {
      Reset_rxDMA_ClearBufer(Bluetooth);
      break;
    }
    IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
    delay_ms(1000);
    if(alarm_cnt==1) SendData_onServer(1100,0); //Отправка аварии на сервер
    
   }
   
   Reset_rxDMA_ClearBufer(Bluetooth);
   ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);//Очистка флага прерывания оконного компаратора
 }
 
 
 if(ADC_GetITStatus(ADC2 , ADC_IT_JEOC)!=RESET)
 {
   ADC_ClearITPendingBit(ADC2, ADC_IT_JEOC);//Очистка флага прерывания АЦП2
   
                /*Обработчик контроля питания*/
   
   if(ADC_GetConversionValue(ADC2)<=1464) //Если входное напряжение меньше 10 влольт
   {
     STATUS.MainPower=DISABLE; //Основное питание отсутствует
     
     BAT=ADC_GetInjectedConversionValue(ADC2 , ADC_InjectedChannel_1);
     if(BAT>=1800) STATUS.BatteryCharge=99;
     else 
     {
       if(BAT<=1500) STATUS.BatteryCharge=0;
       else STATUS.BatteryCharge=((BAT-1550)/3);
       
         
       
     }
     
   }
   else STATUS.MainPower=ENABLE; //Основное питание в норме
 }  
 
}

void DMA1_Channel3_IRQHandler(void) //Обработчик прерывания половины заполнения буфера приема  навигатора
{
 if(DMA_GetITStatus(DMA1_IT_HT3)) //Проверка флага окончания передачи канала4
 {
   DMA_ClearITPendingBit(DMA1_IT_GL3); //Сброс флагов прерывания:половина передачи,конец передачи и глобальное прерывание канала4
  ReadCoordinates();  
 } 
}

void EXTI9_5_IRQHandler(void) //Внешние прерывания линии 5-9
{
  if(EXTI_GetITStatus(EXTI_Line6) != RESET) //Вход стартера
  {
   
    delay_ms(10);
    /*
    if((STATUS.SecurityStatus==DISABLE)&&(STATUS.AUTOSTART==DISABLE)&&(GPIO_ReadInputDataBit(ALARM1,IGN1_IN )==1)&&(GPIO_ReadInputDataBit(ALARM1, ST_IN  )==1))
    {
      
     //GPIO_SetBits(ALARM1 , ST_OUT); //Запуск стартера
     while(GPIO_ReadInputDataBit(ALARM1 , ST_IN )==1)
     {
      if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) break; //Если мотор запустился выход из цикла 
     }
     GPIO_ResetBits(ALARM1 , ST_OUT); //Остановка стартера
    }
    */
    EXTI_ClearITPendingBit(EXTI_Line6); //Очистка флага прерывания

  }
//============================================================================== 
  if(EXTI_GetITStatus(EXTI_Line8) != RESET) //Датчик удара SHOCK1
  {
    delay_ms(10);
    if((GPIO_ReadInputDataBit(ALARM2, SHOCK1 )==1)&&(STATUS.SecurityStatus==ENABLE))
    {
     SIREN_and_LIGHTS(3);  // Тройное мигание фарами и звуковой сигнал
    }
    EXTI_ClearITPendingBit(EXTI_Line8); //Очистка флага прерывания  
  }
  
//==============================================================================  
  if(EXTI_GetITStatus(EXTI_Line9) != RESET) //Датчик удара SHOCK2
  {
    delay_ms(10);
    if((GPIO_ReadInputDataBit(ALARM2, SHOCK2 )==1)&&(STATUS.SecurityStatus==ENABLE))
    {
     SIREN_and_LIGHTS(3);  // Тройное мигание фарами и звуковой сигнал
     SIREN_and_LIGHTS(LONG_SIREN);  // Длительная сирена
     SendData_onServer(1300, 0); //Отправка аварии на сервер
     delay_ms(1000);
     SEND_SMS(SHOCK_ALARM);
      
    }
    
    EXTI_ClearITPendingBit(EXTI_Line9); //Очистка флага прерывания  
  }

 
}
//==============================================================================
void EXTI15_10_IRQHandler(void) //Внешние прерывания линии 10-15
{
  if(EXTI_GetITStatus(EXTI_Line10) != RESET) //Тригер багажника TRUNK_TR
  {
    delay_ms(2000);
    if((GPIO_ReadInputDataBit(ALARM2, TRUNK_TR)==0)&&(STATUS.SecurityStatus==ENABLE))
    {
      /*
     SIREN_and_LIGHTS(3);  // Тройное мигание фарами и звуковой сигнал 
     SIREN_and_LIGHTS(LONG_SIREN);  // Длительная сирена
     if(STATUS.AUTOSTART==ENABLE) AUTOSTART(DISABLE); //Если двигатель запущен остановить
     SendData_onServer(1500, 0); //Отправка аварии на сервер
     delay_ms(1000);
     SEND_SMS(TRUNK_ALARM);
      
      */     
      delay_ms(10);
    }
    
    
    EXTI_ClearITPendingBit(EXTI_Line10); //Очистка флага прерывания  
  }
  
//==============================================================================  
  if(EXTI_GetITStatus(EXTI_Line11) != RESET) //Внешние прерывания линии 11 прием пакета
  {
    char second[] = "FF"; //Буфер секунд
    
    if(TX_state == DISABLE  && STATUS.Transceiver_Status == ENABLE)
    {  
    while(GPIO_ReadInputDataBit(SPI_PORT, TR_GP1 )==1); //Ожидание конца передачи  
 
    RECEIVE_PAKET();
    
    if ((CRYPT == DISABLE)&&(strstr((char*)SPI_buffer , BRELOK_SN) !=NULL))
    {
      CLEAR_SPI_buffer(); //Очистка SPI буфера
      
      for(uint8_t z=0; z<16;z++) BUFER[z] = Navi_RxBuffer[86+z];
      
      SPI_buffer[0] = 17;
      Encrypt((uint8_t *)KEY , BUFER , (SPI_buffer+1));
      
      SEND_PAKET(); //Отправка пакета
      CRYPT = ENABLE;
      TIM_Cmd(TIM6, ENABLE); //Запуск  таймера TTL
    }
    else if(CRYPT == ENABLE)
    {
      FunctionalState COMP_CRYPT = DISABLE;
      
      CRYPT = DISABLE;
      TIM_Cmd(TIM6, DISABLE); //Остановка таймера
      
      for(uint8_t y=0;y<16;y++)
      {
       if(SPI_buffer[y+1] == BUFER[y]) COMP_CRYPT = ENABLE;
       else{
           COMP_CRYPT = DISABLE;
           break;
           }
      }
      if(COMP_CRYPT == ENABLE)
      {
       COMP_CRYPT = DISABLE; 
       
       uint8_t RADIO_COMAND;
       RADIO_COMAND = SPI_buffer[17];
         
       CLEAR_SPI_buffer(); //Очистка SPI буфера 
       
       SPI_buffer[0]=9;
       SPI_buffer[1]='E';
       SPI_buffer[2]='X';
       SPI_buffer[3]='E';
       SPI_buffer[4]='C';
       SPI_buffer[5]='U';
       SPI_buffer[6]='T';
       SPI_buffer[7]='E';
       SPI_buffer[8]='D';
       
       SEND_PAKET(); //Отправка пакета
       
       
       
       switch(RADIO_COMAND)
       {
       case OPEN:
         SECURITY(DISABLE);
         break;
         
       case CLOSE:
         SECURITY(ENABLE);
         break;
         
       case ALARM:
         
          /*Обработчик события аварии*/
         
         
         for(uint8_t alarm_cnt=30 ; alarm_cnt>0 ; alarm_cnt--)
         {
         SendString_InUnit("STATUS:ALARM" , Bluetooth); //Отправка статуса аварии
         sprintf(second , "%02d", alarm_cnt); //Преобразование числа в строку
         SendString_InUnit(second , Bluetooth); //Отправка секунд
         SendString_InUnit("\r\n" , Bluetooth); //Символы конца строки
     
         if (Bluetooth_Parser(Bluetooth_Answer2) != NULL) 
         {
         Reset_rxDMA_ClearBufer(Bluetooth);
         break;
        }
        IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
        delay_ms(1000);
        if(alarm_cnt==1) SendData_onServer(1100, 0); //Отправка аварии на сервер
    
        }
   
        Reset_rxDMA_ClearBufer(Bluetooth);
        break; 
         
       }
       
      }
      
    }
    
    CLEAR_SPI_buffer(); //Очистка SPI буфера
    delay_ms(10);
    STROB(SRX);  //Переход в режим приема
    
 
    }  
    
    GPIO_ResetBits(ALARM2 , SIREN ); //Отключение сирены
    EXTI_ClearITPendingBit(EXTI_Line11); //Очистка флага прерывания  
  }
//==============================================================================  
  if(EXTI_GetITStatus(EXTI_Line12) != RESET) //Тригер капота HOOD_TR
  {
   
    delay_ms(10);
    if((GPIO_ReadInputDataBit(ALARM2, HOOD_TR)==0)&&(STATUS.SecurityStatus==ENABLE))
    {
     SIREN_and_LIGHTS(3);  // Тройное мигание фарами и звуковой сигнал 
     SIREN_and_LIGHTS(LONG_SIREN);  // Длительная сирена
     if(STATUS.AUTOSTART==ENABLE) AUTOSTART(DISABLE); //Если двигатель запущен остановить
     //GPIO_SetBits(ALARM2 , SIREN ); //Включение сирены
     SendData_onServer(1600, 0); //Отправка аварии на сервер
     delay_ms(1000);
     SEND_SMS(HOOD_ALARM);
      
    }
    
    EXTI_ClearITPendingBit(EXTI_Line12); //Очистка флага прерывания  
  }
//==============================================================================  
  if(EXTI_GetITStatus(EXTI_Line15) != RESET) //Контроль стояночного тормоза или датчика нейтрали
  {
    
    delay_ms(10);
    if(GPIO_ReadInputDataBit(ALARM1, BRAKE_CTRL)==1)
    {
     if(STATUS.SecurityStatus==ENABLE)
     {
      SIREN_and_LIGHTS(3);  // Тройное мигание фарами и звуковой сигнал 
      SIREN_and_LIGHTS(LONG_SIREN);  // Длительная сирена
      SendData_onServer(1400,0); //Отправка аварии на сервер
      delay_ms(1000);
      SEND_SMS(DOOR_ALARM);
     } 
     
     if(STATUS.AUTOSTART==ENABLE) 
     {
      AUTOSTART(DISABLE); //Если двигатель запущен остановить
      delay_ms(100);    
      if(GPIO_ReadInputDataBit(ALARM1, IGN1_IN)==0) SIREN_and_LIGHTS(2); //Двойное мигание фарами 
     }
     if(STATUS.AUTOHEAT_MODE !=MODE_AUTOHEAT_OFF) STATUS.AUTOHEAT_MODE = MODE_AUTOHEAT_OFF;
    }
    
    EXTI_ClearITPendingBit(EXTI_Line15); //Очистка флага прерывания  
  }
//========================================================================================
 
  if(EXTI_GetITStatus(EXTI_Line14) != RESET) //Линия прерывания по запуску двигателя
 {
   
 if (MotorCtrlMode == TAHOMETR)
 {
 if((MotorControlTime - Last_MotorControlTime) < 60)
 {
  GPIO_ResetBits(ALARM1 , ST_OUT);//Остановка стартера  
  STATUS.MOTOR_Status = ENABLE;
 }
 else STATUS.MOTOR_Status = DISABLE;
 
 Last_MotorControlTime = MotorControlTime;
 }
 else
 {
  GPIO_ResetBits(ALARM1 , ST_OUT);//Остановка стартера  
  STATUS.MOTOR_Status = ENABLE; 
 }
 
 EXTI_ClearITPendingBit(EXTI_Line14); //Очистка флага прерывания  
 }
  
  
} 
  

//=========================================================================================
void EXTI0_IRQHandler(void)  //Тригер двери DOOR_TR
{
 if(EXTI_GetITStatus(EXTI_Line0) != RESET) 
 {
   delay_ms(10);
    if((GPIO_ReadInputDataBit(ALARM2, DOOR_TR)==1)&&(STATUS.SecurityStatus==ENABLE))
    {
     SIREN_and_LIGHTS(3);  // Тройное мигание фарами и звуковой сигнал 
     SIREN_and_LIGHTS(LONG_SIREN);  // Длительная сирена
     if(STATUS.AUTOSTART==ENABLE) AUTOSTART(DISABLE); //Если двигатель запущен остановить
     SendData_onServer(1400, 0); //Отправка аварии на сервер
     delay_ms(1000);
     SEND_SMS(DOOR_ALARM);
    } 
 
 }
 EXTI_ClearITPendingBit(EXTI_Line0); //Очистка флага прерывания 
}
//==================================================================================
void EXTI3_IRQHandler(void) //Входящий телефонный звонок
{
 if((EXTI_GetITStatus(EXTI_Line3) != RESET) && (STATUS.GSM_Status == ENABLE))
 {
   delay_ms(10);
   if(GPIO_ReadInputDataBit(GSM_MOD, GSM_RING)==0)
   {
     if(STATUS.GSM_DataMode == ENABLE) SendString_InUnit("+++", GSM);
     for(uint8_t j=0;j<150;j++)
     {
       if(strstr(GSM_RxBuffer , "RING") !=NULL)
       {
         delay_ms(100);
         ANSWER_CALL();
         SendString_InUnit("ATH\r\n" , GSM); 
         while(GPIO_ReadInputDataBit(GSM_MOD, GSM_RING)==0); 
         break;
         
       }
       delay_ms(100);
     
     }
  
   Reset_rxDMA_ClearBufer(GSM); //Сброс буфера 
   } 
  
 }
  
  EXTI_ClearITPendingBit(EXTI_Line3); //Очистка флага прерывания
}
//==================================================================================
void TIM6_IRQHandler(void) //TTL шифровальной функции
{
 if(TIM_GetITStatus(TIM6 , TIM_IT_Update)!= RESET) 
 {
  TIM_Cmd(TIM6, DISABLE); //Остановка таймера
  CRYPT = DISABLE; 
 }
 TIM_ClearITPendingBit(TIM6 , TIM_IT_Update); //Очистка флага прерывания 
}
//===============================================================================
void TIM7_IRQHandler(void) //Длительность звучания сирены
{
  if(TIM_GetITStatus(TIM7 , TIM_IT_Update)!= RESET)
  {
   TIM_Cmd(TIM7, DISABLE); //Остановка таймера 
   STATUS.LONG_ALARM = DISABLE; //Сброс режима долгой работы сирены
   GPIO_ResetBits(ALARM2 , SIREN ); //Выключение сирены  
    
  }
    
  
  
 TIM_ClearITPendingBit(TIM7 , TIM_IT_Update); //Очистка флага прерывания 
}
//=============================================================================
void RTC_IRQHandler(void) //Прерывания от часов реального времени
{
  uint32_t time_now;
  
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
  {
    sec_cnt++;
    sec_cnt2++;
    Bluetooth_Read();
    SendString_InUnit("\r\nSTATUS:NORMAL\r\n" , Bluetooth);
  
    RTC_ClearITPendingBit(RTC_FLAG_SEC);    
  }
 //===============================================================================
  if (RTC_GetITStatus(RTC_IT_ALR) != RESET) //Прерывание ALARM
  {
    if(STATUS.AUTOHEAT_MODE != MODE_AUTOHEAT_OFF)
    {
      
        
      switch(STATUS.AUTOHEAT_MODE)
      {
        case MODE_AUTOHEAT_0530:
          if(STATUS.AUTOSTART == ENABLE) //Интервал простоя  
          {            
            AUTOSTART(DISABLE);
            time_now = RTC_GetCounter(); //Считывание времени
            RTC_SetAlarm(time_now+21600);
            RTC_WaitForLastTask(); //Ожидание окончания записи в регистры            
          }
          else
          {
            AUTOSTART(ENABLE);     //Интервал работы двигателя
            time_now = RTC_GetCounter(); //Считывание времени
            RTC_SetAlarm(time_now+900);
            RTC_WaitForLastTask(); //Ожидание окончания записи в регистры
          }
          break;
          
          
       case MODE_AUTOHEAT_1030:
          if(STATUS.AUTOSTART == ENABLE) //Интервал простоя  
          {            
            AUTOSTART(DISABLE);
            time_now = RTC_GetCounter(); //Считывание времени
            RTC_SetAlarm(time_now+14400);
            RTC_WaitForLastTask(); //Ожидание окончания записи в регистры            
          }
          else
          {
            AUTOSTART(ENABLE);     //Интервал работы двигателя
            time_now = RTC_GetCounter(); //Считывание времени
            RTC_SetAlarm(time_now+900);
            RTC_WaitForLastTask(); //Ожидание окончания записи в регистры
          }
          break; 
          
        case MODE_AUTOHEAT_3030:
          if(STATUS.AUTOSTART == ENABLE) //Интервал простоя  
          {            
            AUTOSTART(DISABLE);
            time_now = RTC_GetCounter(); //Считывание времени
            RTC_SetAlarm(time_now+10800);
            RTC_WaitForLastTask(); //Ожидание окончания записи в регистры            
          }
          else
          {
            AUTOSTART(ENABLE);     //Интервал работы двигателя
            time_now = RTC_GetCounter(); //Считывание времени
            RTC_SetAlarm(time_now+1200);
            RTC_WaitForLastTask(); //Ожидание окончания записи в регистры
          }
          break; 
          
        case MODE_AUTOHEAT_3020:
          if(STATUS.AUTOSTART == ENABLE) //Интервал простоя  
          {            
            AUTOSTART(DISABLE);
            time_now = RTC_GetCounter(); //Считывание времени
            RTC_SetAlarm(time_now+7200);
            RTC_WaitForLastTask(); //Ожидание окончания записи в регистры            
          }
          else
          {
            AUTOSTART(ENABLE);     //Интервал работы двигателя
            time_now = RTC_GetCounter(); //Считывание времени
            RTC_SetAlarm(time_now+1200);
            RTC_WaitForLastTask(); //Ожидание окончания записи в регистры
          }
          break; 
          
      }
    }
    
   RTC_ClearITPendingBit(RTC_IT_ALR); 
  }
}




/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
