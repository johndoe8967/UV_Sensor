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

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif


VEML6070 *uvSensor;

char count=0;
void readUV(uint newValue, float avgValue)
{
  debugf("uv value: %d; %f\n\r",newValue, avgValue);
  sendMeasureToClients(newValue, avgValue);
  if (count++ >= 15) {
	  sendData(newValue, avgValue, true);
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
	Serial.systemDebugOutput(true); // Disable debug output

	// You can change pins:
	//Wire.pins(12, 14); // SCL, SDA

	Wire.begin();
	uvSensor = new VEML6070(VEML6070Delegate(&readUV));
	uvSensor->setRsetValue(300);
	uvSensor->setIntegrationTime(1);
	uvSensor->setReduction(8);
	uvSensor->setAlpha(0.3);

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiStation.waitConnection(connectOk);

}
