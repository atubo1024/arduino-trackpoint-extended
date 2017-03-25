/*
 * Created on 14/03/2014
 * Author: Cong Nguyen
 *
 * Using an Arduino Leonardo to interface with a TrackPoint
 * Pin 3 (int.0)	: CLOCK
 * Pin 2			: DATA
 * Pin 4			: RESET
 */
#include <Mouse.h>
#include "TrackPoint.h"

#define DATA		2
#define	CLOCK		3
#define RESET		4
#define CLOCK_INT	0

TrackPoint trackpoint(CLOCK, DATA, RESET, true);

void setup()
{	
	Mouse.begin();
	pinMode(LED_BUILTIN, OUTPUT);

	digitalWrite(LED_BUILTIN, HIGH);
	delay(1000);
	trackpoint.reset();
	digitalWrite(LED_BUILTIN, LOW);

	digitalWrite(LED_BUILTIN, HIGH);
	delay(1000);
	trackpoint.setSensitivityFactor(0xC0);
	digitalWrite(LED_BUILTIN, LOW);

	digitalWrite(LED_BUILTIN, HIGH);
	delay(1000);
	trackpoint.setStreamMode();
	digitalWrite(LED_BUILTIN, LOW);

	digitalWrite(LED_BUILTIN, HIGH);
	delay(1000);
	attachInterrupt(CLOCK_INT, clockInterrupt, FALLING);
	digitalWrite(LED_BUILTIN, LOW);
}

void loop()
{	
	if(trackpoint.reportAvailable()) {
		TrackPoint::DataReport d = trackpoint.getStreamReport();
		Mouse.move(-d.x, d.y, 0);
	} 
}

void clockInterrupt(void) {
	trackpoint.getDataBit();
}
