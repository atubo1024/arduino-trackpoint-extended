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

static void handleSerialRequest(void);
static void responseSerialNoData(byte);
static void sendButtonState(byte state);

#define DATA						2
#define	CLOCK						3
#define RESET						4
#define CLOCK_INT					0

#define TP_MOUSE_LEFT				0x04u
#define TP_MOUSE_RIGHT				0x08u
#define TP_MOUSE_MIDDLE				0x02u

#define OPCODE_GET_CONFIG			1
#define OPCODE_SET_CONFIG			2
#define OPCODE_ECHO					3

struct Config
{
	int8_t debug_enabled;
	int8_t reserve;
	int8_t x_direction;
	int8_t y_direction;
	float x_positive_scale;
	float x_negative_scale;
	float y_positive_scale;
	float y_negative_scale;
};

#define SERIAL_FRAME_MAX_DATALEN	32
#define SERIAL_FRAME_HEADLEN		4
#define SERIAL_FRAME_LEADBYTE		0xe5u

#define SERIAL_REQUEST				0
#define SERIAL_OK					1
#define SERIAL_BAD_FRAME			2
#define SERIAL_BAD_OPCODE			3
#define SERIAL_BAD_PARAMS			4
#define SERIAL_INTERNAL_ERROR		5

struct SerialFrame
{
	/** for tx: leadbyte; for rx: current state */
	byte leadbyte_currstate;
	byte opcode;
	byte datalen;
	/** for tx: flags; for rx: current received bytes */
	byte flags_rxlen;
	byte data[SERIAL_FRAME_MAX_DATALEN];
};

enum SERIAL_STATE_DEF
{
	SERIAL_PENDING,		/* when rx LEADBYTE => SERIAL_START_1 */
	SERIAL_START_1,		/* when rx opcode => SERIAL_START_2 */
	SERIAL_START_2,		/* when rx datalen => SERIAL_RXDATA */
	SERIAL_RXDATA,		/* when rx completed => handling => tx response => SERIAL_PENDING */
};

static struct SerialFrame mSerialFrame;
static struct Config mConfig;

TrackPoint trackpoint(CLOCK, DATA, RESET, true);

void setup()
{	
	mSerialFrame.leadbyte_currstate = SERIAL_PENDING;

	mConfig.debug_enabled = 1;
	mConfig.x_direction = -1;
	mConfig.y_direction = 1;
	mConfig.x_positive_scale = 1.0f;
	mConfig.x_negative_scale = 1.0f;
	mConfig.y_positive_scale = 1.0f;
	mConfig.y_negative_scale = 1.0f;

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

	if (mConfig.debug_enabled) {
		Serial.println("TrackPoint Started.");
	}
}

void loop()
{	
	int8_t debug_enabled = mConfig.debug_enabled;

	if (trackpoint.reportAvailable()) {
		char buffer[128];
		TrackPoint::DataReport d = trackpoint.getStreamReport();
		byte state = d.state;

		if (debug_enabled) {
			sprintf(buffer, "state: 0x%02x, (%d, %d)\r\n", d.state, d.x, d.y);
			Serial.print(buffer);
		}

		if ((state & TP_MOUSE_LEFT) == TP_MOUSE_LEFT) {
			if (debug_enabled) {
				Serial.println("press left");
			}
			Mouse.press(MOUSE_LEFT);
		} else if ((state & TP_MOUSE_RIGHT) == TP_MOUSE_RIGHT) {
			if (debug_enabled) {
				Serial.println("press right");
			}
			Mouse.press(MOUSE_RIGHT);
		} else if ((state & TP_MOUSE_MIDDLE) == TP_MOUSE_MIDDLE) {
			if (debug_enabled) {
				Serial.println("scroll");
			}
			Mouse.move(0, 0, -d.y);
		} else if (Mouse.isPressed(MOUSE_LEFT)) {
			if (debug_enabled) {
				Serial.println("release left");
			}
			Mouse.release(MOUSE_LEFT);
		} else if (Mouse.isPressed(MOUSE_RIGHT)) {
			if (debug_enabled) {
				Serial.println("release right");
			}
			Mouse.release(MOUSE_RIGHT);
		} else {
			int8_t dx, dy;

			dx = d.x * mConfig.x_direction;
			dy = d.y * mConfig.y_direction;
			if (dx > 0) {
				dx = (int8_t) (((float) dx) * mConfig.x_positive_scale);
			} else {
				dx = (int8_t) (((float) dx) * mConfig.x_negative_scale);
			}
			if (dy > 0) {
				dy = (int8_t) (((float) dy) * mConfig.y_positive_scale);
			} else {
				dy = (int8_t) (((float) dy) * mConfig.y_negative_scale);
			}
			Mouse.move(dx, dy, 0);
		}
	} 

	if (Serial.available() > 0) {
		byte inChar = (byte) Serial.read();
		byte rxlen = 0;
		switch (mSerialFrame.leadbyte_currstate) {
			case SERIAL_PENDING:
				if (inChar == SERIAL_FRAME_LEADBYTE) {
					mSerialFrame.leadbyte_currstate = SERIAL_START_1;
				}
				break;
			case SERIAL_START_1:
				if (inChar != SERIAL_REQUEST) {
					mSerialFrame.leadbyte_currstate = SERIAL_PENDING;
				} else {
					mSerialFrame.opcode = inChar;
					mSerialFrame.leadbyte_currstate = SERIAL_START_2;
				}
				break;
			case SERIAL_START_2:
				if (inChar > sizeof(mSerialFrame.data)) {
					/* illegal access */
					responseSerialNoData(SERIAL_BAD_FRAME);
					mSerialFrame.leadbyte_currstate = SERIAL_PENDING;
				}
				mSerialFrame.datalen = inChar;
				mSerialFrame.flags_rxlen = 0;
				mSerialFrame.leadbyte_currstate = SERIAL_RXDATA;
				break;
			case SERIAL_RXDATA:
				rxlen = mSerialFrame.flags_rxlen;
				mSerialFrame.data[rxlen++] = inChar;
				if (rxlen >= mSerialFrame.datalen) {
					/* rx frame completed */
					handleSerialRequest();
					mSerialFrame.leadbyte_currstate = SERIAL_PENDING;
				} else {
					mSerialFrame.flags_rxlen = rxlen;
				}
				break;
			default:
				Serial.println("serial: error state");
				break;
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
				responseSerialNoData(SERIAL_INTERNAL_ERROR);
			} else {
				memcpy(mSerialFrame.data, &mConfig, sizeof(mConfig));
				mSerialFrame.leadbyte_currstate = SERIAL_FRAME_LEADBYTE;
				mSerialFrame.datalen = sizeof(mConfig);
				mSerialFrame.flags_rxlen = SERIAL_OK;
				Serial.write((byte *) &mSerialFrame, SERIAL_FRAME_HEADLEN + sizeof(mConfig));
			}
			break;
		case OPCODE_SET_CONFIG:
			if (mSerialFrame.datalen != sizeof(mConfig)) {
				responseSerialNoData(SERIAL_BAD_PARAMS);
			} else {
				memcpy(&mConfig, mSerialFrame.data, mSerialFrame.datalen);
				responseSerialNoData(SERIAL_OK);
			}
			break;
		case OPCODE_ECHO:
			mSerialFrame.leadbyte_currstate = SERIAL_FRAME_LEADBYTE;
			mSerialFrame.flags_rxlen = SERIAL_OK;
			Serial.write((byte *) &mSerialFrame, mSerialFrame.datalen);
			break;
		default:
			responseSerialNoData(SERIAL_BAD_OPCODE);
			break;
	}
}

static void responseSerialNoData(byte flags)
{
	mSerialFrame.leadbyte_currstate = SERIAL_FRAME_LEADBYTE;
	mSerialFrame.datalen = 0;
	mSerialFrame.flags_rxlen = flags;
	Serial.write((byte *) &mSerialFrame, SERIAL_FRAME_HEADLEN);
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

