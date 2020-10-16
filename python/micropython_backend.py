# file pyOCD/pyocd/probe/pydapaccess/interface/pyusb_backend.py

# limit packet queue to 1 packet. In dap_settings.py put: 
# limit_packets = True

import machine, dap, ubinascii
import ulogging as logging
from dap_access_api import DAPAccessIntf

LOG = logging.getLogger(__name__)
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
        self.closed = True 
    
    @property
    def has_swo_ep(self):
        return False

    def open(self):
        assert self.closed is True
        self.closed = False
        return

    def close(self):
        assert self.closed is False
        self.closed = True
        return

    def write(self, data):
        """! @brief Write data on the OUT endpoint associated to the HID interface
        """

        for _ in range(self.packet_size - len(data)):
            data.append(0)
        request = bytearray(data)
        response = bytearray(64)
        self.rcv_valid = dap.process(request, response)
        self.rcv_data = list(response)
        return

    def read(self, size=-1, timeout=-1):
        """! @brief Read data on the IN endpoint associated to the HID interface
        """

        if not self.rcv_valid:
            LOG.debug('read without write')

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
        assert count == 1

    def set_packet_size(self, size):
        assert size == 64

    def get_packet_size(self):
        return self.packet_size

    def get_serial_number(self):
        return self.serial_number
