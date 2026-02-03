#ifndef IRSEND_H
#define IRSEND_H 

#include <Arduino.h>

class IRsend
{
public:
    IRsend(int pin);
    void liga();
    void desliga();
    void aumenta();
    void diminui();

private:
    int IRledPin;
};

#endif