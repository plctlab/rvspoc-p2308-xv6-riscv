#include "types.h"
#include "memlayout.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"
#include "adc.h"

#define Reg(reg) ((volatile unsigned int *)(ADC0 + reg))
#define ReadReg(reg) (*(Reg(reg)))
#define WriteReg(reg, v) (*(Reg(reg)) = (v))

static int debug = 0;

int channel = 0;
struct spinlock adc_lock;

int
adcdev_read(int user_dst, uint64 dst, int n)
{
  acquire(&adc_lock);

  uchar tmp[2];
  unsigned int chan_result = 0;

  // allow adc to send interrupt
  WriteReg(saradc_intr_en, 1);

  // begin measure

  WriteReg(saradc_ctrl, 1 | ReadReg(saradc_ctrl));

  if (debug) {
    printf("saradc_ctrl = %x %p\n", ReadReg(saradc_ctrl),
                                    ReadReg(saradc_ctrl));
    printf("sleep in adcdev_read\n");
  }

  sleep(&channel, &adc_lock);

  // measure complete
  if (channel == 1)
    chan_result = ReadReg(saradc_ch1_result);
  else if (channel == 2)
    chan_result = ReadReg(saradc_ch2_result);
  else if (channel == 3)
    chan_result = ReadReg(saradc_ch3_result);

  chan_result = chan_result & 0xFFF;

  if (debug) printf("chan_result: %x %p\n", chan_result, chan_result);

  tmp[0] = chan_result;
  tmp[1] = chan_result >> 8;

  release(&adc_lock);

  if (either_copyout(user_dst, dst, tmp, n) == -1)
    return -1;

  return n;
}

int
adcdev_ioctl(int user_src, unsigned int cmd, unsigned long arg)
{
  // cmd is not used
  // arg is channel number

  if (debug) {
    printf("saradc_ctrl = %x %p\n", ReadReg(saradc_ctrl),
                                    ReadReg(saradc_ctrl));
    printf("saradc_cyc_set = %x %p\n", ReadReg(saradc_cyc_set),
                                       ReadReg(saradc_cyc_set));
    printf("arg: %d\n", arg);
  }

  WriteReg(saradc_ctrl, 1 << (4 + arg));

  channel = arg;

  return 0;
}

int
adcdev_close(void)
{
  channel = 0;
  return 0;
}

void
adcdev_init(void)
{
  devsw[ADC].read  = adcdev_read;
  devsw[ADC].ioctl = adcdev_ioctl;
  devsw[ADC].close = adcdev_close;

  initlock(&adc_lock, "adc");
}

void
adcdev_intr(void)
{
  if (debug) {
    printf("adcdev_intr\n");
    printf("saradc_intr_sta: %x %p\n", ReadReg(saradc_intr_sta),
                                       ReadReg(saradc_intr_sta));
  }
  // clear interrupt
  WriteReg(saradc_intr_clr, 1);

  if (debug) {
    printf("saradc_intr_sta: %x %p\n", ReadReg(saradc_intr_sta),
                                       ReadReg(saradc_intr_sta));
  }

  // does not allow adc to send interrupt
  WriteReg(saradc_intr_en, 0);

  wakeup(&channel);
}
