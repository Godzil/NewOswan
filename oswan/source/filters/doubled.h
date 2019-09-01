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
__inline void ws_drawDoubledScanline(int16 *vs, int16 *backbuffer_alias)
{
	register int32	*vs_alias=(int32*)vs;
	register int32	data;

	for (int pixel=0;pixel<224;pixel+=8)
	{
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
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
__inline void ws_drawDoubledRotatedScanline(int16 *vs, int16 *backbuffer_alias)
{
	register int32	*vs_alias=(int32*)vs;
	register int32	data;

	for (int pixel=0;pixel<144;pixel+=8)
	{
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
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
uint16	ws_halfBrightnessTable[32];
#define M_HALFBRIGHTNESS(D) (ws_halfBrightnessTable[(D>>10)&0x1f]<<10)|(ws_halfBrightnessTable[(D>>5)&0x1f]<<5)|(ws_halfBrightnessTable[D&0x1f]);

__inline void ws_drawDoubledHalfBrightnessScanline(int16 *vs, int16 *backbuffer_alias)
{
	register int32	*vs_alias=(int32*)vs;
	register int32	data;
	
	for (int pixel=0;pixel<224;pixel+=4)
	{
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
	}
}

__inline void ws_drawDoubledHalfBrightnessScanlineSpecialEven(int16 *vs, int16 *backbuffer_alias)
{
	register int32	*vs_alias=(int32*)vs;
	register int32	data;
	
	for (int pixel=0;pixel<224;pixel+=4)
	{
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
	}
}
__inline void ws_drawDoubledHalfBrightnessScanlineSpecialOdd(int16 *vs, int16 *backbuffer_alias)
{
	register int32	*vs_alias=(int32*)vs;
	register int32	data;
	
	for (int pixel=0;pixel<224;pixel+=4)
	{
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
	}
}
__inline void ws_drawDoubledHalfBrightnessRotatedScanline(int16 *vs, int16 *backbuffer_alias)
{
	register int32	*vs_alias=(int32*)vs;
	register int32	data;
	
	for (int pixel=0;pixel<144;pixel+=4)
	{
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
	}
}

__inline void ws_drawDoubledHalfBrightnessRotatedScanlineSpecialEven(int16 *vs, int16 *backbuffer_alias)
{
	register int32	*vs_alias=(int32*)vs;
	register int32	data;
	
	for (int pixel=0;pixel<144;pixel+=4)
	{
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
	}
}
__inline void ws_drawDoubledHalfBrightnessRotatedScanlineSpecialOdd(int16 *vs, int16 *backbuffer_alias)
{
	register int32	*vs_alias=(int32*)vs;
	register int32	data;
	
	for (int pixel=0;pixel<144;pixel+=4)
	{
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data|=(data<<16); *vs_alias++=data; 
		data=*backbuffer_alias++; data=M_HALFBRIGHTNESS(data); data|=(data<<16); *vs_alias++=data; 
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
void ws_emulate_doubled(void)
{
	#include "filter_partA.h"
	if (app_rotated)
	{
		surface=new Surface(144*2,224*2,format);
		#include "filter_partB.h"
		console.open(app_window_title,144*2,224*2,format);
		while (1)
		{
				#include "filter_partC.h"
				console.open(app_window_title,144*2,224*2,format);
				#include "filter_partD.h"
				ws_rotate_backbuffer(backbuffer);
				int16 *vs = (int16 *)surface->lock();
				int16	*backbuffer_alias=backbuffer;
				for (int line=0;line<224;line++)
				{
					ws_drawDoubledRotatedScanline(vs,backbuffer_alias);
					vs+=surfacePitch;
					ws_drawDoubledRotatedScanline(vs,backbuffer_alias);
					vs+=surfacePitch;
					backbuffer_alias+=144;
				}
				surface->unlock();
				surface->copy(console);
				console.update();
			}
		}
		#include "filter_partE.h"
	}
	else
	{
		surface=new Surface(224*2,144*2,format);
		#include "filter_partB.h"
		console.open(app_window_title,224*2,144*2,format);
		while (1)
		{
				#include "filter_partC.h"
				console.open(app_window_title,224*2,144*2,format);
				#include "filter_partD.h"
				int16 *vs = (int16 *)surface->lock();
				int16	*backbuffer_alias=backbuffer;
				for (int line=0;line<144;line++)
				{
					ws_drawDoubledScanline(vs,backbuffer_alias);
					vs+=surfacePitch;
					ws_drawDoubledScanline(vs,backbuffer_alias);
					vs+=surfacePitch;
					backbuffer_alias+=224;
				}
				surface->unlock();
				surface->copy(console);
				console.update();
			}
		}
		#include "filter_partE.h"
	}
}
