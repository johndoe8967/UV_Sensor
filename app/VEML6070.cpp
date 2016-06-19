/*
 * VEML6070.cpp
 *
 *  Created on: 10.06.2016
 *      Author: johndoe
 */

#include "VEML6070.h"

//#define debug

VEML6070::VEML6070(VEML6070Delegate newCallbackTimer,uint newRSet, char newTime)
			: callbackTimer(newCallbackTimer),
			  value (0),
			  avgValue(0.0),
			  alpha(1),
			  refreshTime(newTime),
			  reduction(1),
			  count(0),
			  rSet(newRSet),
			  init(false)
{
	Wire.beginTransmission(CMDAddress);
	Wire.write(refreshTime<<2 | 0x02);
	byte error = Wire.endTransmission();
	getCalcRefreshTime();
	if (error == 0) {
		init = true;
#ifdef debug
		debugf("VEML6070 init\n");
#endif
	}
}
VEML6070::VEML6070(uint newRSet, char newTime)
			: VEML6070(nullptr,newRSet,newTime) {
}

VEML6070::VEML6070() : VEML6070(nullptr) {
}


VEML6070::~VEML6070() {
}


void VEML6070::read() {
	uint8_t ret;
	ret = Wire.requestFrom(MSBAddress,1);
#ifdef debug
	debugf("VEML6070 MSB: %x",ret);
#endif
	if (Wire.available()) {
		value = Wire.read();
#ifdef debug
		debugf("VEML6070 read MSB");
#endif
	}
	value <<= 8;
	ret = Wire.requestFrom(LSBAddress,1);
#ifdef debug
	debugf("VEML6070LSB: %x",ret);
#endif
	if (Wire.available()) {
		value |= Wire.read();
#ifdef debug
		debugf("VEML6070 read LSB");
#endif
	}
	avgValue = avgValue*(1-alpha) + (float)value*alpha;
	count++;
	if ((callbackTimer)&&((count%reduction)==0)) {
		callbackTimer(value,avgValue);
	}
}

bool VEML6070::setReduction(char newReduction) {
	if (newReduction > 0) reduction = newReduction;
}

uint VEML6070::getCalcRefreshTime() {
	unsigned int refreshtime = rSet*125/600<<refreshTime;
	readTimer.initializeMs(refreshtime,TimerDelegate(&VEML6070::read,this)).start();
	return refreshtime;
}

void VEML6070::setRsetValue(uint newValue) {
	if (newValue < 1100) {
		rSet = newValue;
		getCalcRefreshTime();
	}
}

bool VEML6070::setIntegrationTime(char newTime) {
	if ((newTime < 0) || (newTime > 3)) return false;
	refreshTime = newTime;
	Wire.beginTransmission(CMDAddress);
	Wire.write(refreshTime<<2 | 0x02);
	byte error = Wire.endTransmission();
	getCalcRefreshTime();
	return (error == 0);
}

