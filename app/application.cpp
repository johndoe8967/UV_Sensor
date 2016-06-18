// --------------------------------------
// UV_Sensor (Sming version)
//
//
//

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "VEML6070.h"

VEML6070 *uvSensor;

void readUV(uint newValue)
{
  debugf("uv value: %d \n\r",newValue);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Disable debug output

	WDT.enable(false); // First (but not the best) option: fully disable watch dog timer

	// You can change pins:
	//Wire.pins(12, 14); // SCL, SDA

	Wire.begin();
	uvSensor = new VEML6070(VEML6070Delegate(&readUV));
	uvSensor->setRsetValue(300);
	uvSensor->setIntegrationTime(1);

}
