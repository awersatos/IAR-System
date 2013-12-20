/*******************************************************************************
********************************************************************************
**                                                                            **
**          ���������� ������� ������ ����������������                        **
**                                                                            **
********************************************************************************
*******************************************************************************/

//******************������������ �����******************************************
#include "Alarm.h"
#include "main.h"
#include "stm32f10x.h"
#include "Service.h"
//*************������������� ���������� ����������******************************

//*************������� ��� ������ � ������������� ******************************
void SECURITY(FunctionalState stat) //������� ������/���������� �� ������ � ���������� ���������� ������
{ uint8_t state_ee[2]; //��������� ���������� �� EEPROM
  
  if(stat==ENABLE) //������� ������� ������
    
  {  GPIO_SetBits(ALARM2 , DOOR_CL); //�������� ������
     delay_ms(1000);
     GPIO_ResetBits(ALARM2 , DOOR_CL);
     
     IWDG_ReloadCounter(); //����� �������� ����������� �������
     delay_ms(10000);
     IWDG_ReloadCounter(); //����� �������� ����������� �������
     delay_ms(10000);
     IWDG_ReloadCounter(); //����� �������� ����������� �������
   
   
    /*�������� ��� ������� ������, ������ � ��������� �������� � ��������� ���������*/
    if((GPIO_ReadInputDataBit(ALARM2,TRUNK_TR )==1) && (GPIO_ReadInputDataBit(ALARM1,IGN1_IN )==0) && (GPIO_ReadInputDataBit(ALARM2,DOOR_TR )==0) && (GPIO_ReadInputDataBit(ALARM2,HOOD_TR )==1))
    {
     
     
     if(STATUS.AUTOSTART==DISABLE) GPIO_SetBits(ALARM3 , M_LOCK); //���� �� ������� ���������� ������������� ���������
     
     SIREN_and_LIGHTS(2); //������� ������� ������ � �������� ������
     
     STATUS.SecurityStatus=ENABLE; //��������� ������� �������� ������
     
      READ_EE_DATA(EE_STATUS, state_ee); //��������� ������ EEPROM
      
      state_ee[1] |= SECUR; //������������� ���
      
      WRITE_EE_DATA(EE_STATUS, state_ee); //���������� ������
      
      
    }
    else
    {
     GPIO_SetBits(ALARM2 , SIREN|LIGHTS );
     delay_ms(2000);
     GPIO_ResetBits(ALARM2 , SIREN|LIGHTS ); 
    }
  }
  else if(stat==DISABLE) //������� ������� ������
  {
   GPIO_SetBits(ALARM2 , DOOR_OP); //�������� ������
   delay_ms(1000);
   GPIO_ResetBits(ALARM2 , DOOR_OP); 
   
   GPIO_ResetBits(ALARM3 , M_LOCK); //�������������� ���������
   
   SIREN_and_LIGHTS(1); //������� ������ � �������� ������
   
   STATUS.SecurityStatus=DISABLE; //��������� ������� "����� � ������" 
   
   READ_EE_DATA(EE_STATUS, state_ee); //��������� ������ EEPROM
   
   state_ee[1] &= ~SECUR; //���������� ���
   
   WRITE_EE_DATA(EE_STATUS, state_ee); //���������� ������
   
  }
 IWDG_ReloadCounter(); //����� �������� ����������� ������� 
}

void AUTOSTART(FunctionalState status) //������������� ������/��������� ���������
{
  uint8_t cnt1; //��������
  uint8_t cnt2; 
  uint8_t start_time=100; //����� ������ ��������
  if(status==ENABLE) //������ ���������
  { 
   if(STATUS.SecurityStatus == DISABLE) SECURITY(ENABLE);
    if(GPIO_ReadInputDataBit(ALARM1,IGN1_IN )==0 ) //�������� ����������� ������� � ��������� ���������
    {
      
     STATUS.AUTOSTART=ENABLE; //������ ����������� �������
     GPIO_ResetBits(ALARM3 , M_LOCK); //�������������� ���������
     GPIO_SetBits(ALARM1 , IGN1_OUT|IGN2|ACC); //��������� ����� ���������
     
     IWDG_ReloadCounter(); //����� �������� ����������� �������
     delay_ms(4000);
     
     for(cnt1=0;cnt1<4;cnt1++) //������� �������
     {
       if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==0) GPIO_SetBits(ALARM1 , ST_OUT); //������ ��������
       
       for(cnt2=0;cnt2<start_time ; cnt2++) //�������� ������ ��������
       {
        delay_ms(10);
        if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) break; //���� ����� ���������� ����� �� ��������
       }
       GPIO_ResetBits(ALARM1 , ST_OUT);//��������� ��������
       GPIO_ResetBits(ALARM1 , ST_OUT);//��������� ��������
       if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) break; //���� ����� ���������� ����� �� �����
       start_time=start_time+40 ; //���������� ������� ������ ��������
       IWDG_ReloadCounter(); //����� �������� ����������� �������
       delay_ms(4000);
     }
     if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) GPIO_SetBits(ALARM1 , ACC); //���� ��������� ������� �������� ACC
     else //���� ������� ������� ����������� ��������
     {
      GPIO_ResetBits(ALARM1 , IGN1_OUT|IGN2|ACC); //���������� ����� ���������
      STATUS.AUTOSTART=DISABLE; //������ ����������� �������
      if(STATUS.SecurityStatus==ENABLE) GPIO_SetBits(ALARM3 , M_LOCK);//���� ������� ����� ������ ������������� ���������
      
      SIREN_and_LIGHTS(2); //������� ������� ������ � �������� ������
     }
    }
    else //���� ������� ����������� �� ���������
    {
     SIREN_and_LIGHTS(2); //������� ������� ������ � �������� ������
    }
  }
  
  else if(status==DISABLE) //��������� ���������
  {
   GPIO_ResetBits(ALARM1 , IGN1_OUT|IGN2|ACC); //���������� ����� ��������� � ACC
   STATUS.AUTOSTART=DISABLE; //������ ����������� �������
   if(STATUS.SecurityStatus==ENABLE) GPIO_SetBits(ALARM3 , M_LOCK);//���� ������� ����� ������ ������������� ���������
  }
  
}

void SIREN_and_LIGHTS(uint8_t flash) //���������� �������� ��� � �������
{
  uint8_t sirencnt;
  
  switch(flash)
  {
  case 1:
    {
      GPIO_SetBits(ALARM2 , SIREN|LIGHTS ); // ������� ������ � �������� ������
      delay_ms(100);
      GPIO_ResetBits(ALARM2 , SIREN|LIGHTS );
      break; }
      
  case 2:
    {
      GPIO_SetBits(ALARM2 , SIREN|LIGHTS ); //������� ������� ������ � �������� ������
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
     GPIO_SetBits(ALARM2 , SIREN|LIGHTS ); // ������� ������ � �������� ������
     delay_ms(100);
     GPIO_ResetBits(ALARM2 , SIREN|LIGHTS ); 
     delay_ms(150);
    }
     break; }
    
  case LONG_SIREN: //������� �������� ������
    {
     STATUS.LONG_ALARM = ENABLE; //��������� ������ ���������� �������
     GPIO_SetBits(ALARM2 , SIREN ); //��������� ������
     TIM_Cmd(TIM7, ENABLE); //������ �������
     break; 
    }
  }
 
 
}
//==============================================================================

void AUTOHEAT(void) //������� ������������ ���������
{
  uint32_t time;
  
  if((STATUS.AUTOHEAT_MODE != MODE_AUTOHEAT_OFF) && (STATUS.AUTOSTART == DISABLE)) AUTOSTART(ENABLE); //������ ���������
  
  time = RTC_GetCounter(); //���������� �������
  
  switch(STATUS.AUTOHEAT_MODE)
  {
  case MODE_AUTOHEAT_OFF:
    if(STATUS.AUTOSTART == ENABLE) AUTOSTART(DISABLE);
     break;
     
  case MODE_AUTOHEAT_0530:
    RTC_SetAlarm(time+900);
    RTC_WaitForLastTask(); //�������� ��������� ������ � ��������
    break;
    
  case MODE_AUTOHEAT_1030:
    RTC_SetAlarm(time+900);
    RTC_WaitForLastTask(); //�������� ��������� ������ � ��������
    break;  
    
  case MODE_AUTOHEAT_3030:
    RTC_SetAlarm(time+1200);
    RTC_WaitForLastTask(); //�������� ��������� ������ � ��������
    break; 
    
  case MODE_AUTOHEAT_3020:
    RTC_SetAlarm(time+1200);
    RTC_WaitForLastTask(); //�������� ��������� ������ � ��������
    break;  
    
  }
  
  
}



/***************************����� �����****************************************/