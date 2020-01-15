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

#ifndef __WS_H__
#define __WS_H__

#define WS_SYSTEM_MONO			0
#define WS_SYSTEM_COLOR			1
#define WS_SYSTEM_AUTODETECT    2

// à supprimer !
extern uint32	ws_cyclesByLine;

int		ws_init(char *rompath);
int		ws_rotated(void);
void	ws_set_colour_scheme(int scheme);
void	ws_set_system(int system);
void	ws_reset(void);
int		ws_executeLine(int16 *framebuffer, int renderLine);
void	ws_patchRom(void);
int		ws_loadState(char *statepath);
int		ws_saveState(char *statepath);
void	ws_done(void);

extern char *ws_sram_path;
extern char *ws_ieep_path;
extern char *ws_rom_path;


#endif
