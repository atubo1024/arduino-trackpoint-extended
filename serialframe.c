
#include "serialframe.h"

enum SERIAL_STATE_DEF
{
	STATE_PENDING,		/* when rx leaduint8_t => STATE_START_1 */
	STATE_START_1,		/* when rx opcode => STATE_START_2 */
	STATE_START_2,		/* when rx datalen => STATE_RXDATA */
	STATE_RXDATA,		/* when rx completed => handling => tx response => STATE_PENDING */
};

void SerialFrame_Init(struct SerialFrame *self)
{
	self->leadbyte_currstate = STATE_PENDING;
}

/**
 * @return enum SerialFrameErrCode. (return SERIALFRAME_PENDING if not completed, or SERIALFRAME_ACK when completed) */
uint8_t SerialFrame_PutChar(struct SerialFrame *self, uint8_t inChar)
{
	uint8_t rxlen = 0;
	switch (self->leadbyte_currstate) {
		case STATE_PENDING:
			if (inChar == SERIAL_FRAME_LEADBYTE) {
				self->leadbyte_currstate = STATE_START_1;
			}
			break;
		case STATE_START_1:
			if (inChar != 0) {
				self->leadbyte_currstate = STATE_PENDING;
			} else {
				self->opcode = inChar;
				self->leadbyte_currstate = STATE_START_2;
			}
			break;
		case STATE_START_2:
			if (inChar > sizeof(self->data)) {
				/* illegal access */
				self->leadbyte_currstate = STATE_PENDING;
				return SERIALFRAME_BAD_FRAME;
			}
			self->datalen = inChar;
			self->flags_rxlen = 0;
			self->leadbyte_currstate = STATE_RXDATA;
			break;
		case STATE_RXDATA:
			rxlen = self->flags_rxlen;
			self->data[rxlen++] = inChar;
			if (rxlen >= self->datalen) {
				/* rx frame completed */
				self->leadbyte_currstate = STATE_PENDING;
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


