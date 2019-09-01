
	Sint32		 startTime, endTime, totalFrames;
	Uint32 nNormalLast=0;
	Sint32			 nNormalFrac=0; 
	Sint32			 nTime=0,nCount=0; int i=0;

	double dTime = 0.0, dNormalLast = 0.0, dTemp;
	Sint32			surfacePitch;

	// 15 bits RGB555
	Format format(16,0x007c00,0x00003e0,0x0000001f);
	Console console;
	Surface *surface;
