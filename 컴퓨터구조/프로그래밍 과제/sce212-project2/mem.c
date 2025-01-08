#include "mem.h"
#include "util.h"

struct mem_region_t g_mem_regions[] = {
    { MEM_TEXT_START, MEM_TEXT_SIZE, NULL },
    { MEM_DATA_START, MEM_DATA_SIZE, NULL },
};

#define MEM_NREGIONS (sizeof(g_mem_regions)/sizeof(struct mem_region_t))

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Allocate and zero memory                        */
/*                                                             */
/***************************************************************/
void init_memory() {
    int i;
    int mem_nr_regions = (sizeof(g_mem_regions)/sizeof(struct mem_region_t));

    for (i = 0; i < mem_nr_regions; i++) {
        g_mem_regions[i].mem = malloc(g_mem_regions[i].size);
        memset(g_mem_regions[i].mem, 0, g_mem_regions[i].size);
    }
}
/***************************************************************/
/*                                                             */
/* Procedure: mem_read_32                                      */
/*                                                             */
/* Purpose: Read a 32-bit word from memory                     */
/*                                                             */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
    int i;
    int mem_nr_regions = (sizeof(g_mem_regions)/sizeof(struct mem_region_t));

    for (i = 0; i < mem_nr_regions; i++) {
        if (address >= g_mem_regions[i].start &&
            address < (g_mem_regions[i].start + g_mem_regions[i].size)) {
            uint32_t offset = address - g_mem_regions[i].start;

            return
                (g_mem_regions[i].mem[offset+3] << 24) |
                (g_mem_regions[i].mem[offset+2] << 16) |
                (g_mem_regions[i].mem[offset+1] <<  8) |
                (g_mem_regions[i].mem[offset+0] <<  0);
        }
    }

    return 0;
}

/***************************************************************/
/*                                                             */
/* Procedure: mem_write_32                                     */
/*                                                             */
/* Purpose: Write a 32-bit word to memory                      */
/*                                                             */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
    int i;
    int mem_nr_regions = (sizeof(g_mem_regions)/sizeof(struct mem_region_t));

    for (i = 0; i < mem_nr_regions; i++) {
        if (address >= g_mem_regions[i].start &&
            address < (g_mem_regions[i].start + g_mem_regions[i].size)) {
            uint32_t offset = address - g_mem_regions[i].start;

            g_mem_regions[i].mem[offset+3] = (value >> 24) & 0xFF;
            g_mem_regions[i].mem[offset+2] = (value >> 16) & 0xFF;
            g_mem_regions[i].mem[offset+1] = (value >>  8) & 0xFF;
            g_mem_regions[i].mem[offset+0] = (value >>  0) & 0xFF;
            return;
        }
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(int start, int stop) {
    int address;

    printf("\n[INFO] Memory content [0x%08x..0x%08x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = start; address <= stop; address += 4)
        printf("0x%08x: 0x%08x\n", address, mem_read_32(address));
    printf("\n");
}


