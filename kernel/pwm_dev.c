#include "types.h"
#include "memlayout.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"
#include "pwm.h"

static unsigned long bases[4] = {PWM0, PWM1, PWM2, PWM3};

#define Reg(nr, reg) ((volatile unsigned int *)(bases[nr] + reg))
#define ReadReg(nr, reg) (*(Reg(nr, reg)))
#define WriteReg(nr, reg, v) (*(Reg(nr, reg)) = (v))

static int debug = 0;

static int minor;

int period_regs[4] = {PERIOD0, PERIOD1, PERIOD2, PERIOD3};
int hlperiod_regs[4] = {HLPERIOD0, HLPERIOD1, HLPERIOD2, HLPERIOD3};
static int channel = -1;
unsigned long period;

int
pwmdev_ioctl(int user_src, unsigned int cmd, unsigned long arg)
{
  if (debug)
    printf("PWM_OE: %x %p\n", ReadReg(minor, PWM_OE), ReadReg(minor, PWM_OE));

  switch (cmd)
  {
  case SET_CHANNEL:
    channel = arg;
    if (debug) printf("channel: %d\n", channel);
    break;

  case SET_PERIOD:
    WriteReg(minor, period_regs[channel], arg);
    if (debug)
      printf("period_regs[channel]: %x %p\n",
             ReadReg(minor, period_regs[channel]),
             ReadReg(minor, period_regs[channel]));
    period = arg;
    break;

  case SET_DUTY:
    WriteReg(minor, hlperiod_regs[channel], period - arg);
    if (debug)
      printf("hlperiod_regs[channel]: %x %p\n",
             ReadReg(minor, hlperiod_regs[channel]),
             ReadReg(minor, hlperiod_regs[channel]));
    break;

  case SET_POLAR:
    WriteReg(minor, POLARITY, arg << channel);
    if (debug)
      printf("POLARITY: %x %p\n", ReadReg(minor, POLARITY),
                                  ReadReg(minor,POLARITY));
    break;

  case ENABLE:
    WriteReg(minor, PWM_OE, 1 << channel);
    WriteReg(minor, PWMSTART, 1 << channel);
    if (debug)
      printf("PWMSTART: %x %p\n", ReadReg(minor, PWMSTART),
                                  ReadReg(minor, PWMSTART));
    break;

  case UPDATE_DUTY:
    WriteReg(minor, PWMUPDATE, 1 << channel);
    WriteReg(minor, PWMUPDATE, 0);
    WriteReg(minor, hlperiod_regs[channel], period - arg);
  }

  return 0;
}

int
pwmdev_close(void)
{
  channel = -1;
  period = 0;
  WriteReg(minor, PWMSTART, 0);
  minor = -1;

  return 0;
}

int
pwmdev_open(struct inode *ip)
{
  minor = ip->minor;
  return 0;
}

void
pwmdev_init(void)
{
  devsw[PWM].open = pwmdev_open;
  devsw[PWM].ioctl = pwmdev_ioctl;
  devsw[PWM].close = pwmdev_close;
}
