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

#ifndef __MEMORY_H__
#define __MEMORY_H__

extern uint8	*ws_staticRam;
extern uint8	*internalRam;
extern uint8	*externalEeprom;

void	ws_memory_init(uint8 *rom, uint32 romSize);
void	ws_memory_reset(void);
uint8	*memory_getRom(void);
uint32	memory_getRomSize(void);
uint16	memory_getRomCrc(void);
void	ws_memory_done(void);
void memory_load(int fp);
void memory_save(int fp);

void ws_sram_load(char *path);
void ws_sram_save(char *path);

void dump_memory();

#define BW_IEEPROM_SIZE (128)
#define COLOR_IEEPROM_SIZE (2048)

#endif

