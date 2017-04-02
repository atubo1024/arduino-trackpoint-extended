#ifndef __HAVE_SERIALFRAME_H
#define __HAVE_SERIALFRAME_H

#include <stdint.h>

#define SERIALFRAME_MAX_DATALEN	32
#define SERIALFRAME_HEADLEN		4
#define SERIALFRAME_LEADBYTE		0xe5u

enum SerialFrameErrCode
{
	SERIALFRAME_PENDING				= 0,
	SERIALFRAME_REQUEST				= 0,
	SERIALFRAME_ACK					= 1,
	SERIALFRAME_BAD_FRAME			= 2,
	SERIALFRAME_BAD_OPCODE			= 3,
	SERIALFRAME_BAD_PARAMS			= 4,
	SERIALFRAME_INTERNAL_ERROR		= 5,
};

struct SerialFrame
{
	/** for tx: leadbyte; for rx: current state */
	uint8_t leadbyte;
	uint8_t opcode;
	uint8_t flags;
	uint8_t datalen;
	uint8_t data[SERIALFRAME_MAX_DATALEN];
	uint8_t __currstate;
	uint8_t __rxlen;
	uint8_t reserve_1;
	uint8_t reserve_2;
};

void SerialFrame_Init(struct SerialFrame *self);
uint8_t SerialFrame_PutChar(struct SerialFrame *self, uint8_t inChar);

#endif  /* #ifndef __HAVE_SERIALFRAME_H */

