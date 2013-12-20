/*******************************************************************************
********************************************************************************
**                                                                            **
**                  �������� ���� ������� TEST_CC1101                         **
**                                                                            **
********************************************************************************
*******************************************************************************/

//****************������������ �����********************************************
#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "main.h"
#include "Transceiver_433MHz.h"
#include "STM32vldiscovery.h"
//************* ������������� ���������� ���������� ****************************
FunctionalState delay_EnableStatus = DISABLE; //������ ���������� ��������� ��������
uint32_t delay_Counter; //�������� ��������� ��������



//******************************************************************************

void main(void) //�������� ������� ���������
{
       /*������������� ������������ ��������� �����������*/
RCC_Configuration(); //��������� ����������� � ��������� ������������� ���������
GPIO_Configuration(); //������������� ������ ����� ������
EXTI_Configuration(); //������������� ����������� ������� ����������
NVIC_Configuration();//������������� ����������
SPI_Configuration();//������������� SPI 
SysTick_Config(SystemCoreClock/100000); //������ ����� ���������� ������� 1 ���
NVIC_SetPriority(SysTick_IRQn,0); //������������������� ���������� ������� ���������

STM32vldiscovery_LEDInit(LED3); //������������� ���������� ���������� �����
STM32vldiscovery_LEDInit(LED4);

STM32vldiscovery_LEDOn(LED3); //�������� ���������





              /*������������� ������� �������*/
Transceiver_Configuration(); //������������� ����������
//*****************************************************************************


//SPI_buffer[0]=STATUS_TR();
//STM32vldiscovery_LEDOn(LED4); //�������� ���������


  
 while(1) //�������� ���� ���������
 {
 
 }
 
 
}

      /*����� �������� ������� ���������*/



void delay_ms(uint16_t msec) //�������� � ������������
{
  delay_us(msec*100);
}  

void delay_us(uint32_t usec) //�������� � �������������
{
  delay_Counter=usec; //����������� �������� ��������
  delay_EnableStatus = ENABLE; //���������� ��������
  while(delay_Counter); //���� ���� ������� �� ����� 0
  delay_EnableStatus = DISABLE; //������������ ��������
  
}  

void RCC_Configuration(void) //������������ � ��������� ������������ ���������
{
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE); //��������� ������������ ������ A,B
RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO , ENABLE); //��������� ������������ �������������� ������� GPIO
RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 , ENABLE); //��������� ������������ SPI1
  
  
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
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_50MHz; //������������� �������
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
GPIO_Init_struct.GPIO_Speed= GPIO_Speed_50MHz; //������������� �������
GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //������������� ����� ������
GPIO_Init(GPIOA, &GPIO_Init_struct);      //�������� ��������� � �������




  
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
  SPI_InitStructure.SPI_BaudRatePrescaler =  SPI_BaudRatePrescaler_8 ; //������������ 8 �������� ������� 3 ���
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //������� ��� ���� ������
  SPI_InitStructure.SPI_CRCPolynomial = 8; //������� ��� �������� CRC
  
  SPI_Init(SPI1, &SPI_InitStructure); //�������� ��������� � �������

  
  
}




/*******************************************************************************
********************************************************************************
**                                                                            **
**                        ����� ���������                                     **
**                                                                            **
********************************************************************************
*******************************************************************************/
