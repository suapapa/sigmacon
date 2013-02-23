env = Environment()
env.SConscript('emsuinput/SConstruct')
env.ParseConfig('pkg-config --cflags --libs libusb-1.0')
env.Program('sigmacon', ['sigmacon.cpp', 'emsuinput/libemsuinput.a'])

Default('sigmacon')
