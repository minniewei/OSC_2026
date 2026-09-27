#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include <stddef.h>
#include <stdint.h>

// Byte swap utility functions
uint32_t bswap32(uint32_t x);
uint64_t bswap64(uint64_t x);

// String utility functions declarations
size_t strlen(const char *s);
int strncmp(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);
char *strchr(const char *s, int c);
const void *align_up(const void *ptr, size_t align);
int align_int(int n, int align);
int hextoi(const char *s, int n);

#endif
