/*******************************************************************************
 * NewOswan
 * wsrom.c:
 *
 *
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <log.h>

#include <file_access.h>
#include <wsrom.h>

static const char *romSizeName[] =
{
    [WSROM_ROMINFO_SIZE_2MBIT] = "2 Mbit",
    [WSROM_ROMINFO_SIZE_4MBIT] = "4 Mbit",
    [WSROM_ROMINFO_SIZE_8MBIT] = "8 Mbit",
    [WSROM_ROMINFO_SIZE_16MBIT] = "16 Mbit",
    [WSROM_ROMINFO_SIZE_24MBIT] = "24 Mbit",
    [WSROM_ROMINFO_SIZE_32MBIT] = "32 Mbit",
    [WSROM_ROMINFO_SIZE_48MBIT] = "48 Mbit",
    [WSROM_ROMINFO_SIZE_64MBIT] = "64 Mbit",
    [WSROM_ROMINFO_SIZE_128MBIT] = "128 Mbit",
};

static const char *eepromSizeName[] =
{
    [WSROM_SAVEINFO_EEPROM_SIZE_NONE] = "none",
    [WSROM_SAVEINFO_EEPROM_SIZE_1KBIT] = "1 Kbits",
    [WSROM_SAVEINFO_EEPROM_SIZE_16KBIT] = "16 Kbits",
    [WSROM_SAVEINFO_EEPROM_SIZE_32KBIT] = "32 Kbits",
    [WSROM_SAVEINFO_EEPROM_SIZE_8KBIT] = "8 Kbits",
    [WSROM_SAVEINFO_EEPROM_SIZE_4KBIT] = "4 Kbits",
    [WSROM_SAVEINFO_EEPROM_SIZE_2KBIT] = "2 Kbits",
};

static const char *sramSizeName[] =
{
    [WSROM_SAVEINFO_SRAM_SIZE_NONE] = "none",
    [WSROM_SAVEINFO_SRAM_SIZE_64KBIT] = "64 Kbit",
    [WSROM_SAVEINFO_SRAM_SIZE_256KBIT] = "256 Kkbit",
    [WSROM_SAVEINFO_SRAM_SIZE_1MBIT] = "1 Mbits",
    [WSROM_SAVEINFO_SRAM_SIZE_2MBIT] = "2 Mbits",
    [WSROM_SAVEINFO_SRAM_SIZE_4MBIT] = "4 Mbits",
};

void wsrom_dumpInfo(uint8_t *wsrom, uint32_t romSize)
{
    wsrom_rom_footer_t *romHeader = wsrom_getFooter(wsrom, romSize);
/*
    Log(TLOG_NORMAL, "rom", "developper Id  0x%.2x", romHeader->developperId);
    Log(TLOG_NORMAL, "rom", "cart Id        0x%.2x", romHeader->cartId);
    Log(TLOG_NORMAL, "rom", "minimum system %s", (romHeader->minimumSupportSystem == 0) ? "Wonderswan mono" : "Wonderswan color");
    Log(TLOG_NORMAL, "rom", "size           %i Mbits", (romSize >> 20) << 3);
    Log(TLOG_NORMAL, "rom", "eeprom         %s", eepromSizeName[romHeader->saveSize & 0xf]);
    Log(TLOG_NORMAL, "rom", "sram           %s", sramSizeName[romHeader->saveSize & 0xF0]);
    Log(TLOG_NORMAL, "rom", "rtc            %s", (romHeader->realtimeClock) ? "Yes" : "None");
    Log(TLOG_NORMAL, "rom", "checksum       0x%.4x", romHeader->checksum);
*/
}

wsrom_rom_footer_t *wsrom_getFooter(uint8_t *wsrom, uint32_t wsromSize)
{
    wsrom_rom_footer_t *wsromHeader = (wsrom_rom_footer_t *)( wsrom + wsromSize - sizeof(wsrom_rom_footer_t));

    return (wsromHeader);
}

uint32_t wsrom_getSramSize(uint8_t *wsrom, uint32_t wsromSize)
{
    wsrom_rom_footer_t *romHeader = wsrom_getFooter(wsrom, wsromSize);

    switch (romHeader->saveInfo & 0x0f)
    {
    case WSROM_SAVEINFO_SRAM_SIZE_NONE:
        return 0;

    case WSROM_SAVEINFO_SRAM_SIZE_64KBIT:
        return (64 * 1024) / 8;

    case WSROM_SAVEINFO_SRAM_SIZE_256KBIT:
        return (256 * 1024) / 8;

    case WSROM_SAVEINFO_SRAM_SIZE_1MBIT:
        return (1 * 1024 * 1024) / 8;

    case WSROM_SAVEINFO_SRAM_SIZE_2MBIT:
        return (2 * 1024 * 1024) / 8;

    case WSROM_SAVEINFO_SRAM_SIZE_4MBIT:
        return (4 * 1024 * 1024) / 8;

    default:
        Log(TLOG_PANIC, "ROM", "Invalid SRAM size (%02X)! Please check cart metadata!", romHeader->saveInfo);
    }

    return (0);
}

uint32_t wsrom_getEepromSize(uint8_t *wsrom, uint32_t wsromSize)
{
    wsrom_rom_footer_t *romHeader = wsrom_getFooter(wsrom, wsromSize);

    switch (romHeader->saveInfo & 0xf0)
    {
    case WSROM_SAVEINFO_EEPROM_SIZE_NONE:
        return 0;

    case WSROM_SAVEINFO_EEPROM_SIZE_1KBIT:
        return (1 * 1024) / 8;

    case WSROM_SAVEINFO_EEPROM_SIZE_16KBIT:
        return (16 * 1024) / 8;

    case WSROM_SAVEINFO_EEPROM_SIZE_32KBIT:
        return (32 * 1024) / 8;

    case WSROM_SAVEINFO_EEPROM_SIZE_8KBIT:
        return (8 * 1024) / 8;

    case WSROM_SAVEINFO_EEPROM_SIZE_4KBIT:
        return (4 * 1024) / 8;

    case WSROM_SAVEINFO_EEPROM_SIZE_2KBIT:
        return (2 * 1024) / 8;

    default:
        Log(TLOG_PANIC, "ROM", "Invalid EEPROM size (%02X)! Please check cart metadata!", romHeader->saveInfo);
    }

    return (0);
}
