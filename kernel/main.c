#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  if(cpuid() == 0){
    consoleinit();
    printfinit();
    printf("\n");
    printf("\nThis system is ported by OStar Team.\n");
    int hartid = cpuid();
    printf("xv6 kernel is booting on hart %d\n", hartid);
    printf("\n");
    volatile uint32* reg = (volatile uint32*) 0x03002900;
    printf("pll_g6_ctrl:   %x %p\n", *reg, *reg);

    reg = (volatile uint32*) 0x03002904;
    printf("pll_g6_status: %x %p\n", *reg, *reg);

    reg = (volatile uint32*) 0x03002910;
    printf("fpll_csr:      %x %p\n", *reg, *reg);

    reg = (volatile uint32*) 0x03002940;
    printf("pll_g6_ssc_syn_ctrl: %x %p\n", *reg, *reg);

    reg = (volatile uint32*) 0x03002000;
    printf("clk_en_0:      %x %p\n", *reg, *reg);

    reg = (volatile uint32*) 0x03002004;
    printf("clk_en_1:      %x %p\n", *reg, *reg);

    reg = (volatile uint32*) 0x03002008;
    printf("clk_en_2:      %x %p\n", *reg, *reg);

    reg = (volatile uint32*) 0x0300200c;
    printf("clk_en_3:      %x %p\n", *reg, *reg);

    reg = (volatile uint32*) 0x03002010;
    printf("clk_en_4:      %x %p\n", *reg, *reg);

    reg = (volatile uint32*) 0x03002030;
    printf("clk_byp_0:     %x %p\n", *reg, *reg);

    reg = (volatile uint32*) 0x030020bc;
    printf("div_clk_axi6:  %x %p\n", *reg, *reg);

    reg = (volatile uint32*) 0x03002104;
    printf("div_clk_i2c:   %x %p\n", *reg, *reg);

    reg = (volatile uint32*) 0x03002120;
    printf("div_clk_pwm_src_0: %x %p\n", *reg, *reg);

    reg = (volatile uint32*) 0x03002094;
    printf("div_clk_gpio_db: %x %p\n", *reg, *reg);
    printf("\n");

    kinit();         // physical page allocator
    printf("Initialized physical page allocator\n");
    kvminit();       // create kernel page table
    printf("Created kernel page table\n");
    kvminithart();   // turn on paging
    printf("Turned on paging\n");
    procinit();      // process table
    printf("Initialized process table\n");
    trapinit();      // trap vectors
    trapinithart();  // install kernel trap vector
    printf("Installed kernel trap vector\n");
    plicinit();      // set up interrupt controller
    printf("Set up interrupt controller\n");
    plicinithart();  // ask PLIC for device interrupts
    binit();         // buffer cache
    printf("Set up buffer cache\n");
    iinit();         // inode table
    printf("Set up inode table\n");
    fileinit();      // file table
    printf("Set up file table\n");
    // virtio_disk_init(); // emulated hard disk
    ramdiskinit();   // ram disk
    printf("Initialized disk\n");
    i2cdev_init();
    printf("Initialized i2c controller\n");
    uartdev_init();
    printf("Initialized uart controller\n");
    adcdev_init();
    printf("Initialized adc controller\n");
    pwmdev_init();
    printf("Initialized pwm controller\n");
    gpiodev_init();
    printf("Initialized gpio controller\n");
    userinit();      // first user process
    __sync_synchronize();
    started = 1;
  } else {
    while(started == 0)
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();    // turn on paging
    trapinithart();   // install kernel trap vector
    plicinithart();   // ask PLIC for device interrupts
  }

  scheduler();        
}
