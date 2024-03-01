#include "i2c.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "types.h"
#include "fs.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"

struct {
  struct spinlock lock;
  struct i2c_client client;
} i2c_dev;

/* /dev/i2c-X ioctl commands. */
#define I2C_SLAVE  0x0703  /* Use this slave address */

/*
 * @adap: Handle to I2C bus
 * @msgs: One or more messages to execute before STOP is issued to
 *        terminate the operation; each message begins with a START.
 * @num: Number of messages to be executed.
 *
 * Returns negative errno, else the number of messages executed.
 */
int
i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
  int ret, try;

  for (ret = 0, try = 0; try <= adap->retries; try++) {
    ret = adap->algo->master_xfer(adap, msgs, num);

    if (ret != -11)
      break;
  }

  return ret;
}

/**
 * i2c_transfer_buffer_flags - issue a single I2C message transferring data
 * to/from a buffer
 * @client: Handle to slave device
 * @buf: Where the data is stored
 * @count: How many bytes to transfer, must be less than 64k since msg.len is u16
 * @flags: The flags to be used for the message, e.g. I2C_M_RD for reads
 *
 * Returns negative errno, or else the number of bytes transferred.
 */
int
i2c_transfer_buffer_flags(const struct i2c_client *client, uchar *buf,
                          int count, uint16 flags)
{
  int ret;
  struct i2c_msg msg = {
    .addr = client->addr,
    .flags = flags,
    .len = count,
    .buf = buf,
  };

  ret = i2c_transfer(client->adapter, &msg, 1);

  /*
   * If everything went ok (i.e. 1 msg transferred), return #bytes
   * transferred, else error code.
   */
  return (ret == 1) ? count : ret;
}

int
i2cdev_read(int user_dst, uint64 dst, int n)
{
  uchar tmp[64];
  int ret;

  ret = i2c_master_recv(&(i2c_dev.client), tmp, n);
  if (ret >= 0)
    ret = either_copyout(user_dst, dst, tmp, n) ? -1 : ret;

  return ret;
}

int
i2cdev_write(int user_src, uint64 src, int n)
{
  uchar tmp[64];
  int ret;
  either_copyin(tmp, user_src, src, n);

  ret = i2c_master_send(&(i2c_dev.client), tmp, n);
  return ret;
}

int
i2cdev_ioctl(int user_src, unsigned int cmd, unsigned long arg)
{
  acquire(&i2c_dev.lock);

  switch (cmd) {
  case I2C_SLAVE:
    i2c_dev.client.addr = arg;
  }

  release(&i2c_dev.lock);
  return 0;
}

int
i2cdev_open(struct inode *ip)
{
  acquire(&i2c_dev.lock);

  i2c_dev.client.adapter = i2c_dw_init(ip->minor);
  release(&i2c_dev.lock);
  return 0;
}

int
i2cdev_close(void)
{
  acquire(&i2c_dev.lock);

  i2c_dev.client.addr = 0;
  i2c_dev.client.adapter = 0;

  i2c_dw_close();

  release(&i2c_dev.lock);
  return 0;
}

void
i2cdev_init(void)
{
  initlock(&i2c_dev.lock, "i2c_dev");

  devsw[I2C].read = i2cdev_read;
  devsw[I2C].write = i2cdev_write;
  devsw[I2C].ioctl = i2cdev_ioctl;
  devsw[I2C].open = i2cdev_open;
  devsw[I2C].close = i2cdev_close;

}
