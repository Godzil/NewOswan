/*******************************************************************************
 * NewOswan
 * rom.h:
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

#ifndef __ROM_H__
#define __ROM_H__

#include <stdint.h>

#define WS_ROM_SIZE_2MBIT 1
#define WS_ROM_SIZE_4MBIT 2
#define WS_ROM_SIZE_8MBIT 3
#define WS_ROM_SIZE_16MBIT 4
#define WS_ROM_SIZE_24MBIT 5
#define WS_ROM_SIZE_32MBIT 6
#define WS_ROM_SIZE_48MBIT 7
#define WS_ROM_SIZE_64MBIT 8
#define WS_ROM_SIZE_128MBIT 9

#define WS_EEPROM_SIZE_NONE 0
#define WS_EEPROM_SIZE_1k 0x10
#define WS_EEPROM_SIZE_16k 0x20
#define WS_EEPROM_SIZE_32k 0x30
/* 40 is not valid */
#define WS_EEPROM_SIZE_8k 0x50
#define WS_EEPROM_SIZE_4k 0x60
#define WS_EEPROM_SIZE_2k 0x70

#define WS_SRAM_SIZE_NONE 0
#define WS_SRAM_SIZE_64k 0x01
#define WS_SRAM_SIZE_256k 0x02
#define WS_SRAM_SIZE_1M 0x03
#define WS_SRAM_SIZE_2M 0x04
#define WS_SRAM_SIZE_4M 0x05

#pragma pack(1)
typedef struct ws_romHeaderStruct
{
    /* Miss "Fixed Data" (F5h) */
    uint8_t developperId;           /* Maker Code L */  
    uint8_t minimumSupportSystem;   /* Maker Code H */
    uint8_t cartId;                 /* Title code */
    uint8_t gameVertion;            /* Version */
    uint8_t romSize;                /* ROM Size */
    uint8_t saveSize;               /* XROM/XEROM Size */
    uint8_t cartFlags;              /* Boot loader */
    uint8_t realtimeClock;          /* Syb System LSI */
    uint16_t checksum;              /* Checksum */
} ws_romHeaderStruct;
#pragma pack()

uint8_t *ws_rom_load(char *path, uint32_t *romSize);
void ws_rom_dumpInfo(uint8_t *wsrom, uint32_t wsromSize);
ws_romHeaderStruct *ws_rom_getHeader(uint8_t *wsrom, uint32_t wsromSize);
uint32_t ws_rom_sramSize(uint8_t *wsrom, uint32_t wsromSize);
uint32_t ws_rom_eepromSize(uint8_t *wsrom, uint32_t wsromSize);

static inline uint8_t *ws_get_page_ptr(uint8_t *wsrom, uint32_t romSize, uint16_t page)
{
    uint32_t temp = page << 16;
    temp &= (romSize - 1);
    return &wsrom[temp];
}

#endif
