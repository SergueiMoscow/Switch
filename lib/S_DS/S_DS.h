#ifndef _S_DS_
#define _S_DS_
#include <OneWire.h>
#define max_DS 10
class S_DS
{
  private:
    OneWire ds;
    int kol_DS;

  public:
  byte addr[max_DS][8];//Создал массив: строки-кол-во датчиков по 8 байт в каждой для хранения адреса(ROM, ID) датчика
  int present = 0;
  byte data[max_DS][12];//Создал массив: строки-ном датчика по 12 байт с температурой
  int type_s;
  byte Gl_buffer_save[16];//Буфер (0-16) сбора информации
    S_DS();
	int Find_DS18b20();
  int col_ds();
  float getTemp(int datchik);
 
};
#endif