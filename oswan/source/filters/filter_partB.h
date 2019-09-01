		int16 *backbuffer=(int16*)malloc(224*144*sizeof(int16));
		memset(backbuffer,0x00,224*144*sizeof(int16));
		surfacePitch=(surface->pitch()>>1);
		console.option("DirectX");
		if (app_fullscreen) 
			console.option("fullscreen output"); 
		else 
			console.option("windowed output");
		console.option("fixed window");
		console.option("center window");
		totalFrames=0;
		startTime=clock();
		nNormalLast=0;// Last value of timeGetTime()
		nNormalFrac=0; // Extra fraction we did
		//nNormalLast=timeGetTime();
		// hopefully, we only care about time delta, not time of day...
		//nNormalLast = SDL_GetTicks();
		dNormalLast = (double) SDL_GetTicks();
		// filter change
		if (gui_command==GUI_COMMAND_FILTER_CHANGE)
		{
			ws_loadState("oswan.wss");
		}
