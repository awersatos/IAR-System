/*******************************************************************************
********************************************************************************
**                                                                            **
**          БИБЛИОТЕКА ФУНКЦИЙ МОДУЛЯ ТРАНСИВЕРА CC1101 433 МГц               **
**                                                                            **
********************************************************************************
*******************************************************************************/

//******************Подключаемые файлы******************************************
#include "Transceiver_433MHz.h"
#include "main.h"
#include "stm32f10x.h"

//*************Инициализация глобальных переменных******************************
char SPI_buffer[spi_bufer_size]; //Буфер приема и передачи трансивера
FunctionalState TX_state = DISABLE; //Статус передачи

union u_type adres;
// Массив инициализации регистров трансивера(старший байт - адрес, младший - значение)
/*
Частота кварца: 27.000 МГц
Модуляция: 2-FSK
Опорная частота: 432.999893 МГц
Разнос каналов: 199.813843 КГц
Номер канала: 47
Несущая частота: 442.390732 МГц
Девиация: 4.943848 КГц
Полоса пропускания приемника: 210.937500 КГц
Скорость передачи: 1.2 Кбод
*/
const uint16_t INIT_REG[35]=
{
 0x0006, //0 IOGFG2 Обнаружение несущей
 0x0206, //1 IOGFG0 Прием-передача пакета
 0x06FF, //2 PKTLEN Длинна пакета
 0x0704, //3 PKRCTRL1 Контроль пакета
 0x0805, //4 PKRCTRL0 Контроль пакета
 0x0901, //5 ADDR Адрес устройства
 0x0A2F, //6 CHANNR Номер канала
 0x0B06, //7 FSCTRL1 Параметры контроля синтезатора частоты
 0x0C00, //8 FSCTRL0 Параметры контроля синтезатора частоты
 0x0D10, //9 FREQ2 Параметы опорной частоты
 0x0E09, //10 FREQ1 Параметы опорной частоты
 0x0F7B, //11 FREQ0 Параметы опорной частоты
 0x1085, //12 MDMCFG4 Конфигурация модема
 0x1178, //13 MDMCFG3 Конфигурация модема
 0x1203, //14 MDMCFG2 Конфигурация модема
 0x1302, //15 MDMCFG1 Конфигурация модема
 0x14E5, //16 MDMCFG0 Конфигурация модема
 0x1514, //17 DEVIATION Девиация
 0x1730, //18 MCSM1 Конфигурация автомата контроля радио
 0x1818, //19 MCSM0 Конфигурация автомата контроля радио
 0x1916, //20 FOCCFG Компенсация сдвига частоты
 0x1A6C, //21 BSCFG Конфигурация побитовой синхронизации
 0x1BC0, //22 AGCCTRL2 Пармаметры приемного тракта
 0x1C00, //23 AGCCTRL1 Пармаметры приемного тракта 
 0x1DB2, //24 AGCCTRL0 Пармаметры приемного тракта
 0x21B6, //25 FREND1 Параметры приемного тракта
 0x2210, //26 FREND0 Параметры передающего тракта
 0x23E9, //27 FSCAL3 Параметры калибровки синтезатора частоты
 0x242A, //28 FSCAL2 Параметры калибровки синтезатора частоты
 0x2500, //29 FSCAL1 Параметры калибровки синтезатора частоты 
 0x261F, //30 FSCAL0 Параметры калибровки синтезатора частоты
 0x2959, //31 FSTEST Проверка синтезаторы частоты
 0x2C81, //32 TEST2
 0x2D35, //33 TEST1
 0x2E09  //34 TEST0
};

//*************Функции для работы с трансивером   ******************************

char SPI_SEND( char data) //Функция приема/передачи символа по SPI
{ 
  char rx_data;
  
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); //Ожидание очистки буфера передачи
  SPI_I2S_SendData(SPI1, data); //Загрузка данныхх в буфер
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET); //Ожидание окончания приема/передачи
  rx_data = SPI_I2S_ReceiveData(SPI1); //Получение принятых данных
  return  rx_data; 
}

void RESET_TR(void) //Сброс трансивера по включению питания
{
  GPIO_InitTypeDef GPIO_Init_struct; // Обявляем структуру инициализации портов
  
  SPI_Cmd(SPI1, DISABLE); //Отключение SPI
  
  /*Переводим выводы SCK и MOSI в состояние обычных двухтактных выходов*/
  GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
  GPIO_Init_struct.GPIO_Pin = SPI_SCK | SPI_MOSI; //Выбираем выходы SCK и MOSI
  GPIO_Init_struct.GPIO_Speed= GPIO_Speed_50MHz; //Устанавливаем частоту
  GPIO_Init_struct.GPIO_Mode=GPIO_Mode_Out_PP; //Устанавливаем режим работы двухтактный выход
  GPIO_Init(SPI_PORT, &GPIO_Init_struct);      //Передаем структуру в функцию
  
  GPIO_SetBits(SPI_PORT , SPI_SCK); //Устанавливаем 1 на SCK
  GPIO_ResetBits(SPI_PORT , SPI_MOSI); //Устанавливаем 0 на MOSI
  GPIO_ResetBits(SPI_PORT , SPI_SS); //Активируем SPI_SS
  delay_ms(1);//Задержка в 1 микросекунду
  GPIO_SetBits(SPI_PORT , SPI_SS); //Деактивируем SPI_SS
  delay_ms(40);
  
  /*Переводим выводы SCK и MOSI в состояние выходов альтернативной функции*/
  GPIO_StructInit(&GPIO_Init_struct);       //Инициализируем структуру начальными значениями
  GPIO_Init_struct.GPIO_Pin = SPI_SCK | SPI_MOSI; //Выбираем выходы SCK и MOSI
  GPIO_Init_struct.GPIO_Speed= GPIO_Speed_50MHz; //Устанавливаем частоту
  GPIO_Init_struct.GPIO_Mode=GPIO_Mode_AF_PP; //Устанавливаем режим работы  выход альтернативной функции
  GPIO_Init(SPI_PORT, &GPIO_Init_struct);      //Передаем структуру в функцию
  
  SPI_Cmd(SPI1, ENABLE ); //Включение SPI
  
  GPIO_ResetBits(SPI_PORT , SPI_SS); //Активируем SPI_SS
  while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //Ждем 0 на MISO
  SPI_SEND(SRES);//Софтовый сброс трансивера       
  GPIO_SetBits(SPI_PORT , SPI_SS); //Деактивируем SPI_SS
  
  
}

void WRITE_TR_REG( uint16_t reg) // Функция записи регистра
{
   union u_type dat; //Объявляем переменную объединенного типа
   
  GPIO_ResetBits(SPI_PORT , SPI_SS); //Активируем SPI_SS
  while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //Ждем 0 на MISO 
  
  dat.halfword=reg;
  SPI_SEND(dat.byte[1]);  //Адрес регистра 
  SPI_SEND(dat.byte[0]);  //Значение регистра 
  delay_ms(1);
  GPIO_SetBits(SPI_PORT , SPI_SS); //Деактивируем SPI_SS
}

char READ_TR_REG( char adr)  // Функция чтения регистра
{
 char reg; // Объявляем переменную
 
 GPIO_ResetBits(SPI_PORT , SPI_SS); //Активируем SPI_SS
 while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //Ждем 0 на MISO 

 SPI_SEND(adr + 0x80);   // Старший бит определяет операцию  
 reg= SPI_SEND(0x00); //Присваеваем переменной значение регистра транивера
 delay_ms(1);
 
 GPIO_SetBits(SPI_PORT , SPI_SS); //Деактивируем SPI_SS
 
 return reg; //Возвращаем значение региста
  
  
}
   
void STROB(char strob)  //Запись строб-команды
{
GPIO_ResetBits(SPI_PORT , SPI_SS); //Активируем SPI_SS
while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //Ждем 0 на MISO 

SPI_SEND(strob); //Отправка команды

GPIO_SetBits(SPI_PORT , SPI_SS); //Деактивируем SPI_SS
  
}

void WRITE_PATABLE(void)    //Запись таблицы мощности
{
GPIO_ResetBits(SPI_PORT , SPI_SS); //Активируем SPI_SS
while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //Ждем 0 на MISO

WRITE_TR_REG(OUTPUT_POWER); //Запись значения выходной мощности передатчика
  
GPIO_SetBits(SPI_PORT , SPI_SS); //Деактивируем SPI_SS
}

char STATUS_TR(void) //Определение статуса трансивера
{
char st;
 
GPIO_ResetBits(SPI_PORT , SPI_SS); //Активируем SPI_SS
while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //Ждем 0 на MISO

st=SPI_SEND(SNOP);

GPIO_SetBits(SPI_PORT , SPI_SS); //Деактивируем SPI_SS
return st;
}


void SEND_PAKET(void) //Функция передачи пакета
{
uint8_t tr_cnt; //Счетчик
  
STROB(SIDLE);  //Переход в режим IDLE
STROB(SFRX);  //Очистка приемного буфера
STROB(SFTX); //Очистка передающего буфера
delay_ms(1);

GPIO_ResetBits(SPI_PORT , SPI_SS); //Активируем SPI_SS
while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //Ждем 0 на MISO

SPI_SEND(0x7F);   //Открытие буфера на запись
SPI_SEND(SPI_buffer[0]); //Запись длинны пакета
for (tr_cnt=1;tr_cnt<SPI_buffer[0];tr_cnt++)  //Запмсь пакета
  {
   SPI_SEND(SPI_buffer[tr_cnt]); 
  }
GPIO_SetBits(SPI_PORT , SPI_SS); //Деактивируем SPI_SS

TX_state=ENABLE; //Передача активна

STROB(STX); //Включение передачи

while(GPIO_ReadInputDataBit(SPI_PORT, TR_GP1 )==0); //Ожидание начала передачи
while(GPIO_ReadInputDataBit(SPI_PORT, TR_GP1 )==1); //Ожидание конца передачи

STROB(SIDLE);  //Переход в режим IDLE
STROB(SFRX);  //Очистка приемного буфера
STROB(SFTX); //Очистка передающего буфера

TX_state=DISABLE; //Передача закончена

CLEAR_SPI_buffer(); //Очистка буфера  
}

void RECEIVE_PAKET(void) //Функция приема пакета
{
  uint8_t tr_cnt; //Счетчик 
  
CLEAR_SPI_buffer(); //Очистка буфера
  
STROB(SIDLE);  //Переход в режим IDLE

GPIO_ResetBits(SPI_PORT , SPI_SS); //Активируем SPI_SS
while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //Ждем 0 на MISO

SPI_SEND(0xFF);  //Открытие буфера приема
SPI_buffer[0] = SPI_SEND(0x00); //Считывание длинны пакета

for (tr_cnt=1;tr_cnt<SPI_buffer[0];tr_cnt++)    //Считывание пакета
   {
   SPI_buffer[tr_cnt]=SPI_SEND(0x00);
   } 
GPIO_SetBits(SPI_PORT , SPI_SS); //Деактивируем SPI_SS

STROB(SFRX);  //Очистка приемного буфера
  
}

 void CLEAR_SPI_buffer(void) //Очистка SPI буфера
 { 
   uint8_t tr_cnt2; //Счетчик
 
   for (tr_cnt2=0;tr_cnt2<spi_bufer_size;tr_cnt2++)
   {
    SPI_buffer[tr_cnt2]=0x00;
   }
 }

void Transceiver_Configuration(void) //Инициализация трансивера
{
  uint8_t tr_cnt3; //Счетчик
  
  RESET_TR(); //Сброс трансивера
  delay_ms(20); 
  for (tr_cnt3=0;tr_cnt3<35;tr_cnt3++) WRITE_TR_REG(INIT_REG[tr_cnt3]); //Запись конфигурационных регистров трансивера
  
  WRITE_PATABLE(); //Запись таблицы мощности
  STROB(SIDLE); //Переход в режим IDLE
  STROB(SFRX);  //Очистка приемного буфера
  STROB(SFTX); //Очистка передающего буфера
  //-----------------------------------------------------------------
/*
 delay_ms(1000); 
GPIO_ResetBits(SPI_PORT , SPI_SS); //Активируем SPI_SS
while(GPIO_ReadInputDataBit(SPI_PORT, SPI_MISO )==1); //Ждем 0 на MISO
SPI_SEND(0xC0);

  
for (tr_cnt3=0;tr_cnt3<35;tr_cnt3++)
{
  
 adres.halfword=INIT_REG[tr_cnt3]; 
 SPI_buffer[tr_cnt3]=READ_TR_REG(adres.byte[1]) ;
 delay_ms(1);
  
  //SPI_buffer[tr_cnt3]=SPI_SEND(0x00);
}

//GPIO_SetBits(SPI_PORT , SPI_SS); //Деактивируем SPI_SS

//SPI_buffer[47]=STATUS_TR();
//SPI_buffer[48]=READ_TR_REG(0x06) ;
*/
//----------------------------------------------------------------------

#ifdef BLACKBOX //Если устройство BLACKBOX
  STROB(SRX);  //Переход в режим приема
  STATUS.Transceiver_Status = ENABLE;

  
#endif 
  
  Transceiver_Status = ENABLE;
  delay_ms(3);

 
  CLEAR_SPI_buffer(); //Очистка SPI буфера
}


/***************************КОНЕЦ ФАЙЛА****************************************/