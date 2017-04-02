#coding=utf-8

"""
This module help switching execeution from a sub thread to main thread in a simple way.
It is useful for multiple threads controll for GUI application.

This module provide a decorator to declear a function that support executing in the main thread only.
It means that, if this function is called from a sub thread, the decorator will post an 
event, and execute it in the main thread later; if the function called from main thread, 
it will be executed immediately just like calling a normal function.
"""

import logging
from threading import current_thread
from functools import wraps

_main_thread_id = current_thread().ident
_async_task_handler = None

def setup(wxappinst=None, messagelooper=None):
    global _async_task_handler
    if wxappinst is not None:
        _async_task_handler = _wxAsyncTaskHandler(wxappinst)
    elif messagelooper is not None:
        _async_task_handler = _wxAsyncTaskHandler(wxappinst)
    else:
        raise AssertionError("require a message loop provider")
def init_for_wxapp(wxapp):
    """ deprecated, replace by setup """
    global _async_task_handler
    _async_task_handler = _wxAsyncTaskHandler(wxapp)
def init_for_console_app(looper):
    """ deprecated, replace by setup """
    global _async_task_handler
    _async_task_handler = _MessageLooperTaskHandler(looper)

def get_mainthread_ident():
    return _main_thread_id

def run_in_mainthread(fn):
    @wraps(fn)
    def inner(*args, **kwargs):
        if _async_task_handler is None:
            raise AssertionError('mainthreadswitcher: calling before initialization')
        if current_thread().ident == _main_thread_id:
            ret = fn(*args, **kwargs)
            if ret != None:
                logging.warning("mainthreadswitcher: return a result in the wrapped function is not recommended, using an event driven model instead.")
        else:
            _async_task_handler.run(fn, *args, **kwargs)
    return inner

class _wxAsyncTaskHandler(object):
    def __init__(self, wxapp):
        import wx
        self.__wxapp = wxapp
        self.__event_type = wx.NewEventType()
        wxapp.Bind(wx.PyEventBinder(self.__event_type), self.__on_message)
    def __on_message(self, event):
        try:
            event.fn(*event.args, **event.kwargs)
        except Exception, ex:
            logging.exception(ex)
    def run(self, fn, *args, **kwargs):
        import wx
        event = wx.PyCommandEvent(self.__event_type)
        event.fn = fn
        event.args = args
        event.kwargs = kwargs
        wx.PostEvent(self.__wxapp, event)
class _MessageLooperTaskHandler(object):
    def __init__(self, looper):
        self.__looper = looper
        self.__looper.add_listener('mainthreadswitcher', self.__on_message)
    def __on_message(self, event=None, data=None):
        try:
            data['fn'](*data['args'], **data['kwargs'])
        except Exception, ex:
            logging.exception(ex)
    def run(self, fn, *args, **kwargs):
        self.__looper.post_event('mainthreadswitcher', data={'fn':fn, 'args':args, 'kwargs':kwargs})
