//******************************************************************************
//            ������������ ���� ���������� ������� ������ ����������
//******************************************************************************

//*****************������������ �����*******************************************
//***************����������������***********************************************
#define NAVIGATOR GPIOB
#define NAVI_STANDBAY GPIO_Pin_12
#define NAVI_RESET GPIO_Pin_13

//************* ������������� ���������� ���������� ****************************
extern char RMC[66]; //������ ��������� RMC
//extern char Timestamp[6]; //����� ������ ���������
//extern char Latitude[10]; //������ 
//extern char Longitude[11]; //�������


//*************���������� ����������� �������***********************************
void NAVI_Configuration(void); //������������� �������������� ���������
void ReadCoordinates(void); //������� ���������� ���������




/***************************����� �����****************************************/