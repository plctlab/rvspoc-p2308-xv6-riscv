#include "kernel/types.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/gpio.h"

struct gpio_info {
  char *name;
  int gpio_group;
  int num;
} info[] = {
  {"XGPIOA_28",   0, 28, }, /* GP0  */
  {"XGPIOA_29",   0, 29, }, /* GP1  */
  {"PWR_GPIO_26", 3, 26, }, /* GP2  */
  {"PWR_GPIO_25", 3, 25, }, /* GP3  */
  {"PWR_GPIO_19", 3, 19, }, /* GP4  */
  {"PWR_GPIO_20", 3, 20, }, /* GP5  */
  {"PWR_GPIO_23", 3, 23, }, /* GP6  */
  {"PWR_GPIO_22", 3, 22, }, /* GP7  */
  {"PWR_GPIO_21", 3, 21, }, /* GP8  */
  {"PWR_GPIO_18", 3, 18, }, /* GP9  */
  {"XGPIOC_9",    2,  9, }, /* GP10 */
  {"XGPIOC_10",   2, 10, }, /* GP11 */
  {"XGPIOA_16",   0, 16, }, /* GP12 */
  {"XGPIOA_17",   0, 17, }, /* GP13 */
  {"XGPIOA_14",   0, 14, }, /* GP14 */
  {"XGPIOA_15",   0, 15, }, /* GP15 */
  {"XGPIOA_23",   0, 23, }, /* GP16 */
  {"XGPIOA_24",   0, 24, }, /* GP17 */
  {"XGPIOA_22",   0, 22, }, /* GP18 */
  {"XGPIOA_25",   0, 25, }, /* GP19 */
  {"XGPIOA_27",   0, 27, }, /* GP20 */
  {"XGPIOA_26",   0, 26, }, /* GP21 */
  {"PWR_GPIO_4",  3,  4, }, /* GP22 */
  {"NULL",       -1, -1  },
  {"NULL",       -1, -1  },
  {"XGPIOC_24",   2, 24, }, /* GP25 */
  {"XGPIOB_3",    1,  3, }, /* GP26 */
  {"XGPIOB_6",    1,  6, }, /* GP27 */
};

void test_gp25(void)
{
    // gp25 of the board
    int pin = 25;

    int group = info[pin].gpio_group;
    int num = info[pin].num;

    char dev_name[] = "gpio-0";
    dev_name[5] = '0' + group;

    int fd = open(dev_name, O_RDWR);
    if (fd < 0) {
        mknod(dev_name, GPIO, group);
        fd = open(dev_name, O_RDWR);
    }
    printf("open %s success!\n", dev_name);

    printf("start flashing led\n");

    // set channel
    ioctl(fd, SET_CHAN, num);

    // 0 for input, 1 for output
    ioctl(fd, SET_DIR, 1);

    unsigned char out[2] = {1, 0};

    for (int i = 0; i < 10; i++) {
        write(fd, &out[i % 2], 1);
        sleep(2);
    }

    close(fd);
}

void test_gp0(void)
{
    // gp0 of the board
    int pin = 0;

    int group = info[pin].gpio_group;
    int num = info[pin].num;

    char dev_name[] = "gpio-0";
    dev_name[5] = '0' + group;

    int fd = open(dev_name, O_RDWR);
    if (fd < 0) {
        mknod(dev_name, GPIO, group);
        fd = open(dev_name, O_RDWR);
    }
    printf("open %s success!\n", dev_name);

    printf("please change input at GP%d\n", pin);

    // set channel
    ioctl(fd, SET_CHAN, num);

    // 0 for input, 1 for output
    ioctl(fd, SET_DIR, 0);

    unsigned char in;

    for (int i = 0; i < 10; i++) {
        read(fd, &in, 1);
        printf("input: %d\n", in);
        sleep(5);
    }

    close(fd);
}

int main(void)
{
    // use GP25 as output
    test_gp25();

    // use GP0 as input
    test_gp0();

    exit(0);
}
