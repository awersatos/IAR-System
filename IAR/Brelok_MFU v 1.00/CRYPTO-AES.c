/*******************************************************************************
********************************************************************************
**                                                                            **
**          БИБЛИОТЕКА ФУНКЦИЙ МОДУЛЯ МОДУЛЯ ШИФРОВАНИЯ CRYPTO_AES            **
**                                                                            **
********************************************************************************
*******************************************************************************/

//******************Подключаемые файлы******************************************
#include "stm32f10x.h"
#include "main.h"
#include "CRYPTO-AES.h"
//*************Инициализация глобальных переменных******************************
uint32_t rkey[N]; //Массив расширеных ключей
uint32_t s[Nb]; //Массив промежуточного результата

             /*ТАБЛИЦА ЗАМЕНЫ БАЙТ*/
const uint8_t subbytes[256] =
  
{       /*0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F   */
/* 0*/    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6f, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76, 

/* 1*/    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,

/* 2*/    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,

/* 3*/    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,

/* 4*/    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,

/* 5*/    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,

/* 6*/    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,

/* 7*/    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,

/* 8*/    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73, 

/* 9*/    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,

/* A*/    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,

/* B*/    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,

/* C*/    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,

/* D*/    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,

/* E*/    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,

/* F*/    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16

};
//-------------------------------------------------------------------------------------------------------

                               /*ТАБЛИЦА ИНВЕРСНОЙ ЗАМЕНЫ БАЙТ*/

const uint8_t invsubbytes[256] =

{       /*0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F   */
/* 0*/    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,

/* 1*/    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,

/* 2*/    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,

/* 3*/    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,

/* 4*/    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
  
/* 5*/    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84, 
  
/* 6*/    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
  
/* 7*/    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,

/* 8*/    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,

/* 9*/    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,

/* A*/    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,

/* B*/    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
  
/* C*/    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,

/* D*/    0x60, 0x51, 0x7f, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,

/* E*/    0xA0, 0xE0, 0x3b, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,

/* F*/    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D 
    
};


 const uint32_t Co = 0x02010103;  //Коэфиценты преобразования 
 const uint32_t InvCo = 0x0E090D0B;  //Коэфиценты преобразования инверсные


//*************      Функции для работы с AES     ******************************
void Encrypt( uint8_t *keyblok, uint8_t *buff, uint8_t *result) //Функция зашифровывания
{ 
  uint8_t i,j; //Счетчики
  
  KeyExpansion( keyblok, ENCRYPT);  //Вычисление раундовых ключей
  
  for(i=j=0; i<Nb; i++, j+=4) s[i]= pack((uint8_t *)&buff[j]); //Заполнение промежуточного массива
  i=0;
  
  AddRoundKey(s , rkey , 0); ////Операция исключающее или с раундовым ключем
  
  for(i=1; i< Nr; i++)
  {
    SubBytes((uint8_t*)s, ENCRYPT);
    ShiftRows((uint8_t*)s, ENCRYPT);
    MixColums(s, Nb, ENCRYPT);
    AddRoundKey(s, rkey, i);
    
  }
  
  SubBytes((uint8_t*)s, ENCRYPT);
  ShiftRows((uint8_t*)s, ENCRYPT);
  AddRoundKey(s, rkey, Nr);
  
  for(i=j=0;i<Nb;i++,j+=4) unpack(s[i], (uint8_t*)& result[j]); 
  
}

//==============================================================================
void Decrypt( uint8_t *keyblok, uint8_t *buff, uint8_t *result) //Функция расшифровывания
{
  uint8_t i,j; //Счетчики
 
  KeyExpansion( keyblok, DECRYPT);  //Вычисление раундовых ключей
 
  for(i=j=0; i<Nb; i++, j+=4) s[i]= pack((uint8_t *)&buff[j]); //Заполнение промежуточного массива
  i=0;
 
  AddRoundKey(s , rkey , 0); ////Операция исключающее или с раундовым ключем
  
  for(i=1; i< Nr; i++)
  {
    ShiftRows((uint8_t*)s, DECRYPT);
    SubBytes((uint8_t*)s, DECRYPT);
    AddRoundKey(s, rkey, i);
    MixColums(s, Nb, DECRYPT);
  }
  
  ShiftRows((uint8_t*)s, DECRYPT);
  SubBytes((uint8_t*)s, DECRYPT);
  AddRoundKey(s, rkey, Nr);
  
  for(i=j=0;i<Nb;i++,j+=4) unpack(s[i], (uint8_t*)& result[j]);
 
}
//==============================================================================
void AddRoundKey(uint32_t *s, uint32_t *key , uint8_t nround ) //Операция исключающее или с раундовым ключем
{
 uint8_t cnt; //Счетчик

 for(cnt=0; cnt<Nb; cnt++) s[cnt]^=key[Nb*nround + cnt]; 
  
}
//==============================================================================
void SubBytes(uint8_t *s, uint8_t direct) //Функция замены байта в массиве
{
 uint8_t j; //Счетчик
 
 switch(direct)
 {
 case ENCRYPT:
   for(j=0; j<Nb*4; j++) s[j] = ByteSub((uint8_t) s[j]);
   break;
   
 case DECRYPT:
   for(j=0; j<Nb*4; j++) s[j] = InvByteSub((uint8_t) s[j]);
   break; 
 }
  
}

//==============================================================================
void ShiftRows(uint8_t *s, uint8_t direct) //Функция сдивига строк
{
   uint8_t temp[8]; //Временные данные
   uint8_t i , j; //Счетчики
   uint8_t shfts[3][4]; //Матрица позиций сдвига
   
   for(i=0;i<3;i++)
   {
     for(j=0;j<4;j++)
     {
       if(i==2 && j==3) shfts[i][j] = 4;
       else shfts[i][j] = j;
       
     }
   }
   
   
   for(i=0;i<4;i++)
   {
     for(j=0;j<Nb;j++) temp[j] = s[j*4+i];
        shiftrow(temp , shfts[Nb/2-2][i] , direct);
        
     for(j=0;j<Nb;j++) s[j*4+i] = temp[j];
   }
  
}

//==============================================================================
void shiftrow(uint8_t *row , uint8_t n , uint8_t direct) //Функция сдвига строки
{
  uint8_t t; 
  uint8_t i_row , j_row;
  
  if(n)
  {
    for(i_row=0; i_row<n; i_row++)
    {
      switch(direct)
      {
       
        case ENCRYPT:
          {
          t=row[0];
          for(j_row=1; j_row<Nb; j_row++) row[j_row-1] = row[j_row];
          row[Nb-1] = t;
          break;
          }
        case DECRYPT:
          {
          t=row[Nb-1];
          for(j_row=Nb-1; j_row>0; j_row--) row[j_row] = row[j_row-1];
          row[0] = t;
          break;
          
          }
            
               
      }
      
            
    }
    
  } 
  
}
//==============================================================================
void MixColums(uint32_t *s, uint8_t lenght, uint8_t direct) //Фукция перемешивания данных в колонке
{
  uint32_t m;
  uint8_t b[4];
  uint8_t cnt; //Счетчик 
  
  
  switch(direct)
  {
  case ENCRYPT:
    {
      m = Co;
      break;
    }
    
  case DECRYPT:
    {
     m = InvCo;
     break; 
    }
  }
  
  for(cnt=0; cnt<lenght; cnt++)
  {
    b[3] = product(m, s[cnt], BPOLY);
    m = ROTL24(m); 
    
    b[2] = product(m, s[cnt], BPOLY);
    m = ROTL24(m);
    
    b[1] = product(m, s[cnt], BPOLY);
    m = ROTL24(m);
    
    b[0] = product(m, s[cnt], BPOLY);
    m = ROTL24(m);
    
    s[cnt] = pack(b);
    
  }
  
  
}
//==============================================================================
void KeyExpansion(uint8_t *key , uint8_t direct) //Функция расширения ключа
{
 uint32_t fkey[N]; //Вспомогательный массив
 uint32_t temp,rcon=1; //Промежуточные переменные
 uint8_t i,j; //Счетчики
 
 for(i=0;i<Nk;i++) fkey[i]=pack(&key[i*4]); //Заполнение первых четырех слов  
 
 for(i=Nk;i<N;i++) //Заполнеие остальных элементов
 {
  temp = fkey[i-1];
  if(i % Nk ==0) 
  {
    temp = SubDWord(ROTL24(temp)) ^ rcon;
    rcon = (uint32_t)xtime((uint8_t )rcon , BPOLY);
  }
  fkey[i] = fkey[i-Nk] ^ temp;
  
 }
 
 for(i=0; i<N;i +=Nb) //Заполнение выходного массива
 {
   for(j=0; j<Nb; j++)
   {
     if(direct==ENCRYPT) rkey[i+j] = fkey[i+j] ; //Заполнение при шифровке 
     else if(direct==DECRYPT) rkey[i+j] = fkey[N-Nb-i+j] ; //Заполнение при  дешифровке
     
   }
   
 }
 
  
}
//==============================================================================
uint8_t product(uint32_t x, uint32_t y, uint8_t mod) //Вспомогательная функция перемешивающая данные в колонке
{
  uint8_t xb[4], yb[4];

  unpack(x, xb);  
  unpack(y, yb);
  
  return bmul(xb[0], yb[0], mod) ^ bmul(xb[1], yb[1], mod) ^ bmul(xb[2], yb[2], mod) ^ bmul(xb[3], yb[3], mod); 
}
//==============================================================================
uint8_t bmul(uint8_t a, uint8_t b, uint8_t mod) //Умножение элементов в поле GF(2^m)
{
  uint8_t t, s, u;
  
  u=b; t=a; s=0;
  
  while(u)
  {
    if(u & 1) s^=t;
    u>>=1;
    t = xtime(t, mod);       
  }
 return (s); 
}
//==============================================================================
uint32_t pack(uint8_t *b) //Преобразование 8-битных элементов массива в 32-битное слово
{
 return ((uint32_t)b[3]<<24) | ((uint32_t)b[2]<<16) | ((uint32_t)b[1]<<8) | (uint32_t)b[0];
}
//==============================================================================
void unpack(uint32_t a , uint8_t *b ) //Преобразование 32-битного слова в 8-битные элементы
{
  b[0] = (uint8_t)a;
  b[1] = (uint8_t)(a>>8);
  b[2] = (uint8_t)(a>>16);
  b[3] = (uint8_t)(a>>24);
}
//==============================================================================
uint32_t SubDWord(uint32_t a) //Функция замены словаиз таблицы
{
  uint8_t b[4];
  
  unpack(a,b);
  b[0]= ByteSub(b[0]);
  b[1]= ByteSub(b[1]);
  b[2]= ByteSub(b[2]);
  b[3]= ByteSub(b[3]);
  
  return pack(b);
  
}
//==============================================================================
uint8_t ByteSub(uint8_t x) //Функция замены байта
{
  return (subbytes[x]);
}
//==============================================================================
uint8_t InvByteSub(uint8_t x) //Функция обратной замены байта
{
  return (invsubbytes[x]);
 
}
//==============================================================================
uint8_t xtime(uint8_t a, uint8_t mod) //Функция смещения байта
{
  return ((a&0x80) ? a<<1^mod : a<<1);
}



/***************************КОНЕЦ ФАЙЛА****************************************/

