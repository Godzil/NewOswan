/*******************************************************************************
 * NewOswan
 * memory.h:
 *
 *
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>

void dump_memory();

/***
 * Set a memory page with a granularity of 4-16
 * @param bank: the bank (0-F) to set
 * @param pointer: a pointer to the memory to set
 */
void set_memory_bank(uint8_t bank, uint8_t *pointer);

/***
 * Set a memory page with a granularity of 8-12
 * @param bank: the bank (0-FF) to set
 * @param pointer: a pointer to the memory to set
 */
void set_memory_page(uint8_t page, uint8_t *pointer);

#define mem_readop mem_readmem20
#define mem_readop_arg mem_readmem20
void mem_writemem20(uint32_t addr, uint8_t value);
uint8_t mem_readmem20(uint32_t addr);

#endif /* __MEMORY_H__ */

