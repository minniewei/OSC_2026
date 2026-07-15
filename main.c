#define UART_BASE 0xD4017000
#define UART_RBR  ((volatile unsigned int*)(UART_BASE + (0x0 << 2)))    // Receiver Buffer Register
#define UART_THR  ((volatile unsigned int*)(UART_BASE + (0x0 << 2)))    // Transmitter Holding Register
#define UART_LSR  ((volatile unsigned int*)(UART_BASE + (0x5 << 2)))    // Line Status Register   
#define LSR_DR    (1 << 0)                                              // Data Ready
#define LSR_TDRQ  (1 << 5)                                              // Transmitter Holding Register Empty

#define BUFFER_SIZE 128

#include "sbi.h"

char uart_getc() {
    while ((*UART_LSR & LSR_DR) == 0) {
        // Wait for data to be available
    }
    return *UART_RBR;
}

void uart_putc(char c) {
    while ((*UART_LSR & LSR_TDRQ) == 0) {
        // Wait for transmitter to be ready
    }
    *UART_THR = c;
}

void uart_puts(const char* s) {
    while (*s) {
        uart_putc(*s);
        s++;
    }
}



// Print an unsigned long value in hexadecimal format
void uart_puthex(unsigned long val) {
    uart_puts("0x");
    for (int i = 15; i >= 0; i--) {
        int nibble = (val >> (i * 4)) & 0xF;
        if (nibble < 10) {
            uart_putc('0' + nibble);
        } else {
            uart_putc('a' + (nibble - 10));
        }
    }
}

// Compare two strings
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}


// Decide what to do based on the command input
void handle_command(const char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        uart_puts("Available commands:\r\n");
        uart_puts("  help  - show all commands.\r\n");
        uart_puts("  hello - print Hello world.\r\n");
        uart_puts("  info  - print system info.\r\n");
    } 
    else if (strcmp(cmd, "hello") == 0) {
        uart_puts("Hello world.\r\n");
    } 
    else if (strcmp(cmd, "info") == 0) {
        // Call SBI functions to get system information
        long spec = sbi_get_spec_version();
        long impl_id = sbi_get_impl_id();
        long impl_ver = sbi_get_impl_version();

        uart_puts("System Information:\r\n");
        
        uart_puts("  OpenSBI specification version: ");
        uart_puthex(spec);
        uart_puts("\r\n");

        uart_puts("  Implementation ID: ");
        uart_puthex(impl_id);
        uart_puts("\r\n");

        uart_puts("  Implementation version: ");
        uart_puthex(impl_ver);
        uart_puts("\r\n");
    } 
    else {
        uart_puts("Unknown command: ");
        uart_puts(cmd);
        uart_puts("\r\nUse help to get commands.\r\n");
    }
}

// Main kernel loop
void start_kernel() {
    char buf[BUFFER_SIZE];
    int idx = 0;

    uart_puts("\nStarting kernel ...\n");

    while (1) {
        char c = uart_getc();

        // Handle Enter key (Carriage Return or Line Feed)
        if (c == '\r' || c == '\n') {
            buf[idx] = '\0';
            uart_puts("\r\n");
            handle_command(buf);
            idx = 0;               
        }
        // Handle Backspace key (ASCII 127 or '\b')
        else if (c == 127 || c == '\b') {
            if (idx > 0) {
                idx--;
                uart_puts("\b \b");
            }
        }
        // Handle regular characters
        else {
            if (idx < BUFFER_SIZE - 1) {
                buf[idx] = c;
                idx++;
                // Echo the character back to the UART
                uart_putc(c);
            }
        }
    }
}