/*******************************************************************************
********************************************************************************
**                                                                            **
**          БИБЛИОТЕКА ФУНКЦИЙ МОДУЛЯ Bluetooth LMX9838                       **
**                                                                            **
********************************************************************************
*******************************************************************************/

//******************Подключаемые файлы******************************************
#include "Bluetooth.h"
#include "main.h"
#include <string.h>
#include <stdio.h>
#include "navigation.h"
#include "GSM.h"
//*************Инициализация глобальных переменных******************************

const char Bluetooth_Answer1[7] = {7 , 0x02,0x69,0x0C,0x07,0x00,0x7C};
const char Bluetooth_Answer2[7] = {7 , 'S','T','O','P','\r','\n'};
uint16_t cnt; //Счетчик
uint8_t BT_Timer; //Счетчик дедтайма Bluetooth 


//*****************Функции для работы с Bluetooth ******************************

void Bluetooth_Configuration(void) //Функция инициализации Bluetooth
{
GPIO_ResetBits(BLUETOOTH , BLUE_RESET);//Сбрасываем модуль
 delay_ms(250);
GPIO_SetBits(BLUETOOTH , BLUE_RESET); //Выводим Bluetooth из состояния сброса  
 //delay_ms(100); 
  Reset_rxDMA_ClearBufer(Bluetooth);
  
  BT_Timer=30;
  
 
}


void Bluetooth_Read(void) //Функция чтения буфера Bluetooth
{

  
  
 if (Bluetooth_Parser( Bluetooth_Answer1) != NULL) 
 {
   SendString_InUnit("\r\nBLACKBOX CONNECT OK\r\n" , Bluetooth);
   BT_Timer=30;
   Reset_rxDMA_ClearBufer(Bluetooth);
     
 }

 
 if (strstr(Bluetooth_RxBuffer , "PLANSHET\r\n") != NULL) 
 {
    SendString_InUnit("\r\nSTATUS:NORMAL\r\n" , Bluetooth);
    BT_Timer=10;
   
   
    
   Reset_rxDMA_ClearBufer(Bluetooth); 
 } 
  BT_Timer--;
 if(BT_Timer==0) Bluetooth_Configuration();
  
}


char *Bluetooth_Parser( const char *answer)
{
 uint16_t j; //Счетчик
 uint16_t k; //Счетчик 2
 uint8_t l; 
 char const *ans;
 char *out;  
 uint8_t size;
 
 out=NULL;
 size=*answer++;
 
for (j=0 ; j<RX_BufferSize ; j++)
  {
    if (Bluetooth_RxBuffer[j]==*answer)
    {
     k=j;
     k++;
     ans=answer;
     ans++;
     for (l=2 ; l<size ; l++)
     {
       if (Bluetooth_RxBuffer[k++]==*ans++) out=&Bluetooth_RxBuffer[j];
       else {
             out=NULL;
             break;
            }
     }
     if (out!=NULL) return out;
     
    }
  
  }
  
  return NULL;
  
  
}
























