#include "types.h"
#include "memlayout.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"
#include "pwm.h"

#define Reg(reg) ((volatile unsigned int *)(PWM1 + reg))
#define ReadReg(reg) (*(Reg(reg)))
#define WriteReg(reg, v) (*(Reg(reg)) = (v))

static int debug = 0;

int period_regs[4] = {PERIOD0, PERIOD1, PERIOD2, PERIOD3};
int hlperiod_regs[4] = {HLPERIOD0, HLPERIOD1, HLPERIOD2, HLPERIOD3};
static int channel = -1;
unsigned long period;

int
pwmdev_ioctl(int user_src, unsigned int cmd, unsigned long arg)
{
  if (debug)
    printf("PWM_OE: %x %p\n", ReadReg(PWM_OE), ReadReg(PWM_OE));

  switch (cmd)
  {
  case SET_CHANNEL:
    channel = arg;
    if (debug) printf("channel: %d\n", channel);
    break;

  case SET_PERIOD:
    WriteReg(period_regs[channel], arg);
    if (debug)
      printf("period_regs[channel]: %x %p\n",
             ReadReg(period_regs[channel]),
             ReadReg(period_regs[channel]));
    period = arg;
    break;

  case SET_DUTY:
    WriteReg(hlperiod_regs[channel], period - arg);
    if (debug)
      printf("hlperiod_regs[channel]: %x %p\n",
             ReadReg(hlperiod_regs[channel]),
             ReadReg(hlperiod_regs[channel]));
    break;

  case SET_POLAR:
    WriteReg(POLARITY, arg << channel);
    if (debug)
      printf("POLARITY: %x %p\n", ReadReg(POLARITY), ReadReg(POLARITY));
    break;

  case ENABLE:
    WriteReg(PWM_OE, 1 << channel);
    WriteReg(PWMSTART, 1 << channel);
    if (debug)
      printf("PWMSTART: %x %p\n", ReadReg(PWMSTART), ReadReg(PWMSTART));
    break;

  case UPDATE_DUTY:
    WriteReg(PWMUPDATE, 1 << channel);
    WriteReg(PWMUPDATE, 0);
    WriteReg(hlperiod_regs[channel], period - arg);
  }

  return 0;
}

int
pwmdev_close(void)
{
  channel = -1;
  period = 0;
  WriteReg(PWMSTART, 0);

  return 0;
}

void
pwmdev_init(void)
{
  devsw[PWM].ioctl = pwmdev_ioctl;
  devsw[PWM].close = pwmdev_close;
}
