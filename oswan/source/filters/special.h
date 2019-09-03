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
void ws_emulate_special(void)
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

         for (int line=0; line<224; line++)
         {
            ws_drawDoubledHalfBrightnessRotatedScanlineSpecialEven(vs,backbuffer_alias);
            vs+=surfacePitch;
            ws_drawDoubledHalfBrightnessRotatedScanlineSpecialOdd(vs,backbuffer_alias);
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

      while (!ws_executeLine(backbuffer,1));

      int16 *vs = (int16 *)surface->lock();
      int16	*backbuffer_alias=backbuffer;

      for (int line=0; line<144; line++)
      {
         ws_drawDoubledHalfBrightnessScanlineSpecialEven(vs,backbuffer_alias);
         vs+=surfacePitch;
         ws_drawDoubledHalfBrightnessScanlineSpecialOdd(vs,backbuffer_alias);
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
