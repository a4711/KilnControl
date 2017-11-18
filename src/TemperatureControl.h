#ifndef TEMPERATURE_CONTROL_H
#define TEMPERATURE_CONTROL_H

#include <functional>
#include "Adafruit_MAX31855.h"
#include "myiot_timer_system.h"

class TemperatureControl : public MyIOT::ITimer
{
private:
  enum {
    MAXDO = 13,
    MAXCS = 12,
    MAXCLK= 14,
    SSR = 16,  // the output which controls the physical heating (solid state relay).

    MAX_AGE = 5, // if "aging" reaches MAX_AGE, the "desired" value is set to 0.0 (which will turn off heating)
  };

public:
	TemperatureControl();

	void setup(
			std::function<void(const char* topic, const char* message)> fpublish);

	void update_desired(double newValue);
  
  virtual void expire();

	virtual void destroy();

  private:
	double getTemperature();

	double getInternalTemperature();

	bool isDesiredValid() const;

	void check_aging();

	void publish_actual(double actual);
  
    void publish_status(double actual, double internal, bool doHeating,
			double delta);    

    void publish_error(const char* msg);
  
    Adafruit_MAX31855 thermocouple;
  
    unsigned int aging = MAX_AGE+1; // counter, which is incremented each cycle; when receiving a mqtt desired update, the counter is reset.
    double desired = 0.0;     // the temperature which should be reached (given my mqtt: <devicename>/desired
    double previous = 0.0;    // temperature of the previous measurement cycle

    std::function<void(const char* topic, const char* message)> publish;
    

};

#endif
