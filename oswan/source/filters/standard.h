////////////////////////////////////////////////////////////////////////////////
// VERY SLOW !!!!
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void ws_rotate_backbuffer(int16 *backbuffer)
{
	static int16	temp[224*144];
	memcpy(temp,backbuffer,224*144*2);

	for (int line=0;line<144;line++)
		for (int column=0;column<224;column++)
			backbuffer[line+((223-column)<<7)+((223-column)<<4)]=temp[column+(line<<7)+(line<<6)+(line<<5)];
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
void ws_emulate_standard(void)
{
	#include "filter_partA.h"
	if (app_rotated)
	{
		surface=new Surface(144,224,format);
		#include "filter_partB.h"
		surfacePitch>>=1;
		console.open(app_window_title,144,224,format);
		while (1)
		{
				#include "filter_partC.h"
				console.open(app_window_title,144,224,format);
				#include "filter_partD.h"
				ws_rotate_backbuffer(backbuffer);
				int32 *vs = (int32 *)surface->lock();
				memcpy(vs,backbuffer,224*144*2);				
				surface->unlock();
				surface->copy(console);
				console.update();
			}
		}
		#include "filter_partE.h"
	}
	else
	{
		surface=new Surface(224,144,format);
		#include "filter_partB.h"
		console.open(app_window_title,224,144,format);
		while (1)
		{
				#include "filter_partC.h"
				console.open(app_window_title,224,144,format);
				#include "filter_partD.h"
				int32 *vs = (int32 *)surface->lock();
				memcpy(vs,backbuffer,224*144*2);
				surface->unlock();
				surface->copy(console);
				console.update();
			}
		}
		#include "filter_partE.h"
	}
}
