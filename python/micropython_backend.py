# file pyOCD/pyocd/probe/pydapaccess/interface/pyusb_backend.py

# backend for micropython built-in free-dap module

import machine, dap, ubinascii

IS_AVAILABLE = True

class PyUSB(object):
    """! @brief CMSIS-DAP interface class using micropython builtin dap for the backend.
    """

    def __init__(self):
        self.vid = 0x0d28
        self.pid = 0x0204
        self.vendor_name = "Micropython"
        self.product_name = "Generic CMSIS-DAP Adapter"
        self.serial_number = str(ubinascii.hexlify(machine.unique_id()),'ascii')
        self.packet_count = 1
        self.packet_size = 64
        self.rcv_data = []
    
    def write(self, data):
        data.extend([0]*(self.packet_size-len(data)))
        request = bytearray(data[0:self.packet_size])
        response = bytearray(self.packet_size)
        response_valid = dap.process(request, response)
        if response_valid:
            self.rcv_data = list(response)
        else:
            self.rcv_data = []
        return

    def read(self, size=-1, timeout=-1):
        data = self.rcv_data
        self.rcv_data = []
        return data

    def set_packet_count(self, count):
        assert count == self.packet_count

    def set_packet_size(self, size):
        assert size == self.packet_size

# not truncated
