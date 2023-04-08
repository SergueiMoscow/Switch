#include "S_DS.h"

S_DS::S_DS(){
  ds=OneWire(D4);
  kol_DS=Find_DS18b20();
	return;
};

int S_DS::Find_DS18b20()
{
    //Serial.println("Initialising DS18B20...");

  for (int i = 0; i < max_DS; i++) 
  {
    //Serial.println("Initialising DS18B20...");
    kol_DS=i;
    if (!ds.search(addr[i]))
    {
      //Serial.println("ErrInit1w");//инициализация не выполнена:DS18B20 не найдены
      return 0;
    }
    else //датчики найдены
    {
//**********вывести номер датчика и его ROM ******     
      Serial.print("Temp. Sensor ");
      Serial.print(i);
      Serial.print(": ");
      for (int j = 0; j < 8; j++)
      {
        Serial.print(addr[i][j], HEX);
        Serial.print("; ");
        //Serial.print(addr[i][j], HEX);
      }
//      Serial.println();
//**************вывел ROM**********
    }//конец else
//    Serial.println("...done!");
    if (OneWire::crc8(addr[i], 7) != addr[i][7]) 
    {
      //Serial.println("ErrCRC");//CRC Failed!
      return i;
    }
    if (addr[i][0] != 0x28)
    {
      //Serial.println("notDS18B20");//"OW Device is not DS18B20!"
    }
  }; //КонецЦикла
  return 0;
}; // end S_DS:Find_DS18b20

int S_DS::col_ds()
{
  return kol_DS;
}

float S_DS::getTemp(int datchik)
{
  float retValue=131.0;
  ds.reset();
  //Serial.println(addr[datchik]);
  ds.select(addr[datchik]);
  ds.write(0x44, 1);
  delay(1000);
  present = ds.reset();
  ds.select(addr[datchik]);
  ds.write(0xBE);
  for ( byte i = 0; i < 9; i++)
  {
    data[datchik][i] = ds.read();//сохраняем температуру(9 байт) в массиве соответствующего датчика
//    Serial.print(data[datchik][i]);
  }
  //Serial.println();

    if ( OneWire::crc8( addr[datchik], 7) != addr[datchik][7])
    {
      Serial.println("ErrCRC");
      //если контрольная сумма не совпала с тем, что мы приняли – работать с такими данными нельзя.
      //Поэтому запишем в массив с результатом такие значения, которых в нормальных условиях мы получить не сможем
      //return;
      retValue = 131;//придумать получше вариант
    }
    else
    {
      int16_t raw = (data[datchik][1] << 8) + data[datchik][0];//Тип 16-разрядных целых
      //byte cfg = (data[datchik][4] & 0x60);
      //if (cfg == 0x00) raw = raw & ~7; // 9 bit resolution, 93.75 ms
      //else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
      //else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms

      float celsius = (float)raw / 16.0+(float(raw%16))*1.0/16; //16.0 будет 2 знака после запятой. Мне надо целое
      //byte s = 0;
      retValue = celsius;
      //для отладки
//      Serial.print("1051  t=");
//      Serial.print(Gl_buffer_save[n_ds18xx + 5]);
//      Serial.println("  ");
    }
   return retValue; 
}
