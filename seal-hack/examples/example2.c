/* example2.c - play a module file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <audio.h>

#if defined(_MSC_VER) || defined(__WATCOMC__) || defined(__BORLANDC__) || defined(__DJGPP__)
#include <conio.h>
#else
#define kbhit() 0
#endif

int main(void)
{
    AUDIOINFO info;
    LPAUDIOMODULE lpModule;
	int ret;

    /* initialize audio library */
    AInitialize();

    /* open audio device */
	info.nDeviceId = 2;// AUDIO_DEVICE_MAPPER;
    info.wFormat = AUDIO_FORMAT_16BITS | AUDIO_FORMAT_STEREO;
    info.nSampleRate = 44100;

#ifdef USEFILTER
    /* enable antialias dynamic filtering */
    info.wFormat |= AUDIO_FORMAT_FILTER;
#endif

    AOpenAudio(&info);

    /* load module file */
    ret = ALoadModuleFile("test.s3m", &lpModule, 0);
	if (lpModule == NULL)
	{
		printf("Error loding file... [%d]\n", ret);
		return -1;
	}

    /* open voices and play module */
    AOpenVoices(lpModule->nTracks);
    APlayModule(lpModule);

    /* program main execution loop */
    printf("Playing module file, press any key to stop.\n");
    while (!kbhit()) {
        BOOL stopped;

        /* check if the module is stopped */
        AGetModuleStatus(&stopped);
        if (stopped) break;

        /* update audio system */
        AUpdateAudio();
    }

    /* stop module and close voices */
    AStopModule();
    ACloseVoices();

    /* release module file */
    AFreeModuleFile(lpModule);

    /* close audio device */
    ACloseAudio();
    return 0;
}
