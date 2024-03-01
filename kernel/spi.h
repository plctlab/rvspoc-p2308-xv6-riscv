#define SPI_CPHA    0x01
#define SPI_CPOL    0x02

#define SPI_MODE_0  (0|0)
#define SPI_MODE_1  (0|SPI_CPHA)
#define SPI_MODE_2  (SPI_CPOL|0)
#define SPI_MODE_3  (SPI_CPOL|SPI_CPHA)

/* Read / Write of SPI mode (SPI_MODE_0..SPI_MODE_3) (limited to 8 bits) */
#define SPI_IOC_RD_MODE 0x1000
#define SPI_IOC_WR_MODE 0x2000

/* Read / Write SPI bit justification */
#define SPI_IOC_RD_LSB_FIRST 0x3000
#define SPI_IOC_WR_LSB_FIRST 0x4000

/* Read / Write SPI device word length (1..N) */
#define SPI_IOC_RD_BITS_PER_WORD 0x5000
#define SPI_IOC_WR_BITS_PER_WORD 0x6000

/* Read / Write SPI device default max speed hz */
#define SPI_IOC_RD_MAX_SPEED_HZ  0x7000
#define SPI_IOC_WR_MAX_SPEED_HZ  0x8000

/* Read / Write of the SPI mode field */
#define SPI_IOC_RD_MODE32 0x9000
#define SPI_IOC_WR_MODE32 0xA000

#define SPI_IOC_MESSAGE 0xF000

struct spi_ioc_transfer {
  uint64 tx_buf;
  uint64 rx_buf;

  uint32 len;
  uint32 speed_hz;
};

/* Register offsets */
#define DW_SPI_CTRLR0      0x00
#define DW_SPI_CTRLR1      0x04
#define DW_SPI_SSIENR      0x08
#define DW_SPI_MWCR        0x0c
#define DW_SPI_SER         0x10
#define DW_SPI_BAUDR       0x14
#define DW_SPI_TXFTLR      0x18
#define DW_SPI_RXFTLR      0x1c
#define DW_SPI_TXFLR       0x20
#define DW_SPI_RXFLR       0x24
#define DW_SPI_SR          0x28
#define DW_SPI_IMR         0x2c
#define DW_SPI_ISR         0x30
#define DW_SPI_RISR        0x34
#define DW_SPI_TXOICR      0x38
#define DW_SPI_RXOICR      0x3c
#define DW_SPI_RXUICR      0x40
#define DW_SPI_MSTICR      0x44
#define DW_SPI_ICR         0x48
#define DW_SPI_DMACR       0x4c
#define DW_SPI_DMATDLR     0x50
#define DW_SPI_DMARDLR     0x54
#define DW_SPI_IDR         0x58
#define DW_SPI_VERSION     0x5c
#define DW_SPI_DR          0x60
#define DW_SPI_RX_SAMPLE_DLY  0xf0
#define DW_SPI_CS_OVERRIDE    0xf4

/* Bit fields in CTRLR0 */
#define SPI_DFS_OFFSET      0

#define SPI_FRF_OFFSET      4
#define SPI_FRF_SPI         0x0
#define SPI_FRF_SSP         0x1
#define SPI_FRF_MICROWIRE   0x2
#define SPI_FRF_RESV        0x3

#define SPI_MODE_OFFSET     6
#define SPI_SCPH_OFFSET     6
#define SPI_SCOL_OFFSET     7

#define SPI_TMOD_OFFSET     8
#define SPI_TMOD_MASK       (0x3 << SPI_TMOD_OFFSET)
#define SPI_TMOD_TR         0x0    /* xmit & recv */
#define SPI_TMOD_TO         0x1    /* xmit only */
#define SPI_TMOD_RO         0x2    /* recv only */
#define SPI_TMOD_EPROMREAD  0x3    /* eeprom read mode */

#define SPI_SLVOE_OFFSET    10
#define SPI_SRL_OFFSET      11
#define SPI_CFS_OFFSET      12

/* Bit fields in SR, 7 bits */
#define SR_MASK         0x7f    /* cover 7 bits */
#define SR_BUSY         (1 << 0)
#define SR_TF_NOT_FULL  (1 << 1)
#define SR_TF_EMPT      (1 << 2)
#define SR_RF_NOT_EMPT  (1 << 3)
#define SR_RF_FULL      (1 << 4)
#define SR_TX_ERR       (1 << 5)
#define SR_DCOL         (1 << 6)
