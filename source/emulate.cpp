////////////////////////////////////////////////////////////////////////////////
// Main emulation loop
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <sys/mman.h>

#include "log.h"
#include "io.h"
#include "ws.h"
#include "rom.h"
#include "./nec/nec.h"
#include "./nec/necintrf.h"
#include "gpu.h"
#include "audio.h"
#include "memory.h"

char        app_window_title[256];
int         app_gameRunning=0;
int         app_terminate=0;
int         app_fullscreen=0;
int         app_rotated=0;


int ws_key_esc = 0;


static void read_keys()
{
#if 0
    while ( SDL_PollEvent(&app_input_event) )
    {
       if ( app_input_event.type == SDL_QUIT )
       {
          ws_key_esc = 1;
       }
    }

#endif
    ws_key_start=0;
    ws_key_x4=0;
    ws_key_x2=0;
    ws_key_x1=0;
    ws_key_x3=0;
    ws_key_y4=0;
    ws_key_y2=0;
    ws_key_y1=0;
    ws_key_y3=0;
    ws_key_button_a=0;
    ws_key_button_b=0;

#if 0
    uint8_t *keystate = SDL_GetKeyState(NULL);

    if ( keystate[SDLK_e])
    {
       dump_memory();
    }

    if ( keystate[SDLK_r])
    {
       printf("Boop\n");
       ws_reset();
    }

    if ( keystate[SDLK_ESCAPE] )
    {
       ws_key_esc = 1;
    }

    if ( keystate[SDLK_UP] )
    {
       ws_key_x1=1;
    }

    if ( keystate[SDLK_DOWN] )
    {
       ws_key_x3=1;
    }

    if ( keystate[SDLK_RIGHT] )
    {
       ws_key_x2=1;
    }

    if ( keystate[SDLK_LEFT] )
    {
       ws_key_x4=1;
    }

    if (keystate[SDLK_RETURN])
    {
       ws_key_start=1;
    }

    if (keystate[SDLK_c])
    {
       ws_key_button_a=1;
    }

    if (keystate[SDLK_x])
    {
       ws_key_button_b=1;
    }

    if (keystate[SDLK_w])
    {
       ws_key_y1=1;
    }

    if (keystate[SDLK_a])
    {
       ws_key_y4=1;
    }

    if (keystate[SDLK_s])
    {
       ws_key_y3=1;
    }

    if (keystate[SDLK_d])
    {
       ws_key_y2=1;
    }

    if (keystate[SDLK_o])
    {
       ws_cyclesByLine+=10;
    }

    if (keystate[SDLK_l])
    {
       ws_cyclesByLine-=10;
    }
#endif
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
void ws_emulate(void)
{
#if 0
    int32_t nCount = 0;
    int i = 0;

    double dTime = 0.0, dNormalLast = 0.0, dTemp;
    int32_t surfacePitch;

// 15 bits RGB555
    //Format format(16, 0x007c00, 0x00003e0, 0x0000001f);
    //Surface *surface;
    if (app_rotated)
    {
        surface = new Surface(144 * 2, 224 * 2, format);
        int16_t *backbuffer = (int16_t *)malloc(224 * 144 * sizeof(int16_t));
        memset(backbuffer, 0x00, 224 * 144 * sizeof(int16_t));
        surfacePitch = (surface->pitch() >> 1);

        dNormalLast = (double)SDL_GetTicks();

        console.open(app_window_title, 144 * 2, 224 * 2, format);

        while (1)
        {

            dTemp = (double)SDL_GetTicks();
            dTime = dTemp - dNormalLast;

            nCount = (int32_t)(dTime * 0.07547); // does this calculation make sense?

            if (nCount <= 0)
            {
                SDL_Delay(2);
            } // No need to do anything for a bit
            else
            {

                dNormalLast += nCount * (1 / 0.07547);

                if (nCount > 10)
                {
                    nCount = 10;
                }

                read_keys();

                if (ws_key_esc)
                {
                    console.close();

                    app_terminate = 1;

                    if ((ws_rom_path != NULL) || (app_terminate))
                    {
                        break;
                    }

                    console.open(app_window_title, 144 * 2, 224 * 2, format);

                }


                for (i = 0 ; i < nCount - 1 ; i++)
                {
                    while (!ws_executeLine(backbuffer, 0))
                    {
                    }
                }

                while (!ws_executeLine(backbuffer, 1))
                {
                }


                ws_rotate_backbuffer(backbuffer);

                int16_t *vs = (int16_t *)surface->lock();
                int16_t *backbuffer_alias = backbuffer;

                for (int line = 0 ; line < 224 ; line++)
                {
                    ws_drawDoubledRotatedScanline(vs, backbuffer_alias);
                    vs += surfacePitch;
                    ws_drawDoubledRotatedScanline(vs, backbuffer_alias);
                    vs += surfacePitch;
                    backbuffer_alias += 144;
                }

                surface->unlock();
                surface->copy(console);
                console.update();
            }
        }

        console.close();
        delete surface;

    }
    else
    {
        surface = new Surface(224 * 2, 144 * 2, format);
        int16_t *backbuffer = (int16_t *)malloc(224 * 144 * sizeof(int16_t));
        memset(backbuffer, 0x00, 224 * 144 * sizeof(int16_t));
        surfacePitch = (surface->pitch() >> 1);

        dNormalLast = (double)SDL_GetTicks();

        console.open(app_window_title, 224 * 2, 144 * 2, format);

        while (1)
        {

            dTemp = (double)SDL_GetTicks();
            dTime = dTemp - dNormalLast;


            nCount = (int32_t)(dTime * 0.07547); // does this calculation make sense?

            if (nCount <= 0)
            {
                SDL_Delay(2);
            } // No need to do anything for a bit
            else
            {
                
                dNormalLast += nCount * (1 / 0.07547);

                if (nCount > 10)
                {
                    nCount = 10;
                }

                read_keys();

                if (ws_key_esc)
                {
                    console.close();

                    app_terminate = 1;

                    if ((ws_rom_path != NULL) || (app_terminate))
                    {
                        break;
                    }

                    console.open(app_window_title, 224 * 2, 144 * 2, format);

                }


                for (i = 0 ; i < nCount - 1 ; i++)
                {
                    while (!ws_executeLine(backbuffer, 0))
                    {
                    }
                }

                while (!ws_executeLine(backbuffer, 1))
                {
                }

                int16_t *vs = (int16_t *)surface->lock();
                int16_t *backbuffer_alias = backbuffer;

                for (int line = 0 ; line < 144 ; line++)
                {
                    ws_drawDoubledScanline(vs, backbuffer_alias);
                    vs += surfacePitch;
                    ws_drawDoubledScanline(vs, backbuffer_alias);
                    vs += surfacePitch;
                    backbuffer_alias += 224;
                }

                surface->unlock();
                surface->copy(console);
                console.update();
            }
        }

        console.close();
        delete surface;
    }
#endif
}
