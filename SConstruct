env = Environment()
env.ParseConfig('pkg-config --cflags --libs libusb-1.0')
env.Program('test', 'sigmacon.cpp')

# Program("multi_remocon_rcv_test.c", LIBS=["usb"])
