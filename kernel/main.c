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
