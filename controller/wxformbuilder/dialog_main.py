# -*- coding: utf-8 -*- 

###########################################################################
## Python code generated with wxFormBuilder (version Jun 17 2015)
## http://www.wxformbuilder.org/
##
## PLEASE DO "NOT" EDIT THIS FILE!
###########################################################################

import wx
import wx.xrc
import wx.propgrid as pg

###########################################################################
## Class MainDialog
###########################################################################

class MainDialog ( wx.Dialog ):
	
	def __init__( self, parent ):
		wx.Dialog.__init__ ( self, parent, id = wx.ID_ANY, title = u"TrackPointController", pos = wx.DefaultPosition, size = wx.Size( 708,506 ), style = wx.CLOSE_BOX|wx.DEFAULT_DIALOG_STYLE|wx.MAXIMIZE_BOX|wx.MINIMIZE_BOX|wx.RESIZE_BORDER )
		
		self.SetSizeHintsSz( wx.DefaultSize, wx.DefaultSize )
		
		bSizer1 = wx.BoxSizer( wx.VERTICAL )
		
		bSizer2 = wx.BoxSizer( wx.HORIZONTAL )
		
		m_choice_serialportsChoices = []
		self.m_choice_serialports = wx.Choice( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, m_choice_serialportsChoices, 0 )
		self.m_choice_serialports.SetSelection( 0 )
		bSizer2.Add( self.m_choice_serialports, 1, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.m_button_open = wx.Button( self, wx.ID_ANY, u"打开", wx.DefaultPosition, wx.DefaultSize, 0 )
		bSizer2.Add( self.m_button_open, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.m_button_close = wx.Button( self, wx.ID_ANY, u"关闭", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_button_close.Enable( False )
		
		bSizer2.Add( self.m_button_close, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.m_button3 = wx.Button( self, wx.ID_ANY, u"刷新", wx.DefaultPosition, wx.DefaultSize, 0 )
		bSizer2.Add( self.m_button3, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )
		
		
		bSizer1.Add( bSizer2, 0, wx.EXPAND, 5 )
		
		self.m_propertygrid_tpconfig = pg.PropertyGrid(self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.propgrid.PG_DEFAULT_STYLE)
		bSizer1.Add( self.m_propertygrid_tpconfig, 1, wx.ALL|wx.EXPAND, 5 )
		
		bSizer3 = wx.BoxSizer( wx.HORIZONTAL )
		
		self.m_checkbox_debug = wx.CheckBox( self, wx.ID_ANY, u"Debug", wx.DefaultPosition, wx.DefaultSize, 0 )
		bSizer3.Add( self.m_checkbox_debug, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )
		
		
		bSizer3.AddSpacer( ( 0, 0), 1, wx.EXPAND, 5 )
		
		self.m_button_reboot = wx.Button( self, wx.ID_ANY, u"重启", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_button_reboot.Enable( False )
		
		bSizer3.Add( self.m_button_reboot, 0, wx.ALL, 5 )
		
		self.m_button_dump = wx.Button( self, wx.ID_ANY, u"Dump", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_button_dump.Enable( False )
		
		bSizer3.Add( self.m_button_dump, 0, wx.ALL, 5 )
		
		self.m_button_saveconfig = wx.Button( self, wx.ID_ANY, u"保存配置", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_button_saveconfig.Enable( False )
		
		bSizer3.Add( self.m_button_saveconfig, 0, wx.ALL, 5 )
		
		
		bSizer1.Add( bSizer3, 0, wx.EXPAND, 5 )
		
		
		self.SetSizer( bSizer1 )
		self.Layout()
		
		self.Centre( wx.BOTH )
		
		# Connect Events
		self.Bind( wx.EVT_CLOSE, self.OnClose )
		self.m_button_open.Bind( wx.EVT_BUTTON, self.OnBtnOpenSerialPort )
		self.m_button_close.Bind( wx.EVT_BUTTON, self.OnBtnCloseSerialPort )
		self.m_button3.Bind( wx.EVT_BUTTON, self.OnBtnRefreshSerialPorts )
		self.m_checkbox_debug.Bind( wx.EVT_CHECKBOX, self.OnCheckBoxDebug )
		self.m_button_reboot.Bind( wx.EVT_BUTTON, self.OnBtnReboot )
		self.m_button_dump.Bind( wx.EVT_BUTTON, self.OnBtnDumping )
		self.m_button_saveconfig.Bind( wx.EVT_BUTTON, self.OnBtnSaveConfig )
	
	def __del__( self ):
		pass
	
	
	# Virtual event handlers, overide them in your derived class
	def OnClose( self, event ):
		event.Skip()
	
	def OnBtnOpenSerialPort( self, event ):
		event.Skip()
	
	def OnBtnCloseSerialPort( self, event ):
		event.Skip()
	
	def OnBtnRefreshSerialPorts( self, event ):
		event.Skip()
	
	def OnCheckBoxDebug( self, event ):
		event.Skip()
	
	def OnBtnReboot( self, event ):
		event.Skip()
	
	def OnBtnDumping( self, event ):
		event.Skip()
	
	def OnBtnSaveConfig( self, event ):
		event.Skip()
	

