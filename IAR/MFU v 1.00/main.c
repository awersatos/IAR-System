/*******************************************************************************
********************************************************************************
**                                                                            **
**                  �������� ���� ������� MFU v1.00                           **
**                                                                            **
********************************************************************************
*******************************************************************************/

//�������������  10,01,2014

//****************������������ �����********************************************
#include "stm32f10x_conf.h"
#include "eeprom.h"
#include "stm32f10x.h"
#include "main.h"
#include "navigation.h"
#include "Bluetooth.h"
#include "GSM.h"
#include "Alarm.h"
#include <string.h>
#include "Transceiver_433MHz.h"
#include "CRYPTO-AES.h"
#include "Service.h"
//************* ������������� ���������� ���������� ****************************




const char SERIAL_NUMBER[] = "1231231230" ; //�������� ����� ����������
const char BRELOK_SN[] = "BR1234567890" ; //�������� ����� ������

const uint8_t KEY[16]=     //���� ����������
{  'B', 'R', 'E', 'L',
   'O', 'K', 'C', 'O',
   'N', 'T', 'R', 'O',
   'L', '_', 'O', 'K'
  
};

uint8_t BUFER[16]; //������ ������ ����������
FunctionalState CRYPT = DISABLE; //������ ����������

StatusStruct_TypeDef STATUS; //����������� ���������� ������� ����������

FunctionalState delay_EnableStatus = DISABLE; //������ ���������� ��������� ��������
uint32_t delay_Counter; //�������� ��������� ��������

char Bluetooth_TxBuffer[TX_BufferSize+1]; //���������� Bluetooth ����� USART1
char Bluetooth_RxBuffer[RX_BufferSize+1]; //�������� Bluetooth ����� USART1
char GSM_TxBuffer[TX_BufferSize+1];       //���������� GSM ����� USART2
char GSM_RxBuffer[RX_BufferSize+1];       //�������� GSM ����� USART2
char Navi_TxBuffer[TX_BufferSize+1];      //���������� Navigation ����� USART3
char Navi_RxBuffer[RX_BufferSize+1];      //�������� Navigation ����� USART3

uint16_t sec_cnt=0;
 
//******************************************************************************

void main()  //�������� ������ ���������
{
          /*������������� ������������ ��������� �����������*/
//SystemInit(); //������������� �������� � ���������� �����������
FLASH_Unlock(); //������������� ����������� ���� ������
EE_Init(); //������������� ������������ EEPROM
RCC_Configuration(); //��������� ����������� � ��������� ������������� ���������
BKP_Configuration(); //������������ ���������� ������ ��������
RTC_Configuration(); //������������ ����� ��������� �������
GPIO_Configuration(); //������������� ������ ����� ������
EXTI_Configuration(); //������������� ����������� ������� ����������
NVIC_Configuration();//������������� ����������
ADC_Configuration();//������������� ���
TIMER_Configuration();//������������� ��������
DMA_Configuration(); //������������� ������� DMA
UART_Configuration();//������������� USART
SPI_Configuration();//������������� SPI
SysTick_Config(2*SystemCoreClock/1000); //������ ����� ���������� ������� 1 ��
NVIC_SetPriority(SysTick_IRQn,0); //������������������� ���������� ������� ���������



Write_Default_Setting();


while(STATUS.MainPower == DISABLE);

IWDG_Configuration();//������������� ����������� �������

         /*������������� ������� �������*/

Transceiver_Configuration(); //������������� ����������
NAVI_Configuration(); //������������� �������������� ���������
Bluetooth_Configuration(); //������������� Bluetooth
GSM_Configuration(); //������������� GSM




delay_ms(500);
SIREN_and_LIGHTS(3); //������� ������� ������ � �������� ������

 


SendString_InUnit("AT+CMGD=0,4\r\n" , GSM); 
delay_ms(500);
SendData_onServer(0,0);



/******************************************************************************/

 while(1) //�������� ���� ���������
 {
  
   
  IWDG_ReloadCounter(); //����� �������� ����������� �������
  delay_ms(1000); 


  
  Bluetooth_Read();
  SendString_InUnit("\r\nSTATUS:NORMAL\r\n" , Bluetooth);
  //ANSWER_CALL(); //������� ������ �� �������� ������
  RECEIVE_SMS(); //������� ��������� ��� ���������
  COMAND_EXEC();  //����������� ������
  
  if(STATUS.EVENT_BUF[0] !=0)
  {
   SendData_onServer(STATUS.EVENT_BUF[0], 0); 
   STATUS.EVENT_BUF[0] = STATUS.EVENT_BUF[1];
   STATUS.EVENT_BUF[1] = STATUS.EVENT_BUF[2]; 
   STATUS.EVENT_BUF[2] = 0; 
  }
  
   
   if(sec_cnt>300)
   {
    sec_cnt=0;
  Transceiver_Configuration(); //������������� ����������
    if(STATUS.MainPower==ENABLE) SendData_onServer(0,0);
    else SendData_onServer((1200+STATUS.BatteryCharge), 0);
         /*�������� ��������� �������*/
     if(Timestamp[1][0]  != 0) SendData_onServer(0,1);
     if(Timestamp[2][0]  != 0) SendData_onServer(0,2);
     if(Timestamp[3][0]  != 0) SendData_onServer(0,3);
     if(Timestamp[4][0]  != 0) SendData_onServer(0,4);
     if(Timestamp[5][0]  != 0) SendData_onServer(0,5);
   }
 }
 
}
//==============================================================================
             /*����� �������� ������� ���������*/
//==============================================================================
void RCC_Configuration(void)
{
 //��������� ������������ ������ A,B,C,D 
RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD), ENABLE); 
RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO , ENABLE); //��������� ������������ �������������� ������� GPIO
RCC_ADCCLKConfig(RCC_PCLK2_Div4); //��������� ������������ ��������� ������� ���
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_ADC2, ENABLE); //��������� ������������ ���1-2
RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM6 | RCC_APB1Periph_TIM7 , ENABLE ); //��������� ������������ �������� 3,6,7
RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 , ENABLE); //��������� ������������ USART1
RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB1Periph_USART3 , ENABLE ); //��������� ������������ USART2-3
RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 , ENABLE); //��������� ������������ SPI1
RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); //��������� ������������ DMA1
RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//��������� ������������ ������ ������� � ���������� ������
RCC_LSEConfig(RCC_LSE_OFF );
RCC_LSICmd(ENABLE); //��������� ����������� RC ����������
while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET); //�������� ���������� ����������� RC ����������
 

}  

void BKP_Configuration(void) //������������ ���������� ������ �������
{
PWR_BackupAccessCmd(ENABLE); //���������� ������ � �������� � ��������������� �������
BKP_DeInit();

BKP_TamperPinCmd(DISABLE); //��� ������� ��������

  
  
}

void RTC_Configuration(void) //������������ ����� ��������� �������
{
  
 RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI); //�������� ��������� ������� ���������� RC ���������
 RCC_RTCCLKCmd(ENABLE); //��������� ������������ �����

 RTC_WaitForSynchro(); //�������� �������������
 RTC_WaitForLastTask(); //�������� ��������� ������ � �������� 
 RTC_SetPrescaler(40000);//��������� ������������
 RTC_WaitForLastTask(); //�������� ��������� ������ � ��������
 
 RTC_ITConfig(RTC_IT_SEC, ENABLE); //���������� ������ �������
 RTC_WaitForLastTask(); //�������� ��������� ������ � ��������
 
 RTC_ITConfig(RTC_IT_ALR, ENABLE); //���������� �����
 RTC_WaitForLastTask(); //�������� ��������� ������ � ��������
}

void NVIC_Configuration(void)
{
NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����������� ����� �����������:2 ���������
NVIC_InitTypeDef NVIC_InitStruct; //��������� ��������� ��������� ����������

NVIC_InitStruct.NVIC_IRQChannel = ADC1_2_IRQn; //���������� �� ��� 1-2
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3; //������������ ���������
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1; // ���������
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //���������� ����������
NVIC_Init(&NVIC_InitStruct); //�������� ��������� � ������� 

NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel3_IRQn; //���������� DMA1 ����� 3 USART3 
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2; //������������ ���������
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1; //���������
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //���������� ����������
NVIC_Init(&NVIC_InitStruct); //�������� ��������� � �������

NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn; //���������� ������� ����� 3 �������� ���������� ������
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //������������ ���������
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1; //���������
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //���������� ����������
NVIC_Init(&NVIC_InitStruct); //�������� ��������� � �������


NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn; //���������� ������� ����� 5-9
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3; //������������ ���������
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2; //���������
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //���������� ����������
NVIC_Init(&NVIC_InitStruct); //�������� ��������� � �������

NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn; //���������� ������� ����� 10-15
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //������������ ���������
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2; //���������
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //���������� ����������
NVIC_Init(&NVIC_InitStruct); //�������� ��������� � �������

NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn; //���������� ������� ����� 0
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2; //������������ ���������
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3; //���������
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //���������� ����������
NVIC_Init(&NVIC_InitStruct); //�������� ��������� � �������

NVIC_InitStruct.NVIC_IRQChannel = TIM6_IRQn; //���������� ������� 6
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3; //������������ ���������
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3; //���������
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //���������� ����������
NVIC_Init(&NVIC_InitStruct); //�������� ��������� � �������

NVIC_InitStruct.NVIC_IRQChannel = TIM7_IRQn; //���������� ������� 7
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3; //������������ ���������
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2; //���������
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; //���������� ����������
NVIC_Init(&NVIC_InitStruct); //�������� ��������� � �������

NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_InitStruct);




}  


void GPIO_Configuration(void)
{
GPIO_InitTypeDef GPIO_Init_struct; // �������� ��������� ������������� ������

 //���� �0-���������� ���� ����� � �������������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_0;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AIN; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �1-���������� ���� ����� Y �������������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_1;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AIN; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �2-����������� �������������� ����� GSM UART2 TX
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_2;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �3- ���� 3-��������� GSM UART2 RX
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_3;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IN_FLOATING; //������������� ����� ������
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
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_50MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �6- ���� 3-��������� SPI_MISO
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_6;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_50MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IN_FLOATING; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �7-����������� �������������� ����� SPI_MOSI
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_7;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_50MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �8-�����������  ����� BLUETOOTH_RESET
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_8;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �9-����������� �������������� ����� BLUETOOTH UART1 TX
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_9;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_10MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �10- ���� 3-��������� BLUETOOTH UART1 RX
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_10;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IN_FLOATING; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �11- ���� 3-��������� TR_GP1 ��������� ���� ��� ����������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_11;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���� �12- ���� 3-��������� TR_GP2 ��������� ���� ��� ����������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_12;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������

//���������� ����������� ������ �0-3, A8-12
GPIO_PinLockConfig(GPIOA, (GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12));

//���� B0-���������� ���� �������� �������� ���������� �������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_0;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AIN; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B1-���������� ���� �������� ������ ��������� �������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_1;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AIN; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B2- ���� � ��������� � ����� �������� ��������� 1
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_2;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B5-�����������  ����� ��������� ��������� 1
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_5;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B6- ���� � ��������� � ����� �� ��������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_6;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B7-�����������  ����� ��������� ��������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_7;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B8-�����������  ����� ��������� ��������� 2
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_8;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B9-�����������  ����� ��������� ���� ACC
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_9;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B10-����������� �������������� ����� NAVIGATION UART3 TX
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_10;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B11- ���� 3-��������� NAVIGATION UART3 RX
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_11;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IN_FLOATING; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B12-�����������  ����� standby NAVIGATION
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_12;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B13-�����������  ����� reset NAVIGATION
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_13;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B14- ���� � ��������� � ����� �� �����������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_14;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPU; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���� B15- ���� � ��������� � ����� �� �������� ������� ������� ��� ��������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_15;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOB, &GPIO_Init_struct);      //�������� ��������� � �������

//���������� ����������� ������ B0-2,5-15
GPIO_PinLockConfig(GPIOB, (GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15));

//���� C0-�����������  ����� GSM_ON/OFF
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_0;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������
GPIO_SetBits(GPIOC, GPIO_Pin_0);          //������������� ���� � 1
//���� C1-�����������  ����� GSM_RESET
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_1;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������
GPIO_SetBits(GPIOC, GPIO_Pin_1);          //������������� ���� � 1
//���� C2- ���� � ��������� � ����� GSM_READY
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_2;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���� C3- ���� � ��������� � ����� GSM_RING
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_3;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���� C4-�����������  ����� �������� ������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_4;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���� C5-�����������  ����� �������� ������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_5;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���� C6-�����������  ����� ���������� ����
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_6;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���� C7-�����������  ����� ������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_7;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���� C8- ���� � ��������� � ����� ������ ����� ��������������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_8;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���� C9- ���� � ��������� � ����� ������ ����� �������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_9;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���� C10- ���� � ��������� � �����  ������ ���������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_10;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���� C11- ���� � ��������� � ����� ������ �����
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_11;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPD; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���� C12- ���� � ��������� � ����� ������ ������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_12;     //��������� ���
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_IPU; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���� C13-�����������  ����� ��������� SIM1
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_13;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���� C14-�����������  ����� ��������� SIM2
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_14;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���� C15-�����������  ����� ��������� SIM3
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_15;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOC, &GPIO_Init_struct);      //�������� ��������� � �������

//���������� ����������� ������ C0-15
GPIO_PinLockConfig(GPIOC, GPIO_Pin_All);

//���� D2-�����������  ����� ���������� ���������
GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
GPIO_Init_struct.GPIO_Pin=GPIO_Pin_2;     //��������� ���
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_2MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������
GPIO_Init(GPIOD, &GPIO_Init_struct);      //�������� ��������� � �������

//���������� ������������ ����� D2
GPIO_PinLockConfig(GPIOC, GPIO_Pin_2); 

/* ����������� ����� ������� ����������*/

GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource3); //����� ���������� GSM_RING
GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6); //����� ���������� ST_IN
GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource8); //����� ���������� SHOCK1
GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource9); //����� ���������� SHOCK2
GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource10); //����� ���������� TRUNK_TR
GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource11); //����� ���������� TR_GP1 ����� ������
GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource12); //����� ���������� HOOD_TR
GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14); //����� ���������� MOTOR_CTRL
GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15); //����� ���������� BRAKE_CTRL
}  

void EXTI_Configuration(void) //������������� ����������� ������� ����������
{
  EXTI_InitTypeDef   EXTI_InitStructure; //��������� ��������� ��� ������������� ������� ����������
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line0; //����� ���������� ������� �����
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //����� ����������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //���������� �� �������������� ������
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //���������� �������
  EXTI_Init(&EXTI_InitStructure); //��������� ��������� � �������
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line3; //����� ���������� ��������� ������
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //����� ����������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //���������� �� �������������� ������
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //���������� �������
  EXTI_Init(&EXTI_InitStructure); //��������� ��������� � �������

  EXTI_InitStructure.EXTI_Line = EXTI_Line6; //����� ���������� 6 ST_IN
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //����� ����������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //���������� �� �������������� ������
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //���������� �������
  EXTI_Init(&EXTI_InitStructure); //��������� ��������� � �������
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line8; //����� ���������� 8 SHOCK1
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //����� ����������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //���������� �� �������������� ������
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //���������� �������
  EXTI_Init(&EXTI_InitStructure); //��������� ��������� � ������� 

  EXTI_InitStructure.EXTI_Line = EXTI_Line9; //����� ���������� 9 SHOCK2
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //����� ����������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //���������� �� �������������� ������
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //���������� �������
  EXTI_Init(&EXTI_InitStructure); //��������� ��������� � ������� 
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line10; //����� ���������� 10 TRUNK_TR
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //����� ����������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //���������� �� �������������� ������
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //���������� �������
  EXTI_Init(&EXTI_InitStructure); //��������� ��������� � ������� 
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line11; //����� ���������� 11 TR_GP1 ����� ������
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //����� ����������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //���������� �� �������������� ������
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //���������� �������
  EXTI_Init(&EXTI_InitStructure); //��������� ��������� � ������� 
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line12; //����� ���������� 12 HOOD_TR
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //����� ����������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //���������� �� �������������� ������
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //���������� �������
  EXTI_Init(&EXTI_InitStructure); //��������� ��������� � ������� 
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line14; //����� ���������� 14 MOTOR_CTRL
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //����� ����������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //���������� �� �������������� ������
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //���������� �������
  EXTI_Init(&EXTI_InitStructure); //��������� ��������� � �������
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line15; //����� ���������� 15 BRAKE_CTRL
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //����� ����������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //���������� �� �������������� ������
  EXTI_InitStructure.EXTI_LineCmd = ENABLE; //���������� �������
  EXTI_Init(&EXTI_InitStructure); //��������� ��������� � ������� 
 
}


void ADC_Configuration(void) //������������� ���
{                     /*���1*/
ADC_InitTypeDef  ADC_InitStructure;//��������� ��������� ��� ��������� ���

ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //����������� ����� ������ ���
ADC_InitStructure.ADC_ScanConvMode = ENABLE; // ������������ ������� ��������
ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; //����������� ����� ������ �������
ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //������� ������ ��������
ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //������ ������������ ������
ADC_InitStructure.ADC_NbrOfChannel = 2; //����������� ������� 2
ADC_Init(ADC1, &ADC_InitStructure); //�������� ��������� � �������

ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5); //��������� ������0
ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_55Cycles5); //��������� ������1
 
ADC_AnalogWatchdogThresholdsConfig(ADC1, 0x090F, 0x07C7); //��������� ������� �������� ������������� +-5g
ADC_AnalogWatchdogCmd(ADC1,  ADC_AnalogWatchdog_AllRegEnable);//�������� ���� ������� ���������� ������
ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE); //��������� ���������� �������� �����������

ADC_Cmd(ADC1, ENABLE); //��������� ���1
ADC_ResetCalibration(ADC1);//����� ���������� ���1
while(ADC_GetResetCalibrationStatus(ADC1));//�������� ��������� ������ ���������� ���
ADC_StartCalibration(ADC1);//������ ���������� ���1
while(ADC_GetCalibrationStatus(ADC1));//�������� ��������� ����������
ADC_SoftwareStartConvCmd(ADC1, ENABLE);//���������� ������ �������������� ���1

                    /*���2*/

ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //����������� ����� ������ ���
ADC_InitStructure.ADC_ScanConvMode = DISABLE; // ������������ ������� ���������
ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //����������� ����� ������ ��������
ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO; //������� ������ �� ������ ������� 3
ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //������ ������������ ������
ADC_InitStructure.ADC_NbrOfChannel = 1; //����������� ������� 2
ADC_Init(ADC2, &ADC_InitStructure); //�������� ��������� � �������

ADC_RegularChannelConfig(ADC2, ADC_Channel_8, 1, ADC_SampleTime_55Cycles5); //��������� ������ 8 ���������� ������

ADC_InjectedSequencerLengthConfig(ADC1, 1); //����������� ������� �������������� ������ 1
ADC_InjectedChannelConfig(ADC2, ADC_Channel_9, 1, ADC_SampleTime_55Cycles5); //��������� ������ 9 �������������� ������
ADC_ExternalTrigInjectedConvConfig(ADC2, ADC_ExternalTrigInjecConv_None); //������� ���������� �������� ��������������� ������ ���������
ADC_AutoInjectedConvCmd(ADC2, ENABLE); //��������� ��������������� ������� ��������������� ������

ADC_ExternalTrigConvCmd(ADC2, ENABLE); //���������� �������� ���������� �������� ���2
ADC_ITConfig(ADC2, ADC_IT_JEOC, ENABLE); //���������� ���������� �� ��������� �������������� ������ 

ADC_Cmd(ADC2, ENABLE); //��������� ���2
ADC_ResetCalibration(ADC2);//����� ���������� ���2
while(ADC_GetResetCalibrationStatus(ADC2));//�������� ��������� ������ ���������� ���
ADC_StartCalibration(ADC2);//������ ���������� ���2
while(ADC_GetCalibrationStatus(ADC2));//�������� ��������� ����������
}

void TIMER_Configuration(void)   //������������� ��������
{
TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure; //���������� ������� ��������� �������
TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); //������� ����� ���������

TIM_TimeBaseStructure.TIM_Period = 0xFFFF;   //������ �����       
TIM_TimeBaseStructure.TIM_Prescaler = 0xFF;   //������������    
TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //�������� ��� ��������   
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //����� ������ �������
TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //�������� ��������� � �������

TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);//�������� ������ �������� �� ������������

TIM_Cmd(TIM3, ENABLE); //������ ������� 

TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); //������� ����� ���������

TIM_TimeBaseStructure.TIM_Prescaler = 5000;   //������������  
TIM_TimeBaseStructure.TIM_Period = 11200 ;  //������ �����  
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //����� ������ �������
TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure); //�������� ��������� � �������

TIM_ITConfig(TIM6 , TIM_IT_Update , ENABLE); //������ �� ���������� �� ������������

TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); //������� ����� ���������

TIM_TimeBaseStructure.TIM_Prescaler = 36000;   //������������  
TIM_TimeBaseStructure.TIM_Period = 36000 ;  //������ �����  
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //����� ������ �������
TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //�������� ��������� � �������

TIM_ITConfig(TIM7 , TIM_IT_Update , ENABLE); //������ �� ���������� �� ������������

}  


void UART_Configuration(void)    //������������� USART
{
 USART_InitTypeDef USART_InitStructure; //���������� ��������� ������������
 USART_StructInit(&USART_InitStructure); //������� ����� ���������
                   /*USART1*/
  USART_InitStructure.USART_BaudRate = 921600;  //�������� 921600 �/�
  USART_InitStructure.USART_WordLength = USART_WordLength_8b; //���� ������ 8
  USART_InitStructure.USART_StopBits = USART_StopBits_1;  //�������� 1
  USART_InitStructure.USART_Parity = USART_Parity_No;     //�������� ���
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //����������� ������� ���
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //���������� � �������� ��������
  
  USART_Init(USART1, &USART_InitStructure); //�������� ��������� � �������
  USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE); //��������� ������ DMA �� ����� � ��������
  USART_Cmd(USART1, ENABLE); //������������ USART1
  
                 /*USART2*/
  USART_InitStructure.USART_BaudRate = 115200;  //�������� 115200 �/�
  
  USART_Init(USART2, &USART_InitStructure); //�������� ��������� � �������
  USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE); //��������� ������ DMA �� ����� � ��������
  USART_Cmd(USART2, ENABLE); //������������ USART2
  
                 /*USART3*/
  USART_Init(USART3, &USART_InitStructure); //�������� ��������� � �������
  USART_DMACmd(USART3, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE); //��������� ������ DMA �� ����� � ��������
  USART_Cmd(USART3, ENABLE); //������������ USART3 
  
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
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; //������������ 8, �������� ������� SPI 7 ���
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //������� ��� ���� ������
  SPI_InitStructure.SPI_CRCPolynomial = 8; //������� ��� �������� CRC
  
  SPI_Init(SPI1, &SPI_InitStructure); //�������� ��������� � �������

  
  
}


void DMA_Configuration(void) //������������� ������� DMA
{
  DMA_InitTypeDef DMA_InitStructure; //������������� ��������� DMA
  DMA_StructInit(&DMA_InitStructure); //������� ����� ���������
  
  /*USART1_TX-����� 4*/
  DMA_DeInit(DMA1_Channel4);  //����� �������� ������ 4
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804; //������� ����� �������� ������ USART1
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Bluetooth_TxBuffer; //���������� ����� USART1
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //����������� ������ �� ������ � ���������
  DMA_InitStructure.DMA_BufferSize = TX_BufferSize; //������ ������
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //��������� ������������� �������� ��������
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //��������� ������ �������� 
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //������ ������ � ��������� 1 ����
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //������ ������ � ������ 1 ����
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //���������� ����� ������ DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low; //���������  ������
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //�������� �� ������ � ������ ���������
  
  DMA_Init(DMA1_Channel4 , &DMA_InitStructure); //�������� ��������� � �������
 // DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE); //��������� ���������� �� ��������� ��������
  
  /*USART1_RX-�����5*/
  DMA_DeInit(DMA1_Channel5);  //����� �������� ������ 5
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804; //������� ����� �������� ������ USART1
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Bluetooth_RxBuffer; //�������� ����� USART1
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //����������� ������ �� ��������� � ������
  DMA_InitStructure.DMA_BufferSize = RX_BufferSize; //������ ������
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //����������� ����� ������ DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; //���������  �������
  
  DMA_Init(DMA1_Channel5 , &DMA_InitStructure); //�������� ��������� � �������
  
  /*USART2_TX-����� 7*/
  DMA_DeInit(DMA1_Channel7);  //����� �������� ������ 7
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404; //������� ����� �������� ������ USART2
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)GSM_TxBuffer; //���������� ����� USART2
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //����������� ������ �� ������ � ���������
  DMA_InitStructure.DMA_BufferSize = TX_BufferSize; //������ ������
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //���������� ����� ������ DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low; //���������  ������
  
  DMA_Init(DMA1_Channel7 , &DMA_InitStructure); //�������� ��������� � �������
 // DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE); //��������� ���������� �� ��������� ��������
  
  /*USART2_RX-����� 6*/ 
  DMA_DeInit(DMA1_Channel6);  //����� �������� ������ 6
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404; //������� ����� �������� ������ USART2
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)GSM_RxBuffer; //�������� ����� USART2
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //����������� ������ �� ��������� � ������
  DMA_InitStructure.DMA_BufferSize = RX_BufferSize; //������ ������
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //����������� ����� ������ DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; //���������  �������
  
  DMA_Init(DMA1_Channel6 , &DMA_InitStructure); //�������� ��������� � �������
  
  /*USART3_TX-����� 2*/
  
  DMA_DeInit(DMA1_Channel2);  //����� �������� ������ 2
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004804; //������� ����� �������� ������ USART3
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Navi_TxBuffer; //���������� ����� USART3
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //����������� ������ �� ������ � ���������
  DMA_InitStructure.DMA_BufferSize = TX_BufferSize; //������ ������
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //���������� ����� ������ DMA
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low; //���������  ������
  
  DMA_Init(DMA1_Channel2 , &DMA_InitStructure); //�������� ��������� � �������
 // DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE); //��������� ���������� �� ��������� ��������
  
   /*USART3_RX-����� 3*/ 
  DMA_DeInit(DMA1_Channel3);  //����� �������� ������ 3
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004804; //������� ����� �������� ������ USART3
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Navi_RxBuffer; //�������� ����� USART3
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //����������� ������ �� ��������� � ������
  DMA_InitStructure.DMA_BufferSize = RX_BufferSize; //������ ������
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //����������� ����� ������
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //���������  �������
  
  DMA_Init(DMA1_Channel3 , &DMA_InitStructure); //�������� ��������� � �������
  DMA_ITConfig(DMA1_Channel3,  DMA_IT_TC, ENABLE); //��������� ���������� �� �������� ��������
  
  /*��������� ������� ������*/
  DMA_Cmd(DMA1_Channel5, ENABLE);//USART1_RX
  DMA_Cmd(DMA1_Channel6, ENABLE);//USART2_RX
  DMA_Cmd(DMA1_Channel3, ENABLE);//USART3_RX
  
  
} 

void delay_ms(uint16_t msec) //�������� � ������������
{
  delay_Counter=msec; //����������� �������� ��������
  delay_EnableStatus = ENABLE; //���������� ��������
  while(delay_Counter); //���� ���� ������� �� ����� 0
  delay_EnableStatus = DISABLE; //������������ ��������
}  



void SendString_InUnit(const char *str , uint8_t Unit) //������� �������� ������ ��������� ������ ����� UART

{
 uint16_t len; //������ ������
 uint16_t i;   //�������
 len=strlen(str); //��������� ������ ������
 
 switch(Unit)  //�������� �� ����� ������ ���������
 {
 case  Bluetooth:{
  for(i=0;i<len;i++)  Bluetooth_TxBuffer[i]=*str++; //��������� ������ � ����� ��������
  DMA_Cmd(DMA1_Channel4 , DISABLE);                 //��������� ����� DMA
  DMA_SetCurrDataCounter(DMA1_Channel4 , len);      //������������� ������� DMA ������
  DMA_ClearFlag(DMA1_FLAG_TC4);                     //���������� ���� ��������� ��������
  DMA_Cmd(DMA1_Channel4 , ENABLE );                 //���������� DMA �����
  while(DMA_GetFlagStatus(DMA1_FLAG_TC4)==RESET);   //���� ��������� ��������
  DMA_Cmd(DMA1_Channel4 , DISABLE);                 //��������� ����� DMA
  ClearBufer(Bluetooth_TxBuffer);                   //������� ����� 
  break; }
                 
 case  GSM:{
  for(i=0;i<len;i++)  GSM_TxBuffer[i]=*str++;       //��������� ������ � ����� ��������
  DMA_Cmd(DMA1_Channel7 , DISABLE);                 //��������� ����� DMA
  DMA_SetCurrDataCounter(DMA1_Channel7 , len);      //������������� ������� DMA ������
  DMA_ClearFlag(DMA1_FLAG_TC7);                     //���������� ���� ��������� ��������
  DMA_Cmd(DMA1_Channel7 , ENABLE );                 //���������� DMA �����
  while(DMA_GetFlagStatus(DMA1_FLAG_TC7)==RESET);   //���� ��������� ��������
  DMA_Cmd(DMA1_Channel7 , DISABLE);                 //��������� ����� DMA
  ClearBufer(GSM_TxBuffer);                         //������� ����� 
  break; }
                 
 case  Navigator:{
  for(i=0;i<len;i++)  Navi_TxBuffer[i]=*str++;      //��������� ������ � ����� ��������
  DMA_Cmd(DMA1_Channel2 , DISABLE);                 //��������� ����� DMA
  DMA_SetCurrDataCounter(DMA1_Channel2 , len);      //������������� ������� DMA ������
  DMA_ClearFlag(DMA1_FLAG_TC2);                     //���������� ���� ��������� ��������
  DMA_Cmd(DMA1_Channel2 , ENABLE );                 //���������� DMA �����
  while(DMA_GetFlagStatus(DMA1_FLAG_TC2)==RESET);   //���� ��������� ��������
  DMA_Cmd(DMA1_Channel2 , DISABLE);                 //��������� ����� DMA
  ClearBufer(Navi_TxBuffer);                        //������� �����  
  break; }                
   
   
 } 
  
  
  
}  

void ClearBufer(char *buf) //������� ������� ������
{
 uint16_t size; //������ ������ 
 uint16_t j; //�������
 /*���������� �������� ��� ���������� �����*/
 if((buf==Bluetooth_RxBuffer) || (buf==GSM_RxBuffer) || (buf==Navi_RxBuffer)) size=RX_BufferSize;
 else size=TX_BufferSize; 
 
 for(j=0;j<size;j++) 
 {
   *buf=0x00; //����������� �������� �������� �������� �� ���������
   buf++; //�������������� ��������� ����������
 }
}





void Reset_rxDMA_ClearBufer(uint8_t Unit) //����� ��������� DMA ������ � ������� ������ ������
{   
 switch(Unit)  //�������� �� ����� ������ ���������  
 { 
 case  Bluetooth:   
   {
     DMA_Cmd(DMA1_Channel5 , DISABLE ); //������������� DMA �����
     ClearBufer(Bluetooth_RxBuffer);                  //������� �����
     DMA_SetCurrDataCounter(DMA1_Channel5 ,RX_BufferSize); //������ ������� � ������ ������
     DMA_Cmd(DMA1_Channel5 , ENABLE ); //���������� DMA �����
   break;}
 case  GSM: 
    
     {
     DMA_Cmd(DMA1_Channel6 , DISABLE ); //������������� DMA �����
     ClearBufer(GSM_RxBuffer);                  //������� �����
     DMA_SetCurrDataCounter(DMA1_Channel6 , RX_BufferSize); //������ ������� � ������ ������
     DMA_Cmd(DMA1_Channel6 , ENABLE ); //���������� DMA �����
     break;}
 case  Navigator:   
    {
     DMA_Cmd(DMA1_Channel3 , DISABLE ); //������������� DMA �����
     ClearBufer(Navi_RxBuffer);                  //������� �����
     DMA_SetCurrDataCounter(DMA1_Channel3 , RX_BufferSize); //������ ������� � ������ ������
     DMA_ClearFlag(DMA1_FLAG_TC3);                     //���������� ���� ��������� ��������
     DMA_Cmd(DMA1_Channel3 , ENABLE ); //���������� DMA �����
   break; }
    
  }
    
}

void COMAND_EXEC(void)  //����������� ������
{
  Autoheatmode_TypeDef LastMode;
  
  LastMode = STATUS.AUTOHEAT_MODE;
    
  switch(STATUS.COMMAND) //������ ������� �������
  {
  case 10:  //������� ������ ����� � ������
    {
     SECURITY(DISABLE);
     SEND_SMS(SECURITY_OFF);
    break;}
    
  case 11:  //������� ������/��������� �� ������
    {
     SECURITY(ENABLE);
     SEND_SMS(SECURITY_ON);
    break;} 
    
  case 13:  //��������� ��������� 
    {
    AUTOSTART(ENABLE); 
    delay_ms(2000);
    if(GPIO_ReadInputDataBit(ALARM1,MOTOR_CTRL )==1) SEND_SMS(MOTOR_ON); 
    else SEND_SMS(MOTOR_START_ERROR);
    break;}
    
  case 14:  //��������� ���������
    {
    AUTOSTART(DISABLE);
    SEND_SMS(MOTOR_OFF);
    break;} 
    
  case 15:  //������������ ����� ������������ 05/30
    {
     STATUS.AUTOHEAT_MODE = MODE_AUTOHEAT_0530;
     AUTOHEAT();
    if(STATUS.AUTOHEAT_MODE != MODE_AUTOHEAT_OFF) 
    { 
     if(LastMode == MODE_AUTOHEAT_OFF) SEND_SMS(AUTOHEAT_ON_0530);
     else SEND_SMS(CHANGE_HEAT_MODE_0530);
    }
    break;}
    
  case 16:  //������������ ����� ������������ 10/30
    {
      STATUS.AUTOHEAT_MODE = MODE_AUTOHEAT_1030;
       AUTOHEAT();
    if(STATUS.AUTOHEAT_MODE != MODE_AUTOHEAT_OFF) 
    { 
     if(LastMode == MODE_AUTOHEAT_OFF) SEND_SMS(AUTOHEAT_ON_1030);
     else SEND_SMS(CHANGE_HEAT_MODE_1030);
    }
    break;} 
    
  case 17:  //������������ ����� ������������ 30/30
    {
      STATUS.AUTOHEAT_MODE = MODE_AUTOHEAT_3030;
       AUTOHEAT();
    if(STATUS.AUTOHEAT_MODE != MODE_AUTOHEAT_OFF) 
    { 
     if(LastMode == MODE_AUTOHEAT_OFF) SEND_SMS(AUTOHEAT_ON_3030);
     else SEND_SMS(CHANGE_HEAT_MODE_3030);
    }
    break;} 
    
  case 18:  //������������ ����� ������������ 30/20
    {
      STATUS.AUTOHEAT_MODE = MODE_AUTOHEAT_3020;
       AUTOHEAT();
    if(STATUS.AUTOHEAT_MODE != MODE_AUTOHEAT_OFF) 
    { 
     if(LastMode == MODE_AUTOHEAT_OFF) SEND_SMS(AUTOHEAT_ON_3020);
     else SEND_SMS(CHANGE_HEAT_MODE_3020);
    }
    break;}  
    
    
 
 
  case 19:  //��������� ����� ������������
    {
     STATUS.AUTOHEAT_MODE = MODE_AUTOHEAT_OFF; 
      AUTOHEAT();
    SEND_SMS(AUTOHEAT_OFF);  
    break;}
    
  case 20: //����� ������
    {
      
    break;}
    
   
  }
  STATUS.COMMAND=0;
}
//================================================================================
void IWDG_Configuration(void)//������������� ����������� �������
{
 IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //��������� ������ � �������� ��������� 
    /*��������� ������ ����������� ������� �� 26,2 �������*/
 IWDG_SetPrescaler(IWDG_Prescaler_256); //��������� ������������
 IWDG_SetReload(0xFFF); //��������� ���������������� ��������
 
 IWDG_ReloadCounter(); //����� �������� ����������� �������
 
 IWDG_Enable(); //��������� ���������� �������
 
 DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE); //��������� ���������� ������� � ������ �������
 
  
}


/*******************************************************************************
********************************************************************************
**                                                                            **
**                        ����� ���������                                     **
**                                                                            **
********************************************************************************
*******************************************************************************/