#coding=utf-8

import logging
from .wxformbuilder.panel_serialselector import SerialSelectorBase
import serial.tools.list_ports

class SerialSelector(SerialSelectorBase):
    def __init__(self, parent, *args, **kwargs):
        super(SerialSelector, self).__init__(parent)

        self.__controller = None
        self.__ports = None
    def set_controller(self, controller):
        self.__controller = controller
        self.__refresh_serial_ports()
    def __get_select_device(self):
        index = self.m_choice_list.GetSelection()
        if index >= 0 and index < len(self.__ports):
            return self.__ports[index]['device']
        else:
            return 0
    def __get_baudrate(self):
        if self.m_radio_9600.GetValue():
            return 9600
        elif self.m_radio_115200.GetValue():
            return 115200
        else:
            return 9600
    def OnBtnSwitchConnection(self, event):
        self.__controller.set_serial_config(self.__get_select_device(), self.__get_baudrate())
    def OnBtnRefresh(self, event):
        self.__refresh_serial_ports(select_device=self.__get_select_device())
    def __refresh_serial_ports(self, select_device=None):
        ports = []
        for port in list(serial.tools.list_ports.comports()):
            # @note pyserial 3.x does not support windows xp, so we use pyserial 2.x instead.
            # @note the 'port' data structure in pyserial 2.x like below:
            #   ('COM4', 'USB Serial Port (COM4)', 'FTDIBUS\\VID_0403+PID_6001+AL00W58LA\\0000')
            #   ('COM3', 'Prolific USB-to-Serial Comm Port (COM3)', 'USB VID:PID=067B:2303 SNR=7')
            # 
            # ports.append({
            #     'device': port.device,
            #     'description': port.description})
            ports.append({
                'device': port[0],
                'description': port[1]})
        self.__ports = ports

        select_index = 0
        self.m_choice_list.Clear()
        for index, port in enumerate(ports):
            self.m_choice_list.Append('%s - %s' % (port['device'], port['description']))
            if port['device'] == select_device:
                select_index = index
        if select_index < len(ports):
            self.m_choice_list.SetSelection(select_index)
    

