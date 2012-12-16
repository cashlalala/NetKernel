moduleName      = "NetKernel"
moduleVersion   = 0.1

def GetNetKernel():
    import subsys.NetKernel.NetKernel
    return subsys.NetKernel.NetKernel

export_interfaces = [
    ("INetKernel", 0.1, GetNetKernel), # (interface, version number, init function)
]

# This class hold a window handle that help AP
# to retrieve it anywhere.
class NetWndStorage(object):
    def __init__(self):
        object.__init__(self)
        self.__windowHandle = None
    
    def __getNetWndHandle(self):
        return self.__windowHandle
    
    def __setNetWndHandle(self, wndHandle):
        self.__windowHandle = wndHandle
        
    windowHandle = property(__getNetWndHandle, __setNetWndHandle)
    
    def hasHandle(self):
        return self.__windowHandle is not None
    
hwndStore = None

def getWndHandle():
    global hwndStore
    if hwndStore is None:
        hwndStore = NetWndStorage()
    return hwndStore

def releaseWndHandle():
    global hwndStore
    hwndStore = None