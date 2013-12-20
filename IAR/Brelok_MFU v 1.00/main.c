/*******************************************************************************
********************************************************************************
**                                                                            **
**                  �������� ���� ������� Brelok_MFU v 1.00                   **
**                                                                            **
********************************************************************************
*******************************************************************************/

//****************������������ �����********************************************
#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "main.h"
#include "Transceiver_433MHz.h"
#include "CRYPTO-AES.h"
#include <string.h>
//************* ������������� ���������� ���������� ****************************
FunctionalState delay_EnableStatus = DISABLE; //������ ���������� ��������� ��������
uint32_t delay_Counter; //�������� ��������� ��������
FunctionalState Transceiver_Status = DISABLE; //������ ���������� ����������
FunctionalState CRYPT = DISABLE; //������ ����������
uint16_t i,j;

const char SERIAL[] = "\015BR1234567890"; //�������� �����
/*
const char COMMAND_1[] = "\015BRELOK_TEST";
const char COMMAND_2[] = "\014TEST_COMAND";
const char COMMAND_3[] = "\013TEST_ALARM";
*/
 uint8_t KEY[16]=    //���� ����������
{  'B', 'R', 'E', 'L',
   'O', 'K', 'C', 'O',
   'N', 'T', 'R', 'O',
   'L', '_', 'O', 'K'
  
};

uint8_t BUFER[16]; //������ ����������


//******************************************************************************

void main(void) //�������� ������� ���������
{
 
  
       /*������������� ������������ ��������� �����������*/
RCC_Configuration(); //��������� ����������� � ��������� ������������� ���������
GPIO_Configuration(); //������������� ������ ����� ������
EXTI_Configuration(); //������������� ����������� ������� ����������
NVIC_Configuration();//������������� ����������
SPI_Configuration();//������������� SPI 
TIMER_Configuration(); //������������� �������
SysTick_Config(SystemCoreClock/1000); //������ ����� ���������� ������� 1 ��
NVIC_SetPriority(SysTick_IRQn,0); //������������������� ���������� ������� ���������
PWR_WakeUpPinCmd(ENABLE); //����������� ����� ��������

              /*������������� ������� �������*/
Transceiver_Configuration(); //������������� ����������
//*****************************************************************************

BUZZER(Long_Bip_1); 
//delay_ms(1000);

/*���������� ������ 1 ������� ������ /����� � ������*/
if(GPIO_ReadInputDataBit(GPIOA , BUTTON_1) == 1 && GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 0)
{
 LED(GREEN); 
 delay_ms(10);
  
 if(GPIO_ReadInputDataBit(GPIOA , BUTTON_1) == 1)
  {
   for(j=0;j<5;j++)
   {  
   for(i=0;i<SERIAL[0];i++) SPI_buffer[i] = SERIAL[i];
   SEND_PAKET();
   delay_ms(10);
   STROB(SRX);
   CRYPT = ENABLE; //���������� �������
   delay_ms(300);
   
   if(CRYPT==DISABLE)
   {
   SPI_buffer[0]=18;
   for(i=1;i<17;i++) SPI_buffer[i] = BUFER[i-1];
   SPI_buffer[17] = OPEN; //������� ����� � ������
   SEND_PAKET();
   delay_ms(10);
   STROB(SRX);
   }
   delay_ms(300);
   if(strstr(SPI_buffer , "EXECUTED") !=NULL) 
   {BUZZER(Medium_Bip_2);
    break;
   }
   }
   if(strstr(SPI_buffer , "EXECUTED") ==NULL) BUZZER(Long_Bip_1);
   

   while(GPIO_ReadInputDataBit(GPIOA , BUTTON_1) == 1); 
  }
}

/*���������� ������ 2 ������� ������/ ��������� �� ������*/
if(GPIO_ReadInputDataBit(GPIOA , BUTTON_1) == 0 && GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1)
{
 LED(RED); 
 delay_ms(10);
  
 if(GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1)
  {
    for(j=0;j<5;j++)
  { 
   for(i=0;i<SERIAL[0];i++) SPI_buffer[i] = SERIAL[i];
   SEND_PAKET();
   delay_ms(10);
   STROB(SRX);
   CRYPT = ENABLE; //���������� �������
   delay_ms(300);
   
   if(CRYPT==DISABLE)
   {
   SPI_buffer[0]=18;
   for(i=1;i<17;i++) SPI_buffer[i] = BUFER[i-1];
   SPI_buffer[17] = CLOSE; //������� ��������� �� ������
   SEND_PAKET();
   delay_ms(10);
   STROB(SRX);
   }
   delay_ms(300);
   if(strstr(SPI_buffer , "EXECUTED") !=NULL)
    {BUZZER(Medium_Bip_3);
    break;}
  }
   if(strstr(SPI_buffer , "EXECUTED") ==NULL) BUZZER(Long_Bip_1);
   

   while(GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1); 
  }
}

/*���������� ������� 2 ������ ������������ ������ �������*/
if(GPIO_ReadInputDataBit(GPIOA , BUTTON_1) == 1 && GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1)
{
 LED(ORANGE); 
 delay_ms(10000);
  
 if(GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1 && GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1)
  {
    for(j=0;j<5;j++)
   {   
    for(i=0;i<SERIAL[0];i++) SPI_buffer[i] = SERIAL[i];
   SEND_PAKET();
   delay_ms(10);
   STROB(SRX);
   CRYPT = ENABLE; //���������� �������
   delay_ms(300);
   
   if(CRYPT==DISABLE)
   {
   SPI_buffer[0]=18;
   for(i=1;i<17;i++) SPI_buffer[i] = BUFER[i-1];
   SPI_buffer[17] = ALARM; //������
   SEND_PAKET();
   delay_ms(10);
   STROB(SRX);
   }
   delay_ms(300);
   if(strstr(SPI_buffer , "EXECUTED") !=NULL) 
    { BUZZER(Short_Bip_15);
      break;
    }
   }
   if(strstr(SPI_buffer , "EXECUTED") ==NULL) BUZZER(Long_Bip_1);
   
   while(GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1 && GPIO_ReadInputDataBit(GPIOA , BUTTON_2) == 1); 
  }
}





 
 while(1) //�������� ���� ���������
 {
   /*
 LED(RED);
 delay_ms(1000);  
 LED(GREEN);
 
 */
 STROB(SIDLE);
 delay_ms(1000);
 LED(OFF);
 
 DBGMCU_Config(DBGMCU_STANDBY , ENABLE); //��������� ������� � ������ ��������
 
 STROB(SPWD); //������� ���������� � ������ ����� 
 delay_ms(100);
 PWR_EnterSTANDBYMode(); //������� ����������� � ����� ��������
 
 }
 
 
}

      /*����� �������� ������� ���������*/



void delay_ms(uint16_t msec) //�������� � ������������
{
  delay_Counter=msec; //����������� �������� ��������
  delay_EnableStatus = ENABLE; //���������� ��������
  while(delay_Counter); //���� ���� ������� �� ����� 0
  delay_EnableStatus = DISABLE; //������������ ��������
}  


void RCC_Configuration(void) //������������ � ��������� ������������ ���������
{
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE); //��������� ������������ ������ A,B
RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO , ENABLE); //��������� ������������ �������������� ������� GPIO
RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 , ENABLE); //��������� ������������ SPI1
RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17 , ENABLE);  //��������� ������������ ������� 17 
RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  //��������� ������������ ������ ������� 
}

void GPIO_Configuration(void) //��������� ���������� ������ ����� ������
{
 GPIO_InitTypeDef GPIO_Init_struct; // �������� ��������� ������������� ������

 //���� �0- ���� � ��������� � - (���� WKUP)
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_0;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �2-A3- ����� � ��������� � - (��������� ����� ���������� TR_GP1 , TR_GP2)
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_2 | GPIO_Pin_3;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �4-�����������  ����� SPI_SS
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_4;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �5-����������� �������������� ����� SPI_SCK
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_5;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �6- ���� 3-��������� SPI_MISO
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_6;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IN_FLOATING; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �7-����������� �������������� ����� SPI_MOSI
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_7;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �8-A9  ����� � ��������� � - (������ S1,S2)
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_8 | GPIO_Pin_9;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B0-B1 -�����������  ������ (���������)
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin= GPIO_Pin_0 | GPIO_Pin_1;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B8-B9 -�����������  ������ (������������)
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin= GPIO_Pin_8 | GPIO_Pin_9;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������
  
/* ����������� ����� ������� ����������*/
GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2); //����� ���������� TR_GP1

}

void EXTI_Configuration(void) //������������� ����������� ������� ����������
{
  EXTI_InitTypeDef   EXTI_InitStructure; //��������� ��������� ��� ������������� ������� ����������

  EXTI_InitStructure.EXTI_Line = EXTI_Line2; //����� ���������� 2 TR_GP1
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //����� ����������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //���������� �� �������������� ������
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //���������� �������
  EXTI_Init(&EXTI_InitStructure); //��������� ��������� � �������
}

void NVIC_Configuration(void) //������������� � ��������� ����������
{
NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//����������� ����� �����������:2 ���������
NVIC_InitTypeDef NVIC_InitStruct; //��������� ��������� ��������� ����������

NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn; //���������� ������� ����� 2 �� TR_GP1
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //������������ ���������
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; //���������
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //���������� ����������
NVIC_Init(&NVIC_InitStruct); //�������� ��������� � �������

NVIC_InitStruct.NVIC_IRQChannel = TIM1_TRG_COM_TIM17_IRQn; //���������� ������� ����� 2 �� ������� 17
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //������������ ���������
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1; //���������
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //���������� ����������
NVIC_Init(&NVIC_InitStruct); //�������� ��������� � �������




}

void SPI_Configuration(void) //������������� SPI
{ 
 SPI_InitTypeDef  SPI_InitStructure; //������������� ��������� ��� ��������� SPI 
 SPI_StructInit(&SPI_InitStructure); //������� ����� ���������
 
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //��������������� ����� ������ SPI
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //����� ������ ������
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //����������� ������ 8 ���
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; //���������� ��������� ������� �������������
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; //���� ������ �����
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; //����������� ���������� NSS
  SPI_InitStructure.SPI_BaudRatePrescaler =  SPI_BaudRatePrescaler_8 ; //������������ 8 �������� ������� 1 ���
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //������� ��� ���� ������
  SPI_InitStructure.SPI_CRCPolynomial = 8; //������� ��� �������� CRC
  
  SPI_Init(SPI1, &SPI_InitStructure); //�������� ��������� � �������

  
  
}

void TIMER_Configuration(void)   //������������� �������
{
uint16_t Period = 690;
TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure; //���������� ������� ��������� �������
TIM_OCInitTypeDef  TIM_OCInitStructure; //���������� ��������� ��������� ���������  

TIM_OCStructInit(&TIM_OCInitStructure); //������� ����� ���������
TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); //������� ����� ���������

TIM_TimeBaseStructure.TIM_Period = Period;   //������ �����       
TIM_TimeBaseStructure.TIM_Prescaler = 0;   //������������    
TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //�������� ��� ��������   
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //����� ������ �������

TIM_TimeBaseInit(TIM17, &TIM_TimeBaseStructure); //�������� ��������� � �������

TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;  //����� ������������
TIM_OCInitStructure.TIM_Pulse = (Period/2); //�������� �������� ��������
TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable; //�������� ����� ��������
TIM_OCInitStructure.TIM_OutputNState = TIM_OutputState_Disable; //��������������� ����� ��������

TIM_OC1Init(TIM17, &TIM_OCInitStructure); //�������� ��������� � �������

TIM_OC1PreloadConfig(TIM17, TIM_OCPreload_Disable); //������������ ������� ��������� ���������

TIM_ITConfig(TIM17, TIM_IT_CC1 | TIM_IT_Update , ENABLE);//���������� �� ������������ �������� � �� ���������� �������

 
}

void LED(Color_TypeDef COLOR) //������� ���������� �����������
{
  switch(COLOR)
  {
  case OFF: //��������� ��������
    {
      GPIO_ResetBits(GPIOB , red);
      GPIO_ResetBits(GPIOB , green);
      break;
    }
    
  case GREEN: //�������
    {
      GPIO_ResetBits(GPIOB , red);
      GPIO_SetBits(GPIOB , green);
      break;
    } 
    
  case RED:  //�������
    {
      GPIO_ResetBits(GPIOB , green);
      GPIO_SetBits(GPIOB , red);
      break;
    }
    
  case ORANGE: //���������
    {
      GPIO_SetBits(GPIOB , green);
      GPIO_SetBits(GPIOB , red);
      break;
    }  
  }
  
}

void BUZZER(Bip_TypeDef BIP) //������� ���������� ��������������
{ 
  uint8_t buz_cnt; //�������
  
  switch(BIP)
  {
  case Long_Bip_1: //��������� ������� ������
    {
     TIM_Cmd(TIM17, ENABLE); //������ �������
     delay_ms(1000); 
     TIM_Cmd(TIM17, DISABLE); //��������� �������
     break;  
    }
    
  case Medium_Bip_2: //2 ������� �������
    {
     for(buz_cnt=0;buz_cnt<2;buz_cnt++)
     {
     TIM_Cmd(TIM17, ENABLE); //������ �������
     delay_ms(300); 
     TIM_Cmd(TIM17, DISABLE); //��������� �������
     delay_ms(200);
     }
     break;  
    }
    
  case Medium_Bip_3:  //3 ������� �������
    {
      for(buz_cnt=0;buz_cnt<3;buz_cnt++)
     {
     TIM_Cmd(TIM17, ENABLE); //������ �������
     delay_ms(300); 
     TIM_Cmd(TIM17, DISABLE); //��������� �������
     delay_ms(200);
     }
     break; 
    }
    
  case Short_Bip_15: //15 �������� ��������
    {
      
      for(buz_cnt=0;buz_cnt<15;buz_cnt++)
     {
     TIM_Cmd(TIM17, ENABLE); //������ �������
     delay_ms(200); 
     TIM_Cmd(TIM17, DISABLE); //��������� �������
     delay_ms(150);
     }
     break; 
    }
    
  }
    
  
  GPIO_ResetBits(GPIOB , BUZZER_PIN_1); //����� � 0 ����� ������� ��������
  GPIO_ResetBits(GPIOB , BUZZER_PIN_1);
}



/*******************************************************************************
********************************************************************************
**                                                                            **
**                        ����� ���������                                     **
**                                                                            **
********************************************************************************
*******************************************************************************/