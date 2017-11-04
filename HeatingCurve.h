#ifndef HEATING_CURVE_H
#define HEATING_CURVE_H

#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include "myiot_timer_system.h"
#include "myiot_heating.h"


class HeatingCurve : public MyIOT::ITimer
{
public:
  HeatingCurve():running(false), startTimeMs(0) {}

  void setup(const std::function<void(const char* topic, const char* message)>& fpublish)
  {
    create_curve("test");

    /* ********************* */
    publish = fpublish;
  }

  void start_curve(const char* url)
  {
	  create_curve(url);
	  start();
  }

  void start()
  {
    startTimeMs = millis();
    running = true;
  }
  
  void stop()
  {
    running = false;
    publish_desired(0.0);
  }
  
  virtual void expire()
  {
    if (running)
    {
      unsigned long dt = millis() - startTimeMs;
      double desired = 0.0;
      
      if (!curve.calculate( dt, desired))
      {
        running = false;
      }
      publish_desired(desired);
    }
  } 
  
  virtual void destroy(){}
private:

  void publish_desired(double desired)
  {
    if (!publish) return;
    char buffer[256] = {0};
    int d_desired = desired;
    int prec_desired = (desired - d_desired) * 100;
    snprintf(buffer, sizeof(buffer), "%d.%d", d_desired, prec_desired);
    publish("desired", buffer);
  }

	void create_curve(const char* url = nullptr) {
		curve.setStartTemperature(20.0);
		curve.clear();

		if (nullptr == url)
		{
			curve.add(MyIOT::Heating::Interval(MyIOT::Heating::Hours(0), 0.0)); // do nothing
		}
		else if (0 == strcmp("test", url))
		{
			/** Define a Test Curve
			 **/
			/*  Schrüh-Brand: */
			curve.add(MyIOT::Heating::Interval(MyIOT::Heating::Hours(5), 600.0)); // ~ 125 °/h
			curve.add(MyIOT::Heating::Interval(MyIOT::Heating::Hours(1, 30), 950.0)); //  350 °/(1.5h)
			curve.add(MyIOT::Heating::Interval(MyIOT::Heating::Hours(0, 30), 950.0));
		}
		else
		{
			Serial.print("loading curve: ");
			Serial.println(url);
			// fetch curve
			 HTTPClient http;
			 http.begin(url);
			 int httpCode = http.GET();
			 if(httpCode != HTTP_CODE_OK)
			 {
				 Serial.print("failed to fetch url: ");
				 Serial.println(url);
				 create_curve(nullptr);
				 return;
			 }
			 String payload = http.getString();

   		     // parse curve
			 DynamicJsonBuffer jsonBuffer;
			 JsonObject& json = jsonBuffer.parseObject(payload);
			 if (!json.success())
			 {
				 Serial.print("failed to parse json: ");
				 Serial.println(url);
				 create_curve(nullptr);
				 return;
			 }
			 /*

{
    "curve": [
        {
            "h": 5,
            "m": 0,
            "s": 0,
            "t": 600
        },
        {
            "h": 1,
            "m": 30,
            "s": 0,
            "t": 950
        },
        {
            "h": 0,
            "m": 25,
            "s": 0,
            "t": 950
        }
    ]
}
			  * */
  		     Serial.println("analyse JSON ...");

			 JsonArray& data = json["curve"].as<JsonArray>();
			 // create curve
			 for (JsonArray::iterator it = data.begin(); it != data.end(); ++it)
			 {
				 int h = 0;
				 int m = 0;
				 int s = 0;
				 double t = 0.0;
				 JsonObject& interval = it->as<JsonObject>();

				 if ( interval.containsKey("h") )
				 {
					 h = ::atoi(interval["h"]);
					 Serial.print("h:"); Serial.print(h);
				 }
				 if (interval.containsKey("m"))
				 {
					 m = ::atoi(interval["m"]);
					 Serial.print(", m:"); Serial.print(m);
				 }
				 if (interval.containsKey("s"))
				 {
					 s = ::atoi(interval["s"]);
					 Serial.print(", s:"); Serial.print(s);
				 }
				 if (interval.containsKey("t"))
				 {
					 t = ::atof(interval["t"]);
					 Serial.print(", t:"); Serial.print((int)t);
				 }
				 Serial.println();
				 curve.add(MyIOT::Heating::Interval(MyIOT::Heating::Hours(h,m,s), t));
			 }
		}
	}

  MyIOT::Heating::Curve curve;

  std::function<void(const char* topic, const char* message)> publish;
  bool running;
  unsigned long startTimeMs;

} heatingCurve;

#endif
