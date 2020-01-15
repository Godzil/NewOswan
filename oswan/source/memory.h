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

extern uint8_t	*ws_staticRam;
extern uint8_t	*internalRam;
extern uint8_t	*externalEeprom;

void	ws_memory_init(uint8 *rom, uint32_t romSize);
void	ws_memory_reset(void);
uint8_t	*memory_getRom(void);
uint32_t	memory_getRomSize(void);
uint16_t	memory_getRomCrc(void);
void	ws_memory_done(void);
void memory_load(int fp);
void memory_save(int fp);

char *create_file(char *filename, uint32_t size);
char *load_file(char *filename);

void dump_memory();

#define BW_IEEPROM_SIZE (128)
#define COLOR_IEEPROM_SIZE (2048)

#endif

