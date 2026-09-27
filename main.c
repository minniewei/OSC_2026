#define UART_RBR_OFFSET 0x0 // Receiver Buffer Register
#define UART_THR_OFFSET 0x0 // Transmitter Holding Register
#define UART_LSR_OFFSET 0x5 // Line Status Register
#define LSR_DR (1 << 0)
#define LSR_TDRQ (1 << 5)
#define BUFFER_SIZE 128

#include <stdint.h>
#include "sbi.h"    // sbi ecall implementation
#include "fdt.h"    // Device Tree parsing functions
#include "initrd.h" // Initramfs and CPIO operations

void *g_dft_ptr = NULL;
unsigned long uart_base;
void *initramfs_ptr = NULL;

// Initialize UART base address from device tree
void uart_init()
{
    // Skip DTB parsing if pointer is NULL or invalid
    if (g_dft_ptr == NULL)
    {
        return; // Use default UART_BASE
    }

    // Try to find the UART node in the device tree
    int offset = fdt_path_offset(g_dft_ptr, "/soc/serial");

    // If found, get the reg property
    if (offset >= 0)
    {
        int len = 0;
        const uint32_t *reg = (const uint32_t *)fdt_getprop(g_dft_ptr, offset, "reg", &len);

        if (reg != NULL && len >= 8)
        {
            // reg property is typically a 64-bit address in big-endian format
            // For now, we extract from the reg property correctly
            uart_base = ((unsigned long)bswap32(reg[0]) << 32) | bswap32(reg[1]);
        }
    }
}

// UART register access functions
static inline unsigned char uart_read(int offset)
{
    return *(unsigned char *)(uart_base + offset);
}

static inline void uart_write(int offset, unsigned char value)
{
    *(unsigned char *)(uart_base + offset) = value;
}

char uart_getc()
{
    while ((uart_read(UART_LSR_OFFSET) & LSR_DR) == 0)
    {
        // Wait for data to be available
    }
    return uart_read(UART_RBR_OFFSET);
}

void uart_putc(char c)
{
    while ((uart_read(UART_LSR_OFFSET) & LSR_TDRQ) == 0)
    {
        // Wait for transmitter to be ready
    }
    uart_write(UART_THR_OFFSET, c);
}

void uart_puts(const char *s)
{
    while (*s)
    {
        uart_putc(*s);
        s++;
    }
}

// Print an unsigned long value in hexadecimal format
void uart_puthex(unsigned long val)
{
    uart_puts("0x");
    for (int i = 15; i >= 0; i--)
    {
        int nibble = (val >> (i * 4)) & 0xF;
        if (nibble < 10)
        {
            uart_putc('0' + nibble);
        }
        else
        {
            uart_putc('a' + (nibble - 10));
        }
    }
}

// Helper function to extract command and argument
static int split_command(const char *cmd, char *command, char *argument)
{
    int i = 0;
    // Extract command
    while (cmd[i] && cmd[i] != ' ')
    {
        command[i] = cmd[i];
        i++;
    }
    command[i] = '\0';

    // Skip spaces
    while (cmd[i] && cmd[i] == ' ')
        i++;

    // Extract argument
    int j = 0;
    while (cmd[i])
    {
        argument[j] = cmd[i];
        i++;
        j++;
    }
    argument[j] = '\0';

    return 1;
}

// Decide what to do based on the command input
void handle_command(const char *cmd)
{
    char command[64];
    char argument[64];

    if (strcmp(cmd, "help") == 0)
    {
        uart_puts("Available commands:\r\n");
        uart_puts("  help  - show all commands.\r\n");
        uart_puts("  hello - print Hello world.\r\n");
        uart_puts("  info  - print system info.\r\n");
        uart_puts("  ls    - list files in initramfs.\r\n");
        uart_puts("  cat   - display file content.\r\n");
    }
    else if (strcmp(cmd, "hello") == 0)
    {
        uart_puts("Hello world.\r\n");
    }
    else if (strcmp(cmd, "info") == 0)
    {
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
    else if (strcmp(cmd, "ls") == 0)
    {
        if (initramfs_ptr == NULL)
        {
            uart_puts("Error: initramfs not loaded\r\n");
        }
        else
        {
            uart_puts("Files in initramfs:\r\n");
            initrd_list(initramfs_ptr);
        }
    }
    else if (strncmp(cmd, "cat ", 4) == 0)
    {
        split_command(cmd, command, argument);
        if (argument[0] == '\0')
        {
            uart_puts("Usage: cat <filename>\r\n");
        }
        else if (initramfs_ptr == NULL)
        {
            uart_puts("Error: initramfs not loaded\r\n");
        }
        else
        {
            initrd_cat(initramfs_ptr, argument);
        }
    }
    else
    {
        uart_puts("Unknown command: ");
        uart_puts(cmd);
        uart_puts("\r\nUse help to get commands.\r\n");
    }
}

// Main kernel loop
void start_kernel()
{
    // Initialize UART address from device tree
    uart_init();

    // Initialize initramfs address from device tree
    initramfs_init();

    char buf[BUFFER_SIZE];
    int idx = 0;

    uart_puts("\nStarting kernel ...\n");

    while (1)
    {
        char c = uart_getc();

        // Handle Enter key (Carriage Return or Line Feed)
        if (c == '\r' || c == '\n')
        {
            buf[idx] = '\0';
            uart_puts("\r\n");
            handle_command(buf);
            idx = 0;
        }
        // Handle Backspace key (ASCII 127 or '\b')
        else if (c == 127 || c == '\b')
        {
            if (idx > 0)
            {
                idx--;
                uart_puts("\b \b");
            }
        }
        // Handle regular characters
        else
        {
            if (idx < BUFFER_SIZE - 1)
            {
                buf[idx] = c;
                idx++;
                // Echo the character back to the UART
                uart_putc(c);
            }
        }
    }
}