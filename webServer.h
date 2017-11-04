#ifndef WEBSERVER_H_
#define WEBSERVER_H_


#include <ESP8266WebServer.h>
#include "myiot_timer_system.h"
#include "DeviceConfig.h"

class WebServer : public MyIOT::ITimer 
{
  public:

   WebServer():server(80){}

   void setup(DeviceConfig& rconfig)
   {
     config = &rconfig;
    
     server.on("/", [this](){ this->printStatus();} );
     server.on("/save", [this](){ this->handleSave();} );
     server.on("/reset", [this](){ this->handleReset();} );
     server.begin();
   }
  
   virtual void expire()
   {
     server.handleClient();
   }

   virtual void destroy(){}

   private:

   void printStatus()
   {
    String html1 = "<!DOCTYPE html>\r\n<html>\r\n\
<head><title>Configuration " + String(config->getDeviceName())+ "</title></head>\r\n\
<body><form action=\"save\" method=\"GET\">\
  DeviceName <INPUT type=\"text\" name=\"deviceName\" value=\"" + String(config->getDeviceName()) + "\"><br> \
  MQTT Server <INPUT type=\"text\" name=\"mqttServer\" value=\"" + String(config->getMqttServer()) + "\"><br> \
  <INPUT type=\"submit\" value=\"Save\"><br>\
  </form>\
  <form action=\"reset\" method=\"GET\"><INPUT type=\"submit\" value=\"Reset\"><br></form>\
</body> \
</html>";
     server.send(200, "text/html", html1);
   }

  void handleSave()
  {
    String deviceName = server.arg("deviceName");
    String mqttServer = server.arg("mqttServer");
    config->setDeviceName(deviceName.c_str());
    config->setMqttServer(mqttServer.c_str());
    config->save();

        String html1 = "<!DOCTYPE html>\r\n<html>\r\n\
<head><title>Configuration " + String(config->getDeviceName())+ "</title></head>\r\n\
<body>Values saved<br>\
<a href=\"/\">Show Values</a><br>\
<a href=\"/reset\">Reset Device</a><br>\
</body></html>\
";
    server.send(200, "text/html", html1);
  }

  void handleReset()
  {
    String html1 = "<!DOCTYPE html>\r\n<html>\r\n\
<head><title>Configuration " + String(config->getDeviceName())+ "</title></head>\r\n\
<body>Reset device ...<br>\
<a href=\"/\">Show Values</a><br>\
</body></html>\
";

    server.send(200, "text/html", html1);
    delay(1000);
    ESP.reset();
    delay(1000);    
  }

  ESP8266WebServer server; 
  DeviceConfig* config;
} webServer;

#endif
