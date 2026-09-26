#ifndef _FDT_H_
#define _FDT_H_

#include <stddef.h>
#include <stdint.h>

// Device tree blob parsing functions
int fdt_path_offset(const void *fdt, const char *path);
const void *fdt_getprop(const void *fdt, int nodeoffset, const char *name, int *lenp);

#endif /* _FDT_H_ */
