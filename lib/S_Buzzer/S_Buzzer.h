#ifndef _S_BUZZER_
#define _S_BUZZER_

#include <Arduino.h>

class S_Buzzer {
private:
    int pin;
    bool active;

public:
    S_Buzzer(int pin);
    void begin();
    void on();  // Включает зуммер
    void off(); // Выключает зуммер
    bool isActive(); // Проверяет состояние
};

#endif
