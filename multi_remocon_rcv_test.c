/* This file is generated with usbsnoop2libusb.pl from a usbsnoop log file. */
/* Latest version of the script should be in http://iki.fi/lindi/usb/usbsnoop2libusb.pl */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <ctype.h>
#include <usb.h>
#if 0
#include <linux/usbdevice_fs.h>
#define LIBUSB_AUGMENT
#include "libusb_augment.h"
#endif

struct usb_dev_handle* devh;

void release_usb_device(int dummy)
{
    int ret;
    ret = usb_release_interface(devh, 0);
    if (!ret)
        printf("failed to release interface: %d\n", ret);
    usb_close(devh);
    if (!ret)
        printf("failed to close interface: %d\n", ret);
    exit(1);
}


struct usb_device* find_device(int vendor, int product) {
    struct usb_bus* bus;

    for (bus = usb_get_busses(); bus; bus = bus->next) {
        struct usb_device* dev;

        for (dev = bus->devices; dev; dev = dev->next) {
            if (dev->descriptor.idVendor == vendor
                    && dev->descriptor.idProduct == product)
                return dev;
        }
    }
    return NULL;
}

void print_bytes(char* bytes, int len)
{
    int i;
    if (len > 0) {
        for (i = 0; i < len; i++) {
            printf("%02x ", (int)((unsigned char)bytes[i]));
        }
        printf("\"");
        for (i = 0; i < len; i++) {
            printf("%c", isprint(bytes[i]) ? bytes[i] : '.');
        }
        printf("\"");
    }
}

int is_empty(char* bytes, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        if (*(bytes + i) != 0x00)
            return 0;
    }
    return 1;
}

#define SIGMA_REMOTE_RECV_VID 0x10c8
#define SIGMA_REMOTE_RECV_PID 0x0910

int main(int argc, char** argv)
{
    int ret, vendor, product;
    struct usb_device* dev;
    int i;
    char buf[65535], *endptr;
#if 0
    usb_urb* isourb;
    struct timeval isotv;
    char isobuf[32768];
#endif

    usb_init();
    //usb_set_debug(255);
    usb_find_busses();
    usb_find_devices();

    vendor = SIGMA_REMOTE_RECV_VID;
    product = SIGMA_REMOTE_RECV_PID;
    dev = find_device(vendor, product);
    assert(dev);

    devh = usb_open(dev);
    assert(devh);

    signal(SIGTERM, release_usb_device);

    ret = usb_get_driver_np(devh, 0, buf, sizeof(buf));
    printf("usb_get_driver_np returned %d\n", ret);
    if (ret == 0) {
        printf("interface 0 already claimed by driver \"%s\", attempting to detach it\n", buf);
        ret = usb_detach_kernel_driver_np(devh, 0);
        printf("usb_detach_kernel_driver_np returned %d\n", ret);
    }
    ret = usb_claim_interface(devh, 0);
    if (ret != 0) {
        printf("claim failed with error %d\n", ret);
        exit(1);
    }

    ret = usb_set_altinterface(devh, 0);
    assert(ret >= 0);

    ret = usb_get_descriptor(devh, 0x0000003, 0x0000000, buf, 0x0000032);
    printf("1 get descriptor returned %d, bytes: ", ret);
    print_bytes(buf, ret);
    printf("\n");
    usleep(3 * 1000);
    ret = usb_get_descriptor(devh, 0x0000002, 0x0000000, buf, 0x0000009);
    printf("2 get descriptor returned %d, bytes: ", ret);
    print_bytes(buf, ret);
    printf("\n");
    usleep(1 * 1000);
    ret = usb_get_descriptor(devh, 0x0000002, 0x0000000, buf, 0x0000019);
    printf("3 get descriptor returned %d, bytes: ", ret);
    print_bytes(buf, ret);
    printf("\n");
    usleep(1 * 1000);
    ret = usb_release_interface(devh, 0);
    if (ret != 0) printf("failed to release interface before set_configuration: %d\n", ret);
    ret = usb_set_configuration(devh, 0x0000001);
    printf("4 set configuration returned %d\n", ret);
    ret = usb_claim_interface(devh, 0);
    if (ret != 0) printf("claim after set_configuration failed with error %d\n", ret);
    ret = usb_set_altinterface(devh, 0);
    printf("4 set alternate setting returned %d\n", ret);
    usleep(889 * 1000);

    //for (i = 0; i< 100; i++)
    while (1) {
        ret = usb_interrupt_read(devh, 0x00000081, buf, 0x0000008, 1000);
        if (
            ret != -1
            && !is_empty(buf, 8)
            && (buf[0] == 0x01 && buf[1] == 0x02 && buf[2] == 0x08)
        ) {
            //printf("%d interrupt read returned %d, bytes: ", (5+i), ret);
            //buf[8] = 0x00;
            //print_bytes(buf, ret);
            //printf("\n");
            printf("keycode : 0x%02x\n", buf[3]);
            fflush(NULL);
        }
        usleep(10 * 1000);
    }
    ret = usb_release_interface(devh, 0);
    assert(ret == 0);
    ret = usb_close(devh);
    assert(ret == 0);
    return 0;
}

