// --------------------------------------
// UV_Sensor (Sming version)
//
//
//

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "VEML6070.h"
#include "credentials.h"

//#define debug

VEML6070 *uvSensor;
Timer sendDelayTimer;

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

String ThingSpeakHost = "http://api.thingspeak.com";  // no need to change this
HttpClient thingSpeak;
String url;

void sleep() {
	SET_PERI_REG_MASK(UART_CONF0(0), UART_TXFIFO_RST);//RESET FIFO
	CLEAR_PERI_REG_MASK(UART_CONF0(0), UART_TXFIFO_RST);
	system_deep_sleep_set_option(2);
	system_deep_sleep(30000000);
}

void onDataSent(HttpClient& client, bool successful)
{
	WifiStation.enable(false);
	sleep();
}

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
}

void sendDelay() {
	if (WifiStation.isConnected()) {
		sendDelayTimer.initializeMs(100,TimerDelegate(&sendData)).startOnce();
	} else {
		sendDelayTimer.initializeMs(100,TimerDelegate(&sendDelay)).startOnce();
	}
}


void readUV(uint newValue, float avgValue, float UVI, float energy)
{

	rtcMemory.count++;
	if (rtcMemory.count > 6) rtcMemory.count=0;
	rtcMemory.value[rtcMemory.count] = newValue;
	rtcMemory.UVI[rtcMemory.count] = UVI;

	actEnergy = energy;

	system_rtc_mem_write(64,&rtcMemory,sizeof(RTCMemMap));
	if (measureOnly) sleep();
	else sendDelay();
}

void init()
{
	WifiAccessPoint.enable(false);
	WifiStation.enable(false);

	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Disable debug output

	// You can change pins:
	Wire.pins(2, 0); // SCL, SDA

	Wire.begin();
	uvSensor = new VEML6070(VEML6070Delegate(&readUV),270,1);
//	uvSensor->setRsetValue(270);
//	uvSensor->setIntegrationTime(1);
//	uvSensor->setReduction(1);
//	uvSensor->setAlpha(0.1);

	rst_info *info = system_get_rst_info();
	if (info->reason == REASON_DEEP_SLEEP_AWAKE) {
		system_rtc_mem_read(64,&rtcMemory,sizeof(RTCMemMap));
		if (rtcMemory.count > 6) {
			memset(&rtcMemory,0,sizeof(RTCMemMap));
		}
	} else {
		memset(&rtcMemory,0,sizeof(RTCMemMap));
	}

	if (rtcMemory.count == 6) {
		WifiStation.config(WIFISSID,WIFIPWD,false);
		WifiStation.enable(true);
		measureOnly = false;
	} else {
		measureOnly = true;
	}
}
