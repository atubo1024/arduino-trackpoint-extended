
from libc.string cimport memcpy
from libc.stdint cimport uintptr_t
from helper cimport *
import struct
import ctypes
import logging

OPCODES = {}
CONFIG_ITEMS = []
CONFIG_FMTSTR = None

_CTYPE_TO_STRUCT_FMT = {
    'float'   : 'f',
    'int'     : 'i',
    'int32_t' : 'l',
    'uint32_t': 'L',
    'int8_t'  : 'b',
    'uint8_t' : 'B',
} 

cdef _init():
    global CONFIG_FMTSTR, OPCODES, CONFIG_ITEMS

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
    CONFIG_FMTSTR = get_config_fmtstr()

def get_config_fmtstr():
    cdef count = GetConfigItemCount()
    cdef char **types = GetConfigItemTypes()
    cdef i

    fmtchars = []
    for i from 0 <= i < count:
        typename = types[i]
        if typename not in _CTYPE_TO_STRUCT_FMT:
            raise AssertionError('unsupported ctype: %s' % typename)
        fmtchars.append(_CTYPE_TO_STRUCT_FMT[typename])

    return ''.join(fmtchars)

cdef int _GenerateSerialFrame(SerialFrame *pSerialFrame, uint8_t opcode, uint8_t* data, uint8_t datalen):
    """
    :return: Total len of SerialFrame, or 0 if error
    """
    if datalen > SERIALFRAME_MAX_DATALEN: return 0
    pSerialFrame.leadbyte = SERIALFRAME_LEADBYTE
    pSerialFrame.flags = 0
    pSerialFrame.opcode = opcode
    pSerialFrame.datalen = datalen
    if datalen > 0:
        memcpy(pSerialFrame.data, data, datalen)
    
    return SERIALFRAME_HEADLEN + datalen

def send_request(pyserial_instance, opcode, datastr=None):
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

def get_response(pyserial_instance, opcode=None):
    cdef SerialFrame frame

    # get response
    SerialFrame_Init(&frame)
    while True:
        ch = pyserial_instance.read(1)
        if ch is not None and len(ch) == 1:
            logging.debug('rx: 0x%02x', ord(ch))
            ret = SerialFrame_PutChar(&frame, ord(ch))
            logging.debug('putchar return: %d', ret)
            if ret == SERIALFRAME_ACK:
                # completed
                if frame.flags != SERIALFRAME_ACK:
                    raise AssertionError('RxSerialFrame Fail, ErrorCode: %d' % frame.flags)
                if opcode is not None and frame.opcode != opcode:
                    logging.debug('RxSerialFrame: unexpected opcode: %d', frame.opcode)
                    return None
                return ctypes.string_at(<uintptr_t>frame.data, frame.datalen)
            elif ret != SERIALFRAME_PENDING:
                # something error
                raise AssertionError('RxSerialFrame Fail, ErrorCode: %d' % ret)

def get_config(pyserial_instance):
    send_request(pyserial_instance, OPCODES['OPCODE_GET_CONFIG'])
    datastr = get_response(pyserial_instance, OPCODES['OPCODE_GET_CONFIG'])
    values = struct.unpack(CONFIG_FMTSTR, datastr)
    config = {}
    for i from 0 <= i < len(values):
        config[CONFIG_ITEMS[i]] = values[i]
    return config

def set_config(pyserial_instance, config):
    values = []
    for name in CONFIG_ITEMS:
        values.append(config[name])
    datastr = struct.pack(CONFIG_FMTSTR, values)
    send_request(pyserial_instance, OPCODES['OPCODE_SET_CONFIG'], datastr)
    get_response(pyserial_instance, OPCODES['OPCODE_SET_CONFIG'])

_init()
