///////////////////////////////////////////////////////////////////////////////
// Wonderswan emulator
////////////////////////////////////////////////////////////////////////////////
//
// 13.04.2002: Fixed a small bug causing crashes
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include "source/log.h"
#include "source/rom.h"
#include "source/nec/nec.h"
#include "source/memory.h"
#include "source/gpu.h"
#include "source/io.h"
#include "source/ws.h"
#include "source/emulate.h"
#include "source/audio.h"

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
#define		LOG_PATH "oswan.log"


int			gui_command=GUI_COMMAND_NONE;
int			gui_mainDialogRunning;
int			gui_controls_configuration_Running;
int			gui_get_key_Running;
int			gui_get_key_key;

int 			ws_videoEnhancementType=0;
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

    if (!log_init(LOG_PATH))
    {
        printf("Warning: cannot open log file %s\n", LOG_PATH);
    }

    snprintf(app_window_title, 255, "Oswan %s - Esc to return to GUI", VERSION);

    fprintf(log_get(), "NewOswan %s (built at: %s %s)\n", VERSION, __DATE__, __TIME__);

   ws_rom_path = NULL;

   for (int n = 1; n < argc; ++n)
   {
      if (argv[n][0] == '-')
      {
         switch(argv[n][1])
         {
         case 'C':
            if (++n < argc)
            {
               ws_cyclesByLine = atoi(argv[n]);
            }

            fprintf(log_get(), "Cycles by line set to %d\n", ws_cyclesByLine);
            break;

         case 'w':
            if (++n < argc)
            {
               ws_system = atoi(argv[n]);
            }

            fprintf(log_get(), "WonderSwan set to %d\n", ws_system);
            break;

         case 's':
            if (++n < argc)
            {
               ws_sram_path = argv[n];
            }

            sram_path_explicit = 1;
            break;

         default:
            break;
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
         app_gameRunning=0;
         exit(0);
      }

      if (ws_rom_path)
      {
         ws_set_system(ws_system);
         if (ws_init(ws_rom_path))
         {
            app_rotated=ws_rotated();
            app_gameRunning=1;

            }

            ws_reset();

            ws_emulate();
         }

         ws_done();
      }
   }

   log_done();
   return(0);
}

