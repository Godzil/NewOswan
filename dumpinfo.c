/*******************************************************************************
 * NewOswan
 * dumpinfo.c: Tool to dump the metadata info about a cart rom image.
 *
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>

#include <wsrom.h>


int main(int argc, char *argv[])
{
    int ret = -1;
    wsrom_game_t *rom;

    if (argc != 2)
    {
        printf("Usage: %s file.ws[c]\n", argv[0]);
    }
    else
    {
        rom = wsrom_loadRom(argv[1]);
        if (rom)
        {
            wsrom_dumpInfo(rom);
            ret = 0;
        }
    }

    return ret;
}
