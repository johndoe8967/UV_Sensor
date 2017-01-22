// --------------------------------------
// UV_Sensor (Sming version)
//
//
//

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "VEML6070.h"
#include "credentials.h"
#include <user_interface.h>

//#define noSleep
//#define debug
//#define USE_MQTT
#define USE_THINGSPEAK
#define LED_PIN 2 // GPIO2
bool blinkState = true;

VEML6070 *uvSensor;
Timer sendDelayTimer;
Timer sleepTimer;
Timer *blinkTimer;
Timer *wpsTimer;
char wpsRepeatCounter;

uint32 actValue=0;
float actUVI;
float actEnergy;

bool measureOnly;

struct RTCMemMap {
	char count;
	unsigned int value [7];
	float UVI[7];
};
RTCMemMap rtcMemory;

uint32 rtcCalib;

#ifdef USE_THINGSPEAK
String ThingSpeakHost = "http://api.thingspeak.com";  // no need to change this
HttpClient thingSpeak;
bool thingSpeakReady=false;
#endif
String url;

#ifdef USE_MQTT
// ... and/or MQTT username and password
#ifndef MQTT_USERNAME
	#define MQTT_USERNAME ""
	#define MQTT_PWD ""
#endif

// ... and/or MQTT host and port
#ifndef MQTT_HOST
	#define MQTT_HOST "192.168.0.5"
	#define MQTT_PORT 32768
#endif
// Forward declarations
void startMqttClient();
void onMessageReceived(String topic, String message);


// MQTT client
MqttClient mqtt(MQTT_HOST, MQTT_PORT, onMessageReceived);
Timer mqttTimer;
bool mqttReady=false;

// Check for MQTT Disconnection
void checkMQTTDisconnect(TcpClient& client, bool flag){

	// Called whenever MQTT connection is failed.
	if (flag == true)
		Serial.println("MQTT Broker Disconnected!!");
	else
		Serial.println("MQTT Broker Unreachable!!");
}

void delayMQTTReady() {
	mqttReady = true;
}

// Publish our message
void publishMessage()
{
	if (mqtt.getConnectionState() != eTCS_Connected)
		startMqttClient(); // Auto reconnect

	Serial.println("Let's publish message now!");
	String msg;
	msg = String(actValue);
	if (mqtt.publish("main/uvValue", msg.c_str())) {
		Serial.println("mtqq successful");
		mqttTimer.initializeMs(500,TimerDelegate(&delayMQTTReady)).startOnce();
	} else {
		Serial.println("mtqq repeate");
		mqttTimer.initializeMs(100,TimerDelegate(&publishMessage)).startOnce();
	}
}

// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
	Serial.print(topic);
	Serial.print(":\r\n\t"); // Pretify alignment for printing
	Serial.println(message);
}

// Run MQTT client
void startMqttClient()
{
	mqtt.connect("esp8266", MQTT_USERNAME, MQTT_PWD);
	// Assign a disconnect callback function
	mqtt.setCompleteDelegate(checkMQTTDisconnect);
	mqtt.subscribe("main/status/#");
}
#else
#define mqttReady TRUE
#endif

#ifdef USE_THINGSPEAK
#else
#define thingSpeakReady TRUE
#endif


void sleep() {
	WifiStation.enable(false);
	SET_PERI_REG_MASK(UART_CONF0(0), UART_TXFIFO_RST);//RESET FIFO
	CLEAR_PERI_REG_MASK(UART_CONF0(0), UART_TXFIFO_RST);
	system_deep_sleep_set_option(2);
	system_deep_sleep(10*1000000);
}

#ifdef USE_THINGSPEAK
void onDataSent(HttpClient& client, bool successful)
{
	thingSpeakReady = true;
}
#endif

void sendData() {

	actValue=0;
	actUVI = 0;
	for (char i=0; i<7; i++) {
		actValue += rtcMemory.value[i];
		actUVI += rtcMemory.UVI[i];
	}
	actValue /= 7;
	actUVI /= 7;

#ifdef debug
	Debug.printf ("value: %d avgValue: %d Time: %s\r\n", value, avgValue, SystemClock.now(eTZ_UTC).toISO8601().c_str());
#endif

#ifdef USE_MQTT
	mqttTimer.initializeMs(50,TimerDelegate(&publishMessage)).startOnce();
#endif
#ifdef USE_THINGSPEAK
	if (thingSpeak.isProcessing()) return;

#ifdef debug
	Debug.print("Send ThingSpeak\r\n");
#endif
	url = ThingSpeakHost;
	url += "/update?key=";
	url += APIKEY;
	url += "&field1=";
	url += actValue;
	url += "&field3=";
	url += WifiStation.getRssi();
	url += "&field4=";
	url += actUVI;
	url += "&field5=";
	url += actEnergy;
	thingSpeak.downloadString(url, onDataSent);
#endif
}

void sleepDelay() {
	if (thingSpeakReady && mqttReady) {
#ifndef noSleep
		sleep();
#endif

	} else {
		sleepTimer.initializeMs(100,TimerDelegate(&sleepDelay)).startOnce();
	}
}

void sendDelay() {
	if (WifiStation.isConnected()) {
#ifdef USE_MQTT
		startMqttClient();
#endif
		sleepTimer.initializeMs(100,TimerDelegate(&sleepDelay)).startOnce();
		sendDelayTimer.initializeMs(100,TimerDelegate(&sendData)).startOnce();
	} else {
		sendDelayTimer.initializeMs(100,TimerDelegate(&sendDelay)).startOnce();
	}
}

void readUV(uint newValue, float avgValue, float UVI, float energy)
{
	Serial.print("measure: ");
	Serial.println(newValue);
	rtcMemory.count++;
	if (rtcMemory.count > 6) rtcMemory.count=0;
	rtcMemory.value[rtcMemory.count] = newValue;
	rtcMemory.UVI[rtcMemory.count] = UVI;

	actEnergy = energy;

	system_rtc_mem_write(64,&rtcMemory,sizeof(RTCMemMap));
	if (measureOnly) {
		sleep();
	}
	else {
#ifndef noSleep
		uvSensor->setDelegate(nullptr);
		uvSensor->stop();
#endif
		sendDelay();
	}
}

/*
 * WPS Configuration methods
 */
void onConnect() {
}

void wpsBlinkIndicator()
{
	digitalWrite(LED_PIN, blinkState);
	blinkState = !blinkState;
}

void wpsStart() {
	wifi_wps_start();
}

void wpsStatus(int status) {
	if (status == WPS_CB_ST_SUCCESS) {
		Serial.println("WPS config successful");
		wifi_wps_disable();
		WifiStation.connect();
		blinkTimer->stop();
	} else {
		wifi_wps_disable();
		wpsRepeatCounter--;
		if (wpsRepeatCounter>0) {
			Serial.print("WPS repeat");
			wifi_wps_enable(WPS_TYPE_PBC);
			wifi_set_wps_cb(&wpsStatus);
			wpsTimer->initializeMs(15000,TimerDelegate(&wpsStart)).startOnce();
		} else {
			sleep();
		}
	}
}

void wpsConnect() {
	blinkTimer->initializeMs(200, wpsBlinkIndicator).start();
	wifi_wps_enable(WPS_TYPE_PBC);
	wifi_set_wps_cb(&wpsStatus);
	wpsStart();
}

void noConnect() {
	Serial.println("WPS config started");
	blinkTimer = new Timer();
	wpsTimer = new Timer();
	wpsRepeatCounter=10;
	pinMode(LED_PIN, OUTPUT);
	blinkTimer->initializeMs(100, wpsBlinkIndicator).start();
	wpsTimer->initializeMs(5000,TimerDelegate(&wpsConnect)).startOnce();
}

void init()
{
	WifiAccessPoint.enable(false);
	WifiStation.enable(false);

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Disable debug output
	rtcCalib = system_rtc_clock_cali_proc();

	debugf("Hello UVSensor");

	Wire.pins(2, 0); // SCL, SDA

	Wire.begin();
	delay(100);
	uvSensor = new VEML6070(VEML6070Delegate(&readUV),270,1);
//	uvSensor->setRsetValue(270);
//	uvSensor->setIntegrationTime(1);
//	uvSensor->setReduction(10);
//	uvSensor->setAlpha(0.1);

	rst_info *info = system_get_rst_info();
	if (info->reason == REASON_DEEP_SLEEP_AWAKE) {
		debugf("Reset from DeepSleep");
		system_rtc_mem_read(64,&rtcMemory,sizeof(RTCMemMap));
		if (rtcMemory.count > 6) {
			memset(&rtcMemory,0,sizeof(RTCMemMap));
		}
	} else {
		memset(&rtcMemory,0,sizeof(RTCMemMap));
	}

#ifndef noSleep
	if (rtcMemory.count == 6) {
#endif
		Serial.println("Send to Thingspeak");
//		WifiStation.config(WIFISSID,WIFIPWD,false);
		WifiStation.enable(true);
		measureOnly = false;
		WifiStation.waitConnection(ConnectionDelegate(&onConnect),10,ConnectionDelegate(&noConnect));
#ifndef noSleep
		} else {
		Serial.println("Measure Only");
		measureOnly = true;
	}
#endif
}
