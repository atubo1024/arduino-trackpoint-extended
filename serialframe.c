
#include "serialframe.h"

enum SERIAL_STATE_DEF
{
	STATE_WAIT_LEADBYTE,			/* when rx leaduint8_t => STATE_WAIT_OPCODE */
	STATE_WAIT_OPCODE,		/* when rx opcode => STATE_WAIT_DATALEN */
	STATE_WAIT_FLAG,
	STATE_WAIT_DATALEN,		/* when rx datalen => STATE_RXDATA */
	STATE_RXDATA,			/* when rx completed => handling => tx response => STATE_WAIT_LEADBYTE */
};

void SerialFrame_Init(struct SerialFrame *self)
{
	self->leadbyte_currstate = STATE_WAIT_LEADBYTE;
}

/**
 * @return enum SerialFrameErrCode. (return SERIALFRAME_PENDING if not completed, or SERIALFRAME_ACK when completed) */
uint8_t SerialFrame_PutChar(struct SerialFrame *self, uint8_t inChar)
{
	uint8_t rxlen = 0;
	switch (self->leadbyte_currstate) {
		case STATE_WAIT_LEADBYTE:
			if (inChar == SERIALFRAME_LEADBYTE) {
				self->leadbyte_currstate = STATE_WAIT_OPCODE;
			}
			break;
		case STATE_WAIT_OPCODE:
			self->opcode = inChar;
			self->leadbyte_currstate = STATE_WAIT_FLAG;
			break;
		case STATE_WAIT_FLAG:
			if (inChar != 0) {
				self->leadbyte_currstate = STATE_WAIT_LEADBYTE;
			} else {
				self->leadbyte_currstate = STATE_WAIT_DATALEN;
			}
			break;
		case STATE_WAIT_DATALEN:
			if (inChar > sizeof(self->data)) {
				/* illegal access */
				self->leadbyte_currstate = STATE_WAIT_LEADBYTE;
				return SERIALFRAME_BAD_FRAME;
			}
			self->datalen = inChar;
			self->flags_rxlen = 0;
			if (inChar == 0) {
				self->leadbyte_currstate = STATE_WAIT_LEADBYTE;
				return SERIALFRAME_ACK;
			} else {
				self->leadbyte_currstate = STATE_RXDATA;
			}
			break;
		case STATE_RXDATA:
			rxlen = self->flags_rxlen;
			self->data[rxlen++] = inChar;
			if (rxlen >= self->datalen) {
				/* rx frame completed */
				self->leadbyte_currstate = STATE_WAIT_LEADBYTE;
				return SERIALFRAME_ACK;
			} else {
				self->flags_rxlen = rxlen;
			}
			break;
		default:
			break;
	}
	
	return SERIALFRAME_PENDING;
}


