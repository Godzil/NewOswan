		endTime=clock();
		float fps=totalFrames/(((float)(endTime-startTime))/(float)CLOCKS_PER_SEC);
		console.close();
		delete surface;
