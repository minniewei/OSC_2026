#define FDT_MAGIC 0xd00dfeed
#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

#define MAX_DEPTH 32

#include <stdint.h>
#include <stddef.h>
#include "fdt.h"
#include "function.h"

struct fdt_header
{
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

int fdt_path_offset(const void *fdt, const char *path)
{
    const struct fdt_header *hdr = (const struct fdt_header *)fdt;
    uint32_t off_dt_struct = bswap32(hdr->off_dt_struct);
    const char *dt_struct = (const char *)fdt + off_dt_struct;

    if (*path == '/')
        path++;
    if (*path == '\0')
        return 0;

    const char *p = dt_struct;

    // Skip root FDT_BEGIN_NODE tag and name
    uint32_t tag = bswap32(*(uint32_t *)p);
    if (tag != FDT_BEGIN_NODE)
        return -1;

    const char *tag_pos = p;
    p += 4;
    size_t name_len = strlen(p) + 1;
    p = (const char *)align_up(p + name_len, 4);

    // Parse path: split by '/' and find each component
    while (*path)
    {
        const char *slash = strchr(path, '/');
        size_t comp_len = slash ? (slash - path) : strlen(path);

        // Search for matching child node
        int found = 0;
        const char *target_tag_pos = NULL;

        while (1)
        {
            tag = bswap32(*(uint32_t *)p);

            if (tag == FDT_PROP)
            {
                // Skip property: tag(4) + len(4) + nameoff(4) + data
                uint32_t plen = bswap32(*(uint32_t *)(p + 4));
                p += 12 + plen;
                p = (const char *)align_up(p, 4);
            }
            else if (tag == FDT_BEGIN_NODE)
            {
                tag_pos = p; // Record tag position
                p += 4;
                const char *node_name = p;
                size_t node_len = strlen(node_name);

                // Match node name: either exact match or match up to '@'
                int node_matches = 0;
                const char *at_pos = strchr(node_name, '@');
                size_t base_name_len = at_pos ? (at_pos - node_name) : node_len;

                if (strncmp(path, node_name, comp_len) == 0 && node_name[comp_len] == '\0')
                {
                    // Exact match
                    node_matches = 1;
                }
                else if (strncmp(path, node_name, comp_len) == 0 &&
                         comp_len == base_name_len &&
                         node_name[comp_len] == '@')
                {
                    // Match base name before '@'
                    node_matches = 1;
                }

                if (node_matches)
                {
                    // Found matching component
                    found = 1;
                    target_tag_pos = tag_pos;
                    p = (const char *)align_up(p + node_len + 1, 4);

                    if (slash == NULL)
                    {
                        // This is the target node
                        return (int)(target_tag_pos - dt_struct);
                    }
                    path = slash + 1;
                    break;
                }
                else
                {
                    // Skip this node and its subtree
                    p = (const char *)align_up(p + node_len + 1, 4);
                    int depth = 1;
                    while (depth > 0)
                    {
                        tag = bswap32(*(uint32_t *)p);

                        if (tag == FDT_BEGIN_NODE)
                        {
                            p += 4;
                            size_t n_len = strlen(p) + 1;
                            p = (const char *)align_up(p + n_len, 4);
                            depth++;
                        }
                        else if (tag == FDT_END_NODE)
                        {
                            p += 4;
                            depth--;
                        }
                        else if (tag == FDT_PROP)
                        {
                            uint32_t plen = bswap32(*(uint32_t *)(p + 4));
                            p += 12 + plen;
                            p = (const char *)align_up(p, 4);
                        }
                        else
                        {
                            p += 4;
                        }
                    }
                }
            }
            else if (tag == FDT_END_NODE)
            {
                // End of children at this level
                p += 4;
                if (!found)
                    return -1;
                break;
            }
            else if (tag == FDT_END)
            {
                return -1;
            }
            else
            {
                p += 4;
            }
        }
    }

    return -1;
}

const void *fdt_getprop(const void *fdt, int nodeoffset, const char *name, int *lenp)
{
    const struct fdt_header *hdr = (const struct fdt_header *)fdt;
    uint32_t off_dt_struct = bswap32(hdr->off_dt_struct);
    uint32_t off_dt_strings = bswap32(hdr->off_dt_strings);
    const char *dt_struct = (const char *)fdt + off_dt_struct;
    const char *dt_strings = (const char *)fdt + off_dt_strings;

    const char *p = dt_struct + nodeoffset;

    // Read FDT_BEGIN_NODE tag
    uint32_t tag = bswap32(*(uint32_t *)p);

    if (tag != FDT_BEGIN_NODE)
        return NULL;

    p += 4;
    // Skip node name
    size_t node_name_len = strlen(p) + 1;
    p = (const char *)align_up(p + node_name_len, 4);

    // Search for the property
    while (1)
    {
        tag = bswap32(*(uint32_t *)p);

        if (tag == FDT_PROP)
        {
            p += 4;
            uint32_t prop_len = bswap32(*(uint32_t *)p);
            uint32_t prop_nameoff = bswap32(*(uint32_t *)(p + 4));
            const char *prop_name = dt_strings + prop_nameoff;

            if (strcmp(prop_name, name) == 0)
            {
                *lenp = prop_len;
                return p + 8;
            }

            // Skip this property
            p = (const char *)align_up(p + 8 + prop_len, 4);
        }
        else if (tag == FDT_END_NODE)
        {
            break;
        }
        else if (tag == FDT_END)
        {
            break;
        }
        else
        {
            break;
        }
    }

    return NULL;
}