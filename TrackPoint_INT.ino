/*
 * Created on 14/03/2014
 * Author: Cong Nguyen
 *
 * Using an Arduino Leonardo to interface with a TrackPoint
 * Pin 3 (int.0)	: CLOCK
 * Pin 2			: DATA
 * Pin 4			: RESET
 */
#include <stdint.h>
#include <string.h>
#include <Mouse.h>
#include "TrackPoint.h"

static void handleSerialRequest(void);
static void responseSerialNoData(uint8_t);

#define DATA		2
#define	CLOCK		3
#define RESET		4
#define CLOCK_INT	0

#define OPCODE_GET_CONFIG			1
#define OPCODE_SET_CONFIG			2

struct Config
{
	/** trackpoint.move(x, y) => (x + i y) * (rotation_real + i rotation_imag) = (x_1 + i y_1) => mouse.move(x_1, y_1)  */
	float rotation_real;
	float rotation_imag;
};

#define SERIAL_FRAME_MAX_DATALEN	32
#define SERIAL_FRAME_HEADLEN		4
#define SERIAL_FRAME_LEADBYTE		0xe5u

#define FLAG_REQUEST				0
#define FLAG_OK						1
#define FLAG_BAD_FRAME				2
#define FLAG_BAD_OPCODE				3
#define FLAG_BAD_PARAMS				4
#define FLAG_INTERNAL_ERROR			5

struct SerialFrame
{
	/** for tx: leadbyte; for rx: current state */
	uint8_t leadbyte_currstate;
	uint8_t opcode;
	uint8_t datalen;
	/** for tx: flags; for rx: current received bytes */
	uint8_t flags_rxlen;
	uint8_t data[SERIAL_FRAME_MAXSIZE - SERIAL_FRAME_HEADLEN];
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

	mConfig.rotation_real = 1.0f;
	mConfig.rotation_imag = 0.0f;

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
}

void loop()
{	
	if (trackpoint.reportAvailable()) {
		float real, imag;

		real = mConfig.rotation_real;
		imag = mConfig.rotation_imag;

		TrackPoint::DataReport d = trackpoint.getStreamReport();
		Mouse.move(
			d.x * real - d.y * imag, 
			d.x * imag + d.y * real, 
			0);
	} 

	if (Serial.available() > 0) {
		uint8_t inChar = (uint8_t) Serial.read();
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
				uint8_t rxlen = mSerialFrame.flags_rxlen;
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
				mSerialFrame.flags = SERIAL_OK;
				Serial.write(&mSerialFrame, SERIAL_FRAME_HEADLEN + sizeof(mConfig));
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
		default:
			responseSerialNoData(SERIAL_BAD_OPCODE);
			break;
	}
}

static void responseSerialNoData(uint8_t flags)
{
	mSerialFrame.leadbyte_currstate = SERIAL_FRAME_LEADBYTE;
	mSerialFrame.datalen = 0;
	mSerialFrame.flags_rxlen = flags;
	Serial.write(&mSerialFrame, SERIAL_FRAME_HEADLEN);
}

