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
const char PHONE_NUMBER[] = "+79619918106";
const char CLIENT_PHONE_NUMBER[] = "+79134725888"; //����� �������

                                  /*+79139207097*/ //�������
const char SERVER[]= "\"bb1.avtoblackbox.com\""; //������
const char PORT[]= ",80\r\n"; //����
char IMEI[] = "123456789012345"; //������ ��� IMEI

//uint8_t call_cont = 0;

//***************��������� ���������� �������***********************************
static FunctionalState START_TCP_IP(void); //������� ������� ����� TCP/IP
//*****************������� ��� ������ � GSM  ***********************************


void GSM_Configuration(void) //������������� GSM
{
 FunctionalState stack = DISABLE; 
 uint8_t nr_sim=1;
 uint8_t x; //�������
 char *im; //��������� ���������� ��� IMEI
 IWDG_ReloadCounter(); //����� �������� ����������� �������
 do {
  Reset_rxDMA_ClearBufer(GSM); //����� ������ 

  do{
  if(GPIO_ReadInputDataBit(GSM_MOD,GSM_READY)==0)
 {
  delay_ms(200); 
 GPIO_ResetBits(GSM_MOD , GSM_ON); //�������� �����
 delay_ms(800);
  SIM(nr_sim); //����� ��� �����
 
 GPIO_SetBits(GSM_MOD , GSM_ON);
 }
 
 else
 {
  GPIO_ResetBits(GSM_MOD , GSM_RESET); //�������������  �����
   SIM(nr_sim); //����� ��� �����
 delay_ms(100);
 GPIO_SetBits(GSM_MOD , GSM_RESET); //������������� ��� ������
   
 }
  delay_ms(3000);
  }while(GPIO_ReadInputDataBit(GSM_MOD,GSM_READY)==0); //�������� ���������� ������
 IWDG_ReloadCounter(); 
  delay_ms(1000);
  
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
  
  IWDG_ReloadCounter(); //����� �������� ����������� �������
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
        
   for(x=0;x<3;x++) //������ TCP/IP �����
  {
   stack =  START_TCP_IP();  
   if(stack == ENABLE) break;
   delay_ms(3000);
   IWDG_ReloadCounter(); //����� �������� ����������� �������
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
   IWDG_ReloadCounter(); //����� �������� ����������� �������
 }while((STATUS.GSM_Status==INACTIVE) && (stack == DISABLE));  
 
 SendString_InUnit("AT+CMGF=1\r\n" , GSM); //������������ � ����� ��������� ���������
 delay_ms(100);
 
Reset_rxDMA_ClearBufer(GSM); //����� ������

}

//===============================================================================

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
//===============================================================================
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
//===============================================================================  
void SendData_onServer(uint16_t state, uint8_t rmc_buf)  //������� �������� ������ �� ������

{
 
  FunctionalState execute = DISABLE; //������ ������ ������� � ��������
  uint8_t cntr; //�������
  char out[]="ffff"; 


  // ���� ������� �� 1 ��� ����� ��� ��� ����������� � ���� �� ���������� ������ � ����� ��������� ����
  if((STATUS.SIM_Card!=1) || (REG_NET()!='R')) GSM_Configuration();  
  Reset_rxDMA_ClearBufer(GSM); //����� ������
  
    SendString_InUnit("AT+WIPCREATE=2,1," , GSM); //�������� ������ 
    SendString_InUnit(SERVER , GSM);
    SendString_InUnit(PORT , GSM);
    
     for (cntr=0;cntr<250;cntr++)
    {
      if(strstr(GSM_RxBuffer , "+WIPREADY: 2,1") !=NULL) //���� ����� �������� 
      {
       SendString_InUnit("AT+WIPDATA=2,1,1\r\n" , GSM); //�������� � ����� �������� ������
       delay_ms(100);
       if(strstr(GSM_RxBuffer , "CONNECT") !=NULL) //���� ����� ����������� 
       {  
         STATUS.GSM_DataMode = ENABLE;
        Reset_rxDMA_ClearBufer(GSM); //����� ������
        
/***************************�������� ���������� �� ������*************************************/
        
        SendString_InUnit("POST /bb/status HTTP/1.1\r\n" , GSM); //��������� POST �������
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
        
        SendString_InUnit("\r\n" , GSM); //����� ������
                    /*********/
        
        SendString_InUnit("utc_time=" , GSM); //�������� �������
        SendString_InUnit(&(Timestamp[rmc_buf][0]) , GSM);
        
        if(STATUS.CoordinatesStatus == 'A')
        { SendString_InUnit("&lat=" , GSM); //�������� ������
        SendString_InUnit(&(Latitude[rmc_buf][0]) , GSM); }
        
        if(STATUS.CoordinatesStatus == 'A')
        {SendString_InUnit("&lon=" , GSM); //�������� �������
        SendString_InUnit(&(Longitude[rmc_buf][0]) , GSM);}
        
        SendString_InUnit("&imei=" , GSM); //�������� IMEI
        SendString_InUnit(IMEI , GSM);
        
        SendString_InUnit("&command=" , GSM); //�������� �������
        sprintf(out , "%04d", state);
        SendString_InUnit(out ,GSM); 
        
        SendString_InUnit("&s_n=" , GSM); //�������� ��������� ������
        SendString_InUnit(SERIAL_NUMBER , GSM);
        
        SendString_InUnit("&phone=" , GSM); //�������� �����������  ������
        SendString_InUnit(PHONE_NUMBER , GSM);
        
        if(STATUS.CoordinatesStatus == 'A')
        {SendString_InUnit("&speed=" , GSM); //�������� ��������  
        SendString_InUnit(&(Speed[rmc_buf][0]) , GSM);}
        
        if(STATUS.CoordinatesStatus == 'A')
        {SendString_InUnit("&date=" , GSM); //�������� ����  
        SendString_InUnit(&(Date[rmc_buf][0]) , GSM);}
        
        SendString_InUnit("\r\n" , GSM); //����� ������
        SendString_InUnit("\r\n" , GSM); //����� ������
/*************************�������� ������ �� �������***************************/       
         for (cntr=0;cntr<13;cntr++)
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
            if(strstr(GSM_RxBuffer , "*15#") !=NULL) STATUS.COMMAND=15;  //������������ ����� ������������ 5/30
            if(strstr(GSM_RxBuffer , "*16#") !=NULL) STATUS.COMMAND=16;  ////������������ ����� ������������ 10/30
            if(strstr(GSM_RxBuffer , "*17#") !=NULL) STATUS.COMMAND=17;  ////������������ ����� ������������ 30/30
            if(strstr(GSM_RxBuffer , "*18#") !=NULL) STATUS.COMMAND=18;  ////������������ ����� ������������ 30/20
            if(strstr(GSM_RxBuffer , "*19#") !=NULL) STATUS.COMMAND=19;  //��������� ����� ������������
            if(strstr(GSM_RxBuffer , "*20#") !=NULL) STATUS.COMMAND=20;  //����� ������
           
            }
           execute = ENABLE; 
           break;
          }
          delay_ms(1000);
          IWDG_ReloadCounter(); //����� �������� ����������� �������
         }
 /***********************�������� ������****************************************/
        delay_ms(100);   
        Reset_rxDMA_ClearBufer(GSM); //����� ������
        
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
      IWDG_ReloadCounter(); //����� �������� ����������� �������
    }
    
 Reset_rxDMA_ClearBufer(GSM); //����� ������ 
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
 /****************������ ������ � ��������� �����******************************/
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
 
 //������� ���������� ������
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

void ANSWER_CALL(void) //������� ������ �� �������� ������
{
if(strstr(GSM_RxBuffer , "RING") !=NULL)
{
if((strstr(GSM_RxBuffer , CLIENT_PHONE_NUMBER) !=NULL)/* &&  (call_cont == 0)*/ )
{
 SendString_InUnit("ATH\r\n" , GSM); 
 Reset_rxDMA_ClearBufer(GSM); //����� ������
 delay_ms(500);
 IWDG_ReloadCounter(); //����� �������� ����������� �������
 
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
Reset_rxDMA_ClearBufer(GSM); //����� ������
IWDG_ReloadCounter(); //����� �������� ����������� �������
}

void RECEIVE_SMS(void) //������� ��������� ��� ���������
{
  char *start;
  char  *end; 
  char *data;
  uint16_t CODE;
  char index;
  char out[] = "AT+CMGD=x\r\n" ;
  
  
  Reset_rxDMA_ClearBufer(GSM); //����� ������
  
 SendString_InUnit("AT+CMGL=\"ALL\"\r\n" , GSM);  //������� ������ ���� ���������
 delay_ms(100);
 
 start = strstr(GSM_RxBuffer , "+CMGL: "); //����������� ������� ������ ���������
 
 if(start!=NULL)
 {
  start = start+7; 
  end = strstr(start , "\r\n");
  data = strstr(GSM_RxBuffer , CLIENT_PHONE_NUMBER); //����������� ��� ������� ��������� � ������������������� ������
  
  if(data>start && data<end)
  {
   end = end+5; //����������� ������ ���������� ����
   data = end-3;
   if((*data == '0' ) && (*end == '#'))
   {
   CODE = (uint16_t)data[2] | ((uint16_t)data[1]<<8); //��������� ���������� ����
   
   switch(CODE) //���������� ���������� ����
   {
   case 0x3130:
     STATUS.COMMAND = 10; //������� ������/����� � ������
     break;
     
   case 0x3131:
     STATUS.COMMAND = 11; //�������������/��������� �� ������
     break;
   
   case 0x3133:
     STATUS.COMMAND = 13; //��������� ���������
     break;  
     
   case 0x3134:
     STATUS.COMMAND = 14; //��������� ���������
     break;  
     
   case 0x3135:
     STATUS.COMMAND = 15; //������������ ����� ������������ 5/30
     break;
     
  case 0x3136:
     STATUS.COMMAND = 16; //������������ ����� ������������ 10/30 
     break; 
     
  case 0x3137:
     STATUS.COMMAND = 17; //������������ ����� ������������ 30/30 
     break;
     
  case 0x3138:
     STATUS.COMMAND = 18; //������������ ����� ������������ 30/20 
     break;
     
  case 0x3139:
     STATUS.COMMAND = 19; //��������� ����� ������������  
     break;   
     
  case 0x3230:
     STATUS.COMMAND = 20; //����� ������
     break;
   }
   
   }
  }
 index = *start; 
 //index = index-30; 
 sprintf(out , "AT+CMGD=%c\r\n", index); //�������� �������� ���������
 SendString_InUnit(out , GSM); 
 delay_ms(100); 
 SendString_InUnit("AT+CMGD=0,4\r\n" , GSM); 
 }
 
delay_ms(100);    
Reset_rxDMA_ClearBufer(GSM); //����� ������
IWDG_ReloadCounter(); //����� �������� ����������� ������� 
}

//===============================================================================

void SEND_SMS(Answer_TypeDef answer) //������� �������� ���
{
 uint8_t i, j; //������� 
 FunctionalState Send = DISABLE;
 char PDU_PHONE_NUMBER[] = "123456789012"; //������ ������ � PDU �������
 
 for(i=0; i<10; i+=2) PDU_PHONE_NUMBER[i] = CLIENT_PHONE_NUMBER[i+2]; //���������� ������� PDU
 PDU_PHONE_NUMBER[10] = 'F';
 for(i=1; i<12; i+=2) PDU_PHONE_NUMBER[i] = CLIENT_PHONE_NUMBER[i];
 
 SendString_InUnit("AT+CMGF=0\r\n" , GSM); //��������� PDU ������
 delay_ms(50);
 for(j=0;j<5;j++)
 {
 SendString_InUnit("AT+CMGS=" , GSM); //������� �������� ���������
 
 switch(answer) //���� ����� ���������
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
 Reset_rxDMA_ClearBufer(GSM); //����� ������
 
 SendString_InUnit("0001000B91" , GSM); //��������� PDU
 SendString_InUnit(PDU_PHONE_NUMBER , GSM); //����� � ������� PDU
 SendString_InUnit("0008" , GSM); //���������
 
 switch(answer) //���� ������ ���������
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
 SendString_InUnit("\x1A" , GSM); //������� �������� ���������
 
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
  Reset_rxDMA_ClearBufer(GSM); //����� ������
IWDG_ReloadCounter(); //����� �������� ����������� �������
if(Send == ENABLE) break;
delay_ms(2000);
 }
 
 SendString_InUnit("AT+CMGF=1\r\n" , GSM); //������������ � ����� ��������� ���������
  delay_ms(100);
  Reset_rxDMA_ClearBufer(GSM); //����� ������
}

//==================================================================================================
static FunctionalState START_TCP_IP(void) //������� ������� ����� TCP/IP
{
  FunctionalState br = DISABLE;
  IWDG_ReloadCounter(); //����� �������� ����������� �������
  
  delay_ms(3000);
SendString_InUnit("AT+WIPCFG=1\r\n" , GSM); //������ TCP/IP �����
  delay_ms(100);
   if(strstr(GSM_RxBuffer , "ERROR") !=NULL) //���� ���� �� ����������� ������������� ��� � ��������� �����
   {
     
    Reset_rxDMA_ClearBufer(GSM); //����� ������
    SendString_InUnit("AT+WIPCFG=0\r\n" , GSM); //��������� TCP/IP �����
    delay_ms(300);
    Reset_rxDMA_ClearBufer(GSM); //����� ������
    SendString_InUnit("AT+WIPCFG=1\r\n" , GSM); //������ TCP/IP �����     
    delay_ms(300);
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
      IWDG_ReloadCounter(); //����� �������� ����������� �������
      br = DISABLE;
    }
  }
  Reset_rxDMA_ClearBufer(GSM); //����� ������  
  
  return br;
  
}

 /***************************����� �����****************************************/ 