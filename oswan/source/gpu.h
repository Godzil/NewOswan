//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __GPU_H__
#define __GPU_H__

#define COLOUR_SCHEME_DEFAULT	0
#define COLOUR_SCHEME_AMBER		1
#define COLOUR_SCHEME_GREEN		2

extern	uint8_t	ws_gpu_scanline;
extern	uint8_t	ws_gpu_operatingInColor;
extern	uint8_t	ws_videoMode;
extern	int16_t	ws_palette[16*4];
extern	int8_t	ws_paletteColors[8];
extern	int16_t	wsc_palette[16*16];
extern  unsigned int ws_gpu_unknownPort;

extern	uint32_t	vblank_count;


void ws_gpu_init(void);
void ws_gpu_done(void);
void ws_gpu_reset(void);
void ws_gpu_renderScanline(int16_t *framebuffer);
void ws_gpu_changeVideoMode(uint8_t value);
void ws_gpu_write_byte(uint32_t offset, uint8_t value);
int ws_gpu_port_write(uint32_t port, uint8_t value);
uint8_t ws_gpu_port_read(uint8_t port);
void ws_gpu_set_colour_scheme(int scheme);
void ws_gpu_changeVideoMode(uint8_t value);
void ws_gpu_forceColorSystem(void);
void ws_gpu_forceMonoSystem(void);
void ws_gpu_clearCache(void);

#endif

