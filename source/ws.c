/*******************************************************************************
 * NewOswan
 * ws.c: Base wonderswan implementation
 *
 * Based on the original Oswan-unix
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <log.h>
#include <rom.h>
#include "nec.h"
#include "necintrf.h"
#include <memory.h>
#include <gpu.h>
#include <io.h>
#include <audio.h>
#include <ws.h>

uint32_t ws_cycles;
uint32_t ws_skip;
uint32_t ws_cyclesByLine = 0;
uint32_t vblank_count = 0;

char *ws_sram_path = NULL;
char *ws_ieep_path = NULL;
char *ws_rom_path = NULL;
wssystem_t systemType;

void ws_patchRom(void)
{

    uint8_t *rom = memory_getRom();
    uint32_t romSize = memory_getRomSize();

    Log(TLOG_DEBUG, "ws", "developper Id: 0x%.2x", rom[romSize - 10]);
    Log(TLOG_DEBUG, "ws", "Game Id: 0x%.2x", rom[romSize - 8]);

    if (!ws_cyclesByLine)
    {
        ws_cyclesByLine = 256;
    }
}

int ws_init(char *rompath)
{
    uint8_t *rom;
    uint32_t romSize;

    if ((rom = ws_rom_load(rompath, &romSize)) == NULL)
    {
        Log(TLOG_PANIC, "ws", "Error: cannot load %s", rompath);
        return (0);
    }

    ws_staticRam = (uint8_t *)load_file(ws_sram_path);
    if (ws_staticRam == NULL)
    {
        ws_staticRam = (uint8_t *)create_file(ws_sram_path, 0x10000);
    }

    if (ws_staticRam == NULL)
    {
        Log(TLOG_PANIC, "ws", "Card SRAM load error!\n");
        return 0;
    }

    externalEeprom = (uint8_t *)load_file(ws_ieep_path);
    if (externalEeprom == NULL)
    {
        externalEeprom = (uint8_t *)create_file(ws_ieep_path, 0x100000);
    }
    if (externalEeprom == NULL)
    {
        Log(TLOG_PANIC, "ws", "Card EEPROM load error!\n");
        return 0;
    }

    ws_memory_init(rom, romSize);
    ws_patchRom();

    io_init();
    ws_audio_init();
    ws_gpu_init();

    if (ws_rotated())
    {
        io_flipControls();
    }

    return (1);
}

void ws_reset(void)
{
    ws_memory_reset();
    io_reset();
    ws_audio_reset();
    ws_gpu_reset();
    nec_reset(NULL);
    nec_set_reg(NEC_SP, 0x2000);
}

int ws_executeLine(int16_t *framebuffer, int renderLine)
{
    int drawWholeScreen = 0;

    ws_audio_process();

    // update scanline register
    //ws_ioRam[2] = ws_gpu_scanline;

    /* Why twice like that and random cycle count???? */
    ws_cycles = nec_execute((ws_cyclesByLine >> 1) + (rand() & 7));
    ws_cycles += nec_execute((ws_cyclesByLine >> 1) + (rand() & 7));

    if (ws_cycles >= ws_cyclesByLine + ws_cyclesByLine)
    {
        ws_skip = ws_cycles / ws_cyclesByLine;
    }
    else
    {
        ws_skip = 1;
    }

    ws_cycles %= ws_cyclesByLine;

    for (uint32_t uI = 0 ; uI < ws_skip ; uI++)
    {
        if (renderLine)
        {
            ws_gpu_renderScanline(framebuffer);
        }

        ws_gpu_scanline++;

        if (ws_gpu_scanline == 144)
        {
            drawWholeScreen = 1;
        }

    }

    if (ws_gpu_scanline > 158)
    {
        ws_gpu_scanline = 0;
        {
#if 0
            if ((ws_ioRam[0xb2] & 32)) /* VBLANK Timer */
            {
                /* TODO: REPAIR THAT SHIT */
                ws_ioRam[0xb6] &= ~32;
                nec_int((ws_ioRam[0xb0] + 5) * 4);
            }
#endif
        }
    }

#if 0
    ws_ioRam[2] = ws_gpu_scanline;
#endif

    if (drawWholeScreen)
    {
#if 0
        if (ws_ioRam[0xb2] & 64) /* VBLANK INT */
        {
            ws_ioRam[0xb6] &= ~64;
            nec_int((ws_ioRam[0xb0] + 6) * 4);
        }
#endif
        vblank_count++;
    }

#if 0
    if (ws_ioRam[0xa4] && (ws_ioRam[0xb2] & 128)) /*HBLANK TMR*/
    {
        /* TODO: Check that shit */
        if (!ws_ioRam[0xa5])
        {
            ws_ioRam[0xa5] = ws_ioRam[0xa4];
        }

        if (ws_ioRam[0xa5])
        {
            ws_ioRam[0xa5]--;
        }

        if ((!ws_ioRam[0xa5]) && (ws_ioRam[0xb2] & 128))
        {

            ws_ioRam[0xb6] &= ~128;
            nec_int((ws_ioRam[0xb0] + 7) * 4);
        }
    }

    if ((ws_ioRam[0x2] == ws_ioRam[0x3]) && (ws_ioRam[0xb2] & 16)) /*SCANLINE INT*/
    {
        ws_ioRam[0xb6] &= ~16;
        nec_int((ws_ioRam[0xb0] + 4) * 4);
    }
#endif

    return (drawWholeScreen);
}

void ws_done(void)
{
    ws_memory_done();
    io_done();
    ws_audio_done();
    ws_gpu_done();
}

void ws_set_system(wssystem_t system)
{
    if (system == WS_SYSTEM_AUTODETECT)
    {
        system = WS_SYSTEM_CRYSTAL;
    }
    systemType = system;
}

wssystem_t ws_get_system()
{
    return systemType;
}

#define MacroLoadNecRegisterFromFile(F, R)        \
        read(fp,&value,sizeof(value));        \
        nec_set_reg(R,value);

int ws_loadState(char *statepath)
{
    // TODO: need a complete rewrite
#if 0
    Log(TLOG_NORMAL, "ws", "loading %s\n", statepath);
    uint16_t crc = memory_getRomCrc();
    uint16_t newCrc;
    unsigned value;
    uint8_t ws_newVideoMode;

    int fp = open(statepath, O_RDONLY);

    if (fp == -1)
    {
        return (0);
    }

    read(fp, &newCrc, 2);

    if (newCrc != crc)
    {
        return (-1);
    }

    MacroLoadNecRegisterFromFile(fp, NEC_IP);
    MacroLoadNecRegisterFromFile(fp, NEC_AW);
    MacroLoadNecRegisterFromFile(fp, NEC_BW);
    MacroLoadNecRegisterFromFile(fp, NEC_CW);
    MacroLoadNecRegisterFromFile(fp, NEC_DW);
    MacroLoadNecRegisterFromFile(fp, NEC_CS);
    MacroLoadNecRegisterFromFile(fp, NEC_DS);
    MacroLoadNecRegisterFromFile(fp, NEC_ES);
    MacroLoadNecRegisterFromFile(fp, NEC_SS);
    MacroLoadNecRegisterFromFile(fp, NEC_IX);
    MacroLoadNecRegisterFromFile(fp, NEC_IY);
    MacroLoadNecRegisterFromFile(fp, NEC_BP);
    MacroLoadNecRegisterFromFile(fp, NEC_SP);
    MacroLoadNecRegisterFromFile(fp, NEC_FLAGS);
    MacroLoadNecRegisterFromFile(fp, NEC_VECTOR);
    MacroLoadNecRegisterFromFile(fp, NEC_PENDING);
    MacroLoadNecRegisterFromFile(fp, NEC_NMI_STATE);
    MacroLoadNecRegisterFromFile(fp, NEC_IRQ_STATE);

    read(fp, internalRam, 65536);
    read(fp, ws_staticRam, 65536);
    read(fp, ws_ioRam, 256);
    read(fp, ws_paletteColors, 8);
    read(fp, ws_palette, 16 * 4 * 2);
    read(fp, wsc_palette, 16 * 16 * 2);
    read(fp, &ws_newVideoMode, 1);
    read(fp, &ws_gpu_scanline, 1);
    read(fp, externalEeprom, 131072);

    ws_audio_readState(fp);
    close(fp);

    // force a video mode change to make all tiles dirty
    ws_gpu_clearCache();
#endif
    return (1);
}

#define MacroStoreNecRegisterToFile(F, R)        \
        value=nec_get_reg(R); \
        write(fp,&value,sizeof(value));

int ws_saveState(char *statepath)
{
    // TODO: need a complete rewrite
#if 0
    uint16_t crc = memory_getRomCrc();
    uint32_t value;
    char newPath[1024];
    Log(TLOG_DEBUG, "ws", "saving %s\n", statepath);

    if (strlen(statepath) < 4)
    {
        sprintf(newPath, "%s.wss", statepath);
    }
    else
    {
        int len = strlen(statepath);

        if ((statepath[len - 1] != 's') && (statepath[len - 1] != 'S'))
        {
            sprintf(newPath, "%s.wss", statepath);
        }
        else if ((statepath[len - 2] != 's') && (statepath[len - 2] != 'S'))
        {
            sprintf(newPath, "%s.wss", statepath);
        }
        else if ((statepath[len - 3] != 'w') && (statepath[len - 3] != 'w'))
        {
            sprintf(newPath, "%s.wss", statepath);
        }
        else if (statepath[len - 4] != '.')
        {
            sprintf(newPath, "%s.wss", statepath);
        }
        else
        {
            sprintf(newPath, "%s", statepath);
        }
    }

    int fp = open(newPath, O_RDWR | O_CREAT, 0644);

    if (fp == -1)
    {
        return (0);
    }

    write(fp, &crc, 2);
    MacroStoreNecRegisterToFile(fp, NEC_IP);
    MacroStoreNecRegisterToFile(fp, NEC_AW);
    MacroStoreNecRegisterToFile(fp, NEC_BW);
    MacroStoreNecRegisterToFile(fp, NEC_CW);
    MacroStoreNecRegisterToFile(fp, NEC_DW);
    MacroStoreNecRegisterToFile(fp, NEC_CS);
    MacroStoreNecRegisterToFile(fp, NEC_DS);
    MacroStoreNecRegisterToFile(fp, NEC_ES);
    MacroStoreNecRegisterToFile(fp, NEC_SS);
    MacroStoreNecRegisterToFile(fp, NEC_IX);
    MacroStoreNecRegisterToFile(fp, NEC_IY);
    MacroStoreNecRegisterToFile(fp, NEC_BP);
    MacroStoreNecRegisterToFile(fp, NEC_SP);
    MacroStoreNecRegisterToFile(fp, NEC_FLAGS);
    MacroStoreNecRegisterToFile(fp, NEC_VECTOR);
    MacroStoreNecRegisterToFile(fp, NEC_PENDING);
    MacroStoreNecRegisterToFile(fp, NEC_NMI_STATE);
    MacroStoreNecRegisterToFile(fp, NEC_IRQ_STATE);

    write(fp, internalRam, 65536);
    write(fp, ws_staticRam, 65536);
    write(fp, ws_ioRam, 256);
    write(fp, ws_paletteColors, 8);
    write(fp, ws_palette, 16 * 4 * 2);
    write(fp, wsc_palette, 16 * 16 * 2);
    write(fp, &ws_videoMode, 1);
    write(fp, &ws_gpu_scanline, 1);
    write(fp, externalEeprom, 131072);

    ws_audio_writeState(fp);
    close(fp);
#endif
    return (1);
}

int ws_rotated(void)
{
    uint8_t *rom = memory_getRom();
    uint32_t romSize = memory_getRomSize();

    return (rom[romSize - 4] & 1);
}
