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

extern uint8_t *ws_ioRam;

uint8_t debug_io_read(void *pdata, uint8_t port)
{
    switch (port)
    {

    }
}

void debug_io_write(void *pdata, uint8_t port, uint8_t value)
{
    switch (port)
    {
    case 0xF1:
        printf("%d\n", (signed short)((value << 8) | ws_ioRam[0xF0]));
        break;

    case 0xF2:
        printf("%c", value);
        fflush(stdout);
        break;
    }
}