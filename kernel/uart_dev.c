#include "types.h"
#include "memlayout.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "uart_dev.h"
#include "riscv.h"
#include "defs.h"

static unsigned long bases[5] = {UART0, UART1, UART2, UART3, UART4};

#define Reg(nr, reg) ((volatile unsigned int *)(bases[nr] + 4 * reg))
#define ReadReg(nr, reg) (*(Reg(nr, reg)))
#define WriteReg(nr, reg, v) (*(Reg(nr, reg)) = (v))

short uart_minor;

static int debug = 0;

int
uartdev_read(int user_dst, uint64 dst, int n)
{
  uchar tmp[64];

  push_off();

  for (int i = 0; i < n; i++) {
    // wait for rx ready to be set in LSR.
    while((ReadReg(uart_minor, LSR) & LSR_RX_READY) == 0);
    tmp[i] = ReadReg(uart_minor, RHR);
  }

  pop_off();

  if (either_copyout(user_dst, dst, tmp, n) == -1)
    return -1;

  return n;
}

int
uartdev_write(int user_src, uint64 src, int n)
{
  uchar tmp[64];
  if (either_copyin(tmp, user_src, src, n) == -1)
    return -1;

  push_off();

  for (int i = 0; i < n; i++) {

    // wait for Transmit Holding Empty to be set in LSR.
    while((ReadReg(uart_minor, LSR) & LSR_TX_IDLE) == 0);
    WriteReg(uart_minor, THR, tmp[i]);
  }

  pop_off();
  return n;
}

void
config_uart(struct uartconfig *cfg)
{
  // disable interrupts.
  WriteReg(uart_minor, IER, 0x00);

  // set baud rate
  WriteReg(uart_minor, LCR, LCR_BAUD_LATCH);
  // baud rate = uart_clk / (16 * (256 * DLH + DLL))
  // 256 * DLH + DLL = uart_clk / (16 * baud rate)
  uint8 DLH, DLL;
  double result = 25000000.0 / (16 * cfg->baud);
  if (result < 255.5) {
    DLL = result + 0.5;
    DLH = 0;
  } else {
    DLL = (int)(result + 0.5);
    DLH = (int)(result + 0.5) >> 8;
  }
  if (debug) printf("DLH: %d, DLL: %d\n", DLH, DLL);
  // set DLL
  WriteReg(uart_minor, 0, DLL);
  // set DLH
  WriteReg(uart_minor, 1, DLH);

  // leave set-baud mode
  WriteReg(uart_minor, LCR, LCR_LEAVE_BAUD);
  // set word length, stop bit, parity.
  int parity = 0;
  if (cfg->parity == 'o') parity = 1;
  else if (cfg->parity == 'e') parity = 3;
  else if (cfg->parity == 's') parity = 5;
  uint32 setting = (cfg->databits - 5) |
                    ((cfg->stopbits - 1) << 2) |
                    (parity << 3);
  WriteReg(uart_minor, LCR, setting);

  // reset and enable FIFOs.
  WriteReg(uart_minor, FCR, FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);
}

int
uartdev_ioctl(int user_src, unsigned int cmd, unsigned long arg)
{
  // arg is user space pointer
  struct uartconfig cfg;
  if (either_copyin(&cfg, 1, arg, sizeof(struct uartconfig)) == -1)
    return -1;

  switch (cmd) {
  case SET_CONFIG:
    config_uart(&cfg);
  }

  return 0;
}

int
uartdev_open(struct inode *ip)
{
  uart_minor = ip->minor;
  return 0;
}

int
uartdev_close(void)
{
  uart_minor = 0;
  return 0;
}

void
uartdev_init(void)
{
  devsw[UART].read  = uartdev_read;
  devsw[UART].write = uartdev_write;
  devsw[UART].ioctl = uartdev_ioctl;
  devsw[UART].open  = uartdev_open;
  devsw[UART].close = uartdev_close;
}
