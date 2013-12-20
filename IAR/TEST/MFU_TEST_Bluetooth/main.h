//******************************************************************************
//              �������� ������������ ���� �������
//******************************************************************************

//**************������������ �����**********************************************
#include "stm32f10x.h"

//**************����������������************************************************

#define TX_BufferSize 64       //������ �������  ��������  ����
#define RX_BufferSize 192       //������ ������� ������ ����
#define  Bluetooth  1
#define  GSM        2
#define  Navigator  3
#define  ACTIVE ENABLE
#define  INACTIVE DISABLE
//**************����������� ����������� �����***********************************
typedef struct
{
FunctionalState MainPower; //�������� �������

uint16_t BatteryCharge;  //����� �������

FunctionalState NavigatorStatus; //������ ����������

uint8_t CoordinatesStatus; //���������� ���������

FunctionalState GSM_Status;//������ GSM

FunctionalState AUTOSTART; //������ �����������

uint8_t SIM_Card; //����� ��� �����

uint8_t OPERATOR;

FunctionalState SecurityStatus; //������ ������

uint8_t COMMAND;//������ �������


}StatusStruct_TypeDef; //��������� ���������� ������� ����������


//**************���������� ���������� ��������� �� ������ ������****************
extern StatusStruct_TypeDef STATUS; //����������� ���������� ������� ����������
extern FunctionalState delay_EnableStatus; //������ ���������� ��������� ��������
extern uint32_t delay_Counter; //�������� ��������� ��������

extern const char SERIAL_NUMBER[]; //�������� ����� ����������

extern char Bluetooth_TxBuffer[TX_BufferSize+1]; //���������� Bluetooth ����� USART1
extern char Bluetooth_RxBuffer[RX_BufferSize+1]; //�������� Bluetooth ����� USART1
extern char GSM_TxBuffer[TX_BufferSize+1];       //���������� GSM ����� USART2
extern char GSM_RxBuffer[RX_BufferSize+1];       //�������� GSM ����� USART2
extern char Navi_TxBuffer[TX_BufferSize+1];      //���������� Navigation ����� USART3
extern char Navi_RxBuffer[RX_BufferSize+1];      //�������� Navigation ����� USART3
//*************���������� ����������� �������***********************************
void delay_ms(uint16_t msec); //�������� � ������������
void delay_us(uint32_t usec); //�������� � �������������
void RCC_Configuration(void); //������������ � ��������� ������������ ���������
void GPIO_Configuration(void); //��������� ���������� ������ ����� ������
void NVIC_Configuration(void); //������������� � ��������� ����������
void ADC_Configuration(void);//������������� ���
void TIMER_Configuration(void); //������������� ��������
void UART_Configuration(void);//������������� USART
void DMA_Configuration(void); //������������� ������� DMA
void ClearBufer(char *buf); //������� ������� ������
void SendString_InUnit(const char *str , uint8_t Unit); //������� �������� ������ ��������� ������ ����� UART
void Reset_rxDMA_ClearBufer(uint8_t Unit); //�������������� DMA ������ � ������� ������ ������
void BKP_Configuration(void); //������������ ���������� ������ �������
void COMAND_EXEC(void);  //����������� ������
void EXTI_Configuration(void); //������������� ����������� ������� ����������
void SPI_Configuration(void); //������������� SPI


/***************************����� �����****************************************/