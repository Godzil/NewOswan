/*
 * NewOswan
 * tom.c:
 * Based on the original Oswan-unix
 * Copyright (c) 2014-2021 986-Studio. All rights reserved.
 *
 */
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "log.h"
#include "rom.h"

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
uint8_t *ws_rom_load(char *path, uint32_t *romSize)
{
    int fd;
    uint8_t *ret_ptr;
    struct stat FileStat;

    fd = open(path, O_RDWR);

    fstat(fd, &FileStat);

    *romSize = FileStat.st_size;

    ret_ptr = (uint8_t *)mmap(NULL, FileStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    close(fd);

    if (ret_ptr == MAP_FAILED)
    {
        ret_ptr = NULL;
        *romSize = 0;
    }

    return ret_ptr;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////

const char *eepromSizeName[] =
{
    [WS_EEPROM_SIZE_NONE] = "none",
    [WS_EEPROM_SIZE_64k] = "64kb",
    [WS_EEPROM_SIZE_256k] = "256kb",
};

const char *sramSizeName[] =
{
    [WS_SRAM_SIZE_NONE] = "none",
    [WS_SRAM_SIZE_1k] = "1kb",
    [WS_SRAM_SIZE_8k] = "8kb",
    [WS_SRAM_SIZE_16k] = "16kb",
};

void ws_rom_dumpInfo(uint8_t *wsrom, uint32_t romSize)
{
    ws_romHeaderStruct *romHeader = ws_rom_getHeader(wsrom, romSize);

    Log(TLOG_NORMAL, "rom", "developper Id  0x%.2x", romHeader->developperId);
    Log(TLOG_NORMAL, "rom", "cart Id        0x%.2x", romHeader->cartId);
    Log(TLOG_NORMAL, "rom", "minimum system %s", (romHeader->minimumSupportSystem == 0) ? "Wonderswan mono" : "Wonderswan color");
    Log(TLOG_NORMAL, "rom", "size           %i Mbits", (romSize >> 20) << 3);
    Log(TLOG_NORMAL, "rom", "eeprom         %s", eepromSizeName[romHeader->eepromSize & 0xf]);
    Log(TLOG_NORMAL, "rom", "sram           %s", sramSizeName[romHeader->eepromSize & 0xF0]);
    Log(TLOG_NORMAL, "rom", "rtc            %s", (romHeader->realtimeClock) ? "Yes" : "None");
    Log(TLOG_NORMAL, "rom", "checksum       0x%.4x", romHeader->checksum);
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
ws_romHeaderStruct *ws_rom_getHeader(uint8_t *wsrom, uint32_t wsromSize)
{
    ws_romHeaderStruct *wsromHeader = (ws_romHeaderStruct *)(wsrom + wsromSize - 10);

    return (wsromHeader);
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
uint32_t ws_rom_sramSize(uint8_t *wsrom, uint32_t wsromSize)
{
    ws_romHeaderStruct *romHeader = ws_rom_getHeader(wsrom, wsromSize);

    switch (romHeader->eepromSize & 0xf0)
    {
    case WS_SRAM_SIZE_NONE:
        return (0);

    case WS_SRAM_SIZE_1k:
        return (0x400);

    case WS_SRAM_SIZE_16k:
        return (0x4000);

    case WS_SRAM_SIZE_8k:
        return (0x2000);
    }

    return (0);
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
uint32_t ws_rom_eepromSize(uint8_t *wsrom, uint32_t wsromSize)
{
    ws_romHeaderStruct *romHeader = ws_rom_getHeader(wsrom, wsromSize);

    switch (romHeader->eepromSize & 0xf)
    {
    case WS_EEPROM_SIZE_NONE:
        return (0);

    case WS_EEPROM_SIZE_64k:
        return (0x10000);

    case WS_EEPROM_SIZE_256k:
        return (0x40000);
    }

    return (0);
}

