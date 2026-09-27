#include "function.h"
#include "initrd.h"
#include "fdt.h"

// External variables
extern void *g_dft_ptr;
extern void *initramfs_ptr;

// External UART functions
extern void uart_puts(const char *s);
extern void uart_putc(char c);

void initrd_list(const void *rd)
{
    const struct cpio_t *header = (const struct cpio_t *)rd;
    const char *ptr = (const char *)rd;

    while (1)
    {
        // Check magic number
        if (strncmp(header->magic, "070701", 6) != 0)
            break;

        // Parse header fields
        int namesize = hextoi(header->namesize, 8);
        int filesize = hextoi(header->filesize, 8);

        // Get filename (after header)
        const char *filename = (const char *)(header + 1);

        // Check for end of archive
        if (strcmp(filename, "TRAILER!!!") == 0)
            break;

        uart_puts(filename);
        uart_puts("\r\n");

        // Move to next file
        int offset = sizeof(struct cpio_t) + namesize + filesize;
        offset = align_int(offset, 4);
        ptr += offset;
        header = (const struct cpio_t *)ptr;
    }
}

void initrd_cat(const void *rd, const char *filename)
{
    const struct cpio_t *header = (const struct cpio_t *)rd;
    const char *ptr = (const char *)rd;

    while (1)
    {
        // Check magic number
        if (strncmp(header->magic, "070701", 6) != 0)
            break;

        // Parse header fields
        int namesize = hextoi(header->namesize, 8);
        int filesize = hextoi(header->filesize, 8);

        // Get filename (after header)
        const char *current_filename = (const char *)(header + 1);

        // Check for end of archive
        if (strcmp(current_filename, "TRAILER!!!") == 0)
            break;

        // If filename matches, print the content
        if (strcmp(current_filename, filename) == 0)
        {
            // File content is after the filename
            const char *file_content = current_filename + namesize;
            uart_puts("=== ");
            uart_puts(filename);
            uart_puts(" ===\r\n");
            for (int i = 0; i < filesize; i++)
                uart_putc(file_content[i]);
            uart_puts("\r\n");
            return;
        }

        // Move to next file
        int offset = sizeof(struct cpio_t) + namesize + filesize;
        offset = align_int(offset, 4);
        ptr += offset;
        header = (const struct cpio_t *)ptr;
    }

    uart_puts("File '");
    uart_puts(filename);
    uart_puts("' not found in initrd\r\n");
}

// Initialize initramfs address from device tree
void initramfs_init()
{
    if (g_dft_ptr == NULL)
        return;

    // Try to find /chosen node
    int offset = fdt_path_offset(g_dft_ptr, "/chosen");
    if (offset < 0)
        return;

    // Get linux,initrd-start property
    int len = 0;
    const uint32_t *initrd_start = (const uint32_t *)fdt_getprop(g_dft_ptr, offset, "linux,initrd-start", &len);

    if (initrd_start != NULL)
    {
        unsigned long addr = 0;

        if (len == 8)
        {
            // 64-bit address from big-endian format
            addr = ((unsigned long)bswap32(initrd_start[0]) << 32) | bswap32(initrd_start[1]);
        }
        else if (len == 4)
        {
            // 32-bit address from big-endian format
            addr = bswap32(initrd_start[0]);
        }

        if (addr != 0)
        {
            initramfs_ptr = (void *)addr;
        }
    }
}
