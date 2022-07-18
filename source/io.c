/*******************************************************************************
 * NewOswan
 * io.c: I/O ports implementaton
 *
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <log.h>
#include <io.h>

//#define IO_DUMP

typedef struct ioregistry_t
{
    io_read read;         /***< Read function for a specific IO port */
    io_write write;       /***< Write function for a specific IO port */
    void *private;        /***< Private data for the peripheral if needed. */
    uint8_t base_address; /***< Base IO address. Used create a "local" value when calling the read/write function */
} ioregistry_t;
ioregistry_t io_registry[0x100];

// TODO: Find a better way
extern uint64_t nec_monotonicCycles;

FILE *ioLogFp = NULL;

void io_init(void)
{
#ifdef IO_DUMP
    ioLogFp = fopen("iodump.csv", "wt");
#endif
}

void io_cleanup(void)
{
#ifdef IO_DUMP
    fclose(ioLogFp);
#endif
}

void io_register_hook(uint8_t baseAddress, uint8_t port, void *pdata, io_read readHook, io_write writeHook)
{
    io_registry[baseAddress + port].base_address = baseAddress;
    io_registry[baseAddress + port].read = readHook;
    io_registry[baseAddress + port].write = writeHook;
    io_registry[baseAddress + port].private = pdata;
}

void
io_register_hook_array(uint8_t baseAddress, const uint8_t *portList, uint8_t listLen, void *pdata, io_read readHook,
                       io_write writeHook)
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
    else
    {
        Log(TLOG_DEBUG, "IO", "CPU tried to read IO %02Xh - May be a bug", port);
    }

#ifdef IO_DUMP
    if (ioLogFp)
    {
        fprintf(ioLogFp, "%lu, R, %02X, %02X\n", nec_monotonicCycles, port, retVal);
    }
#endif

    return retVal;
}

void io_writeport(uint8_t port, uint8_t value)
{
    if (io_registry[port].write)
    {
        uint8_t localPort = port - io_registry[port].base_address;
        io_registry[port].write(io_registry[port].private, localPort, value);
    }
    else
    {
        Log(TLOG_DEBUG, "IO", "CPU tried to write %02Xh to IO %02Xh - MAy be a bug", value, port);
    }

#ifdef IO_DUMP
    if (ioLogFp)
    {
        fprintf(ioLogFp, "%llu, W, %02X, %02X\n", nec_monotonicCycles, port, value);
    }
#endif
}
