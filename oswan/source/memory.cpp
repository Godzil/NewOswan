////////////////////////////////////////////////////////////////////////////////
// Memory
////////////////////////////////////////////////////////////////////////////////
// Notes: need to optimize cpu_writemem20
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "log.h"
#include "rom.h"
#include "./nec/nec.h"
#include "io.h"
#include "gpu.h"
#include "audio.h"
#include "memory.h"

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
#define IO_ROM_BANK_BASE_SELECTOR   0xC0

uint8 *ws_rom;
uint8 *ws_staticRam;
uint8 *internalRam;
uint8 *externalEeprom;
char *internalBWIRom;
char *internalColorIRom;

char *internalBWEeprom;
char *internalColorEeprom;

uint16_t *internalEeprom;

extern uint8 *ws_ioRam;

uint16  ws_rom_checksum;

uint8   ws_haveColorIRom;
uint8   ws_haveBWIRom;

uint32  sramAddressMask;
uint32  externalEepromAddressMask;
uint32  romAddressMask;
uint32  romSize;

int ws_sram_dirty = 0;

extern nec_Regs I;

void dump_memory()
{
   int i;
   FILE *fp;
   printf("Dumping memory....\n");
   fp = fopen("iram.bin", "wb");
   fwrite(internalRam, 1, 0x10000, fp);
   fclose(fp);

   fp = fopen("sram.bin", "wb");
   fwrite(ws_staticRam, 1, 0x10000, fp);
   fclose(fp);

   fp = fopen("rom.bin", "wb");
   fwrite(ws_rom, 1, romSize, fp);
   fclose(fp);

   fp = fopen("memorydump.bin", "wb");
   fwrite(internalRam, 1, 0x10000, fp);
   /* page 1 */
   fwrite(&(ws_staticRam[0 & sramAddressMask]), 1, 0x10000, fp);
   fwrite(&(ws_rom[((ws_ioRam[IO_ROM_BANK_BASE_SELECTOR+2]&((romSize>>16)-1))<<16)]), 1, 0x10000, fp);
   fwrite(&(ws_rom[((ws_ioRam[IO_ROM_BANK_BASE_SELECTOR+3]&((romSize>>16)-1))<<16)]), 1, 0x10000, fp);

   for(i = 4; i < 0x10; i++)
   {
      int romBank=(256-(((ws_ioRam[IO_ROM_BANK_BASE_SELECTOR]&0xf)<<4)|(i&0xf)));
      fwrite(&(ws_rom[(unsigned)(romSize-(romBank<<16))]), 1, 0x10000, fp);
   }

   fclose(fp);

   fp = fopen("registers.bin", "wb");
   fwrite(ws_ioRam, 1, 256, fp);
   fclose(fp);

   fp = fopen("cpuregs.bin", "wb");
   /* CS */
   fwrite(&I.sregs[CS], 1, 2, fp);
   /* IP */
   fwrite(&I.ip, 1, 2, fp);
   fclose(fp);
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
void cpu_writemem20(DWORD addr,BYTE value)
{
   uint32   offset=addr&0xffff;
   uint32   bank=addr>>16;

   // 0 - RAM - 16 KB (WS) / 64 KB (WSC) internal RAM
   if (!bank)
   {
      ws_gpu_write_byte(offset,value);
      ws_audio_write_byte(offset,value);
   }
   else

      // 1 - SRAM (cart)
      if (bank==1)
      {
         ws_staticRam[offset&sramAddressMask]=value;
         ws_sram_dirty = 1;
      }

   // other banks are read-only
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
BYTE cpu_readmem20(DWORD addr)
{
   uint32 offset=addr&0xffff;
   uint32 bank=addr>>16;
   //uint16 romBank;
   uint8_t hwReg;
   uint32_t temp;
   BYTE ret;

   switch (bank)
   {
   case 0:     // 0 - RAM - 16 KB (WS) / 64 KB (WSC) internal RAM
      if (ws_gpu_operatingInColor)
      {
         return(internalRam[offset]);
      }
      else if (offset<0x4000)
      {
         return(internalRam[offset]);
      }

      return(0x90);

   case 1:     // 1 - SRAM (cart)
      return ws_staticRam[offset&sramAddressMask];

   case 2:
      // Bank 2
      hwReg = ws_ioRam[0xC2];
      temp = hwReg << 16;
      temp += offset;
      temp &= (romSize - 1);
      return ws_rom[temp];
   case 3:
      hwReg = ws_ioRam[0xC3];
      temp = hwReg << 16;
      temp += offset;
      temp &= (romSize - 1);
      return ws_rom[temp];

   case 0xF:
      hwReg = ws_ioRam[0xA0];

      if (!(hwReg & 1))
      {
         if (ws_gpu_operatingInColor && ws_haveColorIRom) 
         {
            if (addr >= 0xFE000)
            {
               ret = internalColorIRom[addr & ~0xFE000];
               return ret;
            }
         }
         else if (!ws_gpu_operatingInColor && ws_haveBWIRom)
         {
            if (addr >= 0xFF000)
            {
               ret = internalBWIRom[addr & ~0xFF000];
               return ret;
            }
         }
      }
      // fall through

   default:
      hwReg = ws_ioRam[0xC0];
      temp = hwReg << 20;
      temp += addr & 0xFFFFF;
      temp &= (romSize - 1);
      return ws_rom[temp];      
   }

   return(0xff);
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
char *load_file(char *filename)
{
    int fd;
    char *ret_ptr;
    struct stat FileStat;

    fd = open(filename, O_RDWR);

    fstat(fd, &FileStat);

    printf("Trying to load %s, size = %lu...\n",filename, FileStat.st_size);

    ret_ptr = (char *)mmap(NULL, FileStat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    close(fd);

    if (ret_ptr == MAP_FAILED)
    {
        ret_ptr = NULL;
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
char *create_file(char *filename, uint32_t size)
{
    int fd;
    uint32_t i;
    char *ret_ptr;
    char buf[] = { 0 };

    printf("Trying to create %s, size = %u...\n",filename, size);
    fd = open(filename, O_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | O_TRUNC);
    fchmod(fd, 0644);
    close(fd);
    sync();

    fd = open(filename, O_RDWR);

    for(i = 0; i < size; i++)
    {
      write(fd, buf, 1);
    }

    close(fd);
    sync();    

    fd = open(filename, O_RDWR);
    ret_ptr = (char *)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    close(fd);

    if (ret_ptr == MAP_FAILED)
    {
        ret_ptr = NULL;
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
void ws_memory_init(uint8 *rom, uint32 wsRomSize)
{
   ws_romHeaderStruct   *ws_romHeader;

   ws_rom=rom;
   romSize=wsRomSize;
   ws_romHeader=ws_rom_getHeader(ws_rom,romSize);
   ws_rom_checksum=ws_romHeader->checksum;
   internalRam=(uint8*)malloc(0x10000);
   ws_staticRam=(uint8*)malloc(0x10000);
   externalEeprom=(uint8*)malloc(131072);//ws_rom_eepromSize(ws_rom,romSize));
   sramAddressMask=ws_rom_sramSize(ws_rom,romSize)-1;
   externalEepromAddressMask=ws_rom_eepromSize(ws_rom,romSize)-1;

   internalBWIRom = load_file("ws_irom.bin");
   internalColorIRom  = load_file("wsc_irom.bin");

   internalBWEeprom = load_file("ws_ieeprom.bin");
   if ( internalBWEeprom == NULL )
   {
      internalBWEeprom = create_file("ws_ieeprom.bin", BW_IEEPROM_SIZE);
   }

   internalColorEeprom = load_file("wsc_ieeprom.bin");
   if ( internalColorEeprom == NULL )
   {
      internalColorEeprom = create_file("wsc_ieeprom.bin", COLOR_IEEPROM_SIZE);
   }

   internalEeprom = (uint16_t *)internalBWEeprom;
   if (ws_gpu_operatingInColor)
   {
      internalEeprom = (uint16_t *)internalColorEeprom;
   }
   
   ws_haveBWIRom = false;
   ws_haveColorIRom = false;

   if (internalBWIRom != NULL)
   {
      printf("Color IROM Found!\n");
      ws_haveBWIRom = true;
   }
   if (internalColorIRom != NULL)
   {
      printf("B&W IROM Found!\n");
      ws_haveColorIRom = true;
   }

   romAddressMask=romSize-1;
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
void ws_memory_reset(void)
{
   memset(internalRam,0,0x10000);
   //memset(ws_staticRam,0,0x10000); // should the sram really be cleared? ...
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
void ws_memory_done(void)
{
   free(ws_rom);
   free(ws_staticRam);
   free(internalRam);
   free(externalEeprom);
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
uint8 *memory_getRom(void)
{
   return(ws_rom);
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
uint32   memory_getRomSize(void)
{
   return(romSize);
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
uint16   memory_getRomCrc(void)
{
   return(ws_rom_checksum);
}


void ws_sram_load(char *path)
{
   FILE *f;
   //size_t read;

   f = fopen(path, "r");

   if (NULL == f)
   {
      memset(ws_staticRam, 0, 0x10000);
      return;
   }

   /*read = */fread(ws_staticRam, 1, 0x8000, f);
   //fprintf(log_get(), "read 0x%x (of 0x%x?) bytes of save ram from %s\n", read, ws_rom_sramSize(ws_rom, romSize), path);
   fclose(f);
}

void ws_sram_save(char *path)
{
   FILE *f;
   //size_t wrote;

   f = fopen(path, "wb");

   if (NULL == f)
   {
      fprintf(log_get(), "error opening %s for writing save ram. (%s)\n", path, strerror(errno));
      return;
   }

   /*wrote = */fwrite(ws_staticRam, 1, 0x8000, f);
   fflush(f);
   //fprintf(log_get(), "wrote 0x%x bytes of save ram to %s\n", wrote, path);
   fclose(f);
}

