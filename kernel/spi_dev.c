#include "types.h"
#include "memlayout.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"
#include "spi.h"

static int debug = 0;

static unsigned long bases[4] = {SPI0, SPI1, SPI2, SPI3};

struct dw_spi {
  int     master;
  uint8   mode;
  uint8   bits;
  uint32  max_freq;
  uint32  current_freq;
  uint32  bus_freq;
} dws;

static int min(int a, int b) {
  if (debug) printf("min: %d %d\n", a, b);
  return (a < b) ? a : b; 
}

static inline uint32
dw_readl(struct dw_spi *dws, uint32 reg)
{
  return *((volatile unsigned int *)(bases[dws->master] + reg));
}

static inline void
dw_writel(struct dw_spi *dws, uint32 reg, uint32 val)
{
  *((volatile unsigned int *)(bases[dws->master] + reg)) = val;
}

static inline void
spi_enable_chip(struct dw_spi *dws, int enable)
{
  dw_writel(dws, DW_SPI_SSIENR, (enable ? 1 : 0));
}

void
config_clk(uint32 xfer_freq)
{
  uint32 result;

  if (xfer_freq == 0) {
    result = dws.current_freq;
  } else {
    if (xfer_freq < dws.max_freq)
      result = xfer_freq;
    else
      result = dws.max_freq;
  }

  // update baudr
  dw_writel(&dws, DW_SPI_BAUDR, dws.bus_freq * 1.0 / result);

  if (debug)
    printf("DW_SPI_BAUDR: %x %p\n", dw_readl(&dws, DW_SPI_BAUDR),
                                    dw_readl(&dws, DW_SPI_BAUDR));
}

void
transfer(uint64 xfer_addr, uint xfer_num)
{
  struct spi_ioc_transfer xfer[xfer_num];
  either_copyin(xfer, 1, xfer_addr, sizeof(struct spi_ioc_transfer) * xfer_num);

  // set up hardware
  spi_enable_chip(&dws, 0);

  // config control reg 0 1
  uint32 ctlreg = 7 | // 8-bit
                  (SPI_FRF_SPI << SPI_FRF_OFFSET) | // motorola spi
                  (((dws.mode & SPI_CPHA) ? 1 : 0) << SPI_SCPH_OFFSET) |
                  (((dws.mode & SPI_CPOL) ? 1 : 0) << SPI_SCOL_OFFSET) |
                  (SPI_TMOD_TR << SPI_TMOD_OFFSET); // tx rx

  dw_writel(&dws, DW_SPI_CTRLR0, ctlreg);

  if (debug) {
    printf("control 0: %x %p\n", dw_readl(&dws, DW_SPI_CTRLR0),
                                 dw_readl(&dws, DW_SPI_CTRLR0));
    printf("control 1: %x %p\n", dw_readl(&dws, DW_SPI_CTRLR1),
                                 dw_readl(&dws, DW_SPI_CTRLR1));
  }

  // select
  dw_writel(&dws, DW_SPI_SER, 1);

  // process one transfer
  for (int i = 0; i < xfer_num; i++) {

    config_clk(xfer[i].speed_hz);

    spi_enable_chip(&dws, 1);

    uint8 tx_buf[32];
    uint8 rx_buf[32];
    uint8 *tx_end = tx_buf;
    uint8 *rx_end = rx_buf;
    int len = xfer[i].len;

    if (xfer[i].tx_buf)
      either_copyin(tx_buf, 1, xfer[i].tx_buf, len);

    for (; (tx_end < tx_buf + len) || (rx_end < rx_buf + len); ) {
      if (xfer[i].tx_buf) {
        // min(num of tx to send, tx fifo room)
        int min_num = min(len - (tx_end - tx_buf),
                          8 - dw_readl(&dws, DW_SPI_TXFLR));

        while(min_num--) {
          dw_writel(&dws, DW_SPI_DR, *tx_end);
          if (debug) printf("tx_buf[j]: %d\n", *tx_end);
          tx_end++;
        }
      }

      if (debug)
        printf("DW_SPI_SR: %x %p\n", dw_readl(&dws, DW_SPI_SR),
                                     dw_readl(&dws, DW_SPI_SR));

      if (xfer[i].rx_buf) {
        // min(remaining rx, rx fifo level)
        int min_num = min(len - (rx_end - rx_buf),
                          dw_readl(&dws, DW_SPI_RXFLR));
        while (min_num--) {
          if (debug) printf("rx_buf[j]: %d\n", *rx_end);
          *rx_end = dw_readl(&dws, DW_SPI_DR);
          rx_end++;
        }
        // while(!(dw_readl(&dws, DW_SPI_SR) & SR_RF_NOT_EMPT) ||
        //       (dw_readl(&dws, DW_SPI_SR) & SR_BUSY));
        // if (debug) printf("rx_buf[j]: %d\n", rx_buf[j]);
      }
    }

    int status;
    do {
      status = dw_readl(&dws, DW_SPI_SR);
    }
    while (!(status & SR_TF_EMPT) || (status & SR_BUSY));

    if (xfer[i].rx_buf)
      either_copyout(1, xfer[i].rx_buf, rx_buf, len);

    spi_enable_chip(&dws, 0);
  }
}

int
spidev_ioctl(int user_src, unsigned int cmd, unsigned long arg)
{
  int xfer_num = cmd & 0x0111;
  cmd = ((cmd >> 12) << 12);

  if (debug) printf("xfer_num: %x cmd: %x\n", xfer_num, cmd);

  switch (cmd) {
  case SPI_IOC_WR_MODE:
    dws.mode = arg;
    break;
  case SPI_IOC_WR_MAX_SPEED_HZ:
    dws.max_freq = arg;
    dws.current_freq = arg;
    break;
  case SPI_IOC_MESSAGE:
    transfer(arg, xfer_num);
    break;
  }
  return 0;
}

int
spidev_open(struct inode *ip)
{
  dws.master = ip->minor;
  dws.bits = 8;
  dws.bus_freq = 100 * 1000 * 1000;
  return 0;
}

int
spidev_close(void)
{
  dws.master = -1;
  dws.mode = 0;
  dws.bits = 0;
  dws.max_freq = 0;
  dws.current_freq = 0;
  dws.bus_freq = 0;
  return 0;
}

void
spidev_init(void)
{
  devsw[SPI].open = spidev_open;
  devsw[SPI].ioctl = spidev_ioctl;
  devsw[SPI].close = spidev_close;
}
