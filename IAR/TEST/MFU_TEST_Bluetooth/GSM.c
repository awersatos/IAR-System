/*******************************************************************************
********************************************************************************
**                                                                            **
**          ���������� ������� ������ GSM WISMO 228                           **
**                                                                            **
********************************************************************************
*******************************************************************************/

//******************������������ �����******************************************
#include "GSM.h"
#include "main.h"
#include "navigation.h"
#include <string.h>
#include <stdio.h>
#include "Alarm.h"
//*************������������� ���������� ����������******************************
const char PHONE_NUMBER[] = "+79619918106,";
const char SERVER[]= "\"bb.avtoblackbox.com\""; //������
const char PORT[]= ",31272\r\n"; //����
char IMEI[] = "123456789012345"; //������ ��� IMEI

//*****************������� ��� ������ � GSM  ***********************************
void GSM_Configuration(void) //������������� GSM
{
 uint8_t nr_sim=1;
 uint8_t x; //�������
 char *im; //��������� ���������� ��� IMEI
 do {
  Reset_rxDMA_ClearBufer(GSM); //����� ������ 

 
if(GPIO_ReadInputDataBit(GSM_MOD,GSM_READY)==0)
{
 GPIO_ResetBits(GSM_MOD , GSM_ON); //�������� �����
  SIM(nr_sim); //����� ��� �����
 delay_ms(1000);
 GPIO_SetBits(GSM_MOD , GSM_ON);
}
 else
 {
  GPIO_ResetBits(GSM_MOD , GSM_RESET); //�������������  �����
   SIM(nr_sim); //����� ��� �����
 delay_ms(100);
 GPIO_SetBits(GSM_MOD , GSM_RESET); 
   
 }
  while(GPIO_ReadInputDataBit(GSM_MOD,GSM_READY)==0); //�������� ���������� ������
  delay_ms(2000);
  
  SendString_InUnit("AT+GSN\r\n" , GSM);  //������ IMEI  
  delay_ms(100);
  im=strstr(GSM_RxBuffer , "\r\n\r\n");
   if (im !=NULL)
   {
    im=im+4;
    for(x=0;x<15;x++) IMEI[x]=*im++; //����������� IMEI � ����������
    Reset_rxDMA_ClearBufer(GSM); //����� ������
   }
  
  SendString_InUnit("AT+CLIP=1\r\n" , GSM); //��������� ������������ ������
  delay_ms(100);
  Reset_rxDMA_ClearBufer(GSM); //����� ������
  
  SendString_InUnit("AT+CPIN?\r\n" , GSM);
  delay_ms(500);
  if (strstr(GSM_RxBuffer , "READY") !=NULL) //���� ��� ����� ��������
   {
     Reset_rxDMA_ClearBufer(GSM); //����� ������
     for(x=0;x<10;x++) //�������� ����������� � ����
     {
      if(REG_NET()=='R') //���� ����������� �������
      {
        STATUS.GSM_Status=ACTIVE;
        STATUS.SIM_Card=nr_sim;
        SendString_InUnit("AT+COPS?\r\n" , GSM); //����������� ��������� ������� �����
        delay_ms(100);
        if (strstr(GSM_RxBuffer , "Beeline") !=NULL) STATUS.OPERATOR=Beeline_OP;
        if (strstr(GSM_RxBuffer , "MTS") !=NULL) STATUS.OPERATOR=MTS_OP;
        if (strstr(GSM_RxBuffer , "MegaFon") !=NULL) STATUS.OPERATOR=Megafon_OP;
        Reset_rxDMA_ClearBufer(GSM); //����� ������
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
 }while(STATUS.GSM_Status==INACTIVE);  
Reset_rxDMA_ClearBufer(GSM); //����� ������
}



void SIM(uint8_t sim) //������� ������������ ��� ����
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

char REG_NET(void)  //�������� ���������� � ����
{ char *d;
  Reset_rxDMA_ClearBufer(GSM); //����� ������
  SendString_InUnit("AT+CREG?\r\n" , GSM);
  delay_ms(100);
  d=strstr(GSM_RxBuffer , "+CREG:");
   if(d!=NULL)
   {
     d=d+9;
     if((*d=='1')|(*d=='5')) 
     { Reset_rxDMA_ClearBufer(GSM); //����� ������
       return 'R';}    
   }
    Reset_rxDMA_ClearBufer(GSM); //����� ������
   return NULL; 
}
  
void SendData_onServer(uint16_t state)  //������� �������� ������ �� ������

{
 // char *comand;
  FunctionalState bearer = DISABLE; //������ GPRS �������
  uint8_t cntr; //�������
  char out[]="ffff"; 

  GPIO_SetBits(ALARM2 , LIGHTS);
  
  // ���� ������� �� 1 ��� ����� ��� ��� ����������� � ���� �� ���������� ������ � ����� ��������� ����
  if((STATUS.SIM_Card!=1) | (REG_NET()!='R')) GSM_Configuration();
  
  Reset_rxDMA_ClearBufer(GSM); //����� ������
  SendString_InUnit("AT+WIPCFG=1\r\n" , GSM); //������ TCP/IP �����
  delay_ms(100);
   if(strstr(GSM_RxBuffer , "ERROR") !=NULL) //���� ���� �� ����������� ������������� ��� � ��������� �����
   {
    Reset_rxDMA_ClearBufer(GSM); //����� ������
    SendString_InUnit("AT+WIPCFG=0\r\n" , GSM); //��������� TCP/IP �����
    delay_ms(100);
    Reset_rxDMA_ClearBufer(GSM); //����� ������
    SendString_InUnit("AT+WIPCFG=1\r\n" , GSM); //������ TCP/IP ����� 
    delay_ms(100);
   }
  
  if(strstr(GSM_RxBuffer , "OK") !=NULL) //���� ���� ����������
  {
   Reset_rxDMA_ClearBufer(GSM); //����� ������ 
   SendString_InUnit("AT+WIPBR=1,6\r\n" , GSM);  //�������� GPRS �������
   delay_ms(100);
   
   switch(STATUS.OPERATOR) //��������� ���������� GPRS ������� � ����������� �� ���������
   {
   case Beeline_OP:
     {
      SendString_InUnit("AT+WIPBR=2,6,11,\"internet.beeline.ru\"\r\n" , GSM); //��������� APN
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,0,\"beeline\"\r\n" , GSM); //���� ������ 
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,1,\"beeline\"\r\n" , GSM); //���� ������
      delay_ms(100);
      break;
     }
     
   case MTS_OP:
     {
      SendString_InUnit("AT+WIPBR=2,6,11,\"internet.mts.ru\"\r\n" , GSM); //��������� APN
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,0,\"mts\"\r\n" , GSM); //���� ������ 
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,1,\"mts\"\r\n" , GSM); //���� ������
      delay_ms(100);
      break;
     }  
    case Megafon_OP:
     {
      SendString_InUnit("AT+WIPBR=2,6,11,\"internet\"\r\n" , GSM); //��������� APN
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,0,\"\"\r\n" , GSM); //���� ������ 
      delay_ms(100);
      SendString_InUnit("AT+WIPBR=2,6,1,\"\"\r\n" , GSM); //���� ������
      delay_ms(100);
      break;
     }   
   }
    Reset_rxDMA_ClearBufer(GSM); //����� ������
    SendString_InUnit("AT+WIPBR=4,6,0\r\n" , GSM);
    for (cntr=0;cntr<50;cntr++)
    {
      if(strstr(GSM_RxBuffer , "OK") !=NULL)
      {
       bearer=ENABLE;
       break;
      }
      if(strstr(GSM_RxBuffer , "ERROR") !=NULL)
      {
       bearer=DISABLE;
       break;
      }  
      delay_ms(1000);
      bearer=DISABLE;
    }
  }
  Reset_rxDMA_ClearBufer(GSM); //����� ������
  
  if(bearer==ENABLE) //���� GPRS ������ ���������� �������
  {
    SendString_InUnit("AT+WIPCREATE=2,1," , GSM); //�������� ������ 
    SendString_InUnit(SERVER , GSM);
    SendString_InUnit(PORT , GSM);
    
     for (cntr=0;cntr<20;cntr++)
    {
      if(strstr(GSM_RxBuffer , "+WIPREADY: 2,1") !=NULL) //���� ����� �������� 
      {
       SendString_InUnit("AT+WIPDATA=2,1,1\r\n" , GSM); //�������� � ����� �������� ������
       delay_ms(100);
       if(strstr(GSM_RxBuffer , "CONNECT") !=NULL) //���� ����� ����������� 
       {  
        Reset_rxDMA_ClearBufer(GSM); //����� ������
          /*�������� ���������� �� ������*/
        SendString_InUnit(SERIAL_NUMBER , GSM); //�������� �����
        SendString_InUnit(PHONE_NUMBER , GSM); //����� ��������
        SendString_InUnit(RMC , GSM); //��������� RMC
        
        sprintf(out , ",%04d", state);
        SendString_InUnit(out ,GSM); 
        
        SendString_InUnit(",imei:", GSM); //��������� imei
        SendString_InUnit(IMEI , GSM); //IMEI ������ 
        SendString_InUnit(",101\\x8D\r\n", GSM);//����� ���������
        delay_ms(3000);
         for (cntr=0;cntr<10;cntr++)
         {
           
          if(strstr(GSM_RxBuffer , "OK*") !=NULL)
          {
            if(strstr(GSM_RxBuffer , "*00#") !=NULL) STATUS.COMMAND=00;
            else
            {
            if(strstr(GSM_RxBuffer , "*10#") !=NULL) STATUS.COMMAND=10;  //������� ������ ����� � ������
            if(strstr(GSM_RxBuffer , "*11#") !=NULL) STATUS.COMMAND=11;  //������� ������/��������� �� ������
            if(strstr(GSM_RxBuffer , "*13#") !=NULL) STATUS.COMMAND=13;  //��������� ��������� 
            if(strstr(GSM_RxBuffer , "*14#") !=NULL) STATUS.COMMAND=14;  //��������� ���������
            if(strstr(GSM_RxBuffer , "*15#") !=NULL) STATUS.COMMAND=15;  //������������ ����� ������������
            if(strstr(GSM_RxBuffer , "*16#") !=NULL) STATUS.COMMAND=16;  //��������� ����� ������������
            if(strstr(GSM_RxBuffer , "*20#") !=NULL) STATUS.COMMAND=20;  //����� ������
           
            }
           break;
          }
          delay_ms(1000);
         }
           
        Reset_rxDMA_ClearBufer(GSM); //����� ������
        SendString_InUnit("+++", GSM);
        delay_ms(100);
        if(strstr(GSM_RxBuffer , "OK") !=NULL)
        {SendString_InUnit("AT+WIPCLOSE=2,1\r\n", GSM);
        delay_ms(100);
        SendString_InUnit("AT+WIPCFG=0\r\n" , GSM);
        bearer=DISABLE;
        delay_ms(100);
        
        }
       }
       break;
      }
      if(strstr(GSM_RxBuffer , "ERROR") !=NULL)
      {
 
       break;
      }  
      delay_ms(1000);
      
    }
    
    
    
  }
 Reset_rxDMA_ClearBufer(GSM); //����� ������ 
 //state++; 
  GPIO_ResetBits(ALARM2 , LIGHTS);
 
}


void ANSWER_CALL(void) //������� ������ �� �������� ������
{
if(strstr(GSM_RxBuffer , "RING") !=NULL)

{
SendString_InUnit("ATA\r\n" , GSM); 
//while(1);
  
}


}





 /***************************����� �����****************************************/ 