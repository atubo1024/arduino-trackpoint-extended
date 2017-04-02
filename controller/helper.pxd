from libc.stdint cimport uint8_t

cdef extern from 'helper.h':
    cdef int SERIALFRAME_LEADBYTE
    cdef int SERIALFRAME_HEADLEN
    cdef int SERIALFRAME_MAX_DATALEN
    cdef int SERIALFRAME_PENDING
    cdef int SERIALFRAME_ACK

    struct SerialFrame:
        uint8_t leadbyte_currstate
        uint8_t opcode
        uint8_t datalen
        uint8_t flags
        uint8_t *data

    int    GetConfigItemCount()
    char** GetConfigItemNames()
    char** GetConfigItemTypes()
    
    int    GetOpCodeItemCount()
    char** GetOpCodeItemNames()
    int*   GetOpCodeItemValues()

    int GenerateSerialFrame(SerialFrame *pSerialFrame, uint8_t opcode, uint8_t* data, uint8_t datalen)
    void SerialFrame_Init(SerialFrame *self)
    uint8_t SerialFrame_PutChar(SerialFrame *self, uint8_t inChar)
