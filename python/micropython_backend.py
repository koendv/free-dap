# file pyOCD/pyocd/probe/pydapaccess/interface/pyusb_backend.py

# limit packet queue to 1 packet. In dap_settings.py put: 
# limit_packets = True

import machine, dap, ubinascii

IS_AVAILABLE = True

class PyUSB(object):
    """! @brief CMSIS-DAP interface class using micropython mod_dap for the backend.
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
        self.rcv_valid = False
    
    @property
    def has_swo_ep(self):
        return False

    def open(self):
        return

    def close(self):
        return

    def write(self, data):
        if len(data) != self.packet_size:
            data.extend([0]*self.packet_size)
            data=data[0:self.packet_size-1]
        request = bytearray(data)
        response = bytearray(self.packet_size)
        self.rcv_valid = dap.process(request, response)
        if self.rcv_valid:
            self.rcv_data = list(response)
        else:
            self.rcv_data = []
        return

    def read(self, size=-1, timeout=-1):
        data = self.rcv_data
        self.rcv_data = []
        self.rcv_valid = False
        return data

    def get_info(self):
        return self.vendor_name + " " + \
               self.product_name + " (" + \
               str(hex(self.vid)) + ", " + \
               str(hex(self.pid)) + ")"

    def get_packet_count(self):
        return self.packet_count

    def set_packet_count(self, count):
        assert count == self.packet_count

    def set_packet_size(self, size):
        assert size == self.packet_size

    def get_packet_size(self):
        return self.packet_size

    def get_serial_number(self):
        return self.serial_number
