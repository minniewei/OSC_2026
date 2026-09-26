.section ".text.boot"
.global _start
_start:
    // Save DTB address in a temporary register before clearing BSS
    mv s0, a1
    
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
    la t0, g_dft_ptr
    sd s0, 0(t0)
    jal ra, start_kernel
    
infinite_loop:
    wfi
    j infinite_loop