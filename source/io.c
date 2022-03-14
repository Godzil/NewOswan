/*******************************************************************************
 * NewOswan
 * io.c: I/O ports implementaton
 *
 * Based on the original Oswan-unix
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include <log.h>
#include <rom.h>
#include <nec.h>
#include <necintrf.h>
#include <gpu.h>
#include <audio.h>
#include <memory.h>
#include <ws.h>
#include <io.h>

//#define IO_DUMP
//define EEPROM_DEBUG

typedef struct ioregistry_t
{
    io_read read;       /***< Read function for a specific IO port */
    io_write write;     /***< Write function for a specific IO port */
    void *private;      /***< Private data for the peripheral if needed. */
} ioregistry_t;
ioregistry_t io_registry[0x100];

extern uint32_t romAddressMask;
extern nec_Regs I;
extern uint64_t nec_monotonicCycles;
extern uint32_t sramSize;

uint8_t ws_key_start;
uint8_t ws_key_x4;
uint8_t ws_key_x2;
uint8_t ws_key_x1;
uint8_t ws_key_x3;
uint8_t ws_key_y4;
uint8_t ws_key_y2;
uint8_t ws_key_y1;
uint8_t ws_key_y3;
uint8_t ws_key_button_a;
uint8_t ws_key_button_b;
uint8_t ws_key_flipped;

FILE *ioLogFp = NULL;

void io_reset(void)
{
    ws_key_start = 0;
    ws_key_x4 = 0;
    ws_key_x2 = 0;
    ws_key_x1 = 0;
    ws_key_x3 = 0;
    ws_key_y4 = 0;
    ws_key_y2 = 0;
    ws_key_y1 = 0;
    ws_key_y3 = 0;
    ws_key_button_a = 0;
    ws_key_button_b = 0;
}

void io_init(void)
{
    io_reset();
    ws_key_flipped = 0;

#ifdef IO_DUMP
    ioLogFp = fopen("iodump.csv", "wt");
#endif
}

void io_flipControls(void)
{
    ws_key_flipped = !ws_key_flipped;
}

void io_done(void)
{
#ifdef IO_DUMP
    fclose(ioLogFp);
#endif
}

void register_io_hook(uint8_t port, io_read readHook, io_write writeHook, void *pdata)
{
    io_registry[port].read = readHook;
    io_registry[port].write = writeHook;
    io_registry[port].private = pdata;
}

void register_io_hook_array(uint8_t *portList, uint8_t listLen, io_read readHook, io_write writeHook, void *pdata)
{
    uint16_t i;
    for(i = 0; i < listLen; i++)
    {
        io_registry[portList[i]].read = readHook;
        io_registry[portList[i]].write = writeHook;
        io_registry[portList[i]].private = pdata;
    }
}

uint8_t io_readport(uint8_t port)
{
    uint8_t retVal = 0;

    if (io_registry[port].read)
    {
        retVal = io_registry[port].read(io_registry[port].private, port);
    }

    if (ioLogFp)
    {
        fprintf(ioLogFp, "%llu, R, %02X, %02X\n", nec_monotonicCycles, port, retVal);
    }

    return retVal;

#if 0
    switch (port)
    {
    case 0x4e:
    case 0x4f:
    case 0x50:
    case 0x51:
    case 0x80:
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
    case 0x87:
    case 0x88:
    case 0x89:
    case 0x8a:
    case 0x8b:
    case 0x8c:
    case 0x8d:
    case 0x8e:
    case 0x8f:
    case 0x90:
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
        retVal = ws_audio_port_read(port);
        break;

    case 0xb5:
        w1 = ws_ioRam[0xb5];

        if (w1 & 0x40)
        {
            w2 = 0x00;
            w2 = (ws_key_start << 1) | (ws_key_button_a << 2) | (ws_key_button_b << 3);
            retVal = (uint8_t)((w1 & 0xf0) | w2);
            break;
        }

        if (w1 & 0x20)
        {
            w2 = 0x00;
            w2 = (ws_key_x1 << 0) | (ws_key_x2 << 1) | (ws_key_x3 << 2) | (ws_key_x4 << 3);
            retVal = (uint8_t)((w1 & 0xf0) | w2);
            break;
        }

        if (w1 & 0x10)
        {
            w2 = 0x00;
            w2 = (ws_key_y1 << 0) | (ws_key_y2 << 1) | (ws_key_y3 << 2) | (ws_key_y4 << 3);
            retVal = (uint8_t)((w1 & 0xf0) | w2);
        }

        break;



    case 0x62:
        switch (ws_get_system())
        {
        case WS_SYSTEM_AUTODETECT:
        case WS_SYSTEM_MONO:
        case WS_SYSTEM_COLOR:
            retVal = 0x00;
            break;

        case WS_SYSTEM_CRYSTAL:
            retVal = 0x80;
            break;
        }
        break;



    case 0xc0 : // ???
        retVal = ((ws_ioRam[0xc0] & 0xf) | 0x20);
        goto exit;


    case 0xD0:
        retVal = 0;
        goto exit;

    case 0xCC:
    case 0xCD:
        retVal = 0;
        break;

    default:
        retVal = ws_ioRam[port];
        if (port > 0xD0)
        {
            Log(TLOG_DEBUG, "io", "ReadIO(%02X) <= %02X [%04X:%04Xh];", port, retVal, I.sregs[CS], I.ip);
        }
        break;

    case 0xA0:
    case 0xAA:
    case 0xAB:
    case 0xAC:
    case 0xAD:
        retVal = ws_gpu_port_read(port);
        break;

    }

    if (port == 0xA0)
    {
        Log(TLOG_ALWAYS, "A0", "Hello I'm A0 and read %02X!!!", retVal);
    }

    if (port >= 0xC4)
    {
        Log(TLOG_DEBUG, "io", "ReadMBCIO(%02X) <= %02X [%04X:%04Xh];", port, retVal, I.sregs[CS], I.ip);
    }


exit:

    if (port < 0xC0)
    {
        switch(port)
        {
        case 0x02:
        case 0x84:
        case 0x85:
        case 0x90:
        case 0x91:
        case 0xB5:
            break;

        default:
            Log(TLOG_DEBUG, "io", "ReadIO(%02X) <= %02X [%04X:%04Xh];", port, retVal, I.sregs[CS], I.ip);
            break;
        }
    }
    if (ioLogFp)
    {
        fprintf(ioLogFp, "%llu, R, %02X, %02X\n", nec_monotonicCycles, port, retVal);
    }
    return retVal;
#endif
}

void io_writeport(uint8_t port, uint8_t value)
{
    if (ioLogFp)
    {
        fprintf(ioLogFp, "%llu, W, %02X, %02X\n", nec_monotonicCycles, port, value);
    }

    if (io_registry[port].write)
    {
        io_registry[port].write(io_registry[port].private, port, value);
    }

#if 0
    if ((port == 0xA0) && (ws_ioRam[port] & 0x01) && (~value & 0x01))
    {
        value |= 0x01;
    }

    ws_ioRam[port] = value;

    switch (port)
    {
        /* GPU IOs */
    case 0x00:
        Log(TLOG_DEBUG, "GPU", "Screen enabled: W2:%c W2M:%c, SW:%c, S:%c, S2:%c, S1:%c",
            (value & 0x20)?'Y':'N',
            (value & 0x10)?'I':'O',
            (value & 0x08)?'Y':'N',
            (value & 0x04)?'Y':'N',
            (value & 0x02)?'Y':'N',
            (value & 0x01)?'Y':'N');
        break;

    case 0x04:
        if (ws_gpu_operatingInColor)
        {
            Log(TLOG_DEBUG, "GPU", "Sprite base: %04X", (value & 0x1F) << 9);
        }
        else
        {
            Log(TLOG_DEBUG, "GPU", "Sprite base: %04X", (value & 0x3F) << 9);
        }
        break;
    case 0x07:
        if (ws_gpu_operatingInColor)
        {
            Log(TLOG_DEBUG, "GPU", "Sprite Screen1 base: %04X", (value & 0x7) << 11);
            Log(TLOG_DEBUG, "GPU", "Sprite Screen2 base: %04X", (value & 0x70) << (11-4));
        }
        else
        {
            Log(TLOG_DEBUG, "GPU", "Sprite Screen1 base: %04X", (value & 0xF) << 11);
            Log(TLOG_DEBUG, "GPU", "Sprite Screen2 base: %04X", (value & 0xF0) << (11-4));
        }
        break;
    case 0x10:
        //Log(TLOG_DEBUG, "GPU", "Sprite Screen1 X scroll: %d", value);
        break;
    case 0x11:
        //Log(TLOG_DEBUG, "GPU", "Sprite Screen1 T scroll: %d", value);
        break;
    case 0x12:
        //Log(TLOG_DEBUG, "GPU", "Sprite Screen2 X scroll: %d", value);
        break;
    case 0x13:
        //Log(TLOG_DEBUG, "GPU", "Sprite Screen2 Y scroll: %d", value);
        break;

    case 0x01:
    case 0x02:
    case 0x03:
    case 0x05:
    case 0x06:
    case 0x08:
    case 0x09:
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
    case 0x14:
        break;

    case 0x15:
        Log(TLOG_DEBUG, "io", "Icons %c %c %c %c %c %c %c %c", (value >> 7) & 1 ? '?' : ' ', (value >> 6) & 1 ? '?' : ' ',
               (value >> 5) & 1 ? '3' : ' ', (value >> 4) & 1 ? '2' : ' ', (value >> 3) & 1 ? '1' : ' ',
               (value >> 2) & 1 ? 'H' : ' ', (value >> 1) & 1 ? 'V' : ' ', (value >> 0) & 1 ? 'S' : ' ');
        break;

        /* Palettes ? */
    case 0x1C:
    case 0x25:
    case 0x2F:
    case 0x38:
    case 0x1D:
    case 0x26:
    case 0x30:
    case 0x39:
    case 0x1E:
    case 0x27:
    case 0x31:
    case 0x3A:
    case 0x1F:
    case 0x28:
    case 0x32:
    case 0x3B:
    case 0x20:
    case 0x29:
    case 0x33:
    case 0x3C:
    case 0x21:
    case 0x2A:
    case 0x34:
    case 0x3E:
    case 0x22:
    case 0x2B:
    case 0x35:
    case 0x3F:
    case 0x23:
    case 0x2C:
    case 0x36:
    case 0x24:
    case 0x2E:
    case 0x37:
        break;

        /* DMAs */
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
        break;

    case 0x48:  // DMA

        // bit 7 set to start dma transfer
        if (value & 0x80)
        {
            uint32_t dma_start = (ws_ioRam[0x41] << 8) | (ws_ioRam[0x40]) | (ws_ioRam[0x42] << 16);
            uint32_t dma_dest = (ws_ioRam[0x45] << 8) | (ws_ioRam[0x44]) | (ws_ioRam[0x43] << 16);
            uint32_t dma_size = (ws_ioRam[0x47] << 8) | (ws_ioRam[0x46]);

            uint8_t dma_inc = (value & 0x01) ? -1: 1;

            Log(TLOG_VERBOSE, "DMA", "Starting DMA from %08X to %08X (len: %08X, inc: %d)",
                dma_start, dma_dest, dma_size, dma_inc);



            for (uint32_t ix = 0 ; ix < dma_size ; ix++)
            {
                mem_writemem20(dma_dest, mem_readmem20(dma_start));
                dma_start += dma_inc;
                dma_dest += dma_inc;
            }

            ws_ioRam[0x47] = 0;
            ws_ioRam[0x46] = 0;
            ws_ioRam[0x41] = (uint8_t)(dma_start >> 8);
            ws_ioRam[0x40] = (uint8_t)(dma_start & 0xff);
            ws_ioRam[0x45] = (uint8_t)(dma_dest >> 8);
            ws_ioRam[0x44] = (uint8_t)(dma_dest & 0xff);
            ws_ioRam[0x48] = 0;
        }

        break;

        /* Audio */
    case 0x4a:
    case 0x4b:
    case 0x4c:
    case 0x4d:
    case 0x4e:
    case 0x4f:
        ws_audio_port_write(port, value);
        break;

        /* DMA Start! */
    case 0x52:
        break;

        /* GPU (again) */
    case 0x60:
#ifdef USE_PAGED_MEMORY_ACCESS
        if (ws_get_system() != WS_SYSTEM_MONO)
        {
            if (value & 0x80)
            {
                set_iram_access(IRAM_FULL_ACCESS);
            }
            else
            {
                set_iram_access(IRAM_LIMITED_ACCESS);
            }
        }
#endif
        break;

        /* System */
    case 0x62:
        Log(TLOG_DEBUG, "io", "HeyHo!");
        break;

        /* Audio */
    case 0x80:
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
    case 0x87:
    case 0x88:
    case 0x89:
    case 0x8a:
    case 0x8b:
    case 0x8c:
    case 0x8d:
    case 0x8e:
    case 0x8f:
    case 0x90:
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97:
    case 0x98:
    case 0x99:
    case 0x9A:
    case 0x9B:
    case 0x9C:
    case 0x9D:
    case 0x9E:
        ws_audio_port_write(port, value);
        break;

        /* Hardware */
    case 0xA0:

        /* Force cart handshake to be set */
        ws_ioRam[port] |= 0x80;

        if (value & 0x01)
        {
            Log(TLOG_WARNING, "A0", "Oh yeah %02X BABY", value);
#ifdef USE_PAGED_MEMORY_ACCESS
            uint32_t romSize;
            uint8_t *rom = getRom(&romSize);
            set_memory_bank(0xF, ws_get_page_ptr(rom, romSize, (ws_ioRam[0xC0] & 0x0F << 4) + 0x0F));
#endif
        }
        break;

        /* Timers */
    case 0xA2:
    case 0xA4:
    case 0xA5:
    case 0xA6:
    case 0xA7:
    case 0xA8:
    case 0xA9:
    case 0xAA:
    case 0xAB:
        break;


        /* Intc */
    case 0xB0:
    case 0xB2:
    case 0xB4:
    case 0xB6:
        break;

        /* buttons */
    case 0xB5:
        break;

        /* MBC */
#ifndef USE_PAGED_MEMORY_ACCESS
    case 0xC0:
    case 0xC1:
    case 0xC2:
    case 0xC3:
        break;
#else
    case 0xC0:
    {
        /* page 4 to F */
        uint32_t romSize;
        uint8_t *rom = getRom(&romSize);
        for (int i = 0x04 ; i < 0x10 ; i++)
        {
            set_memory_bank(i, ws_get_page_ptr(rom, romSize, (value << 4) + i));
        }

        if (!(ws_ioRam[0xA0] & 0x01))
        {
            set_irom_overlay();
        }
        break;
    }

    case 0xC1:
        /* Sram bank */
        if (sramSize > 0)
        {
            uint32_t sramSize;
            uint8_t *sram = getSram(&sramSize);
            set_memory_bank(0x1, ws_get_page_ptr(sram, sramSize, value));
        }
        break;

    case 0xC2:
    {
        /* page 4 to F */
        uint32_t romSize;
        uint8_t *rom = getRom(&romSize);
        /* Page 2 */
        set_memory_bank(0x2, ws_get_page_ptr(rom, romSize, value));
        break;
    }

    case 0xC3:
    {
        /* page 4 to F */
        uint32_t romSize;
        uint8_t *rom = getRom(&romSize);
        /* Page 3 */
        set_memory_bank(0x3, ws_get_page_ptr(rom, romSize, value));
        break;
    }

#endif


    case 0xB7:
        break; /* Somwthing to write there, but what? */

    default:
        unknown_io_port = 1;
    }

    if ((ws_gpu_port_write(port, value) == 1) && (unknown_io_port == 1))
    {
        Log(TLOG_DEBUG, "io", "WriteIO(%02X, %02X) [%04X:%04Xh];", port, value, I.sregs[CS], I.ip);
    }
    if (port >= 0xC4)
    {
        Log(TLOG_DEBUG, "io", "WriteMBCIO(%02X, %02X) [%04X:%04Xh];", port, value, I.sregs[CS], I.ip);
    }


    if (port < 0xC0)
    {
        switch(port)
        {
        case 0x05: case 0x06:
        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x1C: case 0x1D: case 0x1E: case 0x1F:
        case 0xB5:
        case 0xB6:
            break;

        default:
            Log(TLOG_DEBUG, "io", "WriteIO(%02X, %02X) [%04X:%04Xh];", port, value, I.sregs[CS], I.ip);
            break;
        }
    }
#endif
}
