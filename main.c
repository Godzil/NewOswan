/******************************************************************************
 * NewOswan
 * main.c: Entry point
 *
 * Based on the original Oswan-unix
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <log.h>
#include <wsrom.h>
#include <nec.h>
#include <memory.h>
#include <gpu.h>
#include <io.h>
#include <ws.h>
#include <emulate.h>
#include <audio.h>

int main(int argc, char *argv[])
{
    if (ws_init(argv[0]))
    {
        //ws_reset();
        //ws_emulate();
    }

    return (0);
}
