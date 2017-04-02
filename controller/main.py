#coding=utf-8

import wx
import wxformbuilder.dialog_main
import serial.tools.list_ports
import serial
import core
import logging

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
        self.__pyserial_instance = serial.Serial(devname, timeout=None, baudrate=9600)
        config = core.get_config(self.__pyserial_instance)
        for key, val in config.iteritems():
            print '%s: %s' % (key, val)
        self.__state = 'opened'
        self.__on_state_changed()

    def OnBtnCloseSerialPort(self, event):
        if self.__pyserial_instance is not None:
            self.__pyserial_instance.close()
            self.__pyserial_instance = None

    def OnBtnSaveConfig(self, event):
        pass

    def OnCheckBoxDebug(self, event):
        if self.m_checkbox_debug.IsChecked():
            logging.getLogger(None).setLevel(logging.DEBUG)
        else:
            logging.getLogger(None).setLevel(logging.INFO)

    def __on_state_changed(self):
        self.m_button_open.Enable(self.__state == 'closed')
        self.m_button_close.Enable(self.__state != 'closed')
        self.m_button_saveconfig.Enable(self.__state != 'closed')

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

class App(wx.App):
    def OnInit(self):
        self.__maindialog = MainDialog(None)
        self.__maindialog.Show()
        self.SetTopWindow(self.__maindialog)
        return True

if __name__ == '__main__':
    app = App(redirect=False)
    app.MainLoop()
