// Physical memory layout

// qemu -machine virt is set up like this,
// based on qemu's hw/riscv/virt.c:
//
// 00001000 -- boot ROM, provided by qemu
// 02000000 -- CLINT
// 0C000000 -- PLIC
// 10000000 -- uart0 
// 10001000 -- virtio disk 
// 80000000 -- boot ROM jumps here in machine mode
//             -kernel loads the kernel here
// unused RAM after 80000000.

/* cv1800b is set up like this:

   04140000 -- UART0
   041C0000 -- UART4
   04310000 -- SD0
   04320000 -- SD1
   04400000 -- boot ROM
   05200000 -- SRAM
   70000000 -- PLIC
   74000000 -- CLINT
   80000000 -- DDR
*/

// the kernel uses physical memory thus:
// 80000000 -- entry.S, then kernel text and data
// end -- start of kernel page allocation area
// PHYSTOP -- end RAM used by the kernel

#define I2C0  0x04000000L
#define I2C1  0x04010000L
#define I2C2  0x04020000L
#define I2C3  0x04030000L
#define I2C4  0x04040000L
#define I2C0_IRQ 49
#define I2C1_IRQ 50
#define I2C2_IRQ 51
#define I2C3_IRQ 52
#define I2C4_IRQ 53

// qemu puts UART registers here in physical memory.
#define UART0 0x04140000L
#define UART1 0x04150000L
#define UART2 0x04160000L
#define UART3 0x04170000L
#define UART4 0x041C0000L
#define UART0_IRQ 44
#define UART4_IRQ 48

// virtio mmio interface
// #define VIRTIO0 0x10001000
// #define VIRTIO0_IRQ 1

// core local interruptor (CLINT), which contains the timer.
#define CLINT 0x74000000L
#define CLINT_MTIMECMP(hartid) (CLINT + 0x4000 + 8*(hartid))
// #define CLINT_MTIME (CLINT + 0xBFF8) // cycles since boot.

// qemu puts platform-level interrupt controller (PLIC) here.
#define PLIC 0x70000000L
#define PLIC_PRIORITY (PLIC + 0x0)
#define PLIC_PENDING (PLIC + 0x1000)
#define PLIC_MENABLE(hart) (PLIC + 0x2000 + (hart)*0x100)
#define PLIC_SENABLE0(hart) (PLIC + 0x2080 + (hart)*0x100)
#define PLIC_SENABLE1(hart) (PLIC + 0x2084 + (hart)*0x100)
#define PLIC_MPRIORITY(hart) (PLIC + 0x200000 + (hart)*0x2000)
#define PLIC_SPRIORITY(hart) (PLIC + 0x201000 + (hart)*0x2000)
#define PLIC_MCLAIM(hart) (PLIC + 0x200004 + (hart)*0x2000)
#define PLIC_SCLAIM(hart) (PLIC + 0x201004 + (hart)*0x2000)
#define PLIC_CTRL (PLIC + 0x1FFFFC)

// the kernel expects there to be RAM
// for use by the kernel and user pages
// from physical address 0x80000000 to PHYSTOP.
#define KERNBASE 0x80000000L
#define PHYSTOP (KERNBASE + 63*1024*1024)

// map the trampoline page to the highest address,
// in both user and kernel space.
#define TRAMPOLINE (MAXVA - PGSIZE)

// map kernel stacks beneath the trampoline,
// each surrounded by invalid guard pages.
#define KSTACK(p) (TRAMPOLINE - ((p)+1)* 2*PGSIZE)

// User memory layout.
// Address zero first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
//   ...
//   TRAPFRAME (p->trapframe, used by the trampoline)
//   TRAMPOLINE (the same page as in the kernel)
#define TRAPFRAME (TRAMPOLINE - PGSIZE)
