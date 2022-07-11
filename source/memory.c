/*******************************************************************************
 * NewOswan
 * memory.c: Memory implementation
 *
 * Based on the original Oswan-unix
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdint.h>
#include <memory.h>

#include <log.h>

void dump_memory()
{
    // TODO: Need complete rewrite
}

/* 256 page of 12 bits */
uint8_t *pagedMemory[0x100];

/* Memory address is 20bit and split in 8 (page) - 12 (offset) */
void mem_write(uint32_t addr, uint8_t value)
{
    uint8_t page = addr >> 12;
    uint16_t offset = addr & 0xFFF;

    /* Everything from 3000:0000h is readonly, so we ignore all tentative to write there. */
    if (page < 0x30)
    {
        /* Unmapped will be NULL so just check to be sure */
        if (pagedMemory[page])
        {
            pagedMemory[page][offset] = value;
        }
    }
}

uint8_t mem_read(uint32_t addr)
{
    uint8_t page = addr >> 12;
    uint16_t offset = addr & 0xFFF;

    if (pagedMemory[page])
    {
        return pagedMemory[page][offset];
    }
    return 0x90;
}

/* Set memory bank with a granularity of 4-16 as it is the most common on the WonderSwan */
void set_memory_bank(uint8_t bank, uint8_t *pointer)
{
    uint8_t page = bank << 4;
    for(int i = 0; i < 16; i++)
    {
        pagedMemory[page | i] = pointer + (i * 0x1000);
    }
}

void set_memory_page(uint8_t page, uint8_t *pointer)
{
    pagedMemory[page] = pointer;
}
