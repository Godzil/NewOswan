////////////////////////////////////////////////////////////////////////////////
// Audio
////////////////////////////////////////////////////////////////////////////////
//
// Sound information thanks to toshi (wscamp wonderswan emulator)
// Note that sound is far from perfect for now.
//
// fixes by zalas 2002-08-21
//
//
//
////////////////////////////////////////////////////////////////////////////////

// alternate the commenting of the following defines to get audio port tracing
#define dbgprintf(...)
//#define dbgprintf(...) printf(...)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include "log.h"
#include "rom.h"
#include "./nec/nec.h"
#include "memory.h"
#include "io.h"
#include "audio.h"

#define	SNDP	ws_ioRam[0x80]
#define SNDV	ws_ioRam[0x88]
#define SNDSWP	ws_ioRam[0x8C]
#define SWPSTP	ws_ioRam[0x8D]
#define NSCTL	ws_ioRam[0x8E]
#define WAVDTP	ws_ioRam[0x8F]
#define SNDMOD	ws_ioRam[0x90]
#define SNDOUT	ws_ioRam[0x91]
#define PCSRL	ws_ioRam[0x92]
#define PCSRH	ws_ioRam[0x93]
#define DMASL	ws_ioRam[0x40]
#define DMASH	ws_ioRam[0x41]
#define DMASB	ws_ioRam[0x42]
#define DMADB	ws_ioRam[0x43]
#define DMADL	ws_ioRam[0x44]
#define DMADH	ws_ioRam[0x45]
#define DMACL	ws_ioRam[0x46]
#define DMACH	ws_ioRam[0x47]
#define DMACTL	ws_ioRam[0x48]
#define SDMASL	ws_ioRam[0x4A]
#define SDMASH	ws_ioRam[0x4B]
#define SDMASB	ws_ioRam[0x4C]
#define SDMACL	ws_ioRam[0x4E]
#define SDMACH	ws_ioRam[0x4F]
#define SDMACTL	ws_ioRam[0x52]

#define BPS			44100
#define BPSMAX		AUDIO_MAX_FREQUENCY
#define BPSMIN		AUDIO_MIN_FREQUENCY
#define BUFSIZE		1024
#define BUFSIZEN	0x10000
#define BUFSIZEP	159
#define PCMNUM		100
#define POFF		128
#define PDIV		3
#define PH			POFF+PDIV*8
#define PL			POFF-PDIV*7

int		WaveMap;
int		ChPerInit;
int		SwpTime;
int		SwpStep;
unsigned int		SwpCurPeriod;

int		MainVol=15;
int		HardVol=3;

int		ChCurVol[6]= {-1,-1,-1,-1,-1,-1};
int		ChCurPer[6]= {-1,-1,-1,-1,-1,-1};
long	ChCurPan[6]= {-1,-1,-1,-1,-1,-1};

unsigned char PData[4][BUFSIZE];
unsigned char PDataP[BUFSIZEP<<4];
unsigned char PDataN[8][BUFSIZEN];

int RandData[BUFSIZEN];

int CntSwp=0;
int PcmWrPos=0;

/*
const long TblChVol[16]=  				// n/15 n=0~15
{
   -10000,-2352,-1750,-1398,-1148,-954,-796,-662,
      -546,-444,-352,-269,-194,-124,-60,0
   };

const long TblMainVol[4]=  				// 1,1/2,1/4,1/8
{
   0,-602,-1204,-1806
};
*/

////////////////////////////////////////////////////////////////////////////////
// seal audio specific
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
//HAC			ws_audio_pcm_voice[4];
//HAC			ws_audio_noise_voice;
//HAC			ws_audio_sweep_voice;

//AUDIOWAVE	ws_audio_pcm_wave[4];
//AUDIOWAVE	ws_audio_noise_wave;
//AUDIOWAVE	ws_audio_sweep_wave;

uint32_t		ws_audio_channel_isPlaying[6];

static unsigned int ws_audio_log;
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
void ws_audio_init(void)
{
   fprintf(log_get(), "audio init\n");
   fflush(log_get());
   ws_audio_log=0;
   //ws_audio_seal_init();
   ws_audio_reset();
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
void ws_audio_reset(void)
{
   WaveMap=-1;

   for (int i=0; i<6; i++)
   {
      ws_audio_stop_channel(i);
      ws_audio_play_channel(i);
      ws_audio_set_channel_frequency(i,0);

      if (i!=4)
      {
         ws_audio_set_channel_pan(i,0,0);
      }

      ws_audio_clear_channel(i);
   }

   ws_audio_set_channel_frequency(4,0);
   ws_audio_set_channel_frequency(4,1792);
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
void ws_audio_port_write(uint32_t port, uint8_t value)
{
   uint32_t n,i,j,k,b;

   ws_ioRam[port]=value;

   switch (port)
   {
   case 0x48:
      if (value&0x80)
      {
         n=(DMACH<<8)|DMACL;
         i=(DMASB<<16)|(DMASH<<8)|DMASL;
         j=(DMADH<<8)|DMADL;

         for(k=0; k<n; k++)
         {
            b=cpu_readmem20(i);
            cpu_writemem20(j,b);
            i++;
            j++;
         }

         n=0;
         DMASB=(uint8_t)((i>>16)&0xFF);
         DMASH=(uint8_t)((i>>8)&0xFF);
         DMASL=(uint8_t)(i&0xFF);
         DMADB=(uint8_t)((j>>16)&0xFF);
         DMADH=(uint8_t)((j>>8)&0xFF);
         DMADL=(uint8_t)(j&0xFF);
         DMACH=(uint8_t)((n>>8)&0xFF);
         DMACL=(uint8_t)(n&0xFF);
         value&=0x7F;
      }

      break;

   case 0x80:
   case 0x81:
      i=(((unsigned int)ws_ioRam[0x81])<<8)+((unsigned int)ws_ioRam[0x80]);
      ws_audio_set_channel_frequency(0,i);
      break;

   case 0x82:
   case 0x83:
      i=(((unsigned int)ws_ioRam[0x83])<<8)+((unsigned int)ws_ioRam[0x82]);
      ws_audio_set_channel_frequency(1,i);
      break;

   case 0x84:
   case 0x85:
      i=(((unsigned int)ws_ioRam[0x85])<<8)+((unsigned int)ws_ioRam[0x84]);
      ws_audio_set_channel_frequency(2,i);
      break;

   case 0x86:
   case 0x87:
      i=(((unsigned int)(ws_ioRam[0x87]&0x07))<<8)+((unsigned int)ws_ioRam[0x86]);
      ws_audio_set_channel_frequency(5,i);
      ws_audio_set_channel_frequency(3,i);
      break;

   case 0x88:
      ws_audio_set_channel_pan(0,(value&0xF0)>>4,value&0x0F);
      break;

   case 0x89:
      ws_audio_set_channel_pan(1,(value&0xF0)>>4,value&0x0F);
      break;

   case 0x8A:
      ws_audio_set_channel_pan(2,(value&0xF0)>>4,value&0x0F);
      break;

   case 0x8B:
      ws_audio_set_channel_pan(5,(value&0xF0)>>4,value&0x0F);
      ws_audio_set_channel_pan(3,(value&0xF0)>>4,value&0x0F);
      break;

   case 0x8C:
      SwpStep=(signed char)value;
      break;

   case 0x8D:
      SwpTime=(((unsigned int)value)+1)<<5;
      break;

   case 0x8E:
      if (value & 0x10)
      {
         ws_audio_set_channel_pdata(5,value&0x07);
      }
      else
      {
         // hmmm.. shut up!
      }

      break;

   case 0x8F:
      WaveMap=((unsigned int)value)<<6;
      break;

   case 0x90:
      if (value&0x01)
      {
         ws_audio_play_channel(0);
      }
      else
      {
         ws_audio_stop_channel(0);
      }

      if ((value&0x22)==0x02)
      {
         ws_audio_play_channel(1);
      }
      else
      {
         ws_audio_stop_channel(1);
      }

      if (value&0x04)
      {
         ws_audio_play_channel(2);
      }
      else
      {
         ws_audio_stop_channel(2);
      }

      if ((value&0x88)==0x08)
      {
         ws_audio_play_channel(3);
      }
      else
      {
         ws_audio_stop_channel(3);
      }

      if ((value&0x88)==0x88)
      {
         ws_audio_play_channel(5);
      }
      else
      {
         ws_audio_stop_channel(5);
      }

      break;

   case 0x91:
      value|=0x80;
      HardVol = (value>>1)&0x3;

      ws_ioRam[port]=value;	// Always have external speaker

      value=ws_ioRam[0x88];

      ws_audio_set_channel_pan(0,(value&0xF0)>>4,value&0x0F);

      value=ws_ioRam[0x89];

      ws_audio_set_channel_pan(1,(value&0xF0)>>4,value&0x0F);

      value=ws_ioRam[0x8A];

      ws_audio_set_channel_pan(2,(value&0xF0)>>4,value&0x0F);

      value=ws_ioRam[0x8B];

      ws_audio_set_channel_pan(3,(value&0xF0)>>4,value&0x0F);

      ws_audio_set_channel_pan(5,(value&0xF0)>>4,value&0x0F);

      break;

   case 0x92:
      dbgprintf("0x92 <- 0x%2x\n", value);
      fflush(stdout);
      break;

   case 0x93:
      dbgprintf("0x93 <- 0x%2x\n", value);
      fflush(stdout);
      break;

   case 0x94:
      dbgprintf("0x94 <- 0x%2x\n", value);
      fflush(stdout);
      MainVol=(value&0x0f)>>2;
      value=ws_ioRam[0x88];

      ws_audio_set_channel_pan(0,(value&0xF0)>>4,value&0x0F);

      value=ws_ioRam[0x89];

      ws_audio_set_channel_pan(1,(value&0xF0)>>4,value&0x0F);

      value=ws_ioRam[0x8A];

      ws_audio_set_channel_pan(2,(value&0xF0)>>4,value&0x0F);

      value=ws_ioRam[0x8B];

      ws_audio_set_channel_pan(3,(value&0xF0)>>4,value&0x0F);

      ws_audio_set_channel_pan(5,(value&0xF0)>>4,value&0x0F);

      break;

   case 0x9A:
      break;
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
uint8_t ws_audio_port_read(uint8_t port)
{
   return(ws_ioRam[port]);
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
void ws_audio_done(void)
{
   ws_audio_seal_done();
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
unsigned int ws_audio_mrand(unsigned int Degree)
{
#define BIT(n) (1<<n)
   typedef struct
   {
      unsigned int N;
      int InputBit;
      int Mask;
   } POLYNOMIAL;

   static POLYNOMIAL TblMask[]=
   {
      { 2,BIT(2),BIT(0)|BIT(1)},
      { 3,BIT(3),BIT(0)|BIT(1)},
      { 4,BIT(4),BIT(0)|BIT(1)},
      { 5,BIT(5),BIT(0)|BIT(2)},
      { 6,BIT(6),BIT(0)|BIT(1)},
      { 7,BIT(7),BIT(0)|BIT(1)},
      { 8,BIT(8),BIT(0)|BIT(2)|BIT(3)|BIT(4)},
      { 9,BIT(9),BIT(0)|BIT(4)},
      {10,BIT(10),BIT(0)|BIT(3)},
      {11,BIT(11),BIT(0)|BIT(2)},
      {12,BIT(12),BIT(0)|BIT(1)|BIT(4)|BIT(6)},
      {13,BIT(13),BIT(0)|BIT(1)|BIT(3)|BIT(4)},
      {14,BIT(14),BIT(0)|BIT(1)|BIT(4)|BIT(5)},
      {15,BIT(15),BIT(0)|BIT(1)},
      {0,0,0},
   };

   static POLYNOMIAL *pTbl=TblMask;
   static int ShiftReg=pTbl->InputBit-1;
   int XorReg=0;
   int Masked;

   if(pTbl->N!=Degree)
   {
      pTbl=TblMask;

      while(pTbl->N)
      {
         if(pTbl->N==Degree)
         {
            break;
         }

         pTbl++;
      }

      if(!pTbl->N)
      {
         pTbl--;
      }

      ShiftReg&=pTbl->InputBit-1;

      if(!ShiftReg)
      {
         ShiftReg=pTbl->InputBit-1;
      }
   }

   Masked=ShiftReg&pTbl->Mask;

   while(Masked)
   {
      XorReg^=Masked&0x01;
      Masked>>=1;
   }

   if(XorReg)
   {
      ShiftReg|=pTbl->InputBit;
   }
   else
   {
      ShiftReg&=~pTbl->InputBit;
   }

   ShiftReg>>=1;

   return ShiftReg;
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
int ws_audio_seal_init(void)
{
#if 0
   int			i, j;
   AUDIOINFO	info;
   AUDIOCAPS	caps;
   uint32_t		rc;
   uint32_t		nDevId;

   fprintf(log_get(),"audio: using seal audio library\n");
   /* initialize audio library */
   AInitialize();

   /* show registered device drivers */
   fprintf(log_get(),"audio: registered sound devices:\n");

   for (nDevId = 0; nDevId < AGetAudioNumDevs(); nDevId++)
   {
      AGetAudioDevCaps(nDevId, &caps);
      fprintf(log_get(),"audio:   %2d. %s\n", nDevId, caps.szProductName);
   }

   /* open audio device */
   info.nDeviceId = 0;
   info.wFormat   = AUDIO_FORMAT_16BITS | AUDIO_FORMAT_STEREO; // | AUDIO_MIXER_BASS;
   info.nSampleRate = 44100;

   if ((rc = AOpenAudio(&info)) != AUDIO_ERROR_NONE)
   {
      CHAR szText[80];
      AGetErrorText(rc, szText, sizeof(szText) - 1);
      fprintf(log_get(),"audio: error: %s\n", szText);
      fflush(log_get());
      return(0);
   }

   // open 6 voices ( 4 pcm, one noise, and one sweep)
   AOpenVoices(6);

   // create the 4 pcm channels
   for (i=0; i<4; i++)
   {
      // create the channel
      ACreateAudioVoice(&ws_audio_pcm_voice[i]);
      ASetVoiceVolume  ( ws_audio_pcm_voice[i], AUDIO_MAX_VOLUME);
      ASetVoicePanning ( ws_audio_pcm_voice[i], AUDIO_MIN_PANNING);


      // create a looped sound buffer
      ws_audio_pcm_wave[i].nSampleRate	= info.nSampleRate;
      ws_audio_pcm_wave[i].dwLength		= BUFSIZE;
      ws_audio_pcm_wave[i].dwLoopStart	= 0;
      ws_audio_pcm_wave[i].dwLoopEnd		= ws_audio_pcm_wave[i].dwLength;
      ws_audio_pcm_wave[i].wFormat		= AUDIO_FORMAT_8BITS | AUDIO_FORMAT_MONO | AUDIO_FORMAT_LOOP;

      ACreateAudioData(&ws_audio_pcm_wave[i]);

      // channel is not playing yet
      ws_audio_channel_isPlaying[i]=0;

      // clear the channel
      ws_audio_clear_channel(i);
   }

   // create the noise channel
   {
      // create the channel
      ACreateAudioVoice(&ws_audio_noise_voice);
      ASetVoiceVolume  ( ws_audio_noise_voice, AUDIO_MAX_VOLUME);
      ASetVoicePanning ( ws_audio_noise_voice, AUDIO_MAX_PANNING>>1);


      // create a looped sound buffer
      ws_audio_noise_wave.nSampleRate	= info.nSampleRate;
      ws_audio_noise_wave.dwLength	= (BUFSIZEP<<4);
      ws_audio_noise_wave.dwLoopStart	= 0;
      ws_audio_noise_wave.dwLoopEnd	= ws_audio_noise_wave.dwLength;
      ws_audio_noise_wave.wFormat		= AUDIO_FORMAT_8BITS | AUDIO_FORMAT_MONO | AUDIO_FORMAT_LOOP;

      ACreateAudioData(&ws_audio_noise_wave);

      // channel is not playing yet
      ws_audio_channel_isPlaying[4]=0;

      // clear the channel
      ws_audio_clear_channel(4);
   }

   // create the sweep channel
   {
      // create the channel
      ACreateAudioVoice(&ws_audio_sweep_voice);
      ASetVoiceVolume  ( ws_audio_sweep_voice, AUDIO_MAX_VOLUME);
      ASetVoicePanning ( ws_audio_sweep_voice, AUDIO_MAX_PANNING);


      // create a looped sound buffer
      ws_audio_sweep_wave.nSampleRate	= info.nSampleRate;
      ws_audio_sweep_wave.dwLength	= BUFSIZEN;
      ws_audio_sweep_wave.dwLoopStart	= 0;
      ws_audio_sweep_wave.dwLoopEnd	= ws_audio_sweep_wave.dwLength;
      ws_audio_sweep_wave.wFormat		= AUDIO_FORMAT_8BITS | AUDIO_FORMAT_MONO | AUDIO_FORMAT_LOOP;

      ACreateAudioData(&ws_audio_sweep_wave);

      // channel is not playing yet
      ws_audio_channel_isPlaying[5]=0;

      // clear the channel
      ws_audio_clear_channel(5);
   }

   // initialize the noise channel data
   int rand;

   for(i=0; i<8; i++)
   {
      for(j=0; j<BUFSIZEN; j++)
      {
         rand=ws_audio_mrand(15-i)&1;

         if(rand)
         {
            PDataN[i][j]=PH;
         }
         else
         {
            PDataN[i][j]=PL;
         }
      }
   }

   for(j=0; j<BUFSIZEN; j++)
   {
      RandData[j]=ws_audio_mrand(15);
   }

   ASetAudioMixerValue(AUDIO_MIXER_MASTER_VOLUME, AUDIO_MAX_VOLUME);


   fflush(log_get());

#endif
   return 1;
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
void ws_audio_seal_done(void)
{
#if 0
   int i;

   // stop channels
   for (i=0; i<6; i++)
   {
      ws_audio_stop_channel(i);
   }

   // destroy pcm wave data
   for (i=0; i<4; i++)
   {
      ADestroyAudioData(&ws_audio_pcm_wave[i]);
   }

   // destroy noise wave data
   ADestroyAudioData(&ws_audio_noise_wave);

   // destroy sweep wave data
   ADestroyAudioData(&ws_audio_sweep_wave);

   // release pcm channels
   for (i=0; i<4; i++)
   {
      ADestroyAudioVoice(ws_audio_pcm_voice[i]);
   }

   // release noise channel
   ADestroyAudioVoice(ws_audio_noise_voice);

   // release sweep channel
   ADestroyAudioVoice(ws_audio_sweep_voice);

   // close A channels
   ACloseVoices();

   // close audio
   ACloseAudio();
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
void ws_audio_clear_channel(int Channel)
{
#if 0
   ChCurVol[Channel]=-1;
   ChCurPer[Channel]=-1;
   ChCurPan[Channel]=-1;

   if(Channel==5)
   {
      memset(ws_audio_sweep_wave.lpData,		0, ws_audio_sweep_wave.dwLength);
      AWriteAudioData(&ws_audio_sweep_wave,	0, ws_audio_sweep_wave.dwLength);
   }
   else if(Channel==4)
   {
      memset(ws_audio_noise_wave.lpData,		0, ws_audio_noise_wave.dwLength);
      AWriteAudioData(&ws_audio_noise_wave,	0, ws_audio_noise_wave.dwLength);
   }
   else
   {
      memset(ws_audio_pcm_wave[Channel].lpData,		0, ws_audio_pcm_wave[Channel].dwLength);
      AWriteAudioData(&ws_audio_pcm_wave[Channel],	0, ws_audio_pcm_wave[Channel].dwLength);
   }
#endif
}
////////////////////////////////////////////////////////////////////////////////
// start playing a channel
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
int ws_audio_play_channel(int Channel)
{
#if 0
   if (ws_audio_channel_isPlaying[Channel])
   {
      return(0);
   }

   ws_audio_channel_isPlaying[Channel]=1;

   if (Channel==5)
   {
      APlayVoice(ws_audio_sweep_voice, &ws_audio_sweep_wave);
   }
   else if (Channel==4)
   {
      APlayVoice(ws_audio_noise_voice, &ws_audio_noise_wave);
   }
   else
   {
//		ASetVoiceFrequency(ws_audio_pcm_voice[Channel],3072000/(2048-ChCurPer[Channel]));
      APlayVoice(ws_audio_pcm_voice[Channel], &ws_audio_pcm_wave[Channel]);
      ASetVoiceFrequency(ws_audio_pcm_voice[Channel],3072000/(2048-ChCurPer[Channel]));
   }
#endif
   return 0;
}
////////////////////////////////////////////////////////////////////////////////
// stop playing a channel
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
int ws_audio_stop_channel(int Channel)
{
#if 0
   if (!ws_audio_channel_isPlaying[Channel])
   {
      return(0);
   }

   ws_audio_channel_isPlaying[Channel]=0;

   if (Channel==5)
   {
      AStopVoice(ws_audio_sweep_voice);
   }
   else if (Channel==4)
   {
      AStopVoice(ws_audio_noise_voice);
   }
   else
   {
      AStopVoice(ws_audio_pcm_voice[Channel]);
   }
#endif
   return(0);
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
void ws_audio_set_channel_frequency(int Channel,int Period)
{
#if 0
   uint32_t Freq;

   if(ChCurPer[Channel]==Period)
   {
      return;
   }

   ChCurPer[Channel]=Period;

   Freq=3072000/(2048-Period);

   if(Channel==2)
   {
      ChPerInit=Period;
      SwpCurPeriod=Period;
   }

   if(Freq>BPSMAX)
   {
      Freq=BPSMAX;
   }

   else if(Freq<BPSMIN)
   {
      Freq=BPSMIN;
   }

   if (Channel==5)
   {
      ASetVoiceFrequency(ws_audio_sweep_voice,Freq);
   }
   else if (Channel==4)
   {
      ASetVoiceFrequency(ws_audio_noise_voice,Freq);
   }
   else
   {
      ASetVoiceFrequency(ws_audio_pcm_voice[Channel],Freq);
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
void ws_audio_set_channel_volume(int Channel,int Vol)
{
#if 0
   long volume;

   if(ChCurVol[Channel]==Vol)
   {
      return;
   }

   ChCurVol[Channel]=Vol;

   volume=(Vol+1)*(MainVol+1)*(HardVol+1)/16-1;

   if (Channel==5)
   {
      ASetVoiceVolume(ws_audio_sweep_voice,volume);
   }
   else if (Channel==4)
   {
      ASetVoiceVolume(ws_audio_noise_voice,volume);
   }
   else
   {
      ASetVoiceVolume(ws_audio_pcm_voice[Channel],volume);
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
void ws_audio_set_channel_pan(int Channel,int Left,int Right)
{
#if 0
   long pan;

   const long TblPan[16][16]=
   {
      {     0, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000},
      {-10000,     0,   602,   954,  1204,  1398,  1556,  1690,  1806,  1908,  2000,  2082,  2158,  2228,  2292,  2352},
      {-10000,  -602,     0,   352,   602,   796,   954,  1088,  1204,  1306,  1398,  1481,  1556,  1626,  1690,  1750},
      {-10000,  -954,  -352,     0,   250,   444,   602,   736,   852,   954,  1046,  1129,  1204,  1274,  1338,  1398},
      {-10000, -1204,  -602,  -250,     0,   194,   352,   486,   602,   704,   796,   879,   954,  1024,  1088,  1148},
      {-10000, -1398,  -796,  -444,  -194,     0,   158,   292,   408,   511,   602,   685,   760,   830,   894,   954},
      {-10000, -1556,  -954,  -602,  -352,  -158,     0,   134,   250,   352,   444,   526,   602,   672,   736,   796},
      {-10000, -1690, -1088,  -736,  -486,  -292,  -134,     0,   116,   218,   310,   393,   468,   538,   602,   662},
      {-10000, -1806, -1204,  -852,  -602,  -408,  -250,  -116,     0,   102,   194,   277,   352,   422,   486,   546},
      {-10000, -1908, -1306,  -954,  -704,  -511,  -352,  -218,  -102,     0,    92,   174,   250,   319,   384,   444},
      {-10000, -2000, -1398, -1046,  -796,  -602,  -444,  -310,  -194,   -92,     0,    83,   158,   228,   292,   352},
      {-10000, -2082, -1481, -1129,  -879,  -685,  -526,  -393,  -277,  -174,   -83,     0,    76,   145,   209,   269},
      {-10000, -2158, -1556, -1204,  -954,  -760,  -602,  -468,  -352,  -250,  -158,   -76,     0,    70,   134,   194},
      {-10000, -2228, -1626, -1274, -1024,  -830,  -672,  -538,  -422,  -319,  -228,  -145,   -70,     0,    64,   124},
      {-10000, -2292, -1690, -1338, -1088,  -894,  -736,  -602,  -486,  -384,  -292,  -209,  -134,   -64,     0,    60},
      {-10000, -2352, -1750, -1398, -1148,  -954,  -796,  -662,  -546,  -444,  -352,  -269,  -194,  -124,   -60,     0},
   };

   if(Left>Right)
   {
      ws_audio_set_channel_volume(Channel,Left);
   }
   else
   {
      ws_audio_set_channel_volume(Channel,Right);
   }

   pan=TblPan[Left][Right];

   if(ChCurPan[Channel]==pan)
   {
      return;
   }

   ChCurPan[Channel]=pan;

   if (pan>10000)
   {
      pan=10000;
   }

   pan=((pan+10000)*AUDIO_MAX_PANNING)/20000;

   if (Channel==5)
   {
      ASetVoicePanning(ws_audio_sweep_voice,pan);
   }
   else if (Channel==4)
   {
      ASetVoicePanning(ws_audio_noise_voice,pan);
   }
   else
   {
      ASetVoicePanning(ws_audio_pcm_voice[Channel],pan);
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
void ws_audio_set_channel_pdata(int Channel,int Index)
{
#if 0
   unsigned char *pData;

   if(Channel==5)
   {
      pData=PDataN[Index];

      memcpy(ws_audio_sweep_wave.lpData,		pData, ws_audio_sweep_wave.dwLength);
      AWriteAudioData(&ws_audio_sweep_wave,	0, ws_audio_sweep_wave.dwLength);
   }
   else if(Channel==4)
   {
      pData=PDataP;
      memcpy(ws_audio_noise_wave.lpData,		pData, ws_audio_noise_wave.dwLength);
      AWriteAudioData(&ws_audio_noise_wave,	0, ws_audio_noise_wave.dwLength);
   }
   else
   {
      pData=PData[Channel];
      memcpy(ws_audio_pcm_wave[Channel].lpData,		pData, ws_audio_pcm_wave[Channel].dwLength);
      AWriteAudioData(&ws_audio_pcm_wave[Channel],	0, ws_audio_pcm_wave[Channel].dwLength);
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
void ws_audio_set_channels_pbuf(int Addr,int Data)
{
#if 0
   int i,j;

   i=(Addr&0x30)>>4;

   for(j=(Addr&0x0F)<<1; j<BUFSIZE; j+=32)
   {
      PData[i][j] = (Data&0x0f)*17-128;
      PData[i][j+1] = ((Data>>4)&0x0f)*17-128;
   }

   if((Addr&0x0F)==0x0F)
   {
      ws_audio_set_channel_pdata(i,0);
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
void ws_audio_rst_channel(int Channel)
{
#if 0
   if(Channel==2)
   {
      ws_audio_set_channel_frequency(2,ChPerInit);
      SwpCurPeriod=ChPerInit;
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
int ws_audio_int(void)
{
#if 0
   unsigned int value;
   static int i;

   if((SwpStep)&&(SNDMOD&0x40))
   {
      if(CntSwp<0)
      {
         CntSwp=SwpTime;
         SwpCurPeriod+=SwpStep;
         SwpCurPeriod&=0x7FF;
         value=3072000/(2048-SwpCurPeriod);

         if(value>100000)
         {
            value=100000;
            ws_audio_set_channel_volume(2,0);
         }

         if(value<100)
         {
            value=100;
         }

         ASetVoiceFrequency(ws_audio_pcm_voice[2],value);
      }

      CntSwp--;
   }

   i++;

   if(i>=BUFSIZEN)
   {
      i=0;
   }

   return RandData[i];
#endif
   return 1;
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
#if 0
static uint32_t PCMPos=0;
uint32_t TickZ=0,PcmCount;
#endif

void ws_audio_set_pcm(int Data)
{
#if 0
   uint32_t tick;
   PDataP[PCMPos++]=(unsigned char)(Data+128);
   tick=SDL_GetTicks();
   PcmCount++;

   if(tick>=TickZ)
   {
      TickZ=tick+125;
      PcmCount<<=3;

      if(PcmCount>=10000)
      {
         PcmCount=12000;
      }

      ASetVoiceFrequency(ws_audio_noise_voice,PcmCount);
      PcmCount=0;
   }

   if(PCMPos>=BUFSIZEP)
   {
      ws_audio_flash_pcm();
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
void ws_audio_flash_pcm(void)
{
#if 0
   uint32_t len1;

   const uint32_t WrPos[16]=
   {
      BUFSIZEP*0,BUFSIZEP*1,BUFSIZEP*2,BUFSIZEP*3,
      BUFSIZEP*4,BUFSIZEP*5,BUFSIZEP*6,BUFSIZEP*7,
      BUFSIZEP*8,BUFSIZEP*9,BUFSIZEP*10,BUFSIZEP*11,
      BUFSIZEP*12,BUFSIZEP*13,BUFSIZEP*14,BUFSIZEP*15,
   };

   len1=BUFSIZEP;

   if (ws_audio_noise_wave.lpData == NULL)
   {
      return;
   }

   memcpy(&ws_audio_noise_wave.lpData[WrPos[PcmWrPos]], PDataP, len1);
   AWriteAudioData(&ws_audio_noise_wave,	0, ws_audio_noise_wave.dwLength);


   PcmWrPos++;
   PcmWrPos&=0xF;
   memset(PDataP,PL,sizeof(PDataP));
   PCMPos=0;
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
void ws_audio_write_byte(uint32_t offset, uint8_t value)
{
   if (!((offset-WaveMap)&0xFFC0))
   {
      ws_audio_set_channels_pbuf(offset&0x003F,value);
      internalRam[offset]=value;
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
void ws_audio_process(void)
{
   uint32_t i, j, b;
   i=ws_audio_int();
   PCSRL=(uint8_t)(i&0xFF);
   PCSRH=(uint8_t)((i>>8)&0xFF);

   if((SDMACTL&0x88)==0x80)
   {
      i=(SDMACH<<8)|SDMACL;
      j=(SDMASB<<16)|(SDMASH<<8)|SDMASL;
      b=cpu_readmem20(j);

      if(!ws_audio_channel_isPlaying[5])
      {
         b=0x80;
      }

      ws_ioRam[0x89]=b;
      i--;
      j++;

      if(i<32)
      {
         i=0;
         SDMACTL&=0x7F;
      }

      SDMASB=(uint8_t)((j>>16)&0xFF);
      SDMASH=(uint8_t)((j>>8)&0xFF);
      SDMASL=(uint8_t)(j&0xFF);
      SDMACH=(uint8_t)((i>>8)&0xFF);
      SDMACL=(uint8_t)(i&0xFF);
   }
   else if((SNDMOD&0x22)==0x22)
   {
      b=ws_ioRam[0x89];

      if(!ws_audio_channel_isPlaying[4])
      {
         b=0x80;
      }
   }
   else
   {
      b=0x80;
   }

   b>>=1;
   b+=0x40;

   if(b>0xAA)
   {
      b=0xAA;
   }
   else if(b<0x55)
   {
      b=0x55;
   }

   ws_audio_set_pcm(b);
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
void ws_audio_readState(int fp)
{
#if 0
   long lpdwPosition;
   long lpdwFrequency;
   unsigned int lpnVolume;
   unsigned int lpnPanning;
   int lpnStatus;
   unsigned char *pData;

   read(fp,&PCMPos,sizeof(uint32_t));
   read(fp,&TickZ,sizeof(uint32_t));
   read(fp,&PcmCount,sizeof(uint32_t));
   read(fp,&WaveMap,sizeof(int));
   read(fp,&ChPerInit,sizeof(int));
   read(fp,&SwpTime,sizeof(int));
   read(fp,&SwpStep,sizeof(int));
   read(fp,&SwpCurPeriod,sizeof(int));
   read(fp,&MainVol,sizeof(int));
   read(fp,&CntSwp,sizeof(int));
   read(fp,&PcmWrPos,sizeof(int));

   read(fp,ws_audio_channel_isPlaying,sizeof(uint32_t)*6);

   read(fp,PData,sizeof(unsigned char)*4*BUFSIZE);
   read(fp,PDataP,sizeof(unsigned char)*(BUFSIZEP<<4));
   read(fp,PDataN,sizeof(unsigned char)*8*BUFSIZEN);
   read(fp,PDataN,sizeof(int)*BUFSIZEN);

   for (int i=0; i<4; i++)
   {
      read(fp,&lpdwPosition,sizeof(long));
      read(fp,&lpdwFrequency,sizeof(long));
      read(fp,&lpnVolume,sizeof(unsigned int));
      read(fp,&lpnPanning,sizeof(unsigned int));
      read(fp,&lpnStatus,sizeof(int));
      ASetVoicePosition(ws_audio_pcm_voice[i],lpdwPosition);
      ASetVoiceFrequency(ws_audio_pcm_voice[i], lpdwFrequency);
      ASetVoiceVolume(ws_audio_pcm_voice[i], lpnVolume);
      ASetVoicePanning(ws_audio_pcm_voice[i], lpnPanning);
      pData=PData[i];
      memcpy(ws_audio_pcm_wave[i].lpData,		pData, ws_audio_pcm_wave[i].dwLength);
      AWriteAudioData(&ws_audio_pcm_wave[i],	0, ws_audio_pcm_wave[i].dwLength);

      if (ws_audio_channel_isPlaying[i])
      {
         APlayVoice(ws_audio_pcm_voice[i], &ws_audio_pcm_wave[i]);
      }
   }

   ASetVoicePosition(ws_audio_noise_voice,lpdwPosition);
   ASetVoiceFrequency(ws_audio_noise_voice, lpdwFrequency);
   ASetVoiceVolume(ws_audio_noise_voice, lpnVolume);
   ASetVoicePanning(ws_audio_noise_voice, lpnPanning);
   pData=PDataP;
   memcpy(ws_audio_noise_wave.lpData,		pData, ws_audio_noise_wave.dwLength);
   AWriteAudioData(&ws_audio_noise_wave,	0, ws_audio_noise_wave.dwLength);

   if (ws_audio_channel_isPlaying[4])
   {
      APlayVoice(ws_audio_noise_voice, &ws_audio_noise_wave);
   }

   read(fp,&lpdwPosition,sizeof(long));
   read(fp,&lpdwFrequency,sizeof(long));
   read(fp,&lpnVolume,sizeof(unsigned int));
   read(fp,&lpnPanning,sizeof(unsigned int));
   read(fp,&lpnStatus,sizeof(int));

   ASetVoicePosition(ws_audio_sweep_voice,lpdwPosition);
   ASetVoiceFrequency(ws_audio_sweep_voice, lpdwFrequency);
   ASetVoiceVolume(ws_audio_sweep_voice, lpnVolume);
   ASetVoicePanning(ws_audio_sweep_voice, lpnPanning);
   pData=PDataN[0];
   memcpy(ws_audio_sweep_wave.lpData,		pData, ws_audio_sweep_wave.dwLength);
   AWriteAudioData(&ws_audio_sweep_wave,	0, ws_audio_sweep_wave.dwLength);

   if (ws_audio_channel_isPlaying[5])
   {
      APlayVoice(ws_audio_sweep_voice, &ws_audio_sweep_wave);
   }

   read(fp,&lpdwPosition,sizeof(long));
   read(fp,&lpdwFrequency,sizeof(long));
   read(fp,&lpnVolume,sizeof(unsigned int));
   read(fp,&lpnPanning,sizeof(unsigned int));
   read(fp,&lpnStatus,sizeof(int));
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
void ws_audio_writeState(int fp)
{
#if 0
   int32_t lpdwPosition;
   int32_t lpdwFrequency;
   uint16_t lpnVolume;
   uint16_t lpnPanning;
   int8_t lpnStatus;

   write(fp,&PCMPos,sizeof(uint32_t));
   write(fp,&TickZ,sizeof(uint32_t));
   write(fp,&PcmCount,sizeof(uint32_t));
   write(fp,&WaveMap,sizeof(int));
   write(fp,&ChPerInit,sizeof(int));
   write(fp,&SwpTime,sizeof(int));
   write(fp,&SwpStep,sizeof(int));
   write(fp,&SwpCurPeriod,sizeof(int));
   write(fp,&MainVol,sizeof(int));
   write(fp,&CntSwp,sizeof(int));
   write(fp,&PcmWrPos,sizeof(int));

   write(fp,ws_audio_channel_isPlaying,sizeof(uint32_t)*6);

   write(fp,PData,sizeof(unsigned char)*4*BUFSIZE);
   write(fp,PDataP,sizeof(unsigned char)*(BUFSIZEP<<4));
   write(fp,PDataN,sizeof(unsigned char)*8*BUFSIZEN);
   write(fp,PDataN,sizeof(int)*BUFSIZEN);

   for (int i=0; i<4; i++)
   {
      AGetVoicePosition(ws_audio_pcm_voice[i],&lpdwPosition);
      AGetVoiceFrequency(ws_audio_pcm_voice[i], &lpdwFrequency);
      AGetVoiceVolume(ws_audio_pcm_voice[i], &lpnVolume);
      AGetVoicePanning(ws_audio_pcm_voice[i], &lpnPanning);
      AGetVoiceStatus(ws_audio_pcm_voice[i], &lpnStatus);

      write(fp,&lpdwPosition,sizeof(long));
      write(fp,&lpdwFrequency,sizeof(long));
      write(fp,&lpnVolume,sizeof(unsigned int));
      write(fp,&lpnPanning,sizeof(unsigned int));
      write(fp,&lpnStatus,sizeof(int));
   }

   AGetVoicePosition(ws_audio_noise_voice,&lpdwPosition);
   AGetVoiceFrequency(ws_audio_noise_voice, &lpdwFrequency);
   AGetVoiceVolume(ws_audio_noise_voice, &lpnVolume);
   AGetVoicePanning(ws_audio_noise_voice, &lpnPanning);
   AGetVoiceStatus(ws_audio_noise_voice, &lpnStatus);

   write(fp,&lpdwPosition,sizeof(long));
   write(fp,&lpdwFrequency,sizeof(long));
   write(fp,&lpnVolume,sizeof(unsigned int));
   write(fp,&lpnPanning,sizeof(unsigned int));
   write(fp,&lpnStatus,sizeof(int));

   AGetVoicePosition(ws_audio_sweep_voice,&lpdwPosition);
   AGetVoiceFrequency(ws_audio_sweep_voice, &lpdwFrequency);
   AGetVoiceVolume(ws_audio_sweep_voice, &lpnVolume);
   AGetVoicePanning(ws_audio_sweep_voice, &lpnPanning);
   AGetVoiceStatus(ws_audio_sweep_voice, &lpnStatus);

   write(fp,&lpdwPosition,sizeof(long));
   write(fp,&lpdwFrequency,sizeof(long));
   write(fp,&lpnVolume,sizeof(unsigned int));
   write(fp,&lpnPanning,sizeof(unsigned int));
   write(fp,&lpnStatus,sizeof(int));
#endif
}
