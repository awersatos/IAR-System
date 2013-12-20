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
//*************Инициализация глобальных переменных******************************

//*************Функции для работы с сигнализацией ******************************
void SECURITY(FunctionalState stat) //Функция снятия/постановки на охрану и закрывания открывания дверей
{
  if(stat==ENABLE) //Команда закрыть машину
  {  /*Проверка что тригеры дверей, капота и багажника замкнуты и зажигание выключено*/
    if((GPIO_ReadInputDataBit(ALARM2,TRUNK_TR )&&GPIO_ReadInputDataBit(ALARM2,DOOR_TR )&&GPIO_ReadInputDataBit(ALARM2,HOOD_TR )==1) && GPIO_ReadInputDataBit(ALARM1,IGN1_IN )==0)
    {
     GPIO_SetBits(ALARM2 , DOOR_CL); //Закрытие дверей
     delay_ms(1000);
     GPIO_ResetBits(ALARM2 , DOOR_CL);
     
     if(STATUS.AUTOSTART==DISABLE) GPIO_ResetBits(ALARM3 , M_LOCK); //Если не активен автозапуск заблокировать двигатель
     
     SIREN_and_LIGHTS(2); //Двойное мигание фарами и звуковой сигнал
     
     STATUS.SecurityStatus=ENABLE; //Установка статуса активной охраны
     
     
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
   
   GPIO_SetBits(ALARM3 , M_LOCK); //Разблокировать двигатель
   
   SIREN_and_LIGHTS(1); //Мигание фарами и звуковой сигнал
   
   STATUS.SecurityStatus=DISABLE; //Установка статуса "Снято с охраны" 
  }
  
}

void AUTOSTART(FunctionalState status) //Дистанционный запуск/остановка двигателя
{
  uint8_t cnt1; //Счетчики
  uint8_t cnt2; 
  uint8_t start_time=20; //Время работы стартера
  if(status==ENABLE) //Запуск двигателя
  {
    if((GPIO_ReadInputDataBit(ALARM1,BRAKE_CTRL )==1) && (GPIO_ReadInputDataBit(ALARM1,IGN1_IN )==0 )) //Проверка стояночного тормоза и включения зажигания
    {
     STATUS.AUTOSTART=ENABLE; //Статус автозапуска активен
     GPIO_SetBits(ALARM3 , M_LOCK); //Разблокировать двигатель
     GPIO_SetBits(ALARM1 , IGN1_OUT|IGN2); //Включение цепей зажигания
     delay_ms(4000);
     
     for(cnt1=0;cnt1<4;cnt1++) //Счетчик попыток
     {
       GPIO_SetBits(ALARM1 , ST_OUT); //Запуск стартера
       for(cnt2=0;cnt2<start_time ; cnt2++) //Интервал работы стартера
       {
        delay_ms(100);
        if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) break; //Если мотор запустился выход из подцикла
       }
       GPIO_ResetBits(ALARM1 , ST_OUT);//Остановка стартера
       if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) break; //Если мотор запустился выход из цикла
       start_time=start_time+2; //Увеличение времени работы стартера
       delay_ms(4000);
     }
     if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) GPIO_SetBits(ALARM1 , ACC); //Если двигатель запущен включить ACC
     else //Если попытки запуска завершились неудачей
     {
      GPIO_ResetBits(ALARM1 , IGN1_OUT|IGN2); //Отключение цепей зажигания
      STATUS.AUTOSTART=DISABLE; //Статус автозапуска сброшен
      if(STATUS.SecurityStatus==ENABLE) GPIO_ResetBits(ALARM3 , M_LOCK);//Если активен режим охраны заблокировать двигатель
      
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
   if(STATUS.SecurityStatus==ENABLE) GPIO_ResetBits(ALARM3 , M_LOCK);//Если активен режим охраны заблокировать двигатель
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
      delay_ms(500);
      GPIO_ResetBits(ALARM2 , SIREN|LIGHTS );
      break; }
      
  case 2:
    {
      GPIO_SetBits(ALARM2 , SIREN|LIGHTS ); //Двойное мигание фарами и звуковой сигнал
      delay_ms(500);
      GPIO_ResetBits(ALARM2 , SIREN|LIGHTS );
      delay_ms(500);
      GPIO_SetBits(ALARM2 , SIREN|LIGHTS );
      delay_ms(500);
      GPIO_ResetBits(ALARM2 , SIREN|LIGHTS );  
      break; }
      
  case 3:
    {
    for(sirencnt=0;sirencnt<3;sirencnt++)
    {
     GPIO_SetBits(ALARM2 , SIREN|LIGHTS ); // Мигание фарами и звуковой сигнал
     delay_ms(500);
     GPIO_ResetBits(ALARM2 , SIREN|LIGHTS ); 
     delay_ms(500);
    }
     break; }
  }
  
}





/***************************КОНЕЦ ФАЙЛА****************************************/