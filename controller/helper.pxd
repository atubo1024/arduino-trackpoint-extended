from libc.stdint cimport uint8_t

cdef extern from 'helper.h':
    int    SERIALFRAME_HEADLEN
    int    SERIALFRAME_MAX_DATALEN
    int    SERIALFRAME_PENDING
    int	   SERIALFRAME_ACK

    struct SerialFrame:
	uint8_t leadbyte_currstate
	uint8_t opcode
	uint8_t datalen
	uint8_t flags_rxlen
	uint8_t data[SERIALFRAME_MAX_DATALEN]

    int    GetConfigItemCount()
    char** GetConfigItemNames()
    char** GetConfigItemTypes()
    
    int    GetOpCodeItemCount()
    char** GetOpCodeItemNames()
    int*   GetOpCodeItemValues()

    int GenerateSerialFrame(struct SerialFrame *pSerialFrame, uint8_t opcode, uint8_t* data, uint8_t datalen)
    void SerialFrame_Init(struct SerialFrame *self)
    uint8_t SerialFrame_PutChar(struct SerialFrame *self, uint8_t inChar)
