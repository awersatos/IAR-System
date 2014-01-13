//******************************************************************************
//            ������������ ���� ���������� ������� ������ ����������
//******************************************************************************

//*****************������������ �����*******************************************
#include "stm32f10x.h"
//***************����������������***********************************************
#define NAVIGATOR GPIOB
#define NAVI_STANDBAY GPIO_Pin_12
#define NAVI_RESET GPIO_Pin_13

//************* ������������� ���������� ���������� ****************************

extern char Timestamp[6][11]; //����� ������ ���������
extern char Latitude[6][9]; //������ 
extern char Longitude[6][10]; //�������
extern char Speed[6][6]; //��������
extern char Date[6][7]; //���� 

extern uint8_t Navi_ResetCounter; //������� ��������� ������ ����������

//*************���������� ����������� �������***********************************
void NAVI_Configuration(void); //������������� �������������� ���������
void ReadCoordinates(void); //������� ���������� ���������
void NAVI_Reset(void); //������� ������ ����������



/***************************����� �����****************************************/