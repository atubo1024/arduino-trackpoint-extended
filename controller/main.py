#coding=utf-8

import wx
import serial.tools.list_ports
import serial
import logging
from threading import Thread
import wxformbuilder.dialog_main
import core
from mainthreadswitcher import run_in_mainthread, init_for_wxapp

logging.basicConfig(level=logging.INFO)

class MainDialog(wxformbuilder.dialog_main.MainDialog):
    def __init__(self, *args, **kwargs):
        super(MainDialog, self).__init__(*args, **kwargs)
        self.__state = 'closed'     # ['closed', 'opened']
        self.__serialports = None
        self.__pyserial_instance = None
        self.__refresh_serialports()

    def OnClose(self, event):
        if self.__pyserial_instance is not None:
            self.__pyserial_instance.close()
            self.__pyserial_instance = None
        self.Destroy()

    def OnBtnRefreshSerialPorts(self, event):
        self.__refresh_serialports()

    def OnBtnOpenSerialPort(self, event):
        devname = self.__get_selected_serial_devname()
        self.__pyserial_instance = serial.Serial(devname, timeout=None, baudrate=115200)
        config = core.get_config(self.__pyserial_instance)
        for key, val in config.iteritems():
            print '%s: %s' % (key, val)
        self.__state = 'opened'
        self.__on_state_changed()

    def OnBtnCloseSerialPort(self, event):
        if self.__pyserial_instance is not None:
            self.__pyserial_instance.close()
            self.__pyserial_instance = None
        self.__state = 'closed'
        self.__on_state_changed()

    def OnBtnSaveConfig(self, event):
        pass

    def OnCheckBoxDebug(self, event):
        if self.m_checkbox_debug.IsChecked():
            logging.getLogger(None).setLevel(logging.DEBUG)
        else:
            logging.getLogger(None).setLevel(logging.INFO)

    def OnBtnReboot(self, event):
        core.send_request(self.__pyserial_instance, core.OPCODES['OPCODE_REBOOT'], no_response=True)

    def OnBtnDumping(self, event):
        if self.__state == 'opened':
            self.__state = 'dumping'
            self.m_button_dump.SetLabel('Stop')
            th = Thread(target=self.__dump_worker)
            th.start()
        else:
            self.__state = 'opened'
            self.m_button_dump.SetLabel('Dump')

    def __on_state_changed(self):
        self.m_button_open.Enable(self.__state == 'closed')
        self.m_button_close.Enable(self.__state != 'closed')
        self.m_button_saveconfig.Enable(self.__state != 'closed')
        self.m_button_reboot.Enable(self.__state != 'closed')
        self.m_button_dump.Enable(self.__state != 'closed')

    def __get_selected_serial_devname(self):
        index = self.m_choice_serialports.GetSelection()
        if index >= 0 and index < len(self.__serialports):
            return self.__serialports[index]['device']
        return None

    def __refresh_serialports(self):
        ports = []
        for port in list(serial.tools.list_ports.comports()):
            if isinstance(port, (list, tuple)):
                # for pyserial 2.x
                ports.append({
                    'device': port[0],
                    'description': port[1]})
            else:
                # for pyserial 3.x
                ports.append({
                   'device': port.device,
                   'description': port.description})
        self.__serialports = ports

        selected_device = self.__get_selected_serial_devname()
        select_index = 0
        self.m_choice_serialports.Clear()
        for index, port in enumerate(ports):
            self.m_choice_serialports.Append('%s - %s' % (port['device'], port['description']))
            if port['device'] == selected_device:
                select_index = index
        if select_index < len(ports):
            self.m_choice_serialports.SetSelection(select_index)

    def __dump_worker(self):
        datalist = []
        core.send_request(self.__pyserial_instance, core.OPCODES['OPCODE_START_TP_DUMP'])
        while self.__state == 'dumping':
            datastr = core.send_request(core.OPCODES['OPCODE_TP_DATA'], no_request=True)
            data = struct.unpack('LBbb', datastr)
            datalist.append(data)
        core.send_request(self.__pyserial_instance, core.OPCODES['OPCODE_STOP_TP_DUMP'])
        self.__dump_completed(datalist)

    @run_in_mainthread
    def __dump_completed(self, datalist):
        for data in datalist:
            print data


class App(wx.App):
    def OnInit(self):
        init_for_wxapp(self)
        self.__maindialog = MainDialog(None)
        self.__maindialog.Show()
        self.SetTopWindow(self.__maindialog)
        return True

if __name__ == '__main__':
    app = App(redirect=False)
    app.MainLoop()
