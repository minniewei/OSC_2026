.section ".text.boot"
.global _start
_start:
    // Set up the stack pointer
    la sp, _end

    // Clear the BSS section
    la t0, __bss_start
    la t1, __bss_stop

clear_bss_loop:
    beq t0, t1, jump_to_kernel
    sb zero, 0(t0)
    addi t0, t0, 1
    j clear_bss_loop

jump_to_kernel:
    tail start_kernel