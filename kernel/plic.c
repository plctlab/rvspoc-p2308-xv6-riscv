#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

//
// the riscv Platform Level Interrupt Controller (PLIC).
//

void
plicinit(void)
{

  *(uint32*)PLIC_CTRL = 1;
  // set desired IRQ priorities non-zero (otherwise disabled).
  *(uint32*)(PLIC + UART0_IRQ*4) = 1;
  // *(uint32*)(PLIC + UART4_IRQ*4) = 1; 
  // *(uint32*)(PLIC + VIRTIO0_IRQ*4) = 1;
  // *(uint32*)(PLIC + I2C0_IRQ*4) = 1;
  *(uint32*)(PLIC + I2C1_IRQ*4) = 1;
  // *(uint32*)(PLIC + I2C2_IRQ*4) = 1;
  // *(uint32*)(PLIC + I2C3_IRQ*4) = 1;
  // *(uint32*)(PLIC + I2C4_IRQ*4) = 1;
  *(uint32*)(PLIC + ADC0_IRQ*4) = 1;
}

void
plicinithart(void)
{
  int hart = cpuid();
  
  // set enable bits for this hart's S-mode
  // for the uart and virtio disk.
  // *(uint32*)PLIC_SENABLE(hart) = (1 << UART0_IRQ) | (1 << VIRTIO0_IRQ);
  *(uint32*)PLIC_SENABLE1(hart) = (1 << (UART0_IRQ - 32)) |
                                  (1 << (I2C1_IRQ - 32));
                                  // (1 << (UART4_IRQ - 32)) |
                                  // (1 << (I2C0_IRQ - 32)) |
                                  // (1 << (I2C2_IRQ - 32)) |
                                  // (1 << (I2C3_IRQ - 32)) |
                                  // (1 << (I2C4_IRQ - 32));
  *(uint32*)PLIC_SENABLE3(hart) = 1 << (ADC0_IRQ - 96);

  // set this hart's S-mode priority threshold to 0.
  *(uint32*)PLIC_SPRIORITY(hart) = 0;
}

// ask the PLIC what interrupt we should serve.
int
plic_claim(void)
{
  int hart = cpuid();
  int irq = *(uint32*)PLIC_SCLAIM(hart);
  return irq;
}

// tell the PLIC we've served this IRQ.
void
plic_complete(int irq)
{
  int hart = cpuid();
  *(uint32*)PLIC_SCLAIM(hart) = irq;
}
