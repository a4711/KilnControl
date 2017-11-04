#include <ESP8266WiFi.h>
#include "TemperatureControl.h"


TemperatureControl::TemperatureControl() :
		thermocouple(MAXCLK, MAXCS, MAXDO) {
}

void TemperatureControl::setup(
		std::function<void(const char* topic, const char* message)> fpublish) {
	publish = fpublish;
	pinMode(SSR, OUTPUT);
}

void TemperatureControl::update_desired(double newValue) {
	aging = 0;
	desired = newValue;
}

void TemperatureControl::expire() {
	check_aging();
	bool doHeating = false;
	double actual = getTemperature();
	if (isnan(actual)) {
		char msg[100];
		snprintf(msg, sizeof(msg), "Something wrong with thermocouple.");
		publish_error(msg);
		Serial.println(msg);
	} else {
		double delta = actual - previous;
		double correction = 0.0;
		if (delta > 0) {
			correction = delta * 3.5; // rising temperature
		} else {
			correction = delta * 0.5; // faling temperature
		}

#ifdef ORIGINAL_CODE
		doHeating = actual < desired;
#else
		doHeating = (actual + correction) < desired; // if temperature rises quickly, we stop heating before we reach the desired value
#endif
		publish_actual(actual);
		publish_status(actual, getInternalTemperature(), doHeating, delta);
		aging++;
		previous = actual; // remember the last temperature
	}
	digitalWrite(SSR, doHeating ? HIGH : LOW);
}

void TemperatureControl::destroy() {
}

double TemperatureControl::getTemperature() {
	for (int i = 0; i < 5; i++) {
		double actual = thermocouple.readCelsius();
		if (isnan(actual)) {
			continue;
		}
		return actual;
	}
	return NAN;
}

double TemperatureControl::getInternalTemperature() {
	for (int i = 0; i < 5; i++) {
		double actual = thermocouple.readInternal();
		if (isnan(actual)) {
			continue;
		}
		return actual;
	}
	return NAN;
}

void TemperatureControl::check_aging() {
	if (aging > MAX_AGE) {
		desired = 0.0;
	}
}

void TemperatureControl::publish_actual(double actual) {
	if (!publish)
		return;

	Serial.print("C = ");
	Serial.println(actual);
	char buffer[256] = { 0 };
	int d_actual = actual;
	snprintf(buffer, sizeof(buffer), "%d", d_actual);
	publish("actual", buffer);
}

void TemperatureControl::publish_status(double actual, double internal,
		bool doHeating, double delta) {
	if (!publish)
		return;

    IPAddress localIp = WiFi.localIP();
	char buffer[1024] = { 0 };
	snprintf(buffer, sizeof(buffer),
			"{\"actual\":%d, \"internal\":%d, \"desired\":%d, \"aging\":%u, \"doHeating\":%u, \"delta\":%d, \"ip\":\"%d.%d.%d.%d\"}",
			(int) (actual), (int) (internal), (int) (desired), aging,
			(doHeating ? 1 : 0), (int) (delta),
			localIp[0],localIp[1],localIp[2],localIp[3]
	);
	publish("status", buffer);
}

void TemperatureControl::publish_error(const char* msg) {
	if (!publish)
		return;

	char buffer[256] = { 0 };
	snprintf(buffer, sizeof(buffer), "{\"error\":%s}", msg);
	publish("status", buffer);
}
