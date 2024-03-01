#define GPIO_SWPORTA_DR  0x000   // Port A data register
#define GPIO_SWPORTA_DDR 0x004   // Port A data direction register
#define GPIO_INTEN       0x030   // Interrupt enable register
#define GPIO_INTMASK     0x034   // Interrupt mask register
#define GPIO_INTTYPE_LEVEL 0x038 // Interrupt level register
#define GPIO_INT_POLARITY  0x03c // Interrupt polarity register
#define GPIO_INTSTATUS     0x040 // Interrupt status of Port A
#define GPIO_RAW_INTSTATUS 0x044 // Raw interrupt status of Port A (pre-masking)
#define GPIO_DEBOUNCE  0x048     // Debounce enable register
#define GPIO_PORTA_EOI 0x04c     // Port A clear interrupt register
#define GPIO_EXT_PORTA 0x050     // Port A external port register
#define GPIO_LS_SYNC   0x060     // Level-sensitive sync enable register

#define SET_CHAN 0x1010
#define SET_DIR  0x2020
