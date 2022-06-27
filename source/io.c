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
    io_read read;         /***< Read function for a specific IO port */
    io_write write;       /***< Write function for a specific IO port */
    void *private;        /***< Private data for the peripheral if needed. */
    uint8_t base_address; /***< Base IO address. Used create a "local" value when calling the read/write function */
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

void register_io_hook(uint8_t baseAddress, uint8_t port, io_read readHook, void *pdata, io_write writeHook)
{
    io_registry[baseAddress + port].base_address = baseAddress;
    io_registry[baseAddress + port].read = readHook;
    io_registry[baseAddress + port].write = writeHook;
    io_registry[baseAddress + port].private = pdata;
}

void register_io_hook_array(uint8_t baseAddress, const uint8_t *portList, uint8_t listLen, io_read readHook, io_write writeHook,
                       void *pdata)
{
    uint16_t i;
    for(i = 0; i < listLen; i++)
    {
        io_registry[baseAddress + portList[i]].base_address = baseAddress;
        io_registry[baseAddress + portList[i]].read = readHook;
        io_registry[baseAddress + portList[i]].write = writeHook;
        io_registry[baseAddress + portList[i]].private = pdata;
    }
}

uint8_t io_readport(uint8_t port)
{
    uint8_t retVal = 0;
    if (io_registry[port].read)
    {
        uint8_t localPort = port - io_registry[port].base_address;
        retVal = io_registry[port].read(io_registry[port].private, localPort);
    }

    if (ioLogFp)
    {
        fprintf(ioLogFp, "%lu, R, %02X, %02X\n", nec_monotonicCycles, port, retVal);
    }

    return retVal;
}

void io_writeport(uint8_t port, uint8_t value)
{
    if (ioLogFp)
    {
        fprintf(ioLogFp, "%lu, W, %02X, %02X\n", nec_monotonicCycles, port, value);
    }

    if (io_registry[port].write)
    {
        uint8_t localPort = port - io_registry[port].base_address;
        io_registry[port].write(io_registry[port].private, localPort, value);
    }
}
