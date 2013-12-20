/*******************************************************************************
********************************************************************************
**                                                                            **
**          БИБЛИОТЕКА ФУНКЦИЙ МОДУЛЯ МОДУЛЯ СЕРВИСНЫХ НАСТРОЕК               **
**                                                                            **
********************************************************************************
*******************************************************************************/

//******************Подключаемые файлы******************************************
#include "main.h"
#include "eeprom.h"
#include "Service.h"
#include "Alarm.h"

//*************Инициализация глобальных переменных******************************

//Массив виртуальных адресов EEPROM
const uint16_t VirtAddVarTab[NumbOfVar] = 
{ 0xAB00, 0xAB01, 0xAB02, 0xAB03, 0xAB04, 0xAB05,0xAB06, 0xAB07, 0xAB08, 0xAB09, 0xAB0A, 0xAB0B, 0xAB0C, 0xAB0D, 0xAB0E, 0xAB0F,
  
  0xAB10, 0xAB11, 0xAB12, 0xAB13, 0xAB14, 0xAB15,0xAB16, 0xAB17, 0xAB18, 0xAB19, 0xAB1A, 0xAB1B, 0xAB1C, 0xAB1D, 0xAB1E, 0xAB1F,
  
  0xAB20, 0xAB21, 0xAB22, 0xAB23, 0xAB24, 0xAB25,0xAB26, 0xAB27, 0xAB28, 0xAB29, 0xAB2A, 0xAB2B, 0xAB2C, 0xAB2D, 0xAB2E, 0xAB2F,
    
  0xAB30, 0xAB31, 0xAB32, 0xAB33, 0xAB34, 0xAB35,0xAB36, 0xAB37, 0xAB38, 0xAB39, 0xAB3A, 0xAB3B, 0xAB3C, 0xAB3D, 0xAB3E, 0xAB3F
};

//***************Прототипы внутренних функций***********************************

//***************Функции сервисных настроек*************************************
void WRITE_EE_DATA(uint8_t virtual_var, uint8_t *data) //Функция записи данных в EEPROM
{
 uint8_t i; 
 uint8_t b1, b0; 
 uint16_t load;
  /*Если переменная в 1 байт*/
 if(/*(virtual_var == EE_STATUS) || */ (virtual_var == START_TIME) || (virtual_var == HEAT_TIME) || (virtual_var == OP_CL_TIME))  
 {
  b0=*data;
  load = (uint16_t)b0;
  EE_WriteVariable(VirtAddVarTab[virtual_var], load);
  return; 
 }
 else
 { 
   /*Если массив в 2байта*/
   if(virtual_var == EE_STATUS)
   {
   b0=*data++;  
   b1 = *data;  
   load = ((uint16_t)b1<<8) | (uint16_t)b0;  
   EE_WriteVariable(VirtAddVarTab[virtual_var], load);
   return;   
   }
   
   
   
   /*Если массив в 12 байт*/
   if((virtual_var == BRELOK_SN_1) || (virtual_var == BRELOK_SN_2) || (virtual_var == PHONE_NUMBER_1) || (virtual_var == PHONE_NUMBER_2) || (virtual_var == PHONE_NUMBER_3) || (virtual_var == CLIENT_PHONE_NUMBER_1) || (virtual_var == CLIENT_PHONE_NUMBER_2))
   {
     for(i=0;i<6;i++)
     {
       b0 = *data++;
       b1 = *data++;
       load = ((uint16_t)b1<<8) | (uint16_t)b0;
       EE_WriteVariable(VirtAddVarTab[virtual_var+i], load);
       
     }
     return;
   }
   /*Если массив в 20 байт*/
    if(virtual_var == SERVER_BB)
    {
      //data++;
      for(i=0;i<10;i++)
     {
       b0 = *data++;
       b1 = *data++;
       load = ((uint16_t)b1<<8) | (uint16_t)b0;
       EE_WriteVariable(VirtAddVarTab[virtual_var+i], load);
    }
    return;
   }
   /*Если массив в 5 байт*/
   if(virtual_var == PORT_BB)
    {
      //data++;
      for(i=0;i<3;i++)
     {
       b0 = *data++;
       b1 = *data++;
       load = ((uint16_t)b1<<8) | (uint16_t)b0;
       EE_WriteVariable(VirtAddVarTab[virtual_var+i], load);
       
     }
     return;
    }
   
 }
  
}

//==============================================================================

void READ_EE_DATA(uint8_t virtual_var, uint8_t *data) //Функция чтения данных из EEPROM
{
 uint8_t i;  
 uint16_t out;
 
 /*Если переменная в 1 байт*/
 if(/*(virtual_var == EE_STATUS) || */(virtual_var == START_TIME) || (virtual_var == HEAT_TIME)|| (virtual_var == OP_CL_TIME))  
 {
  EE_ReadVariable(VirtAddVarTab[virtual_var], &out);
  *data = (uint8_t)out;
  return;
 } 
 else
 { /*Если массив в 2байта*/ 
   if(virtual_var == EE_STATUS)
   {
     EE_ReadVariable(VirtAddVarTab[virtual_var], &out);
     *data++ = (uint8_t)out;
     *data = (uint8_t)(out>>8);
     return;
     
   }
   
   
   /*Если массив в 12 байт*/
  if((virtual_var == BRELOK_SN_1) || (virtual_var == BRELOK_SN_1) || (virtual_var == PHONE_NUMBER_1) || (virtual_var == PHONE_NUMBER_2) || (virtual_var == PHONE_NUMBER_3) || (virtual_var == CLIENT_PHONE_NUMBER_1) || (virtual_var == CLIENT_PHONE_NUMBER_2))
   {
     for(i=0;i<6;i++)
     {
      EE_ReadVariable(VirtAddVarTab[virtual_var+i], &out);
      *data++ = (uint8_t)out;
      *data++ = (uint8_t)(out>>8);
     }
     return;
   }
  /*Если массив в 20 байт*/
   if(virtual_var == SERVER_BB)
    {
      data++;
     for(i=0;i<10;i++)
     {
      EE_ReadVariable(VirtAddVarTab[virtual_var+i], &out);
      *data++ = (uint8_t)out;
      *data++ = (uint8_t)(out>>8);
     }
     return;
    }
  /*Если массив в 5 байт*/
     if(virtual_var == PORT_BB)
    {
       data++;
     for(i=0;i<3;i++)
     {
      EE_ReadVariable(VirtAddVarTab[virtual_var+i], &out);
      *data++ = (uint8_t)out;
      if(i!=2) *data++ = (uint8_t)(out>>8);
     }
     return; 
       
       
    }
 }
  
}
//=============================================================================
void Write_Default_Setting(void) //Запись дефолтных настроек в EEPROM
{
 uint8_t state[2];
 
 uint8_t start_time = 20;
 uint8_t heat_time = 10;
 uint8_t op_cl_time = 100;
 uint8_t ee_status[2] = { 0x80, 0x00 };  
   
   
 READ_EE_DATA(EE_STATUS, state); //Считываем статус EEPROM
 
 //state &=  DS;
 
 if((state[0] &= DS)==0) //Ели дефолтные настройки не были записаны записать их
 {
  WRITE_EE_DATA(BRELOK_SN_1, "BR0123456789"); 
  WRITE_EE_DATA(BRELOK_SN_2, "BR0123456789"); 
  WRITE_EE_DATA(PHONE_NUMBER_1, "+7xxxxxxxxxx");
  WRITE_EE_DATA(PHONE_NUMBER_2, "+7xxxxxxxxxx");
  WRITE_EE_DATA(PHONE_NUMBER_3, "+7xxxxxxxxxx");
  WRITE_EE_DATA(CLIENT_PHONE_NUMBER_1, "+7yyyyyyyyyy");
  WRITE_EE_DATA(CLIENT_PHONE_NUMBER_2, "+7yyyyyyyyyy");
  WRITE_EE_DATA(SERVER_BB, "bb1.avtoblackbox.com");
  WRITE_EE_DATA(PORT_BB, "31272");
  WRITE_EE_DATA(START_TIME,  &start_time);
  WRITE_EE_DATA(HEAT_TIME, &heat_time);
  WRITE_EE_DATA(OP_CL_TIME, &op_cl_time);
  
  WRITE_EE_DATA(EE_STATUS, ee_status);
 }
 else
 {
  if((state[1] &= SECUR)==1)  SECURITY(ENABLE);
  else STATUS.SecurityStatus = DISABLE;
   
   
 }
}
//==============================================================================
void SERVICE(void)  //Режим сервисных наастроек
{
 uint16_t i; //Счетчик

  for(i=0;i<240;i++) 
  {
  
    
    
  if(GPIO_ReadOutputDataBit(ALARM2, LIGHTS)==1) GPIO_ResetBits(ALARM2 , LIGHTS);
  else GPIO_SetBits(ALARM2 , LIGHTS);
  delay_ms(500);  
  }
  
}




/***************************КОНЕЦ ФАЙЛА****************************************/