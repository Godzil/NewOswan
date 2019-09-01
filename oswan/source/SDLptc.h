
/* Some simple emulation classes to get PTC code running on SDL */

#include <stdlib.h>
#include <string.h>

#include "SDL.h"

typedef Uint8 char8;
typedef Sint32 int32;

#define randomize()	srand(time(NULL))
#define random(max)	(rand()%(max))

#ifndef stricmp
#define stricmp strcasecmp
#endif

class Error {

public:
	Error(const char *message) {
		strcpy(_message, message);
	}

	void report(void) {
		printf("Error: %s\n", _message);
	}

private:

	char _message[1024];
};

class Area {

public:
	Area(int left, int top, int right, int bottom) {
		_left = left;
		_top = top;
		_right = right;
		_bottom = bottom;
	}

	int left(void) const {
		return(_left);
	}
	int right(void) const {
		return(_right);
	}
	int top(void) const {
		return(_top);
	}
	int bottom(void) const {
		return(_bottom);
	}
	int width(void) const {
		return(_right-_left);
	}
	int height(void) const {
		return(_bottom-_top);
	}

private:
	int _left, _top, _right, _bottom;
};

	
class Format {

public:
	Format(int bpp, int maskR = 0, int maskG = 0, int maskB = 0) {
		_bpp = bpp;
		_maskR = maskR;
		_maskG = maskG;
		_maskB = maskB;
	}

	Uint8 BPP(void) const { return(_bpp); }
	Uint32 MaskR(void) const { return(_maskR); }
	Uint32 MaskG(void) const { return(_maskG); }
	Uint32 MaskB(void) const { return(_maskB); }

private:
	Uint8 _bpp;
	Uint32 _maskR, _maskG, _maskB;
};

class Surface {

public:
	Surface(int w, int h, const Format &format) {
		surface = SDL_AllocSurface(SDL_SWSURFACE, w, h, format.BPP(),
				format.MaskR(),format.MaskG(),format.MaskB(),0);
		if ( surface == NULL ) {
			throw Error(SDL_GetError());
		}
		nupdates = 0;
		is_console = 0;
	}
	Surface(void) {
		nupdates = 0;
		is_console = 1;
	}
	~Surface() {
		if ( ! is_console ) {
			SDL_FreeSurface(surface);
		}
	}

	virtual int width(void) {
		return surface->w;
	}
	virtual int height(void) {
		return surface->h;
	}
	virtual int pitch(void) {
		return surface->pitch;
	}

	virtual void palette(int32 *pcolors) {
		SDL_Color colors[256];

		for ( int i=0; i<256; ++i ) {
			colors[i].r = (pcolors[i]>>16)&0xFF;
			colors[i].g = (pcolors[i]>>8)&0xFF;
			colors[i].b = (pcolors[i]>>0)&0xFF;
		}
		SDL_SetColors(surface, colors, 0, 256);
	}

	virtual void *lock(void) {
		if ( SDL_MUSTLOCK(surface) ) {
			while ( SDL_LockSurface(surface) < 0 ) {
				SDL_Delay(10);
			}
		}
		return (Uint8 *)surface->pixels;
	}

	virtual void unlock(void) {
		if ( SDL_MUSTLOCK(surface) ) {
			SDL_UnlockSurface(surface);
		}
	}

	virtual void copy(Surface &dst,
				const Area &srcarea, const Area &dstarea) {
		SDL_Rect srcrect, dstrect;
		srcrect.x = srcarea.left();
		srcrect.y = srcarea.top();
		srcrect.w = srcarea.width();
		srcrect.h = srcarea.height();
		dstrect.x = dstarea.left();
		dstrect.y = dstarea.top();
		dstrect.w = dstarea.width();
		dstrect.h = dstarea.height();
		SDL_BlitSurface(surface, &srcrect, dst.surface, &dstrect);
		dst.updates[dst.nupdates++] = dstrect;
	}
	virtual void copy(Surface &dst) {
		SDL_Rect srcrect, dstrect;
		srcrect.x = 0;
		srcrect.y = 0;
		srcrect.w = surface->w;
		srcrect.h = surface->h;
		dstrect.x = 0;
		dstrect.y = 0;
		dstrect.w = surface->w;
		dstrect.h = surface->h;
		SDL_LowerBlit(surface, &srcrect, dst.surface, &dstrect);
		dst.updates[dst.nupdates++] = dstrect;
	}

	virtual void update(void) {
		SDL_UpdateRects(surface, nupdates, updates);
		nupdates = 0;
	}

protected:
	SDL_Surface *surface;
	int nupdates;
	SDL_Rect updates[1];		/* Definitely increase this.. */
	int is_console;
};

class Console : public Surface {
	int fullscreen;
public:
	Console() : Surface() {
		fullscreen=0;
	}
	~Console() {
		
		SDL_Quit();
	}
	void close(void)
	{
		SDL_Quit();
	}
	void option(char *option)
	{
		if (!stricmp(option,"fullscreen output"))
			fullscreen=1;
		else
		if (!stricmp(option,"windowed output"))
			fullscreen=0;
	}
	void open(const char *title, int width, int height, const Format &format) {
		Uint32 flags;

		if ( SDL_InitSubSystem(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0 ) {
			throw Error(SDL_GetError());
		}
		//flags = (SDL_HWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN);
		flags = (SDL_HWSURFACE|SDL_HWPALETTE);
		if (fullscreen)
			flags|=SDL_FULLSCREEN;
		surface = SDL_SetVideoMode(width, height, 0, flags);
		if ( surface == NULL ) {
			throw Error(SDL_GetError());
		}
		SDL_WM_SetCaption(title, title);
	}

	int key(void) {
		SDL_Event event;
		int keyevent;

		keyevent = 0;
		while ( SDL_PollEvent(&event) ) {
			/* Real key events trigger this function */
			if ( event.type == SDL_KEYDOWN ) {
				keyevent = 1;
			}
			/* So do quit events -- let the app know about it */
			if ( event.type == SDL_QUIT ) {
				keyevent = 1;
			}
		}
		return(keyevent);
	}

private:

};
