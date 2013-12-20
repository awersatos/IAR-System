/*******************************************************************************
********************************************************************************
**                                                                            **
**          ���������� ������� ������ ���������� FSATREX IT600                **
**                                                                            **
********************************************************************************
*******************************************************************************/

//******************������������ �����******************************************

#include "navigation.h"
#include "main.h"
#include <string.h>
//*************������������� ���������� ����������******************************
/*
char RMC[66]; //������ ��������� RMC
char RMC_1[66]; //������ ��������� RMC
char RMC_2[66]; //������ ��������� RMC
char RMC_3[66]; //������ ��������� RMC
char RMC_4[66]; //������ ��������� RMC
char RMC_5[66]; //������ ��������� RMC
*/
char Timestamp[6][11]; //����� ������ ���������
char Latitude[6][9]; //������ 
char Longitude[6][10]; //�������
char Speed[6][6]; //��������
char Date[6][7]; //���� 

               /*������� ������ � ������� ������� ��� ����������*/
const char NAVI_Comand1[] = "$PSTMSETPAR,1201,0040*1D\r\n"; //�������� ������ ��������� RMC
const char NAVI_Comand2[] = "$PSTMSAVEPAR*58\r\n";  //��������� ��������� � ����������������� ������
const char NAVI_Answer1[] = "$PSTMSETPAROK"; //����� � ��� ��� ��������� ���������� �������
const char NAVI_Answer2[] = "$GPRMC"; //��������� ��������� RMC

//**************������� ��� ������ � �����������********************************
void NAVI_Configuration(void)   //������������� �������������� ���������
{
  
 IWDG_ReloadCounter(); //����� �������� ����������� ������� 
 delay_ms(60); //�������� 60�� ����� ������� ������
 GPIO_SetBits(NAVIGATOR , NAVI_STANDBAY); //��������� ����� �������� ����������
 GPIO_SetBits(NAVIGATOR , NAVI_RESET); //������� ��������� �� ��������� ������
 delay_ms(5000);
 
 SendString_InUnit(NAVI_Comand1 , Navigator); //�������� ������� "������ RMC"
 delay_ms(10); //�������� 10��

 if (strstr(Navi_RxBuffer , NAVI_Answer1)!=NULL) //���� ����� �������������
 {
  SendString_InUnit(NAVI_Comand2 , Navigator); //�������� ������� ���������� ���������� 
 }
 
 STATUS.NavigatorStatus = ACTIVE; //������ ���������� �������

 IWDG_ReloadCounter(); //����� �������� ����������� �������   
}


void ReadCoordinates(void) //������� ���������� ���������
{
char *s, *s1; //��������� ��������� ����������
uint8_t i; //�������

s = strstr(Navi_RxBuffer , NAVI_Answer2); //���� � ������ ��������� ��������� RMC
if(s!=NULL)                               //���� ������ � ������ ������������
{
  
  if(strchr(Navi_RxBuffer , 'A')!=NULL) //���������� ������� �� ������������ ������
  {
   STATUS.CoordinatesStatus = 'A'; //������������� ������ �������� ���������
   
   s++;
   s1 = s+6; //������������� ����� ��������� ���������� �� ������ �������
   
  // for(i=0;i<65;i++) RMC[i]=*s++; //���������� ������ RMC
  //********************************************************************** 
   
   for(i=0;i<11;i++) Timestamp[0][i] = 0x00; //�������� ��������� ������
   for(i=0;i<6;i++) Speed[0][i] = 0x00;
   for(i=0;i<9;i++) Latitude[0][i] = 0x00;
   for(i=0;i<10;i++) Longitude[0][i] = 0x00;
   for(i=0;i<7;i++) Date[0][i] = 0x00; 
   
   for(i=0;i<10;i++) Timestamp[0][i] = *s1++; //��������� ������ ������� 
   
   s1=s1+3;                                //������������� ����� ��������� ���������� �� ������ ������
   for(i=0;i<8;i++) Latitude[0][i] = *s1++; //��������� ������ ������
   
   s1=s1+3;                                   //������������� ����� ��������� ���������� �� ������ �������
   for(i=0;i<9;i++) Longitude[0][i] = *s1++; //��������� ������ �������  
   s1=s1+3; //������������ ����� �� ������ ��������
   
   for(i=0;i<5;i++) //��������� ������ ��������
   {
    Speed[0][i] = *s1++;
    if(*s1 == ',') break;
     
   }
   for(i=0;i<6;i++)
   {s1++; //������������ ����� �� ������ ����
   if(*s1 == ',') break;}
   s1++;
   for(i=0;i<6;i++) Date[0][i] = *s1++; //��������� ������ ����
  }
  
  else if(strchr(Navi_RxBuffer , 'V')!=NULL)  STATUS.CoordinatesStatus = 'V'; //���� ���������� �� ������� ������������� ��������������� ������ 
 
  
} 
}













/***************************����� �����****************************************/