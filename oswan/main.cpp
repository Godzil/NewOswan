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

/*#include <windows.h>
#include <commctrl.h>

#include <direct.h>*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
//#include "resource.h"
#include "SDL.h"
#include "source/types.h"
#include "source/SDLptc.h"
#include "source/log.h"
#include "source/rom.h"
#include "source/nec/nec.h"
#include "source/memory.h"
#include "source/gpu.h"
#include "source/io.h"
#include "source/ws.h"
#include "source/ticker.h"
#include "source/2xSaI.h"
#include "source/audio.h"

#undef DWORD
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
#define		LOG_PATH "oswan.log"

#define		KEY_ENTER	0x0D
#define		KEY_SPACE	0x20
#define		KEY_ESC		0x1b
#define		KEY_UP		0x26
#define		KEY_DOWN	0x28
#define		KEY_LEFT	0x25
#define		KEY_RIGHT	0x27
#define		KEY_BUTTON1 0x57
#define		KEY_BUTTON2 0x58

#define		GUI_COMMAND_NONE				0
#define		GUI_COMMAND_RESET				1
#define		GUI_COMMAND_SCHEME_CHANGE		2
#define		GUI_COMMAND_FILTER_CHANGE		3

char		app_window_title[256];
int			app_gameRunning=0;
int			app_terminate=0;
int			app_fullscreen=0;
SDL_Event	app_input_event;
int			app_rotated=0;

int			gui_command=GUI_COMMAND_NONE;
int			gui_mainDialogRunning;
int			gui_controls_configuration_Running;
int			gui_get_key_Running;
int			gui_get_key_key;
SDL_Joystick *joystick=NULL;

int			ws_videoEnhancementType=0;
int			ws_colourScheme=COLOUR_SCHEME_DEFAULT;
int			ws_system=WS_SYSTEM_COLOR;
char		*ws_rom_path;
extern char		*ws_sram_path;
int sram_path_explicit = 0;
char		old_rom_path[4096];

#if 0
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
char *gui_getRomPath(HWND hWnd)
{
   static OPENFILENAME ofn;				// common dialog box structure
   static char			szFile[260];		// buffer for file name
   HWND				hwnd;				// owner window
   HANDLE				hf;					// file handle
   static char			oldDir[1024];

   InitCommonControls();
   _getcwd(oldDir,1024);
   szFile[0]=0;
   // Initialize OPENFILENAME
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hwndOwner = NULL;
   ofn.lpstrFile = szFile;
   ofn.nMaxFile = sizeof(szFile);
   ofn.lpstrFilter = "Wonderswan mono\0*.ws\0Wonderswan color\0*.wsc\0\0";
   ofn.nFilterIndex = 1;
   ofn.lpstrFileTitle = NULL;
   ofn.nMaxFileTitle = 0;
   ofn.lpstrInitialDir = NULL;
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
   ofn.hInstance=NULL;
   // Display the Open dialog box.

   if (GetOpenFileName(&ofn)==TRUE)
   {
      return(ofn.lpstrFile);
   }

   chdir(oldDir);
   return(NULL);
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
char *gui_loadState(HWND hWnd)
{
   static OPENFILENAME ofn;				// common dialog box structure
   static char			szFile[260];		// buffer for file name
   HWND				hwnd;				// owner window
   HANDLE				hf;					// file handle
   static char			oldDir[1024];

   InitCommonControls();
   _getcwd(oldDir,1024);
   szFile[0]=0;
   // Initialize OPENFILENAME
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hwndOwner = NULL;
   ofn.lpstrFile = szFile;
   ofn.nMaxFile = sizeof(szFile);
   ofn.lpstrFilter = "Wonderswan state\0*.wss\0";
   ofn.nFilterIndex = 1;
   ofn.lpstrFileTitle = NULL;
   ofn.nMaxFileTitle = 0;
   ofn.lpstrInitialDir = NULL;
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
   ofn.hInstance=NULL;
   // Display the Open dialog box.

   if (GetOpenFileName(&ofn)==TRUE)
   {
      return(ofn.lpstrFile);
   }

   chdir(oldDir);
   return(NULL);
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
char *gui_saveState(HWND hWnd)
{
   static OPENFILENAME ofn;				// common dialog box structure
   static char			szFile[260];		// buffer for file name
   HWND				hwnd;				// owner window
   HANDLE				hf;					// file handle
   static char			oldDir[1024];

   InitCommonControls();
   _getcwd(oldDir,1024);
   szFile[0]=0;
   // Initialize OPENFILENAME
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hwndOwner = NULL;
   ofn.lpstrFile = szFile;
   ofn.nMaxFile = sizeof(szFile);
   ofn.lpstrFilter = "Wonderswan state\0*.wss\0";
   ofn.nFilterIndex = 1;
   ofn.lpstrFileTitle = NULL;
   ofn.nMaxFileTitle = 0;
   ofn.lpstrInitialDir = NULL;
   ofn.Flags = 0;
   ofn.hInstance=NULL;
   // Display the Open dialog box.

   if (GetSaveFileName(&ofn)==TRUE)
   {
      return(ofn.lpstrFile);
   }

   chdir(oldDir);
   return(NULL);
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
BOOL CALLBACK GuiMainDialogProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
   static BOOL bButton = FALSE;


   switch(message)
   {
   case WM_INITDIALOG:
      break;

   case WM_CLOSE:
   {
      gui_mainDialogRunning=0;
      DestroyWindow(hWnd);
      return 1;
   }

   case WM_COMMAND:
   {

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_BUTTON_LOAD))
      {
         ws_rom_path=gui_getRomPath(hWnd);

         if (ws_rom_path)
         {
            SendMessage(hWnd, WM_CLOSE, 0,0);
         }

         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_BUTTON_SAVESTATE))
      {
         char *path=gui_saveState(hWnd);

         if (path)
         {
            int err=ws_saveState(path);

            if (err==0)
            {
               MessageBox(hWnd,"State cannot be saved","Error",MB_OK);
            }
            else
            {
               SendMessage(hWnd, WM_CLOSE, 0,0);
            }
         }

         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_BUTTON_LOADSTATE))
      {
         char *path=gui_loadState(hWnd);

         if (path)
         {
            int err=ws_loadState(path);

            if (err==0)
            {
               MessageBox(hWnd,"State cannot be loaded","Error",MB_OK);
            }
            else if (err==-1)
            {
               MessageBox(hWnd,"Please load the correct rom first","Error",MB_OK);
            }
            else
            {
               SendMessage(hWnd, WM_CLOSE, 0,0);
            }
         }

         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_BUTTON_RESET))
      {
         gui_command=GUI_COMMAND_RESET;
         SendMessage(hWnd, WM_CLOSE, 0,0);
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_BUTTON_CONTINUE))
      {
         SendMessage(hWnd, WM_CLOSE, 0,0);
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_BUTTON_EXIT))
      {
         SendMessage(hWnd, WM_CLOSE, 0,0);
         app_terminate=1;
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_RADIO_STANDARD_MODE))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_STANDARD_MODE),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_videoEnhancementType=0;
         }

         gui_command=GUI_COMMAND_FILTER_CHANGE;
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_RADIO_DOUBLESIZE_MODE))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_DOUBLESIZE_MODE),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_videoEnhancementType=1;
         }

         gui_command=GUI_COMMAND_FILTER_CHANGE;
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_RADIO_SCANLINES_MODE))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_SCANLINES_MODE),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_videoEnhancementType=2;
         }

         gui_command=GUI_COMMAND_FILTER_CHANGE;
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_RADIO_50PRCTSCANLINES_MODE))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_50PRCTSCANLINES_MODE),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_videoEnhancementType=3;
         }

         gui_command=GUI_COMMAND_FILTER_CHANGE;
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_RADIO_SPECIAL_MODE))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_SPECIAL_MODE),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_videoEnhancementType=4;
         }

         gui_command=GUI_COMMAND_FILTER_CHANGE;
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_RADIO_2XSAI))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_2XSAI),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_videoEnhancementType=5;
         }

         gui_command=GUI_COMMAND_FILTER_CHANGE;
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_RADIO_SUPER2XSAI))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_SUPER2XSAI),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_videoEnhancementType=6;
         }

         gui_command=GUI_COMMAND_FILTER_CHANGE;
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_RADIO_SUPEREAGLE))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_SUPEREAGLE),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_videoEnhancementType=7;
         }

         gui_command=GUI_COMMAND_FILTER_CHANGE;
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_RADIO_FULLSCREEN))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_FULLSCREEN),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            app_fullscreen=1;
         }

         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_RADIO_WINDOWED))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_WINDOWED),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            app_fullscreen=0;
         }

         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_COLOUR_DEFAULT))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_COLOUR_DEFAULT),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_colourScheme=COLOUR_SCHEME_DEFAULT;
         }

         gui_command=GUI_COMMAND_SCHEME_CHANGE;
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_COLOUR_AMBER))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_COLOUR_AMBER),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_colourScheme=COLOUR_SCHEME_AMBER;
         }

         gui_command=GUI_COMMAND_SCHEME_CHANGE;
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_COLOUR_GREEN))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_COLOUR_GREEN),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_colourScheme=COLOUR_SCHEME_GREEN;
         }

         gui_command=GUI_COMMAND_SCHEME_CHANGE;
         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_RADIO_SYSTEM_AUTODETECT))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_SYSTEM_AUTODETECT),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_system=WS_SYSTEM_AUTODETECT;
         }

         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_RADIO_SYSTEM_COLOR))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_SYSTEM_COLOR),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_system=WS_SYSTEM_COLOR;
         }

         return 1;
      }

      if ((HIWORD(wParam)==BN_CLICKED)&&(LOWORD(wParam)==IDC_RADIO_SYSTEM_MONO))
      {
         if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_SYSTEM_MONO),BM_GETCHECK,0,0)==BST_CHECKED)
         {
            ws_system=WS_SYSTEM_MONO;
         }

         return 1;
      }
   }
   }

   if(message == WM_INITDIALOG)
   {
      return(TRUE);
   }

   return (FALSE);
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
void gui_open(void)
{
   HWND	hwnd, hCtrl;
   MSG		msg;

   gui_command=GUI_COMMAND_NONE;
   InitCommonControls();
   ws_rom_path=NULL;
   hwnd		= CreateDialog(gui_hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, (DLGPROC)GuiMainDialogProc);

   if (hwnd==NULL)
   {
      MessageBox(NULL,"Cannot create gui","Error",MB_OK);
      return;
   }

   if (ws_colourScheme==COLOUR_SCHEME_DEFAULT)
   {
      SendMessage(GetDlgItem(hwnd, IDC_COLOUR_DEFAULT),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }
   else if (ws_colourScheme==COLOUR_SCHEME_AMBER)
   {
      SendMessage(GetDlgItem(hwnd, IDC_COLOUR_AMBER),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }
   else if (ws_colourScheme==COLOUR_SCHEME_GREEN)
   {
      SendMessage(GetDlgItem(hwnd, IDC_COLOUR_GREEN),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }

   if (ws_videoEnhancementType==0)
   {
      SendMessage(GetDlgItem(hwnd, IDC_RADIO_STANDARD_MODE),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }
   else if (ws_videoEnhancementType==1)
   {
      SendMessage(GetDlgItem(hwnd, IDC_RADIO_DOUBLESIZE_MODE),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }
   else if (ws_videoEnhancementType==2)
   {
      SendMessage(GetDlgItem(hwnd, IDC_RADIO_SCANLINES_MODE),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }
   else if (ws_videoEnhancementType==3)
   {
      SendMessage(GetDlgItem(hwnd, IDC_RADIO_50PRCTSCANLINES_MODE),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }
   else if (ws_videoEnhancementType==4)
   {
      SendMessage(GetDlgItem(hwnd, IDC_RADIO_SPECIAL_MODE),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }
   else if (ws_videoEnhancementType==5)
   {
      SendMessage(GetDlgItem(hwnd, IDC_RADIO_2XSAI),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }
   else if (ws_videoEnhancementType==6)
   {
      SendMessage(GetDlgItem(hwnd, IDC_RADIO_SUPER2XSAI),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }
   else if (ws_videoEnhancementType==7)
   {
      SendMessage(GetDlgItem(hwnd, IDC_RADIO_SUPEREAGLE),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }

   if (app_fullscreen)
   {
      SendMessage(GetDlgItem(hwnd, IDC_RADIO_FULLSCREEN),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }
   else
   {
      SendMessage(GetDlgItem(hwnd, IDC_RADIO_WINDOWED),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }

   if (ws_system==WS_SYSTEM_AUTODETECT)
   {
      SendMessage(GetDlgItem(hwnd, IDC_RADIO_SYSTEM_AUTODETECT),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }
   else if (ws_system==WS_SYSTEM_COLOR)
   {
      SendMessage(GetDlgItem(hwnd, IDC_RADIO_SYSTEM_COLOR),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }
   else if (ws_system==WS_SYSTEM_MONO)
   {
      SendMessage(GetDlgItem(hwnd, IDC_RADIO_SYSTEM_MONO),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
   }

   gui_mainDialogRunning=1;
   ShowWindow(hwnd, SW_SHOWDEFAULT);
   UpdateWindow(hwnd);

   while(gui_mainDialogRunning)
   {
      if (GetMessage(&msg, hwnd, 0, 0))
      {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }

   DestroyWindow(hwnd);
}

#endif

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



////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
#include "./source/filters/standard.h"
#include "./source/filters/doubled.h"
#include "./source/filters/scanlines.h"
#include "./source/filters/halfscanlines.h"
#include "./source/filters/2xsai.h"
#include "./source/filters/super2xsai.h"
#include "./source/filters/supereagle.h"
#include "./source/filters/special.h"


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
   atexit(SDL_Quit);
   SDL_Init(SDL_INIT_TIMER);

   if (!log_init(LOG_PATH))
   {
      printf("Warning: cannot open log file %s\n",LOG_PATH);
   }

   snprintf(app_window_title, 255, "Oswan %s - Esc to return to GUI", VERSION);

   fprintf(log_get(),"Oswan-unix %s (built at: %s %s)\n",VERSION , __DATE__,__TIME__);

   ws_videoEnhancementType=1;
   //ws_system = WS_SYSTEM_AUTODETECT;
   ws_system = WS_SYSTEM_COLOR;

   Init_2xSaI(555);
   ws_rom_path = NULL;
   /*gui_hInstance=hInstance;
   gui_open();*/

   // the hard-core UI, a command line:
   for (int n = 1; n < argc; ++n)
   {
      if (argv[n][0] == '-')
      {
         switch(argv[n][1])
         {
         case 'E':
            if (++n < argc)
            {
               ws_videoEnhancementType = atoi(argv[n]);
            }
            else
            {
               fprintf(stderr, "\n0 - plain\n1 - doubled\n2 - scanlines\n3 - 1/2 bright scanlines\n4 - \"special\"\n5 - 2xSaI\n6 - Super2xSaI (default)\n7 - SuperEagle\n\n");
               exit(1);
            }

            break;

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
      }
   }

   while (!app_terminate)
   {
      if (!ws_rom_path)
      {
         app_gameRunning=0;
         //gui_open();
         exit(0); // nothing to wait for at the moment...
      }

      if (ws_rom_path)
      {
         ws_set_system(ws_system);
         if (ws_init(ws_rom_path))
         {
            app_rotated=ws_rotated();
            app_gameRunning=1;

            if (ws_system == WS_SYSTEM_COLOR)
            {
               ws_gpu_operatingInColor=1;
            }

            ws_set_colour_scheme(ws_colourScheme);
            ws_buildHalfBrightnessTable();
            ws_reset();

            switch (ws_videoEnhancementType)
            {
            case 0:
               ws_emulate_standard();
               break;

            case 1:
               ws_emulate_doubled();
               break;

            case 2:
               ws_emulate_scanlines();
               break;

            case 3:
               ws_emulate_halfBrightnessScanlines();
               break;

            case 4:
               ws_emulate_special();
               break;

            case 5:
               ws_emulate_2xSaI();
               break;

            case 6:
               ws_emulate_Super2xSaI();
               break;

            case 7:
               ws_emulate_SuperEagle();
               break;
            }
         }

         ws_done();
      }
   }

   log_done();
   return(0);
}

