#ifndef __HAVE_SERIALFRAME_H
#define __HAVE_SERIALFRAME_H

#include <stdint.h>

#define SERIAL_FRAME_MAX_DATALEN	32
#define SERIAL_FRAME_HEADLEN		4
#define SERIAL_FRAME_LEADBYTE		0xe5u

enum SerialFrameErrCode
{
	SERIALFRAME_PENDING				= 0,
	SERIALFRAME_ACK					= 1,
	SERIALFRAME_BAD_FRAME			= 2,
	SERIALFRAME_BAD_OPCODE			= 3,
	SERIALFRAME_BAD_PARAMS			= 4,
	SERIALFRAME_INTERNAL_ERROR		= 5,
};

struct SerialFrame
{
	/** for tx: leadbyte; for rx: current state */
	uint8_t leadbyte_currstate;
	uint8_t opcode;
	uint8_t datalen;
	/** for tx: flags; for rx: current received uint8_ts */
	uint8_t flags_rxlen;
	uint8_t data[SERIAL_FRAME_MAX_DATALEN];
};

void SerialFrame_Init(struct SerialFrame *self);
uint8_t SerialFrame_PutChar(struct SerialFrame *self, uint8_t inChar);

#endif  /* #ifndef __HAVE_SERIALFRAME_H */

