/*******************************************************************************
 * NewOswan
 * memory.h:
 * Based on the original Oswan-unix
 * Copyright (c) 2014-2021 986-Studio. All rights reserved.
 *
 ******************************************************************************/

//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>

extern uint8_t *ws_staticRam;
extern uint8_t *internalRam;
extern uint8_t *externalEeprom;

void ws_memory_init(uint8_t *rom, uint32_t romSize);
void ws_memory_reset(void);
uint8_t *memory_getRom(void);
uint32_t memory_getRomSize(void);
uint16_t memory_getRomCrc(void);
void ws_memory_done(void);
void memory_load(int fp);
void memory_save(int fp);

char *create_file(char *filename, uint32_t size);
char *load_file(char *filename);

void dump_memory();
//#define USE_PAGED_MEMORY_ACCESS
#ifdef USE_PAGED_MEMORY_ACCESS
/***
 * Set a memory page with a ganularity of 4-16
 * @param bank: the bank (0-F) to set
 * @param pointer: a pointer to the memory to set
 */
void set_memory_bank(uint8_t bank, uint8_t *pointer);

/***
 * Set a memory page with a ganularity of 8-12
 * @param bank: the bank (0-FF) to set
 * @param pointer: a pointer to the memory to set
 */
void set_memory_page(uint8_t page, uint8_t *pointer);

void set_irom_overlay();

uint8_t *getRom(uint32_t *size);
uint8_t *getSram(uint32_t *size);
#endif

#define mem_readop mem_readmem20
#define mem_readop_arg mem_readmem20
void mem_writemem20(uint32_t addr, uint8_t value);
uint8_t mem_readmem20(uint32_t addr);

#define BW_IEEPROM_SIZE (128)
#define COLOR_IEEPROM_SIZE (2048)

#endif

