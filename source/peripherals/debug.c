/*******************************************************************************
 * NewOswan
 * debug.c: 
 *
 * Created by Manoël Trapier on 19/12/2021.
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>

#include <io.h>

#include <peripherals/debug.h>

typedef struct debug_pdata_t
{
    int16_t value;
    char character;
} debug_pdata_t;

static debug_pdata_t debug_data;

static uint8_t debug_io_read(void *pdata, uint8_t port)
{
    debug_pdata_t *params = (debug_pdata_t *)pdata;

    switch (port)
    {
    case 0x0: return params->value & 0xFF;
    case 0x1: return params->value >> 8;
    case 0x2: return params->character;
    }

    return 0x90;
}

static void debug_io_write(void *pdata, uint8_t port, uint8_t value)
{
    debug_pdata_t *params = (debug_pdata_t *)pdata;

    switch (port)
    {
    case 0x0:
        params->value = (params->value & 0xFF00) | value;
        break;

    case 0x1:
        params->value = (params->value & 0x00FF) | (value << 8);
        printf("%d\n", params->value);
        break;

    case 0x2:
        params->character = value;
        printf("%c", value);
        fflush(stdout);
        break;
    }
}

static void debug_init(uint8_t baseAddress, void *params)
{
    io_register_hook(baseAddress, 0x0, &debug_data, debug_io_read, debug_io_write);
    io_register_hook(baseAddress, 0x1, &debug_data, debug_io_read, debug_io_write);
    io_register_hook(baseAddress, 0x2, &debug_data, debug_io_read, debug_io_write);
}

/* Exported device */
device_t InterruptController =
{
    .init = debug_init,
    .update = NULL,
    .reset = NULL,
    .free = NULL,
    .type = DT_DEBUG,
};