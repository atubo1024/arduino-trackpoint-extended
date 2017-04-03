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
#include <EEPROM.h>
#include "TrackPoint.h"
/* I dont know how add source files to ArduinoIDE project, so include the source file instead of the header file. */
#include "serialframe.c"
#include "main.h"

static void handleSerialRequest(void);

#define DATA						2
#define	CLOCK						3
#define RESET						4
#define CLOCK_INT					0

#define TP_MOUSE_LEFT				0x04u
#define TP_MOUSE_RIGHT				0x08u
#define TP_MOUSE_MIDDLE				0x02u

static byte mDumping = 0;
static struct Config mConfig = { CONFIG_FOREACH(COLLECT_STRUCT_ITEM_DEFAULT_VALUE) };
static struct SerialFrame mSerialFrame;

TrackPoint trackpoint(CLOCK, DATA, RESET, true);

#define EEPROM_LEADER				0xe5u

static void loadConfig(void)
{
	byte *p;
	int i, addr;

	addr = 0;
	if (EEPROM.read(addr++) != EEPROM_LEADER) return;
	if (EEPROM.read(addr++) != CONFIG_VERSION) return;

	p = (byte *) &mConfig;
	for (i = 0; i<sizeof(mConfig); i++) {
		*p++ = EEPROM.read(addr++);
	}
}

static void saveConfig(void)
{
	byte *p;
	int i, addr;

	addr = 0;
	EEPROM.update(addr++, EEPROM_LEADER);
	delay(100);
	EEPROM.update(addr++, CONFIG_VERSION);
	delay(100);

	p = (byte *) &mConfig;
	for (i=0; i<sizeof(mConfig); i++) {
		EEPROM.update(addr++, *p++);
	}
}

void setup()
{	
	loadConfig();

	SerialFrame_Init(&mSerialFrame);

	Serial.begin(115200);

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
		#ifdef DEBUG
		char buffer[128];
		#endif

		TrackPoint::DataReport d = trackpoint.getStreamReport();
		byte state = d.state;
		if (mDumping) {
			struct TrackPointDumpData *pData = (struct TrackPointDumpData *) mSerialFrame.data;

			mSerialFrame.opcode = OPCODE_TP_DATA;
			mSerialFrame.flags = SERIALFRAME_ACK;
			mSerialFrame.datalen = 6;
			pData->timestamp = millis();
			pData->x = d.x;
			pData->y = d.y;
			Serial.write((byte *) &mSerialFrame, SERIALFRAME_HEADLEN + 6);
		}

		#ifdef DEBUG
			sprintf(buffer, "state: 0x%02x, (%d, %d)\r\n", d.state, d.x, d.y);
			Serial.print(buffer);
		#endif

		if (d.x == 0 && d.y == 0) {
			if ((state & TP_MOUSE_LEFT) == TP_MOUSE_LEFT) {
				#ifdef DEBUG
					Serial.println("press left");
				#endif
				Mouse.press(MOUSE_LEFT);
				return;
			} else if (Mouse.isPressed(MOUSE_LEFT)) {
				#ifdef DEBUG
					Serial.println("release left");
				#endif
				Mouse.release(MOUSE_LEFT);
				return;
			}

			if ((state & TP_MOUSE_RIGHT) == TP_MOUSE_RIGHT) {
				#ifdef DEBUG
					Serial.println("press right");
				#endif
				Mouse.press(MOUSE_RIGHT);
				return;
			} else if (Mouse.isPressed(MOUSE_RIGHT)) {
				#ifdef DEBUG
					Serial.println("release right");
				#endif
				Mouse.release(MOUSE_RIGHT);
				return;
			} 
		}

		int8_t dx, dy;

		if ((state & TP_MOUSE_MIDDLE) == TP_MOUSE_MIDDLE) {
			#ifdef DEBUG
				Serial.println("scroll");
			#endif
			dy = d.y * mConfig.scroll_direction;
			dy = (int8_t)(mConfig.scale_scroll * dy);
			Mouse.move(0, 0, dy);
			return;
		}

		dx = d.x * mConfig.x_direction;
		dy = d.y * mConfig.y_direction;

		if (dx < 0) {
			dx = (int8_t) (((float) dx) * mConfig.scale_left);
		} else if (dx > 0) {
			dx = (int8_t) (((float) dx) * mConfig.scale_right);
		}
		if (dy < 0) {
			dy = (int8_t) (((float) dy) * mConfig.scale_up);
		} else if (dy > 0) {
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
				saveConfig();
				sendSerialFrameWithoutData(SERIALFRAME_ACK);
			}
			break;
		case OPCODE_ECHO:
			mSerialFrame.leadbyte = SERIALFRAME_LEADBYTE;
			mSerialFrame.flags = SERIALFRAME_ACK;
			Serial.write((byte *) &mSerialFrame, mSerialFrame.datalen);
			break;
		case OPCODE_START_TP_DUMP:
			mDumping = 1;
			sendSerialFrameWithoutData(SERIALFRAME_ACK);
			break;
		case OPCODE_STOP_TP_DUMP:
			mDumping = 0;
			sendSerialFrameWithoutData(SERIALFRAME_ACK);
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

