/*
 * sendData.cpp
 * Original Author: https://github.com/johndoe8967
 *
 * sends data to ThingSpeak
 * expects 3 fields:
 * 		CPM, dose, RSSI
 *
 * sends data to RadMon
 * 		based on GKMon, rewritten to use in Sming framework
 *
 *  Created on: 15.05.2016
 */
#include "../include/sendData.h"
#include <SmingCore/Debug.h>
#include "credentials.h"

//#define debug

#define useThingSpeak
#ifdef useThingSpeak
String ThingSpeakHost = "http://api.thingspeak.com";  // no need to change this
HttpClient thingSpeak;
#endif


String url;

void onDataSent(HttpClient& client, bool successful)
{
#ifdef debug
	if (successful)
		Debug.printf("Success sent\r\n");
	else
		Debug.printf("Failed\r\n");

	Debug.printf("Server response: '%s'\r\n",client.getResponseString().c_str());
#endif
}


#ifdef useThingSpeak
void sendThingSpeak () {

}
#endif

void sendData(uint value, float avgValue, bool send) {
#ifdef debug
	Debug.printf ("value: %d avgValue: %d Time: %s\r\n", value, avgValue, SystemClock.now(eTZ_UTC).toISO8601().c_str());
#endif

	if (send) {
#ifdef useThingSpeak
		if (thingSpeak.isProcessing()) {
#ifdef debug
			Debug.print("!!!!ThingSpeak not ready -> close");
#endif
			thingSpeak.reset();
		} else {
#ifdef debug
			Debug.print("Send ThingSpeak\r\n");
#endif
			url = ThingSpeakHost;
			url += "/update?key=";
			url += APIKEY;
			url += "&field1=";
			url += value;
			url += "&field2=";
			url += avgValue;
			url += "&field3=";
			url += WifiStation.getRssi();
			thingSpeak.downloadString(url, onDataSent);
		}
#endif
	}
}


