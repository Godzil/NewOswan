/*
 * $Id: lnxdrv.c 1.5 1996/08/05 18:51:19 chasan released $
 *
 * SDL audio driver. Turns SEAL into a mixer on top of SDL.
 *
 * Copyright 2002 Greg Velichansky (hmaon@bumba.net)
 *
 * Based in part on the Linux Voxware driver,
 * Copyright (C) 1995-1999 Carlos Hasan
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "SDL.h"

#include "audio.h"
#include "drivers.h"


/*
 * fragments defines
 */
#define NUMFRAGS		16
#define FRAGSIZE		11
#define BUFFERSIZE	  (1 << FRAGSIZE)

/*
 * configuration structure
 */
static struct AudioStruct {
	SDL_AudioSpec desired, spec;
	BYTE	aBuffer[BUFFERSIZE*4];
	LPFNAUDIOWAVE lpfnAudioWave;
	WORD	wFormat;
} Audio;


/*
 * Linux driver API interface
 */
static UINT AIAPI GetAudioCaps(LPAUDIOCAPS lpCaps)
{
	static AUDIOCAPS Caps =
	{
		AUDIO_PRODUCT_LINUX, "SDL",
		AUDIO_FORMAT_1M08 | AUDIO_FORMAT_1S08 |
		AUDIO_FORMAT_1M16 | AUDIO_FORMAT_1S16 |
		AUDIO_FORMAT_2M08 | AUDIO_FORMAT_2S08 |
		AUDIO_FORMAT_2M16 | AUDIO_FORMAT_2S16 |
		AUDIO_FORMAT_4M08 | AUDIO_FORMAT_4S08 |
		AUDIO_FORMAT_4M16 | AUDIO_FORMAT_4S16
	};

	memcpy(lpCaps, &Caps, sizeof(AUDIOCAPS));
	return AUDIO_ERROR_NONE;
}

static UINT AIAPI PingAudio(VOID)
{
	return 0;
/* return (SDL_INIT_AUDIO == SDL_WasInit(SDL_INIT_AUDIO)); */
}

static void updatecallback(void *userdata, Uint8 *stream, int len);

static UINT AIAPI OpenAudio(LPAUDIOINFO lpInfo)
{
	//int nBitsPerSample, nStereoOn, nSampleRate, nFrags;
	int nStereoOn;
	
	memset(&Audio, 0, sizeof(Audio));

	if (!SDL_WasInit(SDL_INIT_EVERYTHING))
	{
		SDL_Init(SDL_INIT_AUDIO);
	} else
	{
		if (!SDL_WasInit(SDL_INIT_AUDIO) && SDL_InitSubSystem(SDL_INIT_AUDIO))
			return AUDIO_ERROR_NODEVICE;
	}
	if (SDL_INIT_AUDIO != SDL_WasInit(SDL_INIT_AUDIO)) 
		return AUDIO_ERROR_NODEVICE;


	//nBitsPerSample = lpInfo->wFormat & AUDIO_FORMAT_16BITS ? 16 : 8;
	nStereoOn = lpInfo->wFormat & AUDIO_FORMAT_STEREO ? 1 : 0;
	/*nSampleRate = lpInfo->nSampleRate;*/

	Audio.desired.freq = lpInfo->nSampleRate;
	Audio.desired.samples = 512;
	Audio.desired.channels = nStereoOn ? 2 : 1;

	Audio.desired.format = (lpInfo->wFormat & AUDIO_FORMAT_16BITS) ? AUDIO_U16SYS : AUDIO_U8;

	Audio.desired.userdata = (void*)&Audio;

	Audio.desired.callback = updatecallback;

	SDL_OpenAudio(&(Audio.desired), &(Audio.spec));

	/* we should probably do something here... blah... whatever :/ */

	/* setup number and size of buffer fragments */
	/*nFrags = (NUMFRAGS << 16) + (FRAGSIZE);
	ioctl(Audio.nHandle, SNDCTL_DSP_SETFRAGMENT, &nFrags);*/

	/* setup audio playback encoding format and sampling frequency */
	/*if (ioctl(Audio.nHandle, SNDCTL_DSP_SAMPLESIZE, &nBitsPerSample) < 0 ||
		ioctl(Audio.nHandle, SNDCTL_DSP_STEREO, &nStereoOn) < 0 ||
		ioctl(Audio.nHandle, SNDCTL_DSP_SPEED, &nSampleRate) < 0) {
		close(Audio.nHandle);
		return AUDIO_ERROR_BADFORMAT;
	}*/

	/*Audio.wFormat = lpInfo->wFormat;*/
	return AUDIO_ERROR_NONE;
}

static UINT AIAPI CloseAudio(VOID)
{
	SDL_CloseAudio();
	return AUDIO_ERROR_NONE;
}

static UINT AIAPI UpdateAudio(UINT nFrames)
{
	/* compute frame size */
	/*if (Audio.wFormat & AUDIO_FORMAT_16BITS) nFrames <<= 1;
	if (Audio.wFormat & AUDIO_FORMAT_STEREO) nFrames <<= 1;
	if (nFrames <= 0 || nFrames > sizeof(Audio.aBuffer))
		nFrames = sizeof(Audio.aBuffer);*/

	/* send PCM samples to the DSP audio device */
	/*if (Audio.lpfnAudioWave != NULL) {
		Audio.lpfnAudioWave(Audio.aBuffer, nFrames);
		write(Audio.nHandle, Audio.aBuffer, nFrames);
	}*/

	SDL_PauseAudio(0); /* the only use I can think of for UpdateAudio() ...*/

	return AUDIO_ERROR_NONE;
}



static void updatecallback(void *userdata, Uint8 *stream, int len)
{
	struct AudioStruct *aud = userdata; 
	/* maybe this isn't necessary, we know where Audio is, after all... */
	aud->lpfnAudioWave(stream, len); /* God speed... */
}



static UINT AIAPI SetAudioCallback(LPFNAUDIOWAVE lpfnAudioWave)
{
	/* set up DSP audio device user's callback function */
	Audio.lpfnAudioWave = lpfnAudioWave;
	return AUDIO_ERROR_NONE;
}


/*
 * Linux driver public interface
 */
AUDIOWAVEDRIVER SDLWaveDriver =
{
	GetAudioCaps, PingAudio, OpenAudio, CloseAudio,
	UpdateAudio, SetAudioCallback
};

AUDIODRIVER SDLDriver =
{
	&SDLWaveDriver, NULL
};
