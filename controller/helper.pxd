from libc.stdint cimport uint8_t, int16_t, uint16_t, uint16_t

cdef extern from 'helper.h':
    int    GetConfigItemCount()
    char** GetConfigItemNames()
    char** GetConfigItemTypes()
    
    int    GetOpCodeItemCount()
    char** GetOpCodeItemNames()
    int*   GetOpCodeItemValues()

cdef extern from '../serialframe.h':
    cdef int SERIALFRAME_LEADBYTE
    cdef int SERIALFRAME_HEADLEN
    cdef int SERIALFRAME_MAX_DATALEN
    cdef int SERIALFRAME_PENDING
    cdef int SERIALFRAME_ACK

    struct SerialFrame:
        uint8_t leadbyte
        uint8_t opcode
        uint8_t datalen
        uint8_t flags
        uint8_t *data

    void SerialFrame_Init(SerialFrame *self)
    uint8_t SerialFrame_PutChar(SerialFrame *self, uint8_t inChar)

cdef extern from '../lease_square.h':
    struct MovingLeaseSquare:
        int16_t current_value
        uint16_t count
    void MLS_Init(
    	MovingLeaseSquare *self, 
    	uint16_t  winsize, 
    	uint16_t *buffer_time, 
    	int16_t  *buffer_value, 
    	uint16_t  timeout_interval)
    void MLS_RemoveTimeout(MovingLeaseSquare *self, uint16_t now)
    void MLS_Append(MovingLeaseSquare *self, uint16_t now, int16_t value)


