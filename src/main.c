#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "log.h"
#include "usb.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

int fd = -1;
size_t fsize;
void *fbuf = MAP_FAILED;

int run_fastboot(void *data, size_t len);
int run_fastboot_reboot(void);

static int map_file(const char *name)
{
    fd = open(name, O_RDONLY);
    if (fd < 0)
        return -errno;
    struct stat sb;
    int rv = fstat(fd, &sb);
    if (rv < 0) {
        rv = -errno;
        goto err0;
    }
    fsize = sb.st_size;
    fbuf = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (fbuf == MAP_FAILED) {
        rv = -errno;
        goto err0;
    }
    return 0;

err0:
    close(fd);
    fd = -1;
    return rv;
}

void cleanup(void)
{
    if (fbuf != MAP_FAILED)
        munmap(fbuf, fsize);
    if (fd >= 0)
        close(fd);
    usb_close();
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        errorf("usage: %s {ota.bin|-r}\n", argv[0]);
        return EXIT_FAILURE;
    }
    atexit(cleanup);

    int ret = usb_open();
    if (ret < 0) {
        errorf("usb_open: %s\n", libusb_strerror(ret));
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-r")) {
        ret = map_file(argv[1]);
        if (ret < 0) {
            errorf("file_open: %s\n", strerror(-ret));
            return EXIT_FAILURE;
        }
        if (run_fastboot(fbuf, fsize))
            return EXIT_FAILURE;
    } else {
        if (run_fastboot_reboot())
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
