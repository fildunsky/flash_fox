#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <libusb-1.0/libusb.h>

#include "log.h"

typedef struct usb_dev {
    libusb_device_handle *hdl;
    int ifn;
    int in_ep;
    int out_ep;
    size_t in_maxpktsize;
    size_t out_maxpktsize;
    size_t out_psize;
} usb_dev_t;

usb_dev_t *usb_get(void);
int usb_open(void);
void usb_close(void);
int usb_write(void *buf, int len, unsigned int tmo);
int usb_read(void *buf, int len, unsigned int tmo);
int usb_reset(void);
