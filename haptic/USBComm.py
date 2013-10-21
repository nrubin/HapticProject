import usb.core
import time
class USBCom:

    def __init__(self):
        print "Your UART cable better be plugged into the USB 2.0 port, or else kittens will cry"
        self.HELLO = 0
        self.SET_VALS = 1
        self.GET_VALS = 2
        self.PRINT_VALS = 3
        self.dev = usb.core.find(idVendor = 0x6666, idProduct = 0x0003)
        if self.dev is None:
            raise ValueError('no USB device found matching idVendor = 0x6666 and idProduct = 0x0003')
        self.dev.set_configuration()

    def close(self):
        self.dev = None

    def hello(self):
        try:
            self.dev.ctrl_transfer(0x40, self.HELLO)
        except usb.core.USBError:
            print "Could not send HELLO vendor request."

    def set_vals(self, val1, val2):
        try:
            self.dev.ctrl_transfer(0x40, self.SET_VALS, int(val1), int(val2))
        except usb.core.USBError as e:
            print e
            print "Could not send SET_VALS vendor request."
            raise e

    def get_vals(self):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.GET_VALS, 0, 0, 4)
        except usb.core.USBError:
            print "Could not send GET_VALS vendor request."
        else:
            return [int(ret[0])+int(ret[1])*256, int(ret[2])+int(ret[3])*256]

    def print_vals(self):
        try:
            self.dev.ctrl_transfer(0x40, self.PRINT_VALS)
        except usb.core.USBError:
            print "Could not send PRINT_VALS vendor request."

if __name__ == '__main__':
    u = USBCom()
    stack = [0]
    # for speed in xrange(8000,50000,100):
    #     u.set_vals(speed,0)
    #     time.sleep(0.1)
    #     current = u.get_vals()[1]
    #     print "The speed is %s and the current detected is %s" % (speed,current)
    # while True:
    #     speed = int(raw_input("set the motor speed: 0 - 65536\n"))
    #     u.set_vals(speed,0)
    #     vals = u.get_vals()
    #     print "the vals are %s" % vals
    while True:
        v = u.get_vals()[1]
        if v > stack[0]:
            stack = [v] + stack
            print v
