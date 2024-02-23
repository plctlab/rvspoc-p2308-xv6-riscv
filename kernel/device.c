#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "intr_conf.h"

#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "proc.h"
#include "defs.h"
#include "file.h"

struct spinlock devlock;

/* device func */
int gpioinit(void);
int gpioread(int user_dst, uint64 dst, int n);
int gpiowrite(int user_src, uint64 src, int n);

int pwminit(void);
int pwmread(int user_dst, uint64 dst, int n);
int pwmwrite(int user_src, uint64 src, int n);

int spilcdinit(void);
int spilcdread(int user_dst, uint64 dst, int n);
int spilcdwrite(int user_src, uint64 src, int n);

void
clockintr()
{
  acquire(&tickslock);
  ticks++;
  wakeup(&ticks);
  release(&tickslock);
}

void
devinit()
{
  initlock(&devlock, "dev");

  /* gpio init */
  if (gpioinit() == 0) {
    devsw[GPIO].read = gpioread;
    devsw[GPIO].write = gpiowrite;
  }

  if (pwminit() == 0) {
    devsw[PWM].read = pwmread;
    devsw[PWM].write = pwmwrite;
  }

  if (spilcdinit() == 0) {
    devsw[SPI].read = spilcdread;
    devsw[SPI].write = spilcdwrite;
  }

  /* i2c init */
  /* adc init */

  /* pwm init */
  /* spi init */

}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int
devintr()
{
  uint64 scause = r_scause();

  if((scause & 0x8000000000000000L) &&
     (scause & 0xff) == 9){
    // this is a supervisor external interrupt, via PLIC.

    // irq indicates which device interrupted.
    int irq = plic_claim();

    if(irq == UART0_IRQ){
      uartintr();
    } else if(irq == VIRTIO0_IRQ){
      virtio_disk_intr();
    } else if(irq){
      printf("unexpected interrupt irq=%d\n", irq);
    }

    // the PLIC allows each device to raise at most one
    // interrupt at a time; tell the PLIC the device is
    // now allowed to interrupt again.
    if(irq)
      plic_complete(irq);

    return 1;
  } else if(scause == 0x8000000000000005L){
    if(cpuid() == 0){
      struct sbi_ret ret;

      clockintr();

      ret = SBI_CALL1(SBI_EXT_ID_TIME, SBI_TIME_SET_TIMER, r_time() + (MTIMER_FREQ / TICKRATE));
      if (ret.error != SBI_SUCCESS)
        panic("ticks");
    }

    return 2;
  } else {
    return 0;
  }
}

int request_irq(int irqn, irq_handler_t handler,
		       unsigned long flags, const char *name, void *priv)
{
    // TODO
    //printf("irqn %d flags :lx\n",irqn,flags);
    // csi_irq_attach2(irqn, handler, &g_irq_dev_table[irqn], priv);
    //csi_plic_set_prio(PLIC_BASE, dev.irq_num, 31); //temp
    // csi_irq_enable(irqn);

    return 0;
}
