/*******************************************************************************
********************************************************************************
**                                                                            **
**          ���������� ������� ������ ���������� CC1101 433 ���               **
**                                                                            **
********************************************************************************
*******************************************************************************/

//******************������������ �����******************************************
#include "Transceiver_433MHz.h"
#include "main.h"
#include "stm32f10x.h"

//*************������������� ���������� ����������******************************
char SPI_buffer[spi_bufer_size]; //����� ������ � �������� ����������
FunctionalState TX_state = DISABLE; //������ ��������

union u_type adres;
// ������ ������������� ��������� ����������(������� ���� - �����, ������� - ��������)
/*
������� ������: 27.000 ���
���������: 2-FSK
������� �������: 432.999893 ���
������ �������: 199.813843 ���
����� ������: 47
������� �������: 442.390732 ���
��������: 4.943848 ���
������ ����������� ���������: 210.937500 ���
�������� ��������: 1.2 ����
*/
const uint16_t INIT_REG[35]=
{
 0x0006, //0 IOGFG2 ����������� �������
 0x0206, //1 IOGFG0 �����-�������� ������
 0x06FF, //2 PKTLEN ������ ������
 0x0704, //3 PKRCTRL1 �������� ������
 0x0805, //4 PKRCTRL0 �������� ������
 0x0901, //5 ADDR ����� ����������
 0x0A2F, //6 CHANNR ����� ������
 0x0B06, //7 FSCTRL1 ��������� �������� ����������� �������
 0x0C00, //8 FSCTRL0 ��������� �������� ����������� �������
 0x0D10, //9 FREQ2 �������� ������� �������
 0x0E09, //10 FREQ1 �������� ������� �������
 0x0F7B, //11 FREQ0 �������� ������� �������
 0x1085, //12 MDMCFG4 ������������ ������
 0x1178, //13 MDMCFG3 ������������ ������
 0x1203, //14 MDMCFG2 ������������ ������
 0x1302, //15 MDMCFG1 ������������ ������
 0x14E5, //16 MDMCFG0 ������������ ������
 0x1514, //17 DEVIATION ��������
 0x1730, //18 MCSM1 ������������ �������� �������� �����
 0x1818, //19 MCSM0 ������������ �������� �������� �����
 0x1916, //20 FOCCFG ����������� ������ �������
 0x1A6C, //21 BSCFG ������������ ��������� �������������
 0x1BC0, //22 AGCCTRL2 ���������� ��������� ������
 0x1C00, //23 AGCCTRL1 ���������� ��������� ������ 
 0x1DB2, //24 AGCCTRL0 ���������� ��������� ������
 0x21B6, //25 FREND1 ��������� ��������� ������
 0x2210, //26 FREND0 ��������� ����������� ������
 0x23E9, //27 FSCAL3 ��������� ���������� ����������� �������
 0x242A, //28 FSCAL2 ��������� ���������� ����������� �������
 0x2500, //29 FSCAL1 ��������� ���������� ����������� ������� 
 0x261F, //30 FSCAL0 ��������� ���������� ����������� �������
 0x2959, //31 FSTEST �������� ����������� �������
 0x2C81, //32 TEST2
 0x2D35, //33 TEST1
 0x2E09  //34 TEST0
};

//*************������� ��� ������ � �����������   ******************************

char SPI_SEND( char data) //������� ������/�������� ������� �� SPI
{ 
  char rx_data;
  
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); //�������� ������� ������ ��������
  SPI_I2S_SendData(SPI1, data); //�������� ������� � �����
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET); //�������� ��������� ������/��������
  rx_data = SPI_I2S_ReceiveData(SPI1); //��������� �������� ������
  return  rx_data; 
}

void RESET_TR(void) //����� ���������� �� ��������� �������
{
  GPIO_InitTypeDef GPIO_Init_struct; // �������� ��������� ������������� ������
  
  SPI_Cmd(SPI1, DISABLE); //���������� SPI
  
  /*��������� ������ SCK � MOSI � ��������� ������� ����������� �������*/
  GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
  GPIO_Init_struct.GPIO_Pin = SPI_SCK | SPI_MOSI; //�������� ������ SCK � MOSI
  GPIO_Init_struct.GPIO_Speed= GPIO_Speed_50MHz; //������������� �������
  GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //������������� ����� ������ ����������� �����
  GPIO_Init(SPI_PORT, &GPIO_Init_struct);      //�������� ��������� � �������
  
  GPIO_SetBits(SPI_PORT , SPI_SCK); //������������� 1 �� SCK
  GPIO_ResetBits(SPI_PORT , SPI_MOSI); //������������� 0 �� MOSI
  GPIO_ResetBits(SPI_PORT , SPI_SS); //���������� SPI_SS
  delay_ms(1);//�������� � 1 ������������
  GPIO_SetBits(SPI_PORT , SPI_SS); //������������ SPI_SS
  delay_ms(40);
  
  /*��������� ������ SCK � MOSI � ��������� ������� �������������� �������*/
  GPIO_StructInit(&GPIO_Init_struct);       //�������������� ��������� ���������� ����������
  GPIO_Init_struct.GPIO_Pin = SPI_SCK | SPI_MOSI; //�������� ������ SCK � MOSI
  GPIO_Init_struct.GPIO_Speed= GPIO_Speed_50MHz; //������������� �������
  GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //������������� ����� ������  ����� �������������� �������
  GPIO_Init(SPI_PORT, &GPIO_Init_struct);      //�������� ��������� � �������
  
  SPI_Cmd(SPI1, ENABLE ); //��������� SPI
  
  GPIO_ResetBits(SPI_PORT , SPI_SS); //���������� SPI_SS
  while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //���� 0 �� MISO
  SPI_SEND(SRES);//�������� ����� ����������       
  GPIO_SetBits(SPI_PORT , SPI_SS); //������������ SPI_SS
  
  
}

void WRITE_TR_REG( uint16_t reg) // ������� ������ ��������
{
   union u_type dat; //��������� ���������� ������������� ����
   
  GPIO_ResetBits(SPI_PORT , SPI_SS); //���������� SPI_SS
  while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //���� 0 �� MISO 
  
  dat.halfword=reg;
  SPI_SEND(dat.byte[1]);  //����� �������� 
  SPI_SEND(dat.byte[0]);  //�������� �������� 
  delay_ms(1);
  GPIO_SetBits(SPI_PORT , SPI_SS); //������������ SPI_SS
}

char READ_TR_REG( char adr)  // ������� ������ ��������
{
 char reg; // ��������� ����������
 
 GPIO_ResetBits(SPI_PORT , SPI_SS); //���������� SPI_SS
 while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //���� 0 �� MISO 

 SPI_SEND(adr + 0x80);   // ������� ��� ���������� ��������  
 reg= SPI_SEND(0x00); //����������� ���������� �������� �������� ���������
 delay_ms(1);
 
 GPIO_SetBits(SPI_PORT , SPI_SS); //������������ SPI_SS
 
 return reg; //���������� �������� �������
  
  
}
   
void STROB(char strob)  //������ �����-�������
{
GPIO_ResetBits(SPI_PORT , SPI_SS); //���������� SPI_SS
while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //���� 0 �� MISO 

SPI_SEND(strob); //�������� �������

GPIO_SetBits(SPI_PORT , SPI_SS); //������������ SPI_SS
  
}

void WRITE_PATABLE(void)    //������ ������� ��������
{
GPIO_ResetBits(SPI_PORT , SPI_SS); //���������� SPI_SS
while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //���� 0 �� MISO

WRITE_TR_REG(OUTPUT_POWER); //������ �������� �������� �������� �����������
  
GPIO_SetBits(SPI_PORT , SPI_SS); //������������ SPI_SS
}

char STATUS_TR(void) //����������� ������� ����������
{
char st;
 
GPIO_ResetBits(SPI_PORT , SPI_SS); //���������� SPI_SS
while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //���� 0 �� MISO

st=SPI_SEND(SNOP);

GPIO_SetBits(SPI_PORT , SPI_SS); //������������ SPI_SS
return st;
}


void SEND_PAKET(void) //������� �������� ������
{
uint8_t tr_cnt; //�������
  
STROB(SIDLE);  //������� � ����� IDLE
STROB(SFRX);  //������� ��������� ������
STROB(SFTX); //������� ����������� ������
delay_ms(1);

GPIO_ResetBits(SPI_PORT , SPI_SS); //���������� SPI_SS
while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //���� 0 �� MISO

SPI_SEND(0x7F);   //�������� ������ �� ������
SPI_SEND(SPI_buffer[0]); //������ ������ ������
for (tr_cnt=1;tr_cnt<SPI_buffer[0];tr_cnt++)  //������ ������
  {
   SPI_SEND(SPI_buffer[tr_cnt]); 
  }
GPIO_SetBits(SPI_PORT , SPI_SS); //������������ SPI_SS

TX_state=ENABLE; //�������� �������

STROB(STX); //��������� ��������

while(GPIO_ReadInputDataBit(SPI_PORT, TR_GP1 )==0); //�������� ������ ��������
while(GPIO_ReadInputDataBit(SPI_PORT, TR_GP1 )==1); //�������� ����� ��������

STROB(SIDLE);  //������� � ����� IDLE
STROB(SFRX);  //������� ��������� ������
STROB(SFTX); //������� ����������� ������

TX_state=DISABLE; //�������� ���������

CLEAR_SPI_buffer(); //������� ������  
}

void RECEIVE_PAKET(void) //������� ������ ������
{
  uint8_t tr_cnt; //������� 
  
CLEAR_SPI_buffer(); //������� ������
  
STROB(SIDLE);  //������� � ����� IDLE

GPIO_ResetBits(SPI_PORT , SPI_SS); //���������� SPI_SS
while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //���� 0 �� MISO

SPI_SEND(0xFF);  //�������� ������ ������
SPI_buffer[0] = SPI_SEND(0x00); //���������� ������ ������

for (tr_cnt=1;tr_cnt<SPI_buffer[0];tr_cnt++)    //���������� ������
   {
   SPI_buffer[tr_cnt]=SPI_SEND(0x00);
   } 
GPIO_SetBits(SPI_PORT , SPI_SS); //������������ SPI_SS

STROB(SFRX);  //������� ��������� ������
  
}

 void CLEAR_SPI_buffer(void) //������� SPI ������
 { 
   uint8_t tr_cnt2; //�������
 
   for (tr_cnt2=0;tr_cnt2<spi_bufer_size;tr_cnt2++)
   {
    SPI_buffer[tr_cnt2]=0x00;
   }
 }

void Transceiver_Configuration(void) //������������� ����������
{
  uint8_t tr_cnt3; //�������
  
  RESET_TR(); //����� ����������
  delay_ms(20); 
  for (tr_cnt3=0;tr_cnt3<35;tr_cnt3++) WRITE_TR_REG(INIT_REG[tr_cnt3]); //������ ���������������� ��������� ����������
  
  WRITE_PATABLE(); //������ ������� ��������
  STROB(SIDLE); //������� � ����� IDLE
  STROB(SFRX);  //������� ��������� ������
  STROB(SFTX); //������� ����������� ������
  //-----------------------------------------------------------------
/*
 delay_ms(1000); 
GPIO_ResetBits(SPI_PORT , SPI_SS); //���������� SPI_SS
while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //���� 0 �� MISO
SPI_SEND(0xC0);

  
for (tr_cnt3=0;tr_cnt3<35;tr_cnt3++)
{
  
 adres.halfword=INIT_REG[tr_cnt3]; 
 SPI_buffer[tr_cnt3]=READ_TR_REG(adres.byte[1]) ;
 delay_ms(1);
  
  //SPI_buffer[tr_cnt3]=SPI_SEND(0x00);
}

//GPIO_SetBits(SPI_PORT , SPI_SS); //������������ SPI_SS

//SPI_buffer[47]=STATUS_TR();
//SPI_buffer[48]=READ_TR_REG(0x06) ;
*/
//----------------------------------------------------------------------

#ifdef BLACKBOX //���� ���������� BLACKBOX
  STROB(SRX);  //������� � ����� ������
  STATUS.Transceiver_Status = ENABLE;

  
#endif 
  
  Transceiver_Status = ENABLE;
  delay_ms(3);

 
  CLEAR_SPI_buffer(); //������� SPI ������
}


/***************************����� �����****************************************/