/******************************************************************************
 * NewOswan
 * main.c: Entry point
 * Based on the original Oswan-unix
 * Copyright (c) 2014-2021 986-Studio. All rights reserved.
 *
 ******************************************************************************/

////////////////////////////////////////////////////////////////////////////////
//
// 13.04.2002: Fixed a small bug causing crashes
//
//
//
//
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <log.h>
#include <rom.h>
#include <nec.h>
#include <memory.h>
#include <gpu.h>
#include <io.h>
#include <ws.h>
#include <emulate.h>
#include <audio.h>

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
#define        LOG_PATH "oswan.log"

int sram_path_explicit = 0;
int ieep_path_explicit = 0;

int ws_mk_savpath()
{
    char *w;

    if (sram_path_explicit)
    {
        return 0;
    }

    if (ws_sram_path != NULL)
    {
        free(ws_sram_path);
    }

    ws_sram_path = (char *)malloc(strlen(ws_rom_path) + 2);
    strcpy(ws_sram_path, ws_rom_path);
    w = strrchr(ws_sram_path, '.');

    if (NULL == w)
    {
        strcpy(ws_sram_path, "error.sav");
        return 1;
    }

    strcpy(w, ".sav");
    return 0;
}

int ws_mk_ieppath()
{
    char *w;

    if (ieep_path_explicit)
    {
        return 0;
    }

    if (ws_ieep_path != NULL)
    {
        free(ws_ieep_path);
    }

    ws_ieep_path = (char *)malloc(strlen(ws_rom_path) + 2);
    strcpy(ws_ieep_path, ws_rom_path);
    w = strrchr(ws_ieep_path, '.');

    if (NULL == w)
    {
        strcpy(ws_ieep_path, "error.iep");
        return 1;
    }

    strcpy(w, ".iep");
    return 0;
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

int main(int argc, char *argv[])
{
    wssystem_t ws_system = WS_SYSTEM_AUTODETECT;

    snprintf(app_window_title, 255, "Oswan %s - Esc to return to GUI", VERSION);

    Log(TLOG_ALWAYS, NULL, "NewOswan %s (built at: %s %s)", VERSION, __DATE__, __TIME__);

    ws_rom_path = NULL;

    for (int n = 1 ; n < argc ; ++n)
    {
        if (argv[n][0] == '-')
        {
            switch (argv[n][1])
            {
            case 'C':
                if (++n < argc)
                {
                    ws_cyclesByLine = atoi(argv[n]);
                }

                Log(TLOG_ALWAYS, "main", "Cycles by line set to %d", ws_cyclesByLine);
                break;

            case 'w':
                if (++n < argc)
                {
                    ws_system = atoi(argv[n]);
                }

                Log(TLOG_ALWAYS, "main", "WonderSwan set to %d", ws_system);
                break;

            case 's':
                if (++n < argc)
                {
                    ws_sram_path = argv[n];
                }

                sram_path_explicit = 1;
                break;

            default:break;
            }
        }
        else
        {
            ws_rom_path = argv[n];
            ws_mk_savpath();
            ws_mk_ieppath();
        }
    }

    while (!app_terminate)
    {
        if (!ws_rom_path)
        {
            exit(0);
        }

        if (ws_rom_path)
        {
            ws_set_system(ws_system);
            if (ws_init(ws_rom_path))
            {
                ws_reset();
                ws_emulate();
            }

            ws_done();
        }
    }

    return (0);
}

