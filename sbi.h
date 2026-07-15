#ifndef _SBI_H_
#define _SBI_H_

// SBI return structure
struct sbiret {
    long error;
    long value;
};

// SBI Extension ID (EID)
#define SBI_EXT_BASE 0x10

// SBI Function ID (FID) for SBI_EXT_BASE
#define SBI_FID_GET_SPEC_VERSION     0x0
#define SBI_FID_GET_IMPL_ID          0x1
#define SBI_FID_GET_IMPL_VERSION     0x2

// SBI ecall function
struct sbiret sbi_ecall(unsigned long ext, unsigned long fid, 
                        unsigned long arg0, unsigned long arg1, unsigned long arg2, 
                        unsigned long arg3, unsigned long arg4, unsigned long arg5);

// SBI_EXT_BASE functions
long sbi_get_spec_version(void);
long sbi_get_impl_id(void);
long sbi_get_impl_version(void);

#endif /* _SBI_H_ */