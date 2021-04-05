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

#ifndef __ROM_H__
#define __ROM_H__

#include <stdint.h>

#define WS_ROM_SIZE_2MBIT		1
#define WS_ROM_SIZE_4MBIT		2
#define WS_ROM_SIZE_8MBIT		3
#define WS_ROM_SIZE_16MBIT		4
#define WS_ROM_SIZE_24MBIT		5
#define WS_ROM_SIZE_32MBIT		6
#define WS_ROM_SIZE_48MBIT		7
#define WS_ROM_SIZE_64MBIT		8
#define WS_ROM_SIZE_128MBIT		9

#define WS_EEPROM_SIZE_NONE		0
#define WS_SRAM_SIZE_NONE		0
#define WS_EEPROM_SIZE_64k		1
#define WS_EEPROM_SIZE_256k		2
#define WS_SRAM_SIZE_1k			10
#define WS_SRAM_SIZE_16k		20
#define WS_SRAM_SIZE_8k			50


typedef struct ws_romHeaderStruct
{
   uint8_t	developperId;
   uint8_t	minimumSupportSystem;
   uint8_t	cartId;
   uint8_t	romSize;
   uint8_t	eepromSize;
   uint8_t	additionnalCapabilities;
   uint8_t	realtimeClock;
   uint16_t	checksum;
} ws_romHeaderStruct;


uint8_t				*ws_rom_load(char *path, uint32_t *romSize);
void				ws_rom_dumpInfo(uint8_t *wsrom, uint32_t wsromSize);
ws_romHeaderStruct	*ws_rom_getHeader(uint8_t *wsrom, uint32_t wsromSize);
uint32_t				ws_rom_sramSize(uint8_t *wsrom, uint32_t wsromSize);
uint32_t				ws_rom_eepromSize(uint8_t *wsrom, uint32_t wsromSize);

#endif

