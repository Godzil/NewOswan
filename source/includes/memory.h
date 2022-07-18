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

/***
 * Set a memory page with a granularity of 4-16
 * @param bank: the bank (0-F) to set
 * @param pointer: a pointer to the memory to set
 */
void mem_set_bank(uint8_t bank, uint8_t *pointer);

/***
 * Set a memory page with a granularity of 8-12
 * @param bank: the bank (0-FF) to set
 * @param pointer: a pointer to the memory to set
 */
void mam_set_page(uint8_t page, uint8_t *pointer);

void mem_write(uint32_t addr, uint8_t value);
uint8_t mem_read(uint32_t addr);

#endif /* __MEMORY_H__ */

