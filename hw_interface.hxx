#ifndef HW_INTERFACE_HXX
#define HW_INTERFACE_HXX

#include <wiringPi.h>
#include <mcp23017.h>

#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#define MCP_BASE 100
#define RELAY_PIN 103
#define MOTION_SENSOR_PIN 0

// Ignore sensor for x seconds after off light is called
#define SENSOR_TIMEOUT 15

using namespace std;

class HwInterface {
public:
	HwInterface();
	void onRelay();
	void offRelay();
	void toggleRelay();
	void setSensorState(bool);
	void parseQuery(string query);

private:
	boost::mutex mtx_sensor;
	boost::mutex mtx_relay;

	bool sensorState;
	bool relayState;

	bool getSensorState();
	void togglePin(int, int);

	void sensorThread(bool delayStartup);
};

HwInterface::HwInterface() : sensorState(true), relayState(false)
{
	wiringPiSetup();
	mcp23017Setup(MCP_BASE, 0x20);
	
	pinMode(RELAY_PIN, OUTPUT);
	pinMode(MOTION_SENSOR_PIN, INPUT);

	boost::thread t{&HwInterface::sensorThread, this, false};
}

void HwInterface::sensorThread(bool delayStartup) {
	if(delayStartup)
		boost::this_thread::sleep_for(boost::chrono::seconds{SENSOR_TIMEOUT});
	while(getSensorState()){
		if(digitalRead(MOTION_SENSOR_PIN) == 1){
			onRelay();
			while(digitalRead(MOTION_SENSOR_PIN) == 1);
		}
		//Free up some CPU time
		boost::this_thread::sleep_for(boost::chrono::seconds{1});
	}
}

void HwInterface::parseQuery(string query){
	boost::algorithm::to_lower(query);
	if(query == "on light" || query == "on the light"){
		onRelay();
	} else if (query == "off light" || query == "off the light") {
		offRelay();
		setSensorState(false);
		//Allow thread to quit
		boost::this_thread::sleep_for(boost::chrono::seconds{1});
		setSensorState(true);
		//Start sensing after delay
		boost::thread t{&HwInterface::sensorThread, this, true};
	}
}

void HwInterface::togglePin(int pinNumber, int pinMode) {
	digitalWrite(pinNumber, pinMode);
}

void HwInterface::onRelay() {
	boost::lock_guard<boost::mutex> guard(mtx_relay);
	if(relayState == true) return;
	togglePin(RELAY_PIN, 0);
	relayState = true;
}

void HwInterface::offRelay() {
	boost::lock_guard<boost::mutex> guard(mtx_relay);
	togglePin(RELAY_PIN, 1);
	relayState = false;
}

void HwInterface::setSensorState(bool state){
	boost::lock_guard<boost::mutex> guard(mtx_sensor);
	sensorState = state;
}

bool HwInterface::getSensorState(){
	boost::lock_guard<boost::mutex> guard(mtx_sensor);
	return sensorState;
}

void HwInterface::toggleRelay() {
	if(relayState)
		offRelay();
	else
		onRelay();
}

#endif