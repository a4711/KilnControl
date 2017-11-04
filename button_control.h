#ifndef BUTTON_CONTROL_H
#define BUTTON_CONTROL_H

#include "myiot_timer_system.h"
#include <ESP8266WiFi.h>

class ButtenControl : public MyIOT::ITimer
{
  const static unsigned long TEN_SECONDS = 10e3;
  const static unsigned long THREE_SECONDS = 3e3;

  static const int PIN = 5;
 
public:
  ButtenControl():start(0), measuring(false){}
  void setup(const std::function<void(const char* topic, const char* message)>& fpublish)
  {
    publish = fpublish;
    pinMode(PIN, INPUT); 
  }  
  virtual void expire()
  {
    bool high = digitalRead(PIN);
    if (!high)
    {
      if (!measuring)
      {
        measuring = true;
        start = millis();
      }
    }
    else
    {
      if (measuring)
      {
        measuring = false;
        unsigned long hold_time = millis() - start;

        if (hold_time > TEN_SECONDS)
        {
          Serial.println("Reset Flash");
          WiFi.disconnect();
          delay(1000);
          ESP.reset();
          delay(1000);
        }
        else if (hold_time > THREE_SECONDS)
        {
          Serial.println("Reset Device");
          ESP.reset();
          delay(1000);        }
        else
        {
          const char* msg = "Button pressed";
          Serial.println(msg);
          if (publish) publish("button", msg);
        }
      }
    }
  }
  virtual void destroy(){}
  private:

    std::function<void(const char* topic, const char* message)> publish;

    unsigned long start;
    bool measuring;
} buttonControl;

#endif
