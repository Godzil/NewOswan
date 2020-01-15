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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "types.h"
#include "log.h"
#include "rom.h"


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
uint8	*ws_rom_load(char *path, uint32 *romSize)
{
    int fd;
    uint8 *ret_ptr;
    struct stat FileStat;

    fd = open(path, O_RDWR);

    fstat(fd, &FileStat);

    *romSize = FileStat.st_size;

    ret_ptr = (uint8 *)mmap(NULL, FileStat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    close(fd);

    if (ret_ptr == MAP_FAILED)
    {
        ret_ptr = NULL;
        *romSize = 0;
    }

    return ret_ptr;
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
void	ws_rom_dumpInfo(uint8 *wsrom, uint32 romSize)
{
   ws_romHeaderStruct		*romHeader=ws_rom_getHeader(wsrom,romSize);

   fprintf(log_get(),"rom: developper Id  0x%.2x\n",romHeader->developperId);
   fprintf(log_get(),"rom: cart Id        0x%.2x\n",romHeader->cartId);
   fprintf(log_get(),"rom: minimum system %s\n",(romHeader->minimumSupportSystem==WS_SYSTEM_MONO)?"Wonderswan mono":"Wonderswan color");
   fprintf(log_get(),"rom: size           %i Mbits\n",(romSize>>20)<<3);
   fprintf(log_get(),"rom: eeprom         ");

   switch (romHeader->eepromSize&0xf)
   {
   case WS_EEPROM_SIZE_NONE:
   {
      fprintf(log_get(),"none\n");
      break;
   }

   case WS_EEPROM_SIZE_64k:
   {
      fprintf(log_get(),"64 kb\n");
      break;
   }

   case WS_EEPROM_SIZE_256k:
   {
      fprintf(log_get(),"256 kb\n");
      break;
   }
   }

   fprintf(log_get(),"rom: sram           ");

   switch (romHeader->eepromSize&0xf0)
   {
   case WS_SRAM_SIZE_NONE:
   {
      fprintf(log_get(),"none\n");
      break;
   }

   case WS_SRAM_SIZE_1k:
   {
      fprintf(log_get(),"1 kb\n");
      break;
   }

   case WS_SRAM_SIZE_16k:
   {
      fprintf(log_get(),"16 kb\n");
      break;
   }

   case WS_SRAM_SIZE_8k:
   {
      fprintf(log_get(),"8 kn\n");
      break;
   }
   }

   fprintf(log_get(),"rom: rtc            %s\n",(romHeader->realtimeClock)?"Yes":"None");
   fprintf(log_get(),"checksum            0x%.4x\n",romHeader->checksum);

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
ws_romHeaderStruct		*ws_rom_getHeader(uint8 *wsrom, uint32 wsromSize)
{
    ws_romHeaderStruct *wsromHeader = (ws_romHeaderStruct *)(wsrom + wsromSize - 10);

   return(wsromHeader);
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
uint32				ws_rom_sramSize(uint8 *wsrom, uint32 wsromSize)
{
   ws_romHeaderStruct		*romHeader=ws_rom_getHeader(wsrom,wsromSize);

   switch (romHeader->eepromSize&0xf0)
   {
   case WS_SRAM_SIZE_NONE:
      return(0);

   case WS_SRAM_SIZE_1k:
      return(0x400);

   case WS_SRAM_SIZE_16k:
      return(0x4000);

   case WS_SRAM_SIZE_8k:
      return(0x2000);
   }

   return(0);
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
uint32				ws_rom_eepromSize(uint8 *wsrom, uint32 wsromSize)
{
   ws_romHeaderStruct		*romHeader=ws_rom_getHeader(wsrom,wsromSize);

   switch (romHeader->eepromSize&0xf)
   {
   case WS_EEPROM_SIZE_NONE:
      return(0);

   case WS_EEPROM_SIZE_64k:
      return(0x10000);

   case WS_EEPROM_SIZE_256k:
      return(0x40000);
   }

   return(0);
}

