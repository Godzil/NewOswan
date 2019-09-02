			//nTime=SDL_GetTicks()-nNormalLast;					  // calcule le temps écoulé depuis le dernier affichage
			dTemp = (double) SDL_GetTicks();
			dTime = dTemp - dNormalLast;

																  // nTime est en mili-secondes.
			// détermine le nombre de trames à passer + 1
			//nCount = (Sint32) ((((double)nTime)*600.0 - (double)nNormalFrac) / 10000.0);
			nCount = (Sint32) (dTime * 0.07547); // does this calculation make sense? 
			// 75.47Hz vblank is according to wstech22.txt
			//printf("%d ", nCount);

			// si le nombre de trames à passer + 1 est nul ou négatif,
			// ne rien faire pendant 2 ms
			//AUpdateAudio();
			if (nCount<=0) 
			{ 
				SDL_Delay(2);
			} // No need to do anything for a bit
			else
			{
				//nNormalFrac+=nCount*10000;				// 
				//nNormalLast+=nNormalFrac/600;				// add the duration of nNormalFrac frames
				//nNormalFrac%=600;							// 
				//dNormalLast = dTemp;
				dNormalLast += nCount * (1/0.07547);

				// Pas plus de 9 (10-1) trames non affichées 
				if (nCount>10) 
				  nCount=10; 

/*
				ws_key_start=0;
				ws_key_left=0;
				ws_key_right=0;
				ws_key_up=0;
				ws_key_down=0;
				ws_key_button_1=0;
				ws_key_button_2=0;
*/
				int ws_key_esc=0;

				#include "source/temp/key.h"
				if (ws_key_esc)
				{
					console.close();
					if (ws_rom_path)
						strcpy(old_rom_path,ws_rom_path);
					//gui_open();
#ifndef GUI_OPEN_WARNED
//#warning XXX something ought to take place here...
#define GUI_OPEN_WARNED
#endif
					app_terminate = 1;

					if ((ws_rom_path!=NULL)||(app_terminate))
						break;
					if (gui_command)
					{
						if (gui_command==GUI_COMMAND_RESET)
							ws_reset();
						if (gui_command==GUI_COMMAND_SCHEME_CHANGE)
							ws_set_colour_scheme(ws_colourScheme);
						if (gui_command==GUI_COMMAND_FILTER_CHANGE)
						{
							ws_saveState("oswan.wss");
							ws_rom_path=old_rom_path;
							delete surface;
							return;
						}
					}
					console.option("DirectX");
					if (app_fullscreen) 
						console.option("fullscreen output"); 
					else 
						console.option("windowed output");
					console.option("fixed window");
					console.option("center window");
