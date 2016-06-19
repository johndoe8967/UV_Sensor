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


#define useThingSpeak
#ifdef useThingSpeak
String ThingSpeakHost = "http://api.thingspeak.com";  // no need to change this
HttpClient thingSpeak;
#define APIKEY "--";
#endif


String url;

void onDataSent(HttpClient& client, bool successful)
{
	if (successful)
		Debug.printf("Success sent\r\n");
	else
		Debug.printf("Failed\r\n");

	Debug.printf("Server response: '%s'\r\n",client.getResponseString().c_str());
}


#ifdef useThingSpeak
void sendThingSpeak () {

}
#endif

void sendData(uint value, float avgValue, bool send) {

	Debug.printf ("value: %d avgValue: %d Time: %s\r\n", value, avgValue, SystemClock.now(eTZ_UTC).toISO8601().c_str());

	if (send) {
#ifdef useThingSpeak
		if (thingSpeak.isProcessing()) {
			Debug.print("!!!!ThingSpeak not ready -> close");
			thingSpeak.reset();
		} else {
			Debug.print("Send ThingSpeak\r\n");
			url = ThingSpeakHost;
			url += "/update?key=";
			url += APIKEY;
			url += "&field1=";
			url += value;
			url += "&field2=";
			url += avgValue;
			url += "&field3=";
			url += WifiStation.getRssi();
			url += "&created_at=";
			url += SystemClock.now(eTZ_UTC).toISO8601();
			thingSpeak.downloadString(url, onDataSent);
		}
#endif
	}
}


