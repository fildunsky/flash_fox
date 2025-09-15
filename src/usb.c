#include "usb.h"

static usb_dev_t *udev;

static int usb_try_open(libusb_device *dev)
{
    struct libusb_device_descriptor desc;
    int ret = libusb_get_device_descriptor(dev, &desc);
    if (ret < 0)
        return 0;
    if (desc.idVendor != 0x18d1)
        return 0;
    if (desc.idProduct != 0xd00d)
        return 0;

    struct libusb_config_descriptor *config;
    ret = libusb_get_active_config_descriptor(dev, &config);
    if (ret < 0)
        return 0;

    int in = -1, out = -1;
    uint16_t in_size = 0, out_size = 0;
    libusb_device_handle *handle;
    int rv = 0;

    for (int k = 0; k < config->bNumInterfaces; k++) {
        const struct libusb_interface_descriptor *ifc = config->interface[k].altsetting;
        if (ifc->bNumEndpoints != 2)
            continue;
        if (ifc->bInterfaceClass != 255)
            continue;
        if (ifc->bInterfaceSubClass != 66)
            continue;
        if (ifc->bInterfaceProtocol != 3)
            continue;

        int ep_found = 0;
        for (int l = 0; l < ifc->bNumEndpoints; l++) {
            const struct libusb_endpoint_descriptor *endpoint = &ifc->endpoint[l];
            uint8_t type = endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK;
            if (type != LIBUSB_ENDPOINT_TRANSFER_TYPE_BULK)
                continue;
            ep_found = 1;
            if (endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN) {
                in = endpoint->bEndpointAddress;
                in_size = endpoint->wMaxPacketSize;
            } else {
                out = endpoint->bEndpointAddress;
                out_size = endpoint->wMaxPacketSize;
            }
        }
        if (!ep_found)
            continue;

        ret = libusb_open(dev, &handle);
        if (ret < 0)
            continue;
        libusb_detach_kernel_driver(handle, ifc->bInterfaceNumber);
        ret = libusb_claim_interface(handle, ifc->bInterfaceNumber);
        if (ret < 0) {
            libusb_close(handle);
            continue;
        }
        udev->hdl = handle;
        udev->ifn = ifc->bInterfaceNumber;
        udev->in_ep = in;
        udev->out_ep = out;
        udev->in_maxpktsize = in_size;
        udev->out_maxpktsize = out_size;
        infof("insz: %u, outsz: %u\n", in_size, out_size);
        rv++;
        break;
    }
    libusb_free_config_descriptor(config);
    return rv;
}

usb_dev_t *usb_get(void)
{
    return udev;
}

int usb_open(void)
{
    int rv = libusb_init(NULL);
    if (rv < 0) {
        errorf("failed to init libusb\n");
        return rv;
    }

    libusb_device **devs;
    rv = libusb_get_device_list(NULL, &devs);
    if (rv < 0) {
        libusb_exit(NULL);
        return rv;
    }

    int found = 0;
    udev = malloc(sizeof(usb_dev_t));
    for (int i = 0; devs[i]; i++) {
        if (usb_try_open(devs[i])) {
            found++;
            break;
        }
    }
    libusb_free_device_list(devs, 1);

    if (found)
        return 0;

    free(udev);
    udev = NULL;
    libusb_exit(NULL);
    return LIBUSB_ERROR_NO_DEVICE;
}

void usb_close(void)
{
    if (!udev)
        return;
    libusb_release_interface(udev->hdl, udev->ifn);
    libusb_attach_kernel_driver(udev->hdl, udev->ifn);
    libusb_close(udev->hdl);
    free(udev);
    udev = NULL;
    libusb_exit(NULL);
}

int usb_write(void *buf, int len, unsigned int tmo)
{
    if (!udev)
        return LIBUSB_ERROR_NO_DEVICE;

    //int zlp = len % udev->out_maxpktsize == 0;
    int zlp = 0;
    int xfer, ret, actual, count = 0;

    while (len > 0) {
        xfer = len > udev->out_maxpktsize ? udev->out_maxpktsize : len;
        ret = libusb_bulk_transfer(udev->hdl, udev->out_ep, buf, xfer, &actual, tmo);
        if (ret < 0)
            return ret;
        buf += actual;
        len -= actual;
        count += actual;
    }
    if (zlp) {
        ret = libusb_bulk_transfer(udev->hdl, udev->out_ep, NULL, 0, &actual, tmo);
        if (ret < 0)
            return ret;
    }
    return count;
}

int usb_read(void *buf, int len, unsigned int tmo)
{
    if (!udev)
        return LIBUSB_ERROR_NO_DEVICE;

    int actual;
    int ret = libusb_bulk_transfer(udev->hdl, udev->in_ep, buf, len, &actual, tmo);
    return ret < 0 ? ret : actual;
}

int usb_reset(void)
{
    if (!udev)
        return LIBUSB_ERROR_NO_DEVICE;

    return libusb_reset_device(udev->hdl);
}
