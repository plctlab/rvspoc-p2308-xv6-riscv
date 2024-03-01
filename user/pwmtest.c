#include "kernel/types.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/pwm.h"

#define DUMMY 0x0f0f

int main(void)
{
    int fd = open("pwm-1", O_RDONLY);

    if (fd < 0) {
        // pwm controller 1
        mknod("pwm-1", PWM, 1);
        fd = open("pwm-1", O_RDONLY);
    }

    printf("open pwm success!\n");

    // config pwm
    // 1 is channel number, channel 1 of pwm1 is pwm5 (4+1)
    ioctl(fd, SET_CHANNEL, 1);
    // unit of period is 1/100MHZ, 10ns, 1000000 is 10 ms
    ioctl(fd, SET_PERIOD, 1000000);
    // unit of duty is 10 ns, 50000 is 0.5ms
    ioctl(fd, SET_DUTY, 50000);
    // 0 for normal polarity, 1 for inversed polarity
    ioctl(fd, SET_POLAR, 0);
    // enable pwm
    ioctl(fd, ENABLE, DUMMY);

    sleep(1);

    // increase 0.1ms for duty every time
    for (int i = 50000 + 10000; i <= 50000 * 5; i += 10000) {
        ioctl(fd, UPDATE_DUTY, i);
        printf("duty: %d\n", i);
        sleep(2);
    }

    ioctl(fd, UPDATE_DUTY, 50000);

    sleep(2);

    close(fd);

    exit(0);
}
