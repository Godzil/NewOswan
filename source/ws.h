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

#include <stdint.h>

typedef enum wssystem_t {
    WS_SYSTEM_AUTODETECT = 0,
    WS_SYSTEM_MONO,
    WS_SYSTEM_COLOR,
    WS_SYSTEM_CRYSTAL,
} wssystem_t;

extern uint32_t	ws_cyclesByLine;

int		ws_init(char *rompath);
int		ws_rotated(void);
void	ws_set_system(wssystem_t system);
wssystem_t	ws_get_system();
void	ws_reset(void);
int		ws_executeLine(int16_t *framebuffer, int renderLine);
void	ws_patchRom(void);
int		ws_loadState(char *statepath);
int		ws_saveState(char *statepath);
void	ws_done(void);

extern char *ws_sram_path;
extern char *ws_ieep_path;
extern char *ws_rom_path;


#endif
