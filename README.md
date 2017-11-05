# KilnControl
Control a Kiln with ESP8266.
This project is based on MQTT. I am using node-red to create a front end to the device.
The heating curve can be specified by simple JSON format.

The ESP modul can be configured via WifiManager.
You have to configure your local WIFI settings, the MQTT server and the "DeviceName".
The program creates a simple web server (port 80) where you can change the "DeviceName" and the MQTT server.

The following messages are distributed via MQTT:
* status : JSON string containing multiple information: 
    "actual", "internal", "desired" and "doHeating"
    
 The program subscribes for the following messages:
 * start : the message can contain any url providing a heating curve in JSON format 
 * stop : stop the current heating process 

Heating Curve
-------------

The JSON object has to have an attribute "curve" which is an array of intervals.
Each interval has attributes: "h" (hours), "m" (minutes), "s" (seconds) and "t" (temperature).
"h", "m" and "s" specify the duration of the interval and "t" is the end temperature of the interval.
(Internally the program starts with 20°C, which is hard coded.)

Example:

    {
        "curve": [
            {
                "h": 0,
                "m": 5,
                "s": 0,
                "t": 600
            },
            {
                "h": 0,
                "m": 5,
                "s": 0,
                "t": 950
            }
        ]
    }