/*
 * Created on 14/03/2014
 * Author: Cong Nguyen
 *
 * Using an Arduino Leonardo to interface with a TrackPoint
 * Pin 3 (int.0)	: CLOCK
 * Pin 2			: DATA
 * Pin 4			: RESET
 */
#include <string.h>
#include <Mouse.h>
#include "TrackPoint.h"
/* I dont know how add source files to ArduinoIDE project, so include the source file instead of the header file. */
#include "serialframe.c"
#include "main.h"

static void handleSerialRequest(void);
static void sendButtonState(byte state);

#define DATA						2
#define	CLOCK						3
#define RESET						4
#define CLOCK_INT					0

#define TP_MOUSE_LEFT				0x04u
#define TP_MOUSE_RIGHT				0x08u
#define TP_MOUSE_MIDDLE				0x02u

static byte mDebugEnabled = 1;
static struct Config mConfig = { CONFIG_FOREACH(COLLECT_STRUCT_ITEM_DEFAULT_VALUE) };
static struct SerialFrame mSerialFrame;

TrackPoint trackpoint(CLOCK, DATA, RESET, true);

void setup()
{	
	SerialFrame_Init(&mSerialFrame);

	Serial.begin(9600);

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

	Serial.println("TrackPoint Started.");
}

static void handleTrackpointEvent(void)
{
	if (trackpoint.reportAvailable()) {
		char buffer[128];
		TrackPoint::DataReport d = trackpoint.getStreamReport();
		byte state = d.state;

		if (mDebugEnabled) {
			sprintf(buffer, "state: 0x%02x, (%d, %d)\r\n", d.state, d.x, d.y);
			Serial.print(buffer);
		}

		if (d.x == 0 && d.y == 0) {
			if ((state & TP_MOUSE_LEFT) == TP_MOUSE_LEFT) {
				if (mDebugEnabled) {
					Serial.println("press left");
				}
				Mouse.press(MOUSE_LEFT);
				return;
			} else if (Mouse.isPressed(MOUSE_LEFT)) {
				if (mDebugEnabled) {
					Serial.println("release left");
				}
				Mouse.release(MOUSE_LEFT);
				return;
			}

			if ((state & TP_MOUSE_RIGHT) == TP_MOUSE_RIGHT) {
				if (mDebugEnabled) {
					Serial.println("press right");
				}
				Mouse.press(MOUSE_RIGHT);
				return;
			} else if (Mouse.isPressed(MOUSE_RIGHT)) {
				if (mDebugEnabled) {
					Serial.println("release right");
				}
				Mouse.release(MOUSE_RIGHT);
				return;
			} 
		}

		if ((state & TP_MOUSE_MIDDLE) == TP_MOUSE_MIDDLE) {
			if (mDebugEnabled) {
				Serial.println("scroll");
			}
			Mouse.move(0, 0, d.y * mConfig.scroll_direction);
			return;
		}

		int8_t dx, dy;

		dx = d.x * mConfig.x_direction;
		dy = d.y * mConfig.y_direction;
		if (dx > 0) {
			dx = (int8_t) (((float) dx) * mConfig.scale_left);
		} else if (dx < 0) {
			dx = (int8_t) (((float) dx) * mConfig.scale_right);
		}
		if (dy > 0) {
			dy = (int8_t) (((float) dy) * mConfig.scale_up);
		} else if (dy < 0) {
			dy = (int8_t) (((float) dy) * mConfig.scale_down);
		}
		Mouse.move(dx, dy, 0);
	} 
}

void loop()
{	
	handleTrackpointEvent();
	if (Serial.available() > 0) {
		uint8_t inChar = (uint8_t) Serial.read();
		uint8_t errCode = SerialFrame_PutChar(&mSerialFrame, inChar);
		if (errCode == SERIALFRAME_ACK && mSerialFrame.flags == SERIALFRAME_REQUEST) {
			handleSerialRequest();
		} else if (errCode != SERIALFRAME_PENDING) {
			sendSerialFrameWithoutData(errCode);
		}
	}
}

void clockInterrupt(void) {
	trackpoint.getDataBit();
}

static void handleSerialRequest(void)
{
	switch (mSerialFrame.opcode) {
		case OPCODE_GET_CONFIG:
			if (sizeof(mConfig) > sizeof(mSerialFrame.data)) {
				sendSerialFrameWithoutData(SERIALFRAME_INTERNAL_ERROR);
			} else {
				memcpy(mSerialFrame.data, &mConfig, sizeof(mConfig));
				mSerialFrame.leadbyte = SERIALFRAME_LEADBYTE;
				mSerialFrame.datalen = sizeof(mConfig);
				mSerialFrame.flags = SERIALFRAME_ACK;
				Serial.write((byte *) &mSerialFrame, SERIALFRAME_HEADLEN + sizeof(mConfig));
			}
			break;
		case OPCODE_SET_CONFIG:
			if (mSerialFrame.datalen != sizeof(mConfig)) {
				sendSerialFrameWithoutData(SERIALFRAME_BAD_PARAMS);
			} else {
				memcpy(&mConfig, mSerialFrame.data, mSerialFrame.datalen);
				sendSerialFrameWithoutData(SERIALFRAME_ACK);
			}
			break;
		case OPCODE_ECHO:
			mSerialFrame.leadbyte = SERIALFRAME_LEADBYTE;
			mSerialFrame.flags = SERIALFRAME_ACK;
			Serial.write((byte *) &mSerialFrame, mSerialFrame.datalen);
			break;
		default:
			sendSerialFrameWithoutData(SERIALFRAME_BAD_OPCODE);
			break;
	}
}

static void sendSerialFrameWithoutData(byte flags)
{
	mSerialFrame.leadbyte = SERIALFRAME_LEADBYTE;
	mSerialFrame.datalen = 0;
	mSerialFrame.flags = flags;
	Serial.write((byte *) &mSerialFrame, SERIALFRAME_HEADLEN);
}

static void sendButtonState(byte state)
{
	static const byte hidStates[] = {MOUSE_LEFT, MOUSE_RIGHT};

  	for (byte i = 0; i < sizeof(hidStates); i++) {
		byte hidState = hidStates[i];
  	  	if (state & (1 << i)) {
			Mouse.press(hidState);
  	  	} else if (Mouse.isPressed(hidState)) {
			Mouse.release(hidState);
  	  	}
  	}
}

