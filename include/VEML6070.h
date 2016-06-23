/*
 * VEML6070.h
 *	Implementation of a VEML6070 UV light I2C sensor
 *	the class will perform autonomous measurements according to the timing of the sensor
 *	values can be read by function or obtained by delegation
 *
 *  Created on: 10.06.2016
 *      Author: johndoe
 */

#ifndef APP_VEML6070_H_
#define APP_VEML6070_H_
#include <SmingCore/SmingCore.h>

#define UVIk 0.005353733
#define UVId -0.997052

typedef Delegate<void(uint value, float avgValue, float UVI, float E)> VEML6070Delegate;

class VEML6070 {
public:
	VEML6070();
	VEML6070(uint rSet=300, char time=1);
	VEML6070(VEML6070Delegate newCallbackTimer,uint rSet=300, char time=1);

	virtual ~VEML6070();

	bool setIntegrationTime(char time);
	bool setReduction(char newReduction);
	void setRsetValue(uint newValue);

	uint getValue() const {
		return value;
	}

	float getAvgValue() const {
		return avgValue;
	}

	float getUVI() const {
		float integrationFactor = (1<<refreshTime);
		integrationFactor /= 2;
		float UVI = ((float)value/integrationFactor)*UVIk+UVId;
		if (UVI < 0) UVI = 0;
		return UVI;
	}

	float getEnergy() const {
		return getUVI()*0.025;
	}
	float getAlpha() const { return alpha; }

	void setAlpha(float alpha) {
		if ((alpha > 0) && (alpha <=1)) this->alpha = alpha;
	}

	;

private:
	void read();
	uint getCalcRefreshTime();
	uint value;
	float avgValue;
	float alpha;
	char refreshTime;
	char reduction;
	char count;
	uint rSet;
	bool init;
	Timer readTimer;
	VEML6070Delegate callbackTimer;
};

#endif /* APP_VEML6070_H_ */
