/*
 * VEML6070.h
 *
 *  Created on: 10.06.2016
 *      Author: johndoe
 */

#ifndef APP_VEML6070_H_
#define APP_VEML6070_H_
#include <SmingCore/SmingCore.h>

#define CMDAddress 0x38
#define LSBAddress 0x38
#define MSBAddress 0x40

typedef Delegate<void(uint value)> VEML6070Delegate;

class VEML6070 {
public:
//	VEML6070();
	VEML6070(VEML6070Delegate newCallbackTimer);
	virtual ~VEML6070();

	bool setIntegrationTime(char time);
	void setRsetValue(uint newValue);

	unsigned int getValue() const {return value;};

private:
	void read();
	uint getCalcRefreshTime();
	unsigned int value;
	char time=1;
	uint rSet=300;
	bool init=false;
	Timer readTimer;
	VEML6070Delegate callbackTimer;
};

#endif /* APP_VEML6070_H_ */
