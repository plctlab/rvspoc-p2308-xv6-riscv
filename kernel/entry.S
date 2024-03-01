        # qemu -kernel loads the kernel at 0x80000000
        # and causes each hart (i.e. CPU) to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.
.section .text
.global _entry
_entry:
        # set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        la sp, stack0
        li a0, 1024*4
        csrr a1, mhartid
        addi a1, a1, 1
        mul a0, a0, a1
        add sp, sp, a0

        li   t3, 0
        csrr t4, mhartid
        bne  t3, t4, callstart

        la   t0, bss_start
        la   t1, bss_end
        li   t2, 0
clearbss:
        sb   t2, 0(t0)
        addi  t0, t0, 1
        bleu t0, t1, clearbss
callstart:
        # jump to start() in start.c
        call start
spin:
        j spin
