#include "kernel/types.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main(void)
{
    int fd = open("adc", O_RDWR);

    if (fd < 0) {
        mknod("adc", ADC, 0);
        fd = open("adc", O_RDWR);
    }

    printf("open adc success!\n");

    unsigned int anything = 0x1234;
    // config which channel to use with ioctl
    // channel 1
    ioctl(fd, anything, 1);

    for (int i = 0; i < 10; i++) {
        // read
        unsigned char buf[2];
        if (read(fd, buf, 2) != 2) {
            close(fd);
            exit(1);
        }
        unsigned int result = (buf[1] << 8) + buf[0];
        printf("adc result: %d\n\n", result);
        sleep(20);
    }

    close(fd);
    exit(0);
}
