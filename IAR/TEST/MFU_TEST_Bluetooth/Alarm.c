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
//*************������������� ���������� ����������******************************

//*************������� ��� ������ � ������������� ******************************
void SECURITY(FunctionalState stat) //������� ������/���������� �� ������ � ���������� ���������� ������
{
  if(stat==ENABLE) //������� ������� ������
  {  /*�������� ��� ������� ������, ������ � ��������� �������� � ��������� ���������*/
    if((GPIO_ReadInputDataBit(ALARM2,TRUNK_TR )&&GPIO_ReadInputDataBit(ALARM2,DOOR_TR )&&GPIO_ReadInputDataBit(ALARM2,HOOD_TR )==1) && GPIO_ReadInputDataBit(ALARM1,IGN1_IN )==0)
    {
     GPIO_SetBits(ALARM2 , DOOR_CL); //�������� ������
     delay_ms(1000);
     GPIO_ResetBits(ALARM2 , DOOR_CL);
     
     if(STATUS.AUTOSTART==DISABLE) GPIO_ResetBits(ALARM3 , M_LOCK); //���� �� ������� ���������� ������������� ���������
     
     SIREN_and_LIGHTS(2); //������� ������� ������ � �������� ������
     
     STATUS.SecurityStatus=ENABLE; //��������� ������� �������� ������
     
     
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
   
   GPIO_SetBits(ALARM3 , M_LOCK); //�������������� ���������
   
   SIREN_and_LIGHTS(1); //������� ������ � �������� ������
   
   STATUS.SecurityStatus=DISABLE; //��������� ������� "����� � ������" 
  }
  
}

void AUTOSTART(FunctionalState status) //������������� ������/��������� ���������
{
  uint8_t cnt1; //��������
  uint8_t cnt2; 
  uint8_t start_time=20; //����� ������ ��������
  if(status==ENABLE) //������ ���������
  {
    if((GPIO_ReadInputDataBit(ALARM1,BRAKE_CTRL )==1) && (GPIO_ReadInputDataBit(ALARM1,IGN1_IN )==0 )) //�������� ����������� ������� � ��������� ���������
    {
     STATUS.AUTOSTART=ENABLE; //������ ����������� �������
     GPIO_SetBits(ALARM3 , M_LOCK); //�������������� ���������
     GPIO_SetBits(ALARM1 , IGN1_OUT|IGN2); //��������� ����� ���������
     delay_ms(4000);
     
     for(cnt1=0;cnt1<4;cnt1++) //������� �������
     {
       GPIO_SetBits(ALARM1 , ST_OUT); //������ ��������
       for(cnt2=0;cnt2<start_time ; cnt2++) //�������� ������ ��������
       {
        delay_ms(100);
        if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) break; //���� ����� ���������� ����� �� ��������
       }
       GPIO_ResetBits(ALARM1 , ST_OUT);//��������� ��������
       if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) break; //���� ����� ���������� ����� �� �����
       start_time=start_time+2; //���������� ������� ������ ��������
       delay_ms(4000);
     }
     if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) GPIO_SetBits(ALARM1 , ACC); //���� ��������� ������� �������� ACC
     else //���� ������� ������� ����������� ��������
     {
      GPIO_ResetBits(ALARM1 , IGN1_OUT|IGN2); //���������� ����� ���������
      STATUS.AUTOSTART=DISABLE; //������ ����������� �������
      if(STATUS.SecurityStatus==ENABLE) GPIO_ResetBits(ALARM3 , M_LOCK);//���� ������� ����� ������ ������������� ���������
      
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
   if(STATUS.SecurityStatus==ENABLE) GPIO_ResetBits(ALARM3 , M_LOCK);//���� ������� ����� ������ ������������� ���������
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
      delay_ms(500);
      GPIO_ResetBits(ALARM2 , SIREN|LIGHTS );
      break; }
      
  case 2:
    {
      GPIO_SetBits(ALARM2 , SIREN|LIGHTS ); //������� ������� ������ � �������� ������
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
     GPIO_SetBits(ALARM2 , SIREN|LIGHTS ); // ������� ������ � �������� ������
     delay_ms(500);
     GPIO_ResetBits(ALARM2 , SIREN|LIGHTS ); 
     delay_ms(500);
    }
     break; }
  }
  
}





/***************************����� �����****************************************/