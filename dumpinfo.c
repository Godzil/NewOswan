/*******************************************************************************
 * NewOswan
 * dumpinfo.c: Tool to dump the metadata info about a cart rom image.
 *
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <wsrom.h>


int main(int argc, char *argv[])
{
    int ret = -1;
    char *filepath;
    int n;
    wsrom_game_t *rom;
    bool jsonOutput = false;

    for (n = 1 ; n < argc ; n++)
    {
        if (argv[n][0] == '-')
        {
            switch (argv[n][1])
            {
            case 'j':
                jsonOutput = true;
                break;

            default:
                fprintf(stderr, "Unknown option: %s\n", argv[n]);

            case 'h':
                fprintf(stderr, "Usage: %s [-j] file.ws(c)\n", argv[0]);
                goto exit;
                break;
            }
        }
        else
        {
            break;
        }
    }

    ret = 0;

    if (jsonOutput)
    {
        printf("{\n");
    }

    for(; n < argc ; n++)
    {
        rom = wsrom_loadRom(argv[n]);
        if (jsonOutput)
        {
            printf("\"%s\": ", argv[n]);
            if ( rom )
            {
                wsrom_jsonSerialise(stdout, rom);
            }
            else
            {
                printf("\"not valid\"");
            }
            if ( n < ( argc - 1 ))
            {
                printf(",");
            }
            printf("\n");
        }
        else
        {
            if ( rom )
            {
                wsrom_dumpInfo(rom);
            }
        }

        if ( rom )
        {
            wsrom_unloadRom(rom);
        }
    }

    if (jsonOutput)
    {
        printf("}\n");
    }
exit:
    return ret;
}
