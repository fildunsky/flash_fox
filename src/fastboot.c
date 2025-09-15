#include <string.h>

#include "log.h"
#include "usb.h"

static int read_data(char *buf, int len)
{
    int n = usb_read(buf, len - 1, 60000);
    if (n < 0) {
        errorf("read: %s\n", libusb_strerror(n));
    } else {
        buf[n] = '\0';
    }
    return n;
}

static int write_data(char *buf, int len)
{
    int n = usb_write(buf, len, 60000);
    if (n < 0) {
        errorf("write: %s\n", libusb_strerror(n));
    }
    return n;
}

static void print_buf(char *buf, int recv)
{
    char *bp = recv ? buf + 4 : buf;
    if (bp[0] == '\0')
        return;
    if (recv) {
        infof("<- %s\n", bp);
    } else {
        infof("-> %s\n", bp);
    }
}

static int wait_expect(const char *expect)
{
    int rv;
    char buf[65];

    for (;;) {
        rv = read_data(buf, sizeof(buf));
        if (rv < 0)
            return 0;
        print_buf(buf, 1);
        if (memcmp(buf, "INFO", 4))
            break;
    }
    return memcmp(buf, expect, 4) == 0;
}

static int send_cmd(char *cmd, const char *expect)
{
    int n = strlen(cmd);
    print_buf(cmd, 0);
    int rv = write_data(cmd, n);
    if (rv < 0)
        return 0;
    return wait_expect(expect);
}

static int send_data(void *buf, size_t len, const char *expect)
{
    int rv = write_data(buf, len);
    if (rv < 0)
        return 0;
    return wait_expect(expect);
}

int run_fastboot_reboot(void)
{
    return send_cmd("reboot", "OKAY") ? 0 : -1;
}

int run_fastboot(void *data, size_t len)
{
    char buf[65];

    sprintf(buf, "download:%08lx", len);
    if (!send_cmd(buf, "DATA"))
        return -1;

    if (!send_data(data, len, "OKAY"))
        return -1;

    if (!send_cmd("flash:ota", "OKAY"))
        return -1;

    return run_fastboot_reboot();
}
