
#include "DeviceConfig.h"

#include "myiot_timer_system.h"
#include "button_control.h"
#include "ota.h"
#include "led.h"
#include "TemperatureControl.h"
#include "Mqtt.h"
#include "HeatingCurve.h"
#include "webServer.h"

MyIOT::TimerSystem tsystem;
Mqtt mqtt;
DeviceConfig config;
TemperatureControl temperatureControl;
/*
  Ideensammlung:

    * Heizkurven über HTTP beziehen.
      MQTT "start" könnte als message die URL haben.
      -> 2017-11-04, erledigt

    * Parametrierung (config) des Device-Namens und des MQTT Servers auch anders erlauben (z.B. HTTP)
      -> 2017-11-04, erledigt mit "webServer.h"

    * Genauigkeit im MQTT "status" auf Nachkommastellen erhöhen.

    * Hardware: im Ofen das Relais mittels SSR über den ESP8266 steuern

    * Hardware: Display zur Anzeige von: aktuelle Temperature, Sollwert, Heizung (an/aus), verbleibende Zeit ...

    * Ab 800° ist das vorzeitige Bremsen unsgünstig, weil der Ofen sehr, sehr schnell abkühlt. (Vielleicht sollte man die Bremsung ab 800° einfach weg lassen.
      Mann könnte auch verschieden Korrekturen abhängig vom Temperaturbereich wählen.



*/


void setup() {
   Serial.begin(115200);

   config.setup();

   Serial.println(config.getDeviceName());
   Serial.println(config.getMqttServer());

   mqtt.setup(config.getDeviceName(), config.getMqttServer());
   std::function<void(const char* topic, const char* message)> publish = [](const char* topic, const char* message){mqtt.publish(topic,message);};

   if (!mqtt.subscribe("fire", [](const char* message) { Serial.print("fire action "); Serial.println(message); }))
   {
     Serial.println("failed to add fire subscription");
   }

   if (!mqtt.subscribe("desired", [](const char* message) { temperatureControl.update_desired( atof(message) ); }))
   {
     Serial.println("failed to add desired subscription");
   }

   if (!mqtt.subscribe("start", [](const char* message) { heatingCurve.start_curve(message);  }))
   {
     Serial.println("failed to add start subscription");
   }

   if (!mqtt.subscribe("stop", [](const char* message) { heatingCurve.stop(); Serial.println(message);}))
   {
     Serial.println("failed to add stop subscription");
   }

   webServer.setup(config);
   ota.setup(config.getDeviceName());
   blink.setup();
   buttonControl.setup(publish);
   temperatureControl.setup(publish);
   heatingCurve.setup([](const char* topic, const char* message) {temperatureControl.update_desired(atof(message)); ; Serial.println(topic); });


   tsystem.add(&ota, MyIOT::TimerSystem::TimeSpec(0,10e6));
   tsystem.add(&webServer, MyIOT::TimerSystem::TimeSpec(0,10e6));
   tsystem.add(&blink, MyIOT::TimerSystem::TimeSpec(0,200000000));
   tsystem.add(&buttonControl, MyIOT::TimerSystem::TimeSpec(0,100000000));
   tsystem.add(&temperatureControl, MyIOT::TimerSystem::TimeSpec(5))  ;
   tsystem.add(&mqtt, MyIOT::TimerSystem::TimeSpec(0,100000000));
   tsystem.add(&heatingCurve, MyIOT::TimerSystem::TimeSpec(1));

}



void loop() {
  tsystem.run_loop(10,1);
}
