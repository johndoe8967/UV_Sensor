// --------------------------------------
// UV_Sensor (Sming version)
//
//
//

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "VEML6070.h"
#include "webserver.h"
#include "sendData.h"
#include "credentials.h"


VEML6070 *uvSensor;
Timer sendDelay;

char count=0;
uint actValue;
float actAvgValue;

void send() {
	sendData(actValue, actAvgValue, true);
}

void readUV(uint newValue, float avgValue)
{
	actValue=newValue;
	actAvgValue=avgValue;
	sendMeasureToClients(newValue, avgValue);
	if (count++ >= 15) {
		sendDelay.startOnce();
		count = 0;
	}
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");

	startWebServer();
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Disable debug output

//	delayMilliseconds(10000);

	// You can change pins:
	Wire.pins(2, 0); // SCL, SDA

	Wire.begin();
	uvSensor = new VEML6070(VEML6070Delegate(&readUV));
	uvSensor->setRsetValue(270);
	uvSensor->setIntegrationTime(1);
	uvSensor->setReduction(8);
	uvSensor->setAlpha(0.3);

	WifiStation.enable(true);
	WifiStation.config(WIFISSID, WIFIPWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiStation.waitConnection(connectOk);
	sendDelay.initializeMs(100,TimerDelegate(&send));
}
