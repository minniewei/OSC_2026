#include "sbi.h"

struct sbiret sbi_ecall(unsigned long ext, unsigned long fid, 
                        unsigned long arg0, unsigned long arg1, unsigned long arg2, 
                        unsigned long arg3, unsigned long arg4, unsigned long arg5) 
{
    // Bind the arguments to the appropriate registers for the ecall
    register unsigned long a0 __asm__("a0") = arg0;
    register unsigned long a1 __asm__("a1") = arg1;
    register unsigned long a2 __asm__("a2") = arg2;
    register unsigned long a3 __asm__("a3") = arg3;
    register unsigned long a4 __asm__("a4") = arg4;
    register unsigned long a5 __asm__("a5") = arg5;
    register unsigned long a6 __asm__("a6") = fid;
    register unsigned long a7 __asm__("a7") = ext;

    // Execute the ecall instruction and retrieve the results from a0 and a1
    __asm__ volatile (
        "ecall"
        : "+r"(a0), "+r"(a1)                                    // Outputs: a0 and a1 will be modified by the ecall
        : "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)  // Inputs: a2-a7 are read-only inputs to the ecall
        : "memory"                                              // Clobbers: indicate that memory may be affected by the ecall
    );

    // Return the results in a struct sbiret
    struct sbiret ret;
    ret.error = (long)a0;
    ret.value = (long)a1;
    return ret;
}

// Implementation: Get the SBI specification version
long sbi_get_spec_version(void) {
    struct sbiret ret = sbi_ecall(SBI_EXT_BASE, SBI_FID_GET_SPEC_VERSION, 0, 0, 0, 0, 0, 0);
    return ret.value;
}

// Implementation: Get the SBI implementation ID
long sbi_get_impl_id(void) {
    struct sbiret ret = sbi_ecall(SBI_EXT_BASE, SBI_FID_GET_IMPL_ID, 0, 0, 0, 0, 0, 0);
    return ret.value;
}

long sbi_get_impl_version(void) {
    struct sbiret ret = sbi_ecall(SBI_EXT_BASE, SBI_FID_GET_IMPL_VERSION, 0, 0, 0, 0, 0, 0);
    return ret.value;
}