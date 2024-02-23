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

extern struct spinlock devlock;
extern pagetable_t kernel_pagetable;

#define CVI_SPI2_BASE 0x041A0000

/* Register offsets */
#define DW_SPI_CTRLR0			0x00
#define DW_SPI_CTRLR1			0x04
#define DW_SPI_SSIENR			0x08
#define DW_SPI_MWCR			0x0c
#define DW_SPI_SER			0x10
#define DW_SPI_BAUDR			0x14
#define DW_SPI_TXFTLR			0x18
#define DW_SPI_RXFTLR			0x1c
#define DW_SPI_TXFLR			0x20
#define DW_SPI_RXFLR			0x24
#define DW_SPI_SR			0x28
#define DW_SPI_IMR			0x2c
#define DW_SPI_ISR			0x30
#define DW_SPI_RISR			0x34
#define DW_SPI_TXOICR			0x38
#define DW_SPI_RXOICR			0x3c
#define DW_SPI_RXUICR			0x40
#define DW_SPI_MSTICR			0x44
#define DW_SPI_ICR			0x48
#define DW_SPI_DMACR			0x4c
#define DW_SPI_DMATDLR			0x50
#define DW_SPI_DMARDLR			0x54
#define DW_SPI_IDR			0x58
#define DW_SPI_VERSION			0x5c
#define DW_SPI_DR			0x60
#define DW_SPI_RX_SAMPLE_DLY			0xF0

int
spiread(int user_dst, uint64 dst, int n)
{
  uint8_t value;
  uint32_t tmp;
  int i;

  acquire(&devlock);

  /* read gpio reg */
  for (i = 0; i < n; i += sizeof(value)) {

    /* check spi rx buffer not empty */
    do {
      tmp = mmio_read_32(CVI_SPI2_BASE + DW_SPI_SR);
    } while(!(tmp & 0x8));

    value = readl((volatile uint8_t *)CVI_SPI2_BASE + DW_SPI_DR);

    if(either_copyout(user_dst, dst + i, &value, sizeof(value)) == -1) {
      release(&devlock);
      return i;
    }
  }

  release(&devlock);
  return i;
}

int
spiwrite(int user_src, uint64 src, int n)
{
  uint8_t value;
  uint32_t tmp;
  int i;

  acquire(&devlock);

  for (i = 0; i < n; i += sizeof(value)) {
    if(either_copyin(&value, user_src, src + i, sizeof(value)) == -1) {
      release(&devlock);
      return i;
    }

    /* check spi tx buffer not full */
    do {
      tmp = mmio_read_32(CVI_SPI2_BASE + DW_SPI_SR);
    } while(!(tmp & 0x2));

    writel(value & 0xFF, (volatile uint8_t *)CVI_SPI2_BASE + DW_SPI_DR);
  }

  release(&devlock);
  return i;
}

int
spiinit(void)
{
  uint32_t tmp;

  /* map device */
  kvmmap(kernel_pagetable, CVI_SPI2_BASE, CVI_SPI2_BASE, PGSIZE, PTE_R | PTE_W | PMA_DEVICE);

  /* set pinmux */
  PINMUX_CONFIG(SD1_CMD, SPI2_SDO); // SPI MOSI
  PINMUX_CONFIG(SD1_CLK, SPI2_SCK); // SPI CLK
  PINMUX_CONFIG(SD1_D0, SPI2_SDI);  // SPI MISO

  /* disable spi */
  mmio_write_32(CVI_SPI2_BASE + DW_SPI_SSIENR, 0);
  /* mask interrupts */
  mmio_write_32(CVI_SPI2_BASE + DW_SPI_IMR, 0xff);

  /*
   * SCPH = 0 (0 << 6)
   * SCPOL = 0 (0 << 7)
   * Transfer mode = Transmit Only (0x1 << 8)
   * Data Frame Size = 8bit (7 << 0)
   */
  tmp = mmio_read_32(CVI_SPI2_BASE + DW_SPI_CTRLR0);
  //mmio_write_32(CVI_SPI2_BASE + DW_SPI_CTRLR0, tmp | 7 | (1 << 8));
  printf("SPI CTRLR0: 0x%x\n", tmp);

  /* div = 4 */
  mmio_write_32(CVI_SPI2_BASE + DW_SPI_BAUDR, 32);

  mmio_write_32(CVI_SPI2_BASE + DW_SPI_SER, 1);

  /* enable spi */
  mmio_write_32(CVI_SPI2_BASE + DW_SPI_SSIENR, 1);

  return 0;
}
