#include "types.h"

struct i2c_msg {
  uint16 addr;      /* slave address            */
  uint16 flags;
#define I2C_M_RD           0x0001    /* read data, from slave to master */
                    /* I2C_M_RD is guaranteed to be 0x0001! */
#define I2C_M_WRSTOP       0x0002    /* if allow stop between msg. */
#define I2C_M_TEN          0x0010    /* this is a ten bit chip address */
#define I2C_M_DMA_SAFE     0x0200    /* the buffer of this message is DMA safe */
                    /* makes only sense in kernelspace */
                    /* userspace buffers are copied anyway */
#define I2C_M_RECV_LEN     0x0400    /* length will be first received byte */
#define I2C_M_NO_RD_ACK    0x0800    /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK   0x1000    /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_REV_DIR_ADDR 0x2000    /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NOSTART      0x4000    /* if I2C_FUNC_NOSTART */
#define I2C_M_STOP         0x8000    /* if I2C_FUNC_PROTOCOL_MANGLING */
  uint16 len;       /* msg length                */
  uint8 *buf;       /* pointer to msg data            */
};

struct i2c_adapter;

struct i2c_algorithm {
  /*
   * master_xfer should return the number of messages successfully
   * processed, or a negative value on error
   */
  int (*master_xfer)(struct i2c_adapter *adap, struct i2c_msg *msgs,
                     int num);
};

/*
 * i2c_adapter is the structure used to identify a physical i2c bus along
 * with the access algorithms necessary to access it.
 */
struct i2c_adapter {
  const struct i2c_algorithm *algo; /* the algorithm to access the bus */
  void *algo_data;

  int retries;

  int nr;
};

struct i2c_client {

  unsigned short addr;    /* chip address - NOTE: 7bit    */
                          /* addresses are stored in the  */
                          /* _LOWER_ 7 bits               */

  struct i2c_adapter *adapter;  /* the adapter we sit on  */

};

int i2c_transfer_buffer_flags(const struct i2c_client *client, uchar *buf,
                              int count, uint16 flags);

/**
 * i2c_master_recv - issue a single I2C message in master receive mode
 * @client: Handle to slave device
 * @buf: Where to store data read from slave
 * @count: How many bytes to read, must be less than 64k since msg.len is u16
 *
 * Returns negative errno, or else the number of bytes read.
 */
static inline int i2c_master_recv(const struct i2c_client *client,
                                  uchar *buf, int count)
{
  return i2c_transfer_buffer_flags(client, buf, count, I2C_M_RD);
};

/**
 * i2c_master_send - issue a single I2C message in master transmit mode
 * @client: Handle to slave device
 * @buf: Data that will be written to the slave
 * @count: How many bytes to write, must be less than 64k since msg.len is u16
 *
 * Returns negative errno, or else the number of bytes written.
 */
static inline int i2c_master_send(const struct i2c_client *client,
                                  const unsigned char *buf, int count)
{
  return i2c_transfer_buffer_flags(client, (uchar *)buf, count, 0);
};
