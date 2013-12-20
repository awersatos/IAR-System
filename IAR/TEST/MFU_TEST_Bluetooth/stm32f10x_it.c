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
#include <string.h> 
#include <stdio.h>
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
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
 GPIO_SetBits(GPIOA , GPIO_Pin_11|GPIO_Pin_12);
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
    delay_ms(1000);
    //if(alarm_cnt==1) SendData_onServer(1100); //Отправка аварии на сервер
    
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
    if((STATUS.SecurityStatus==DISABLE)&&(STATUS.AUTOSTART==DISABLE)&&(GPIO_ReadInputDataBit(ALARM1,IGN1_IN )==1)&&(GPIO_ReadInputDataBit(ALARM1, ST_IN  )==1))
    {
     GPIO_SetBits(ALARM1 , ST_OUT); //Запуск стартера
     while(GPIO_ReadInputDataBit(ALARM1 , ST_IN )==1);
     GPIO_ResetBits(ALARM1 , ST_OUT); //Остановка стартера
    }
    
    EXTI_ClearITPendingBit(EXTI_Line6); //Очистка флага прерывания
//==============================================================================
  }
  
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
     GPIO_SetBits(ALARM2 , SIREN ); //Включение сирены
    // SendData_onServer(1300); //Отправка аварии на сервер
     GPIO_ResetBits(ALARM2 , SIREN ); //Выключение сирены
      
    }
    
    EXTI_ClearITPendingBit(EXTI_Line9); //Очистка флага прерывания  
  }

 
}

void EXTI15_10_IRQHandler(void) //Внешние прерывания линии 10-15
{
  if(EXTI_GetITStatus(EXTI_Line10) != RESET) //Тригер багажника TRUNK_TR
  {
    delay_ms(10);
    if((GPIO_ReadInputDataBit(ALARM2, TRUNK_TR)==0)&&(STATUS.SecurityStatus==ENABLE))
    {
     SIREN_and_LIGHTS(3);  // Тройное мигание фарами и звуковой сигнал
     if(STATUS.AUTOSTART==ENABLE) AUTOSTART(DISABLE); //Если двигатель запущен остановить
     GPIO_SetBits(ALARM2 , SIREN ); //Включение сирены
     //SendData_onServer(1500); //Отправка аварии на сервер
     GPIO_ResetBits(ALARM2 , SIREN ); //Выключение сирены
      
    }
    
    
    EXTI_ClearITPendingBit(EXTI_Line10); //Очистка флага прерывания  
  }
  
//==============================================================================  
  if(EXTI_GetITStatus(EXTI_Line11) != RESET) //Тригер двери DOOR_TR
  {
    delay_ms(10);
    if((GPIO_ReadInputDataBit(ALARM2, DOOR_TR)==0)&&(STATUS.SecurityStatus==ENABLE))
    {
     SIREN_and_LIGHTS(3);  // Тройное мигание фарами и звуковой сигнал
     if(STATUS.AUTOSTART==ENABLE) AUTOSTART(DISABLE); //Если двигатель запущен остановить
     GPIO_SetBits(ALARM2 , SIREN ); //Включение сирены
    // SendData_onServer(1400); //Отправка аварии на сервер
     GPIO_ResetBits(ALARM2 , SIREN ); //Выключение сирены
      
    }
    
    EXTI_ClearITPendingBit(EXTI_Line11); //Очистка флага прерывания  
  }
//==============================================================================  
  if(EXTI_GetITStatus(EXTI_Line12) != RESET) //Тригер капота HOOD_TR
  {
   
    delay_ms(10);
    if((GPIO_ReadInputDataBit(ALARM2, HOOD_TR)==0)&&(STATUS.SecurityStatus==ENABLE))
    {
     SIREN_and_LIGHTS(3);  // Тройное мигание фарами и звуковой сигнал
     if(STATUS.AUTOSTART==ENABLE) AUTOSTART(DISABLE); //Если двигатель запущен остановить
     GPIO_SetBits(ALARM2 , SIREN ); //Включение сирены
    // SendData_onServer(1600); //Отправка аварии на сервер
     GPIO_ResetBits(ALARM2 , SIREN ); //Выключение сирены
      
    }
    
    EXTI_ClearITPendingBit(EXTI_Line12); //Очистка флага прерывания  
  }
//==============================================================================  
  if(EXTI_GetITStatus(EXTI_Line15) != RESET) //Контроль стояночного тормоза или датчика нейтрали
  {
    
    delay_ms(10);
    if(GPIO_ReadInputDataBit(ALARM1, BRAKE_CTRL)==0)
    {
     if(STATUS.SecurityStatus==ENABLE)
     {
      SIREN_and_LIGHTS(3);  // Тройное мигание фарами и звуковой сигнал
      GPIO_SetBits(ALARM2 , SIREN ); //Включение сирены
     // SendData_onServer(1400); //Отправка аварии на сервер
      GPIO_ResetBits(ALARM2 , SIREN ); //Выключение сирены
     } 
     
     if(STATUS.AUTOSTART==ENABLE) 
     {
      AUTOSTART(DISABLE); //Если двигатель запущен остановить
      delay_ms(100);    
      if(GPIO_ReadInputDataBit(ALARM1, IGN1_IN)==0) SIREN_and_LIGHTS(2); //Двойное мигание фарами 
     }
    }
    
    EXTI_ClearITPendingBit(EXTI_Line15); //Очистка флага прерывания  
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
