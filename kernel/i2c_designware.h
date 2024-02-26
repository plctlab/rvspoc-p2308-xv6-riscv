#define DW_IC_CON_MASTER                0x1
#define DW_IC_CON_SPEED_STD             0x2
#define DW_IC_CON_SPEED_FAST            0x4
#define DW_IC_CON_SPEED_HIGH            0x6
#define DW_IC_CON_SPEED_MASK            0x6
#define DW_IC_CON_10BITADDR_SLAVE       0x8
#define DW_IC_CON_10BITADDR_MASTER      0x10
#define DW_IC_CON_RESTART_EN            0x20
#define DW_IC_CON_SLAVE_DISABLE         0x40
#define DW_IC_CON_STOP_DET_IFADDRESSED  0x80
#define DW_IC_CON_TX_EMPTY_CTRL         0x100
#define DW_IC_CON_RX_FIFO_FULL_HLD_CTRL 0x200

#define DW_IC_CON            0x0
#define DW_IC_TAR            0x4
#define DW_IC_SAR            0x8
#define DW_IC_DATA_CMD       0x10
#define DW_IC_SS_SCL_HCNT    0x14
#define DW_IC_SS_SCL_LCNT    0x18
#define DW_IC_FS_SCL_HCNT    0x1c
#define DW_IC_FS_SCL_LCNT    0x20
#define DW_IC_HS_SCL_HCNT    0x24
#define DW_IC_HS_SCL_LCNT    0x28
#define DW_IC_INTR_STAT      0x2c
#define DW_IC_INTR_MASK      0x30
#define DW_IC_RAW_INTR_STAT  0x34
#define DW_IC_RX_TL          0x38
#define DW_IC_TX_TL          0x3c
#define DW_IC_CLR_INTR       0x40
#define DW_IC_CLR_RX_UNDER   0x44
#define DW_IC_CLR_RX_OVER    0x48
#define DW_IC_CLR_TX_OVER    0x4c
#define DW_IC_CLR_RD_REQ     0x50
#define DW_IC_CLR_TX_ABRT    0x54
#define DW_IC_CLR_RX_DONE    0x58
#define DW_IC_CLR_ACTIVITY   0x5c
#define DW_IC_CLR_STOP_DET   0x60
#define DW_IC_CLR_START_DET  0x64
#define DW_IC_CLR_GEN_CALL   0x68
#define DW_IC_ENABLE         0x6c
#define DW_IC_STATUS         0x70
#define DW_IC_TXFLR          0x74
#define DW_IC_RXFLR          0x78
#define DW_IC_SDA_HOLD       0x7c
#define DW_IC_TX_ABRT_SOURCE 0x80
#define DW_IC_SDA_SETUP      0x94
#define DW_IC_ENABLE_STATUS  0x9c
#define DW_IC_FS_SPKLEN      0xa0

#define DW_IC_INTR_RX_UNDER     0x001
#define DW_IC_INTR_RX_OVER      0x002
#define DW_IC_INTR_RX_FULL      0x004
#define DW_IC_INTR_TX_OVER      0x008
#define DW_IC_INTR_TX_EMPTY     0x010
#define DW_IC_INTR_RD_REQ       0x020
#define DW_IC_INTR_TX_ABRT      0x040
#define DW_IC_INTR_RX_DONE      0x080
#define DW_IC_INTR_ACTIVITY     0x100
#define DW_IC_INTR_STOP_DET     0x200
#define DW_IC_INTR_START_DET    0x400
#define DW_IC_INTR_GEN_CALL     0x800
#define DW_IC_INTR_RESTART_DET  0x1000

#define DW_IC_INTR_DEFAULT_MASK (DW_IC_INTR_RX_FULL | \
                                 DW_IC_INTR_TX_ABRT | \
                                 DW_IC_INTR_STOP_DET)
#define DW_IC_INTR_MASTER_MASK  (DW_IC_INTR_DEFAULT_MASK | \
                                 DW_IC_INTR_TX_EMPTY)
#define DW_IC_INTR_SLAVE_MASK   (DW_IC_INTR_DEFAULT_MASK | \
                                 DW_IC_INTR_RX_DONE | \
                                 DW_IC_INTR_RX_UNDER | \
                                 DW_IC_INTR_RD_REQ)

#define DW_IC_STATUS_ACTIVITY 0x1
