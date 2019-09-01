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
#define M_R(A) ((A&0x7c00)>>10)
#define M_G(A) ((A&0x03e0)>>5)
#define M_B(A) (A&0x1f)
#define NB_BUFFERS		8
#define NB_BUFFERS_DIV	3
void ws_mergeBackbuffers(int16 **buffers)
{
	int16	*buffersAlias[NB_BUFFERS+1];

	memcpy(buffersAlias,buffers,sizeof(int16*)*(NB_BUFFERS+1));

	for (int i=0;i<224*144;i++)
	{
		int r,g,b;
		r=g=b=0;
		for (int j=0;j<NB_BUFFERS;j++)
		{
			r+=M_R(*buffersAlias[j]);
			g+=M_G(*buffersAlias[j]);
			b+=M_B(*buffersAlias[j]);
			buffersAlias[j]++;
		}
		r>>=NB_BUFFERS_DIV;
		g>>=NB_BUFFERS_DIV;
		b>>=NB_BUFFERS_DIV;
		*buffersAlias[NB_BUFFERS]++=(r<<10)|(g<<5)|b;
	}
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
void ws_emulate_standard_interpolate(void)
{
	#define KEY_ENTER	0x0D
	#define KEY_ESC		0x1b
	#define KEY_UP		0x26
	#define KEY_DOWN	0x28
	#define KEY_LEFT	0x25
	#define KEY_RIGHT	0x27
	#define KEY_BUTTON1 0x57
	#define KEY_BUTTON2 0x58

	uint32		 startTime, endTime, totalFrames;
	unsigned int nNormalLast=0;
	int			 nNormalFrac=0; 
	int			 nTime=0,nCount=0; int i=0;

	// 15 bits RGB555
	Format format(16,0x007c00,0x00003e0,0x0000001f);
	Console console;
	Surface *surface;
	if (app_rotated)
		surface=new Surface(144,224,format);
	else
		surface=new Surface(224,144,format);

	console.option("DirectX");
	if (app_fullscreen) 
		console.option("fullscreen output"); 
	else 
		console.option("windowed output");
	console.option("fixed window");
	console.option("center window");

	console.open("Oswan",224,144,format);
	int16 *backbuffer[NB_BUFFERS+1];
	for (int fr=0;fr<NB_BUFFERS+1;fr++)
	{
		backbuffer[fr]=(int16*)malloc(224*144*sizeof(int16));
		memset(backbuffer[fr],0x00,224*144*sizeof(int16));
	}	
	totalFrames=0;
	startTime=clock();
	nNormalLast=0;// Last value of timeGetTime()
	nNormalFrac=0; // Extra fraction we did
	nNormalLast=timeGetTime();
	// filter change
	if (gui_command==GUI_COMMAND_FILTER_CHANGE)
	{
		ws_loadState("oswan.wss");
	}
	while (1)
	{
		nTime=timeGetTime()-nNormalLast;					  // calcule le temps écoulé depuis le dernier affichage
															  // nTime est en mili-secondes.
		// détermine le nombre de trames à passer + 1
		nCount=(nTime*600 - nNormalFrac) /10000; 	

		// si le nombre de trames à passer + 1 est nul ou négatif,
		// ne rien faire pendant 2 ms
		if (nCount<=0) 
		{ 
			Sleep(2); 
		} // No need to do anything for a bit
		else
		{
			nNormalFrac+=nCount*10000;				// 
			nNormalLast+=nNormalFrac/600;				// add the duration of nNormalFrac frames
			nNormalFrac%=600;							// 

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
*/			int ws_key_esc=0;

			#include "./source/temp/key.h"
			if (ws_key_esc)
			{
				console.close();
				strcpy(old_rom_path,ws_rom_path);
				gui_open();

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
				console.open("Oswan",224,144,format);
			}

			
			for (i=0;i<nCount-1;i++) 
				while (!ws_executeLine(backbuffer[0],0));
			while (!ws_executeLine(backbuffer[totalFrames&(NB_BUFFERS-1)],1));

			ws_mergeBackbuffers(backbuffer);
			if (app_rotated)
				ws_rotate_backbuffer(backbuffer[NB_BUFFERS]);
			totalFrames++;
			int32 *vs = (int32 *)surface->lock();
			memcpy(vs,backbuffer[NB_BUFFERS],224*144*2);
			surface->unlock();
			surface->copy(console);
			console.update();
		}
	}
	endTime=clock();
	float fps=totalFrames/(((float)(endTime-startTime))/(float)CLOCKS_PER_SEC);
	printf("%f fps (%i %% the original speed)\n",fps, (int)((fps*100)/60));
	console.close();
	delete surface;
}
