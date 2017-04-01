#coding=utf-8

import wx
import wxformbuilder.dialog_main
import serial.tools.list_ports

class MainDialog(wxformbuilder.dialog_main.MainDialog):
    def __init__(self, *args, **kwargs):
        super(MainDialog, self).__init__(*args, **kwargs)
        self.__serialports = None
        self.__refresh_serialports()

    def OnClose(self, event):
        self.Destroy()

    def OnBtnRefreshSerialPorts(self, event):
        self.__refresh_serialports()

    def OnBtnOpenSerialPort(self, event):
        pass

    def OnBtnCloseSerialPort(self, event):
        pass

    def OnBtnSaveConfig(self, event):
        pass

    def __refresh_serialports(self):
        selected_device = None
        index = self.m_choice_serialports.GetSelection()
        if index >= 0 and index < len(self.__serialports):
            selected_device = self.__serialports[index]['device']

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
