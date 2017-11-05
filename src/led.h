#ifndef BLINK_H
#define BLINK_H


#include "myiot_timer_system.h"
class Blink : public MyIOT::ITimer
{
  public:
  Blink(int xpin = LED_BUILTIN):state(0),pin(xpin){}
  void setup()
  {
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(pin, OUTPUT);    
  }
  virtual void expire()
  {
    digitalWrite(pin, 0 != state%2 ? HIGH : LOW); 
    state++;
  }
  virtual void destroy(){}

  private:
    int state;
    int pin;
} blink(2);

#endif
