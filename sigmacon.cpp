#include <iostream>
#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>

#include "emsuinput/src/emsuinput.h"

#define SIGMACON_VID 0x10c8
#define SIGMACON_PID 0x0910

using namespace std;

static unsigned char r_prefix[3] = {0x01, 0x02, 0x80};
static unsigned char r_btn_v[] = {
    0x12, 0x1a, 0x1e,
    0x01, 0x02, 0x03,
    0x04, 0x05, 0x06,
    0x07, 0x08, 0x09,
    0x0a, 0x1b, 0x1f,
    0x0c, 0x0d, 0x0e,
    0x00, 0x0f, 0x19,
};

static int r_key_map[] = {
    KEY_MUTE, KEY_CHANNEL, KEY_POWER,
    0, KEY_AUDIO, KEY_VIDEO,
    KEY_PLAYPAUSE, KEY_UP, KEY_EXIT,
    KEY_LEFT, KEY_OK, KEY_RIGHT,
    KEY_SETUP, KEY_DOWN, KEY_CALENDAR,
    KEY_VOLUMEUP,  KEY_BACK, KEY_PREVIOUS,
    KEY_VOLUMEDOWN,  KEY_FORWARD, KEY_NEXT,
};

static void fatal_if(bool is_fatal, const char* err_msg)
{
    if (!is_fatal)
        return;

    cerr << err_msg << endl;
    exit(1);
}

int main()
{
    int r;

    libusb_context* ctx = NULL;
    r = libusb_init(&ctx);
    fatal_if(r < 0, "libusb init error!");

    libusb_set_debug(ctx, 3);

    libusb_device_handle* dev_hdl;
    dev_hdl = libusb_open_device_with_vid_pid(ctx, SIGMACON_VID, SIGMACON_PID);
    fatal_if(dev_hdl == NULL, "Cannont open device!");

    // find out if kernel driver is attached
    r = libusb_kernel_driver_active(dev_hdl, 0);
    fatal_if(r == 1, "Kernel driver actived");

    r = libusb_claim_interface(dev_hdl, 0);
    fatal_if(r < 0, "Cannot claim interface");

    emsuinput_context* ui_ctx = emsuinput_new_context("sigmacon",
                                r_key_map, 21, NULL, 0);

    unsigned char code_in[8] = { 0 };
    int len = 0;
    while (1) {
        r = libusb_interrupt_transfer(dev_hdl, (1 | LIBUSB_ENDPOINT_IN),
                                      code_in, 8, &len, 1000);
        fatal_if(r == -1, "Exit int. trasfer loop");

        if (memcmp(code_in, r_prefix, 3) != 0) {
            usleep(10 * 1000);
            continue;
        }

        bool found = false;
        for (int i = 0; i < 21; i++) {
            if (r_btn_v[i] == code_in[3]) {
                emsuinput_send_key_down(ui_ctx, r_key_map[i]);
                usleep(10 * 1000);
                emsuinput_send_key_up(ui_ctx, r_key_map[i]);
                found = true;
                break;
            }
        }

        if (!found) {
            cout << "no match!" << endl;
            usleep(10 * 1000);
        }

        /* fflush(NULL); */
    }

    r = libusb_release_interface(dev_hdl, 0);
    fatal_if(r != 0, "Cannot release interface");

    emsuinput_release_context(ui_ctx);
    libusb_close(dev_hdl);
    libusb_exit(ctx);

    return 0;
}
