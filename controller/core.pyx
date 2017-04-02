
from libc.string cimport memcpy
from helper cimport *
import struct
import ctypes
import logging

OPCODES = {}
CONFIG_ITEMS = []

_CTYPE_TO_STRUCT_FMT = {
    'float'   : 'f',
    'int'     : 'i',
    'int32_t' : 'l',
    'uint32_t': 'L',
    'int8_t'  : 'b',
    'uint8_t' : 'B',
} 

cdef _init():
    cdef count
    cdef char **names
    cdef int *values
    cdef i

    count = GetOpCodeItemCount()
    names = GetOpCodeItemNames()
    values = GetOpCodeItemValues()
    for i from 0 <= i < count:
        OPCODES[names[i]] = values[i]

    count = GetConfigItemCount()
    names = GetConfigItemNames()
    for i from 0 <= i < count:
        CONFIG_ITEMS.append(names[i])
_init()

def get_struct_fmtstr_of_config():
    cdef count = GetConfigItemCount()
    cdef char **ctypes = GetConfigItemTypes()
    cdef i

    fmtchars = []
    for i from 0 <= i < count:
        typename = ctypes[i]
        if typename not in _CTYPE_TO_STRUCT_FMT:
            raise AssertionError('unsupported ctype: %s' % typename)
        fmtchars.append(_CTYPE_TO_STRUCT_FMT[typename])

    return ''.join(fmtchars)

cdef int _GenerateSerialFrame(SerialFrame *pSerialFrame, uint8_t opcode, uint8_t* data, uint8_t datalen):
    """
    :return: Total len of SerialFrame, or 0 if error
    """
    if datalen > SERIALFRAME_MAX_DATALEN: return 0
    pSerialFrame.leadbyte_currstate = SERIALFRAME_LEADBYTE
    pSerialFrame.flags_rxlen = 0
    pSerialFrame.opcode = opcode
    pSerialFrame.datalen = datalen
    if datalen > 0:
        memcpy(pSerialFrame.data, data, datalen)
    
    return SERIALFRAME_HEADLEN + datalen

def send_request(pyserial_instance, opcode, datastr):
    """
    :return: datastr or raise Exception if fail
    """
    cdef SerialFrame frame
    cdef int ret
    cdef int i
    cdef uint8_t *pBuffer
    cdef uint8_t *pData
    cdef uint8_t datalen

    # send request
    if datastr is None:
        pData = NULL
        datalen = 0
    else:
        pData = datastr
        datalen = len(datastr)
    ret = _GenerateSerialFrame(&frame, opcode, pData, datalen);
    assert(ret > 0);
    pBuffer = <uint8_t *>(&frame)
    for i from 0 <= i < ret:
        logging.debug('tx: 0x%02x', pBuffer[i])
        pyserial_instance.write(chr(pBuffer[i]))

    # get response
    SerialFrame_Init(&frame)
    while True:
        ch = pyserial_instance.read(1)
        logging.debug('rx: 0x%02x', ch)
        if ch is not None and len(ch) == 1:
            ret = SerialFrame_PutChar(&frame, ord(ch))
            if ret == SERIALFRAME_ACK:
                # completed
                if frame.flags_rxlen != SERIALFRAME_ACK:
                    raise AssertionError('RxSerialFrame Fail, ErrorCode: %d' % frame.flags_rxlen)
                return ctypes.string_at(frame.data, frame.datalen)
            elif ret != SERIALFRAME_PENDING:
                # something error
                raise AssertionError('RxSerialFrame Fail, ErrorCode: %d' % ret)


def get_config(pyserial_instance):
    datastr = send_request(pyserial_instance, OPCODES['OPCODE_GET_CONFIG'], None)
    values = struct.unpack(get_struct_fmtstr_of_config(), datastr)
    config = {}
    for i from 0 <= i < len(values):
        config[CONFIG_ITEMS[i]] = values[i]
    return config

def set_config(pyserial_instance, config):
    values = []
    for name in CONFIG_ITEMS:
        values.append(config[name])
    datastr = struct.pack(get_struct_fmtstr_of_config(), values)
    send_request(pyserial_instance, OPCODES['OPCODE_SET_CONFIG'], datastr)

