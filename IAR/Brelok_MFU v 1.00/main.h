//******************************************************************************
//              �������� ������������ ���� �������
//******************************************************************************

//**************������������ �����**********************************************
#include "stm32f10x.h"

//**************����������������************************************************
#define red GPIO_Pin_0
#define green GPIO_Pin_1
#define BUZZER_PIN_1 GPIO_Pin_8
#define BUZZER_PIN_2 GPIO_Pin_9
#define BUTTON_1 GPIO_Pin_8
#define BUTTON_2 GPIO_Pin_9

#define OPEN 'O'   // ������� ����������
#define CLOSE 'C'
#define ALARM 'A'
//**************����������� ����������� �����***********************************

typedef enum  //����������� ����� ����������
{
 OFF,
 GREEN,
 RED,
 ORANGE 
}Color_TypeDef;

typedef enum //���������� ��������
{
  Long_Bip_1, 
  Medium_Bip_2,
  Medium_Bip_3,
  Short_Bip_15,
  
}Bip_TypeDef;


//**************���������� ���������� ��������� �� ������ ������****************
extern FunctionalState delay_EnableStatus; //������ ���������� ��������� ��������
extern uint32_t delay_Counter; //�������� ��������� ��������
extern FunctionalState Transceiver_Status; //������ ���������� ����������
extern uint8_t KEY[]; //���� ���������� 
extern uint8_t BUFER[16]; //������ ����������
extern FunctionalState CRYPT; //������ ����������

//*************���������� ����������� �������***********************************
void delay_ms(uint16_t msec); //�������� � ������������
void RCC_Configuration(void); //������������ � ��������� ������������ ���������
void GPIO_Configuration(void); //��������� ���������� ������ ����� ������
void EXTI_Configuration(void); //������������� ����������� ������� ����������
void NVIC_Configuration(void); //������������� � ��������� ����������
void SPI_Configuration(void); //������������� SPI
void TIMER_Configuration(void); //������������� �������
void LED(Color_TypeDef COLOR); //������� ���������� �����������
void BUZZER(Bip_TypeDef BIP); //������� ���������� ��������������
/***************************����� �����****************************************/
