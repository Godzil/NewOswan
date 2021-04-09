/*
 * NewOswan
 * memory.c: Memory implementatoion
 * Based on the original Oswan-unix
 * Copyright (c) 2014-2021 986-Studio. All rights reserved.
 *
 */
////////////////////////////////////////////////////////////////////////////////
// Notes: need to optimize cpu_writemem20
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
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "ws.h"
#include "log.h"
#include "rom.h"
#include "./nec/nec.h"
#include "io.h"
#include "gpu.h"
#include "audio.h"
#include "memory.h"

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
#define IO_ROM_BANK_BASE_SELECTOR 0xC0

uint8_t *ws_rom;
uint8_t *ws_staticRam;
uint8_t *internalRam;
uint8_t *externalEeprom;
char *internalBWIRom;
char *internalColorIRom;
char *internalCrystalIRom;

char *internalBWEeprom;
char *internalColorEeprom;
char *internalCrystalEeprom;

uint16_t *internalEeprom;

extern uint8_t *ws_ioRam;

uint16_t ws_rom_checksum;

uint8_t ws_haveCrystalIRom;
uint8_t ws_haveColorIRom;
uint8_t ws_haveBWIRom;

uint32_t sramAddressMask;
uint32_t externalEepromAddressMask;
uint32_t romAddressMask;
uint32_t romSize;

int ws_sram_dirty = 0;

extern nec_Regs I;

void dump_memory()
{
    int i;
    FILE *fp;
    printf("Dumping memory....\n");
    fp = fopen("iram.bin", "wb");
    fwrite(internalRam, 1, 0x10000, fp);
    fclose(fp);

    fp = fopen("sram.bin", "wb");
    fwrite(ws_staticRam, 1, 0x10000, fp);
    fclose(fp);

    fp = fopen("rom.bin", "wb");
    fwrite(ws_rom, 1, romSize, fp);
    fclose(fp);

    fp = fopen("memorydump.bin", "wb");
    fwrite(internalRam, 1, 0x10000, fp);
    /* page 1 */
    fwrite(&(ws_staticRam[0 & sramAddressMask]), 1, 0x10000, fp);
    fwrite(&(ws_rom[((ws_ioRam[IO_ROM_BANK_BASE_SELECTOR + 2] & ((romSize >> 16) - 1)) << 16)]), 1, 0x10000, fp);
    fwrite(&(ws_rom[((ws_ioRam[IO_ROM_BANK_BASE_SELECTOR + 3] & ((romSize >> 16) - 1)) << 16)]), 1, 0x10000, fp);

    for (i = 4 ; i < 0x10 ; i++)
    {
        int romBank = (256 - (((ws_ioRam[IO_ROM_BANK_BASE_SELECTOR] & 0xf) << 4) | (i & 0xf)));
        fwrite(&(ws_rom[(unsigned)(romSize - (romBank << 16))]), 1, 0x10000, fp);
    }

    fclose(fp);

    fp = fopen("registers.bin", "wb");
    fwrite(ws_ioRam, 1, 256, fp);
    fclose(fp);

    fp = fopen("cpuregs.bin", "wb");
    /* CS */
    fwrite(&I.sregs[CS], 1, 2, fp);
    /* IP */
    fwrite(&I.ip, 1, 2, fp);
    fclose(fp);
}

#ifndef USE_PAGED_MEMORY_ACCESS
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
void cpu_writemem20(uint32_t addr, uint8_t value)
{
    uint32_t offset = addr & 0xffff;
    uint32_t bank = addr >> 16;

    if (!bank)
    {
        // 0 - RAM - 16 KB (WS) / 64 KB (WSC) internal RAM
        ws_gpu_write_byte(offset, value);
        ws_audio_write_byte(offset, value);
    }
    else if (bank == 1)
    {
        // 1 - SRAM (cart)
        ws_staticRam[offset & sramAddressMask] = value;
        ws_sram_dirty = 1;
    }

    // other banks are read-only
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
uint8_t cpu_readmem20(uint32_t addr)
{
    uint32_t offset = addr & 0xffff;
    uint32_t bank = addr >> 16;
    //uint16_t romBank;
    uint8_t hwReg;
    uint32_t temp;
    uint8_t ret;
    wssystem_t currentSystem = ws_get_system();

    switch (bank)
    {
    case 0:
        /*
         * So the original WonderSwan, and Color/Crystal in B&W mode have 16KB of IRAM Mapped.
         * Color/Crystal in color mode have 64 KB IRAM mapped.
         */
        if (ws_gpu_operatingInColor)
        {
            return (internalRam[offset]);
        }
        else if (offset < 0x4000)
        {
            return (internalRam[offset]);
        }

        return (0x90);

    case 1:     // 1 - SRAM (cart)
        return ws_staticRam[offset & sramAddressMask];

    case 2:
        // Bank 2
        hwReg = ws_ioRam[0xC2];
        temp = hwReg << 16;
        temp += offset;
        temp &= (romSize - 1);
        return ws_rom[temp];
    case 3:
        // Bank 3
        hwReg = ws_ioRam[0xC3];
        temp = hwReg << 16;
        temp += offset;
        temp &= (romSize - 1);
        return ws_rom[temp];

    case 0xF:
        hwReg = ws_ioRam[0xA0];

        if (!(hwReg & 1))
        {
            if (currentSystem == WS_SYSTEM_COLOR && ws_haveColorIRom)
            {
                if (addr >= 0xFE000)
                {
                    ret = internalColorIRom[addr & ~0xFE000];
                    return ret;
                }
            }
            else if (currentSystem == WS_SYSTEM_CRYSTAL && ws_haveColorIRom)
            {
                if (addr >= 0xFE000)
                {
                    ret = internalCrystalIRom[addr & ~0xFE000];
                    return ret;
                }
            }
            else if (currentSystem == WS_SYSTEM_MONO && ws_haveBWIRom)
            {
                if (addr >= 0xFF000)
                {
                    ret = internalBWIRom[addr & ~0xFF000];
                    return ret;
                }
            }
        }
        // fall through

    default:
        hwReg = ws_ioRam[0xC0];
        temp = hwReg << 20;
        temp += addr & 0xFFFFF;
        temp &= (romSize - 1);
        return ws_rom[temp];
    }

    return (0x90);
}
#else
/* 256 page of 12 bits */
uint8_t *pagedMemory[0x100];

/* Memory address is 20bit and split in 8 (page) - 12 (offset) */
void cpu_writemem20(uint32_t addr, uint8_t value)
{
    uint8_t page = addr >> 12;
    uint16_t offset = addr & 0xFFF;
    if (page < 0x30)
    {
        /* Unmapped will be NULL so just check to be sure */
        if (pagedMemory[page])
        {
            pagedMemory[page][offset] = value;
        }
    }
}

uint8_t cpu_readmem20(uint32_t addr)
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

void set_irom_overlay()
{
    /* Setup the boot rom */
    if (ws_get_system() == WS_SYSTEM_MONO)
    {
        set_memory_page(0xFF, internalBWIRom);
    }
    else if (ws_get_system() == WS_SYSTEM_COLOR)
    {
        set_memory_page(0xFE, internalColorIRom);
        set_memory_page(0xFF, internalColorIRom + 0x1000);
    }
    else if (ws_get_system() == WS_SYSTEM_CRYSTAL)
    {
        set_memory_page(0xFE, internalCrystalIRom);
        set_memory_page(0xFF, internalCrystalIRom + 0x1000);
    }
}

uint8_t *getRom(uint32_t *size)
{
    *size = romSize;
    return ws_rom;
}

uint8_t *getSram(uint32_t *size)
{
    *size = sramSize;
    return ws_staticRam;
}
#endif
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
char *load_file(char *filename)
{
    int fd;
    char *ret_ptr;
    struct stat FileStat;

    fd = open(filename, O_RDWR);

    fstat(fd, &FileStat);

    Log(TLOG_DEBUG, "memory", "Trying to load %s, size = %lu...", filename, (unsigned long)FileStat.st_size);

    ret_ptr = (char *)mmap(NULL, FileStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    close(fd);

    if (ret_ptr == MAP_FAILED)
    {
        ret_ptr = NULL;
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
char *create_file(char *filename, uint32_t size)
{
    int fd;
    uint32_t i;
    char *ret_ptr;
    char buf[] = {0};

    Log(TLOG_DEBUG, "memory", "Trying to create %s, size = %u...\n", filename, size);
    fd = open(filename, O_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | O_TRUNC, 0644);
    fchmod(fd, 0644);
    close(fd);
    sync();

    fd = open(filename, O_RDWR);

    for (i = 0 ; i < size ; i++)
    {
        write(fd, buf, 1);
    }

    close(fd);
    sync();

    fd = open(filename, O_RDWR);
    ret_ptr = (char *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    close(fd);

    if (ret_ptr == MAP_FAILED)
    {
        ret_ptr = NULL;
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
void ws_memory_init(uint8_t *rom, uint32_t wsRomSize)
{
    ws_romHeaderStruct *ws_romHeader;

    ws_rom = rom;
    romSize = wsRomSize;
    ws_romHeader = ws_rom_getHeader(ws_rom, romSize);
    ws_rom_checksum = ws_romHeader->checksum;
    internalRam = (uint8_t *)malloc(0x10000);
    sramAddressMask = ws_rom_sramSize(ws_rom, romSize) - 1;
    externalEepromAddressMask = ws_rom_eepromSize(ws_rom, romSize) - 1;

    internalBWIRom = load_file("ws_irom.bin");
    internalColorIRom = load_file("wsc_irom.bin");
    internalCrystalIRom = load_file("wc_irom.bin");

    internalBWEeprom = load_file("ws_ieeprom.bin");
    if (internalBWEeprom == NULL)
    {
        internalBWEeprom = create_file("ws_ieeprom.bin", BW_IEEPROM_SIZE);
    }

    internalColorEeprom = load_file("wsc_ieeprom.bin");
    if (internalColorEeprom == NULL)
    {
        internalColorEeprom = create_file("wsc_ieeprom.bin", COLOR_IEEPROM_SIZE);
    }

    internalCrystalEeprom = load_file("wc_ieeprom.bin");
    if (internalCrystalEeprom == NULL)
    {
        internalCrystalEeprom = create_file("wc_ieeprom.bin", COLOR_IEEPROM_SIZE);
    }

    internalEeprom = (uint16_t *)internalBWEeprom;
    if (ws_get_system() == WS_SYSTEM_COLOR)
    {
        internalEeprom = (uint16_t *)internalColorEeprom;
    }
    else if (ws_get_system() == WS_SYSTEM_CRYSTAL)
    {
        internalEeprom = (uint16_t *)internalCrystalEeprom;
    }

    ws_haveBWIRom = false;
    ws_haveColorIRom = false;
    ws_haveCrystalIRom = false;

    if (internalColorIRom != NULL)
    {
        Log(TLOG_DEBUG, "memory", "B&W IROM Found!");
        ws_haveColorIRom = true;
    }
    if (internalBWIRom != NULL)
    {
        Log(TLOG_DEBUG, "memory", "Color IROM Found!");
        ws_haveBWIRom = true;
    }
    if (internalCrystalIRom != NULL)
    {
        Log(TLOG_DEBUG, "memory", "Crystal IROM Found!");
        ws_haveCrystalIRom = true;
    }

    romAddressMask = romSize - 1;


#ifdef USE_PAGED_MEMORY_ACCESS
    for(int i = 0; i < 0x100; i++)
    {
        pagedMemory[i] = NULL;
    }

    /* IRAM */
    set_memory_bank(0, internalRam);
    for(int i = 0x4; i < 0x10; i++)
    {
        set_memory_page(i, NULL);
    }

    /* Cart SRAM */
    if (sramSize > 0)
    {
        set_memory_bank(0x1, ws_get_page_ptr(ws_staticRam, sramSize, 0xFF));
    }

    set_memory_bank(0x2, ws_get_page_ptr(ws_rom, romSize, 0xFF));
    set_memory_bank(0x3, ws_get_page_ptr(ws_rom, romSize, 0xFF));

    for(int i = 0x04; i < 0x10; i++)
    {
        set_memory_bank(i, ws_get_page_ptr(ws_rom, romSize, 0xF0 + i));
    }

    set_irom_overlay();
#endif
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
void ws_memory_reset(void)
{
    memset(internalRam, 0, 0x10000);
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
void ws_memory_done(void)
{
    free(internalRam);
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
uint8_t *memory_getRom(void)
{
    return (ws_rom);
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
uint32_t memory_getRomSize(void)
{
    return (romSize);
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
uint16_t memory_getRomCrc(void)
{
    return (ws_rom_checksum);
}
