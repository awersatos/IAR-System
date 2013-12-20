/*******************************************************************************
********************************************************************************
**                                                                            **
**          БИБЛИОТЕКА ФУНКЦИЙ МОДУЛЯ АВТОСИГНАЛИЗАЦИИ                        **
**                                                                            **
********************************************************************************
*******************************************************************************/

//******************Подключаемые файлы******************************************
#include "Alarm.h"
#include "main.h"
#include "stm32f10x.h"
#include "Service.h"
//*************Инициализация глобальных переменных******************************

//*************Функции для работы с сигнализацией ******************************
void SECURITY(FunctionalState stat) //Функция снятия/постановки на охрану и закрывания открывания дверей
{ uint8_t state_ee[2]; //Статусная переменная из EEPROM
  
  if(stat==ENABLE) //Команда закрыть машину
    
  {  GPIO_SetBits(ALARM2 , DOOR_CL); //Закрытие дверей
     delay_ms(1000);
     GPIO_ResetBits(ALARM2 , DOOR_CL);
     
     IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
     delay_ms(10000);
     IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
     delay_ms(10000);
     IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
   
   
    /*Проверка что тригеры дверей, капота и багажника замкнуты и зажигание выключено*/
    if((GPIO_ReadInputDataBit(ALARM2,TRUNK_TR )==1) && (GPIO_ReadInputDataBit(ALARM1,IGN1_IN )==0) && (GPIO_ReadInputDataBit(ALARM2,DOOR_TR )==0) && (GPIO_ReadInputDataBit(ALARM2,HOOD_TR )==1))
    {
     
     
     if(STATUS.AUTOSTART==DISABLE) GPIO_SetBits(ALARM3 , M_LOCK); //Если не активен автозапуск заблокировать двигатель
     
     SIREN_and_LIGHTS(2); //Двойное мигание фарами и звуковой сигнал
     
     STATUS.SecurityStatus=ENABLE; //Установка статуса активной охраны
     
      READ_EE_DATA(EE_STATUS, state_ee); //Считываем статус EEPROM
      
      state_ee[1] |= SECUR; //Устанавливаем бит
      
      WRITE_EE_DATA(EE_STATUS, state_ee); //Записываем данные
      
      
    }
    else
    {
     GPIO_SetBits(ALARM2 , SIREN|LIGHTS );
     delay_ms(2000);
     GPIO_ResetBits(ALARM2 , SIREN|LIGHTS ); 
    }
  }
  else if(stat==DISABLE) //Команда открыть машину
  {
   GPIO_SetBits(ALARM2 , DOOR_OP); //Открытие дверей
   delay_ms(1000);
   GPIO_ResetBits(ALARM2 , DOOR_OP); 
   
   GPIO_ResetBits(ALARM3 , M_LOCK); //Разблокировать двигатель
   
   SIREN_and_LIGHTS(1); //Мигание фарами и звуковой сигнал
   
   STATUS.SecurityStatus=DISABLE; //Установка статуса "Снято с охраны" 
   
   READ_EE_DATA(EE_STATUS, state_ee); //Считываем статус EEPROM
   
   state_ee[1] &= ~SECUR; //Сбрасываем бит
   
   WRITE_EE_DATA(EE_STATUS, state_ee); //Записываем данные
   
  }
 IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера 
}

void AUTOSTART(FunctionalState status) //Дистанционный запуск/остановка двигателя
{
  uint8_t cnt1; //Счетчики
  uint8_t cnt2; 
  uint8_t start_time=100; //Время работы стартера
  if(status==ENABLE) //Запуск двигателя
  { 
   if(STATUS.SecurityStatus == DISABLE) SECURITY(ENABLE);
    if(GPIO_ReadInputDataBit(ALARM1,IGN1_IN )==0 ) //Проверка стояночного тормоза и включения зажигания
    {
      
     STATUS.AUTOSTART=ENABLE; //Статус автозапуска активен
     GPIO_ResetBits(ALARM3 , M_LOCK); //Разблокировать двигатель
     GPIO_SetBits(ALARM1 , IGN1_OUT|IGN2|ACC); //Включение цепей зажигания
     
     IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
     delay_ms(4000);
     
     for(cnt1=0;cnt1<4;cnt1++) //Счетчик попыток
     {
       if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==0) GPIO_SetBits(ALARM1 , ST_OUT); //Запуск стартера
       
       for(cnt2=0;cnt2<start_time ; cnt2++) //Интервал работы стартера
       {
        delay_ms(10);
        if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) break; //Если мотор запустился выход из подцикла
       }
       GPIO_ResetBits(ALARM1 , ST_OUT);//Остановка стартера
       GPIO_ResetBits(ALARM1 , ST_OUT);//Остановка стартера
       if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) break; //Если мотор запустился выход из цикла
       start_time=start_time+40 ; //Увеличение времени работы стартера
       IWDG_ReloadCounter(); //Сброс счетчика сторожевого таймера
       delay_ms(4000);
     }
     if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) GPIO_SetBits(ALARM1 , ACC); //Если двигатель запущен включить ACC
     else //Если попытки запуска завершились неудачей
     {
      GPIO_ResetBits(ALARM1 , IGN1_OUT|IGN2|ACC); //Отключение цепей зажигания
      STATUS.AUTOSTART=DISABLE; //Статус автозапуска сброшен
      if(STATUS.SecurityStatus==ENABLE) GPIO_SetBits(ALARM3 , M_LOCK);//Если активен режим охраны заблокировать двигатель
      
      SIREN_and_LIGHTS(2); //Двойное мигание фарами и звуковой сигнал
     }
    }
    else //Если условия автозапуска не соблюдены
    {
     SIREN_and_LIGHTS(2); //Двойное мигание фарами и звуковой сигнал
    }
  }
  
  else if(status==DISABLE) //Остановка двигателя
  {
   GPIO_ResetBits(ALARM1 , IGN1_OUT|IGN2|ACC); //Отключение цепей зажигания и ACC
   STATUS.AUTOSTART=DISABLE; //Статус автозапуска сброшен
   if(STATUS.SecurityStatus==ENABLE) GPIO_SetBits(ALARM3 , M_LOCK);//Если активен режим охраны заблокировать двигатель
  }
  
}

void SIREN_and_LIGHTS(uint8_t flash) //Управление миганием фар и сиреной
{
  uint8_t sirencnt;
  
  switch(flash)
  {
  case 1:
    {
      GPIO_SetBits(ALARM2 , SIREN|LIGHTS ); // Мигание фарами и звуковой сигнал
      delay_ms(100);
      GPIO_ResetBits(ALARM2 , SIREN|LIGHTS );
      break; }
      
  case 2:
    {
      GPIO_SetBits(ALARM2 , SIREN|LIGHTS ); //Двойное мигание фарами и звуковой сигнал
      delay_ms(100);
      GPIO_ResetBits(ALARM2 , SIREN|LIGHTS );
      delay_ms(150);
      GPIO_SetBits(ALARM2 , SIREN|LIGHTS );
      delay_ms(100);
      GPIO_ResetBits(ALARM2 , SIREN|LIGHTS );  
      break; }
      
  case 3:
    {
    for(sirencnt=0;sirencnt<3;sirencnt++)
    {
     GPIO_SetBits(ALARM2 , SIREN|LIGHTS ); // Мигание фарами и звуковой сигнал
     delay_ms(100);
     GPIO_ResetBits(ALARM2 , SIREN|LIGHTS ); 
     delay_ms(150);
    }
     break; }
    
  case LONG_SIREN: //Длинная звучание сирены
    {
     STATUS.LONG_ALARM = ENABLE; //Включение режима длительной тревоги
     GPIO_SetBits(ALARM2 , SIREN ); //Включение сирены
     TIM_Cmd(TIM7, ENABLE); //Запуск таймера
     break; 
    }
  }
 
 
}
//==============================================================================

void AUTOHEAT(void) //Функция автопрогрева двигателя
{
  uint32_t time;
  
  if((STATUS.AUTOHEAT_MODE != MODE_AUTOHEAT_OFF) && (STATUS.AUTOSTART == DISABLE)) AUTOSTART(ENABLE); //Запуск двигателя
  
  time = RTC_GetCounter(); //Считывание времени
  
  switch(STATUS.AUTOHEAT_MODE)
  {
  case MODE_AUTOHEAT_OFF:
    if(STATUS.AUTOSTART == ENABLE) AUTOSTART(DISABLE);
     break;
     
  case MODE_AUTOHEAT_0530:
    RTC_SetAlarm(time+900);
    RTC_WaitForLastTask(); //Ожидание окончания записи в регистры
    break;
    
  case MODE_AUTOHEAT_1030:
    RTC_SetAlarm(time+900);
    RTC_WaitForLastTask(); //Ожидание окончания записи в регистры
    break;  
    
  case MODE_AUTOHEAT_3030:
    RTC_SetAlarm(time+1200);
    RTC_WaitForLastTask(); //Ожидание окончания записи в регистры
    break; 
    
  case MODE_AUTOHEAT_3020:
    RTC_SetAlarm(time+1200);
    RTC_WaitForLastTask(); //Ожидание окончания записи в регистры
    break;  
    
  }
  
  
}



/***************************КОНЕЦ ФАЙЛА****************************************/