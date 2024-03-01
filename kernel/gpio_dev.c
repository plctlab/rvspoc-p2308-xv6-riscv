#include "types.h"
#include "memlayout.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"
#include "gpio.h"

static unsigned long bases[4] = {GPIO0, GPIO1, GPIO2, GPIO3};

#define Reg(nr, reg) ((volatile unsigned int *)(bases[nr] + reg))
#define ReadReg(nr, reg) (*(Reg(nr, reg)))
#define WriteReg(nr, reg, v) (*(Reg(nr, reg)) = (v))

short gpio_minor = -1;
short gpio_channel = -1;

static int debug = 0;

int
gpiodev_read(int user_dst, uint64 dst, int n)
{
  uchar tmp;

  unsigned int result = ReadReg(gpio_minor, GPIO_EXT_PORTA);

  tmp = (result >> gpio_channel) & 0x0001;

  if (debug)
    printf("read value %d\n", tmp);

  if (either_copyout(user_dst, dst, &tmp, n) == -1)
    return -1;

  return n;
}

int
gpiodev_write(int user_src, uint64 src, int n)
{
  uchar tmp;
  if (either_copyin(&tmp, user_src, src, n) == -1)
    return -1;

  if (debug)
    printf("write value %d\n", tmp);

  WriteReg(gpio_minor, GPIO_SWPORTA_DR, tmp << gpio_channel);

  return n;
}

int
gpiodev_ioctl(int user_src, unsigned int cmd, unsigned long arg)
{
  switch (cmd) {
  case SET_CHAN:
    gpio_channel = arg;
    break;
  case SET_DIR:
    WriteReg(gpio_minor, GPIO_SWPORTA_DDR, arg << gpio_channel);
    if (debug) printf("GPIO_SWPORTA_DDR: %x %p\n",
                      ReadReg(gpio_minor, GPIO_SWPORTA_DDR),
                      ReadReg(gpio_minor, GPIO_SWPORTA_DDR));
    break;
  }

  return 0;
}

int
gpiodev_open(struct inode *ip)
{
  gpio_minor = ip->minor;
  return 0;
}

int
gpiodev_close(void)
{
  gpio_minor = -1;
  gpio_channel = -1;
  return 0;
}

void
gpiodev_init(void)
{
  devsw[GPIO].read  = gpiodev_read;
  devsw[GPIO].write = gpiodev_write;
  devsw[GPIO].ioctl = gpiodev_ioctl;
  devsw[GPIO].open  = gpiodev_open;
  devsw[GPIO].close = gpiodev_close;
}
