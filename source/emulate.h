#ifndef EMULATE_H
#define EMULATE_H

#include <stdint.h>

#define		KEY_ENTER	0x0D
#define		KEY_SPACE	0x20
#define		KEY_ESC		0x1b
#define		KEY_UP		0x26
#define		KEY_DOWN	0x28
#define		KEY_LEFT	0x25
#define		KEY_RIGHT	0x27
#define		KEY_BUTTON1 0x57
#define		KEY_BUTTON2 0x58

#define		GUI_COMMAND_NONE				0
#define		GUI_COMMAND_RESET				1
#define		GUI_COMMAND_SCHEME_CHANGE		2
#define		GUI_COMMAND_FILTER_CHANGE		3


extern char		app_window_title[256];
extern int			app_gameRunning;
extern int			app_terminate;
extern int			app_fullscreen;
extern int			app_rotated;


void ws_emulate(void);


#endif /* EMULATE_H */