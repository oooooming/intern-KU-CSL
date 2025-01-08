#ifndef __MEM_H__
#define __MEM_H__

#include <stdint.h>

/* Basic Information */
#define MEM_TEXT_START	0x00400000
#define MEM_TEXT_SIZE	0x00100000
#define MEM_DATA_START	0x10000000
#define MEM_DATA_SIZE	0x00100000


/* All simulated memory will be managed by this structure
 * use the mem_write_32() and mem_read_32() functions to
 * access/modify the simulated memory */
struct mem_region_t {
    uint32_t start, size;
    uint8_t *mem;
};


void		init_memory();
uint32_t	mem_read_32(uint32_t address);
void		mem_write_32(uint32_t address, uint32_t value);
void		mdump(int start, int stop);

extern struct mem_region_t g_mem_regions[2];

#endif
