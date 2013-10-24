import usb.core
import time
class USBCom:

    def __init__(self):
        print "Your UART cable better be plugged into the USB 2.0 port, or else kittens will cry"
        self.SET_VELOCITY = 0
        self.GET_VALS = 1
        self.dev = usb.core.find(idVendor = 0x6666, idProduct = 0x0003)
        if self.dev is None:
            raise ValueError('no USB device found matching idVendor = 0x6666 and idProduct = 0x0003')
        self.dev.set_configuration()

    def close(self):
        self.dev = None

    def set_velocity(self, duty, direction):
        try:
            self.dev.ctrl_transfer(0x40, self.SET_VELOCITY, int(duty), int(direction))
        except usb.core.USBError as e:
            print e
            print "Could not send SET_VELOCITY vendor request."
            raise e

    def get_vals(self):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.GET_VALS, 0, 0, 8)
        except usb.core.USBError:
            print "Could not send GET_VALS vendor request."
        else:
            return [int(ret[0])+int(ret[1])*256, int(ret[2])+int(ret[3])*256, int(ret[4])+int(ret[5])*256, int(ret[6])+int(ret[7])*256]

def values(u):
    l = u.get_vals()
    rev = l[0]
    fb = l[1]
    dir_sense = l[2]
    vemf = l[3]
    print "rev = %s \n fb = %s \n dir = %s \n vemf = %s" % (rev,fb,dir_sense,vemf)

if __name__ == '__main__':
    u = USBCom()
    stack = [0]
    # for speed in xrange(8000,50000,100):
    #     u.set_vals(speed,0)
    #     time.sleep(0.1)
    #     current = u.get_vals()[1]
    #     print "The speed is %s and the current detected is %s" % (speed,current)
    speed = int(raw_input("speed: 0 - 65536 \n"))
    direction = int(raw_input("direction: 1 or 0 \n"))
    u.set_velocity(speed,direction)
    while True:
        values(u)
        time.sleep(0.5)
    # while True:
    #     v = u.get_vals()[1]
    #     if v > stack[0]:
    #         stack = [v] + stack
    #         print v
