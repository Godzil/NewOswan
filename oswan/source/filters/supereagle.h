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
void ws_emulate_SuperEagle(void)
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
				SuperEagle ((u8*)backbuffer,144*2, NULL,(u8*)vs, surfacePitch<<1,144,224);
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
				SuperEagle ((u8*)backbuffer,224*2, NULL,(u8*)vs, surfacePitch<<1,224,144);
				surface->unlock();
				surface->copy(console);
				console.update();
			}
		}
		#include "filter_partE.h"
	}
}
