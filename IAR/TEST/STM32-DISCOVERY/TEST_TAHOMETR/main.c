/*******************************************************************************
********************************************************************************
**                                                                            **
**                  ОСНОВНОЙ ФАЙЛ ПРОЕКТА Test_tahometr                            **
**                                                                            **
********************************************************************************
*******************************************************************************/

//****************Подключаемые файлы********************************************
#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "main.h"
#include "STM32vldiscovery.h"

//************* Инициализация глобальных переменных ****************************
const uint16_t Motor_run = 400;
const uint16_t Motor_start = 2000;
const uint16_t half_period = 7200;

uint16_t Prescaler;

FunctionalState Motor_Status = DISABLE;
//==============================================================================

void main(void) //Основная функция программы
{
  Prescaler = Motor_start;

  /*ИНИЦИАЛИЗАЦИЯ ПЕРЕФЕРИЙНЫХ УСТРОЙСТВ КОНТРОЛЛЕРА*/
RCC_Configuration(); //Включение тактироания и настройка перриферийных устройств
GPIO_Configuration(); //Инициализация портов ввода вывода
TIMER_Configuration(); //Инициализация таймера 

  STM32vldiscovery_LEDInit(LED3);
  STM32vldiscovery_LEDInit(LED4);
  STM32vldiscovery_PBInit(BUTTON_USER, BUTTON_MODE_EXTI); 

  
  while(1) //Основной цикл программы
  {
   
      if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)
      {
       if(Motor_Status == DISABLE) TIM_Cmd(TIM3, ENABLE);
        STM32vldiscovery_LEDOn(LED4);
      }
      else
      {
      if(Motor_Status == DISABLE) TIM_Cmd(TIM3, DISABLE);
      STM32vldiscovery_LEDOff(LED4);
      }
    
  }
  
}

  /*КОНЕЦ ОСНОВНОЙ ФУНКЦИИ ПРОГРАММЫ*/
void RCC_Configuration(void) //Включение тактироания и настройка перриферийных устройств
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  
}
//=================================================================================
void GPIO_Configuration(void) //Инициализация портов ввода вывода
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOA, &GPIO_InitStructure);

  
}
//===============================================================================
void TIMER_Configuration(void) //Инициализация таймера
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  
  TIM_TimeBaseStructure.TIM_Period = half_period *2;
  TIM_TimeBaseStructure.TIM_Prescaler = Prescaler;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = half_period ;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM3, &TIM_OCInitStructure);

  //TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
  
 // TIM_ARRPreloadConfig(TIM3, ENABLE);
  
  

}

/*******************************************************************************
********************************************************************************
**                                                                            **
**                        КОНЕЦ ПРОГРАММЫ                                     **
**                                                                            **
********************************************************************************
*******************************************************************************/
