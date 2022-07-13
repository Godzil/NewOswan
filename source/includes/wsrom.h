/*******************************************************************************
 * NewOswan
 * wsrom.h:
 *
 * Based on the original Oswan-unix
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#ifndef __ROM_H__
#define __ROM_H__

#include <stdint.h>
#include <stdbool.h>

#define WSROM_ROMINFO_SIZE_1MBIT (0x00)
#define WSROM_ROMINFO_SIZE_2MBIT (0x01)
#define WSROM_ROMINFO_SIZE_4MBIT (0x02)
#define WSROM_ROMINFO_SIZE_8MBIT (0x03)
#define WSROM_ROMINFO_SIZE_16MBIT (0x04)
#define WSROM_ROMINFO_SIZE_24MBIT (0x05)
#define WSROM_ROMINFO_SIZE_32MBIT (0x06)
#define WSROM_ROMINFO_SIZE_48MBIT (0x07)
#define WSROM_ROMINFO_SIZE_64MBIT (0x08)
#define WSROM_ROMINFO_SIZE_128MBIT (0x09)

#define WSROM_SAVEINFO_EEPROM_SIZE_MASK (0xF0)
#define WSROM_SAVEINFO_EEPROM_SIZE_NONE (0x00)
#define WSROM_SAVEINFO_EEPROM_SIZE_1KBIT (0x10)
#define WSROM_SAVEINFO_EEPROM_SIZE_16KBIT (0x20)
#define WSROM_SAVEINFO_EEPROM_SIZE_32KBIT (0x30)
/* 0x40 is not valid */
#define WSROM_SAVEINFO_EEPROM_SIZE_8KBIT (0x50)
#define WSROM_SAVEINFO_EEPROM_SIZE_4KBIT (0x60)
#define WSROM_SAVEINFO_EEPROM_SIZE_2KBIT (0x70)

#define WSROM_SAVEINFO_SRAM_SIZE_MASK (0x0F)
#define WSROM_SAVEINFO_SRAM_SIZE_NONE (0x00)
#define WSROM_SAVEINFO_SRAM_SIZE_64KBIT (0x01)
#define WSROM_SAVEINFO_SRAM_SIZE_256KBIT (0x02)
#define WSROM_SAVEINFO_SRAM_SIZE_1MBIT (0x03)
#define WSROM_SAVEINFO_SRAM_SIZE_2MBIT (0x04)
#define WSROM_SAVEINFO_SRAM_SIZE_4MBIT (0x05)

#define WSROM_VALID_RESET_OPCODE (0xEA)

/* Card boot flags */
#define WSROM_BOOTFLAGS_DISALLOW_BOOTSPLASH (1 << 7)
#define WSROM_BOOTFLAGS_NONBOOTABLE_MASK (0x0F)

/* Card extended flags */
#define WSROM_EXTFLAGS_IEEPROM_WRITEENABLE (1 << 7)

/* Card flags */
#define WSROM_FLAGS_ROMCYCLE_MASK (1 << 2)
#define WSROM_FLAGS_ROMCYCLE_1  (1 << 2)
#define WSROM_FLAGS_ROMCYCLE_3 (0 << 2)

#define WSROM_FLAGS_DBUSSIZE_MASK (1 << 1)
#define WSROM_FLAGS_DBUSSIZE_8  (1 << 1)
#define WSROM_FLAGS_DBUSSIZE_16 (0 << 1)

#define WSROM_FLAGS_RTC_MASK (1 << 8)

#define WSROM_FLAGS_DFLT_ORIENTATION_MASK (1 << 0)
#define WSROM_FLAGS_DFLT_ORIENTATION_VERTICAL (1 << 0)
#define WSROM_FLAGS_DFLT_ORIENTATION_HORIZONTAL (0 << 0)

#pragma pack(1)
/* This structure is the data stored in the last 16 bytes of a cartridge */
typedef struct wsrom_rom_footer_t
{
    uint8_t resetOpcode;
    uint16_t resetOffset;
    uint16_t resetSegment;
    uint8_t cartBootFlags;
    uint16_t publisherId;           /* Maker Code L/H */
    uint8_t gameId;                 /* Title code */
    uint8_t cartFlagsExt;           /*  */
    uint8_t romInfo;                /* ROM Size */
    uint8_t saveInfo;               /* XROM/XEROM Size */
    uint16_t cartFlags;             /* Boot loader */
    uint16_t checksum;              /* Checksum */
} wsrom_rom_footer_t;
#pragma pack()

typedef struct wsrom_game_t
{
    uint8_t *romData;      /***< Content of the ROM */
    uint8_t *saveData;     /***< Content of the save SRAM or EEPROM */

    uint32_t romMask;
    uint32_t saveMask;

    bool saveIsSram;

    size_t romFileSize;
    uint16_t calculatedChecksum;

    wsrom_rom_footer_t *footer;
} wsrom_game_t;

wsrom_game_t *wsrom_loadRom(const char *filepath);
void wsrom_dumpInfo(wsrom_game_t *rom);
void wsrom_jsonSerialise(FILE *fp, wsrom_game_t *rom);
uint32_t wsrom_getSramSize(wsrom_game_t *rom);
uint32_t wsrom_getEepromSize(wsrom_game_t *rom);
uint32_t wsrom_getRomSize(wsrom_game_t *rom);
uint16_t wsrom_getChecksum(wsrom_game_t *rom);

static inline uint8_t *ws_get_page_ptr(uint8_t *wsrom, uint32_t romSize, uint16_t page)
{
    uint32_t temp = page << 16;
    temp &= (romSize - 1);
    return &wsrom[temp];
}

#endif /* __ROM_H__ */
