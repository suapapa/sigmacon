#include <iostream>
#include <libusb.h>

#include "emsuinput/src/emsuinput.h"

#define SIGMACON_VID 0x10c8
#define SIGMACON_PID 0x0910

using namespace std;

int main()
{
    libusb_context* ctx = NULL; //a libusb session
    int r; //for return values
    r = libusb_init(&ctx); //initialize the library for the session we just declared
    if (r < 0) {
        cout << "Init Error " << r << endl; //there was an error
        return 1;
    }
    libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation

#if 0
    libusb_device** devs; //pointer to pointer of device, used to retrieve a list of devices
    ssize_t cnt; //holding number of devices in list
    cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
    if (cnt < 0) {
        cout << "Get Device Error" << endl; //there was an error
        return 1;
    }
    cout << cnt << " Devices in list." << endl;
    libusb_free_device_list(devs, 1); //free the list, unref the devices in it
#endif

    libusb_device_handle* dev_hdl;
    dev_hdl = libusb_open_device_with_vid_pid(ctx, SIGMACON_VID, SIGMACON_PID);
    if (dev_hdl == NULL) {
        cout << "Cannot open device" << endl;
        return 1;
    }
    //libusb_free_device_list(devs, 1); //free the list, unref the devices in it

    // find out if kernel driver is attached
    if (libusb_kernel_driver_active(dev_hdl, 0) == 1) {
        cout << "Kernel Driver Active" << endl;
        return 1;
        // if (libusb_detach_kernel_driver(dev_hdl, 0) == 0)
        //     cout << "Kernel Driver Detached!" << endl;
    }

    r = libusb_claim_interface(dev_hdl, 0);
    if (r < 0) {
        cout << "Cannot Claim Interface" << endl;
        return 1;
    }

    emsuinput_context* ui_ctx = emsuinput_new_context("sigmacon",
                                NULL, 0, NULL, 0);

    unsigned char code_in[8] = { 0 };
    int len = 0;
    while (1) {
        r = libusb_interrupt_transfer(dev_hdl, (1 | LIBUSB_ENDPOINT_IN), code_in, 8, &len, 1000);
        if (r == -1) {
            cerr << "exit int. trasfer loop" << endl;
            break;
        }

        if (code_in[0] == 0x01 || code_in[1] == 0x02 || code_in[2] == 0x08) {
            cout << "keycode : " << code_in[3] << endl;
        }
        /* fflush(NULL); */
        usleep(10 * 1000);
    }

    r = libusb_release_interface(dev_hdl, 0); //release the claimed interface
    if (r != 0) {
        cout << "Cannot Release Interface" << endl;
        return 1;
    }
    cout << "Released Interface" << endl;

    emsuinput_release_context(ui_ctx);
    libusb_close(dev_hdl); //close the device we opened
    libusb_exit(ctx); //needs to be called to end the

    return 0;
}
