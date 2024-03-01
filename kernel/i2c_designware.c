#include "types.h"
#include "i2c.h"
#include "i2c_designware.h"
#include "riscv.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"

struct dw_i2c_dev {
  struct spinlock     lock;

  struct i2c_msg     *msgs;
  int                 msgs_num;
  int                 msg_write_idx;
  uint32              tx_buf_len;
  uint8              *tx_buf;
  int                 msg_read_idx;
  uint32              rx_buf_len;
  uint8              *rx_buf;
  struct i2c_adapter  adapter;
  int                 rx_outstanding;
};

struct dw_i2c_dev *dw_dev;

unsigned long bases[5] = {I2C0, I2C1, I2C2, I2C3, I2C4};

int debug = 0;

inline void
regmap_read(struct dw_i2c_dev *dev, unsigned int reg, unsigned int *val)
{
    *val = *((volatile unsigned int *)(bases[(dev->adapter).nr] + reg));
}

inline void
regmap_write(struct dw_i2c_dev *dev, unsigned int reg, unsigned int val)
{
    *((volatile unsigned int *)(bases[(dev->adapter).nr] + reg)) = val;
}

void
__i2c_dw_disable(struct dw_i2c_dev *dev)
{
  int timeout = 100;
  uint32 status;

  do {
    regmap_write(dev, DW_IC_ENABLE, 0);
    /*
     * The enable status register may be unimplemented, but
     * in that case this test reads zero and exits the loop.
     */
    regmap_read(dev, DW_IC_ENABLE_STATUS, &status);
    if ((status & 1) == 0)
      return;

    /*
     * Wait 10 times the signaling period of the highest I2C
     * transfer supported by the driver (for 400KHz this is
     * 25us) as described in the DesignWare I2C databook.
     */
    for (int i = 0; i <= 10000; i++)
      if (i % 100 == 0)
        printf("waiting in i2c_designware.c\n");
  } while (timeout--);
}

/*
 * Waiting for bus not busy
 */
int
i2c_dw_wait_bus_not_busy(struct dw_i2c_dev *dev)
{
  int timeout = 10;
  uint32 status;

  do {
    regmap_read(dev, DW_IC_STATUS, &status);
    if ((status & DW_IC_STATUS_ACTIVITY) == 0)
    return 0;
  } while (timeout--);

  return -1;
}

void
i2c_dw_xfer_init(struct dw_i2c_dev *dev)
{
  if (debug) printf("i2c_dw_xfer_init\n");

  struct i2c_msg *msgs = dev->msgs;
  uint32 dummy;

  __i2c_dw_disable(dev);

  regmap_write(dev, DW_IC_TAR, msgs[dev->msg_write_idx].addr);

  /* Enforce disabled interrupts (due to HW issues) */
  regmap_write(dev, DW_IC_INTR_MASK, 0);

  /* Enable the adapter */
  regmap_write(dev, DW_IC_ENABLE, 1);

  /* Dummy read to avoid the register getting stuck on Bay Trail */
  regmap_read(dev, DW_IC_ENABLE_STATUS, &dummy);

  /* Clear and enable interrupts */
  regmap_read(dev, DW_IC_CLR_INTR, &dummy);
  regmap_write(dev, DW_IC_INTR_MASK, DW_IC_INTR_MASTER_MASK);

  // 立刻产生 tx empty 中断
}

// Returns negative errno, else the number of messages executed.
int
i2c_dw_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
  if (debug) printf("i2c_dw_xfer\n");

  int ret;

  struct dw_i2c_dev *dev = dw_dev;

  acquire(&(dev->lock));

  dev->msgs = msgs;
  dev->msgs_num = num;
  dev->msg_write_idx = 0;
  dev->msg_read_idx = 0;
  dev->rx_outstanding = 0;

  ret = i2c_dw_wait_bus_not_busy(dev);

  if (ret)
    return ret;

  i2c_dw_xfer_init(dev);

  if (debug) printf("sleep in i2c_dw_xfer\n");

  sleep(&(dev->adapter), &(dev->lock));

  regmap_write(dev, DW_IC_ENABLE, 0);

  release(&(dev->lock));

  return num;
}

const struct i2c_algorithm i2c_dw_algo = {
  .master_xfer = i2c_dw_xfer,
};

struct i2c_adapter*
i2c_dw_init(short minor)
{
  dw_dev = (struct dw_i2c_dev *)kalloc();

  (dw_dev->adapter).nr = minor;
  (dw_dev->adapter).retries = 3;
  (dw_dev->adapter).algo = &i2c_dw_algo;

  initlock(&(dw_dev->lock), "i2c_dw_lock");

  /* initialize hardware */

  /* Disable interrupt */
  regmap_write(dw_dev, DW_IC_INTR_MASK, 0);

  /* Disable the adapter */
  __i2c_dw_disable(dw_dev);

  /* Write standard speed timing parameters */
  regmap_write(dw_dev, DW_IC_SS_SCL_HCNT, 460);
  regmap_write(dw_dev, DW_IC_SS_SCL_LCNT, 540);

  /* Write fast mode/fast mode plus timing parameters */
  regmap_write(dw_dev, DW_IC_FS_SCL_HCNT, 90);
  regmap_write(dw_dev, DW_IC_FS_SCL_LCNT, 160);

  /* Write SDA hold time if supported */
  regmap_write(dw_dev, DW_IC_SDA_HOLD, 1);
  regmap_write(dw_dev, DW_IC_SDA_SETUP, 25);
  regmap_write(dw_dev, DW_IC_FS_SPKLEN, 5);

  /* Configure Tx/Rx FIFO threshold levels */
  regmap_write(dw_dev, DW_IC_TX_TL, 32);
  regmap_write(dw_dev, DW_IC_RX_TL, 0);

  /* Configure the I2C master */
  regmap_write(dw_dev, DW_IC_CON, DW_IC_CON_MASTER |
                                  DW_IC_CON_SPEED_STD |
                                  DW_IC_CON_RESTART_EN |
                                  DW_IC_CON_SLAVE_DISABLE);

  return &(dw_dev->adapter);
}

void
i2c_dw_close(void)
{
  kfree(dw_dev);
}

/* ---------------- interrupt-related functions below ------------------- */

uint32
i2c_dw_read_clear_intrbits(struct dw_i2c_dev *dev)
{
  uint32 stat, dummy;

  /*
   * The IC_INTR_STAT register just indicates "enabled" interrupts.
   * The unmasked raw version of interrupt status bits is available
   * in the IC_RAW_INTR_STAT register.
   *
   * That is,
   *   stat = readl(IC_INTR_STAT);
   * equals to,
   *   stat = readl(IC_RAW_INTR_STAT) & readl(IC_INTR_MASK);
   *
   * The raw version might be useful for debugging purposes.
   */
  regmap_read(dev, DW_IC_INTR_STAT, &stat);

  /*
   * Do not use the IC_CLR_INTR register to clear interrupts, or
   * you'll miss some interrupts, triggered during the period from
   * readl(IC_INTR_STAT) to readl(IC_CLR_INTR).
   *
   * Instead, use the separately-prepared IC_CLR_* registers.
   */
  if (stat & DW_IC_INTR_RX_UNDER)
    regmap_read(dev, DW_IC_CLR_RX_UNDER, &dummy);
  if (stat & DW_IC_INTR_RX_OVER)
    regmap_read(dev, DW_IC_CLR_RX_OVER, &dummy);
  if (stat & DW_IC_INTR_TX_OVER)
    regmap_read(dev, DW_IC_CLR_TX_OVER, &dummy);
  if (stat & DW_IC_INTR_RD_REQ)
    regmap_read(dev, DW_IC_CLR_RD_REQ, &dummy);
  if (stat & DW_IC_INTR_TX_ABRT) {
    /*
     * The IC_TX_ABRT_SOURCE register is cleared whenever
     * the IC_CLR_TX_ABRT is read.  Preserve it beforehand.
     */
    // reg_read(adap->nr, DW_IC_TX_ABRT_SOURCE, &dev->abort_source);
    regmap_read(dev, DW_IC_CLR_TX_ABRT, &dummy);
  }
  if (stat & DW_IC_INTR_RX_DONE)
    regmap_read(dev, DW_IC_CLR_RX_DONE, &dummy);
  if (stat & DW_IC_INTR_ACTIVITY)
    regmap_read(dev, DW_IC_CLR_ACTIVITY, &dummy);
  if (stat & DW_IC_INTR_STOP_DET)
    regmap_read(dev, DW_IC_CLR_STOP_DET, &dummy);
  if (stat & DW_IC_INTR_START_DET)
    regmap_read(dev, DW_IC_CLR_START_DET, &dummy);
  if (stat & DW_IC_INTR_GEN_CALL)
    regmap_read(dev, DW_IC_CLR_GEN_CALL, &dummy);

  return stat;
}

void
i2c_dw_read(struct dw_i2c_dev *dev)
{
  if (debug) printf("i2c_dw_read\n");

  struct i2c_msg *msgs = dev->msgs;
  unsigned int rx_valid;

  for (; dev->msg_read_idx < dev->msgs_num; dev->msg_read_idx++) {
    uint32 len, tmp;
    uint8 *buf;

    if (!(msgs[dev->msg_read_idx].flags & I2C_M_RD))
      continue;

    len = msgs[dev->msg_read_idx].len;
    buf = msgs[dev->msg_read_idx].buf;

    regmap_read(dev, DW_IC_RXFLR, &rx_valid);

    for (; len > 0 && rx_valid > 0; len--, rx_valid--) {

      regmap_read(dev, DW_IC_DATA_CMD, &tmp);

      *buf++ = tmp;
      dev->rx_outstanding--;
    }
  }
}

void
i2c_dw_xfer_msg(struct dw_i2c_dev *dev)
{
  if (debug) printf("i2c_dw_xfer_msg\n");

  struct i2c_msg *msgs = dev->msgs;
  uint32 buf_len;
  uint8* buf;

  uint32 intr_mask = DW_IC_INTR_MASTER_MASK;

  for (; dev->msg_write_idx < dev->msgs_num; dev->msg_write_idx++) {

    buf = msgs[dev->msg_write_idx].buf;
    buf_len = msgs[dev->msg_write_idx].len;

    while (buf_len > 0) {
      uint32 cmd = 0;

      if ((dev->msg_write_idx == dev->msgs_num - 1) && buf_len == 1) {
        cmd |= (1 << 9);
      }

      if (msgs[dev->msg_write_idx].flags & I2C_M_RD) {
        regmap_write(dev, DW_IC_DATA_CMD, cmd | 0x100);
        dev->rx_outstanding++; //待接收个数
      } else {
        regmap_write(dev, DW_IC_DATA_CMD, cmd | *buf++);
      }
      buf_len--;
    }
  }

  // mask tx empty interrupt
  if (dev->msg_write_idx == dev->msgs_num)
    intr_mask &= ~DW_IC_INTR_TX_EMPTY;
  regmap_write(dev,  DW_IC_INTR_MASK, intr_mask);

}

int
i2c_dw_irq_handler(struct dw_i2c_dev *dev)
{
  if (debug) printf("i2c_dw_irq_handler\n");

  uint32 stat;
  stat = i2c_dw_read_clear_intrbits(dev);

  if (stat & DW_IC_INTR_RX_FULL) {
    if (debug) printf("DW_IC_INTR_RX_FULL\n");
    i2c_dw_read(dev);
  }

  if (stat & DW_IC_INTR_TX_EMPTY) {
    if (debug) printf("DW_IC_INTR_TX_EMPTY\n");
    i2c_dw_xfer_msg(dev);
  }

  if (stat & DW_IC_INTR_STOP_DET) {
    if (debug) printf("DW_IC_INTR_STOP_DET\n");
    wakeup(&(dev->adapter));
  }

  return 0;
}

void
i2c_dw_intr(void)
{
  if (debug) printf("i2c_dw_intr\n");

  struct dw_i2c_dev *dev = dw_dev;
  uint32 stat, enabled;

  regmap_read(dev, DW_IC_ENABLE, &enabled);
  regmap_read(dev, DW_IC_RAW_INTR_STAT, &stat);

  if (enabled && (stat & ~DW_IC_INTR_ACTIVITY))
    i2c_dw_irq_handler(dev);
}
