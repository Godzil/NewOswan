////////////////////////////////////////////////////////////////////////////////
// I/O ports
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <sys/mman.h>

#include "log.h"
#include "rom.h"
#include "./nec/nec.h"
#include "./nec/necintrf.h"
#include "initialIo.h"
#include "gpu.h"
#include "audio.h"
#include "memory.h"

#define IO_DUMP

extern uint8_t *externalEeprom;
extern uint32_t romAddressMask;
extern uint16_t *internalEeprom;
extern nec_Regs I;
extern uint64_t nec_monotonicCycles;

enum
{
   EEPROM_SUBCOMMAND = 0,  /* 00 00 */
   EEPROM_WRITE,           /* 01 xx */
   EEPROM_READ,            /* 10 xx */
   EEPROM_ERASE,           /* 11 xx */
   EEPROM_WRITEDISABLE,    /* 00 00 */
   EEPROM_WRITEALL,        /* 00 01 */
   EEPROM_ERASEALL,        /* 00 10 */
   EEPROM_WRITEENABLE      /* 00 11 */
};

char *eii_CommandName[] =
{
   "SUB",
   "WRI",
   "RED",
   "ERA",
   "WRD",
   "WRA",
   "ERL",
   "WRE",
};

uint8_t iee_WriteEnable = false;
uint16_t iee_SelAddress = 0;
uint16_t iee_Databuffer = 0;
uint8_t iee_Mode = EEPROM_READ;

uint8_t cee_WriteEnable = true;
uint16_t cee_SelAddress = 0;
uint16_t cee_Databuffer = 0;
uint8_t cee_Mode = EEPROM_READ;


uint8_t *ws_ioRam=NULL;

uint8_t ws_key_start;
uint8_t ws_key_x4;
uint8_t ws_key_x2;
uint8_t ws_key_x1;
uint8_t ws_key_x3;
uint8_t ws_key_y4;
uint8_t ws_key_y2;
uint8_t ws_key_y1;
uint8_t ws_key_y3;
uint8_t ws_key_button_a;
uint8_t ws_key_button_b;
uint8_t ws_key_flipped;

int      rtcDataRegisterReadCount=0;

FILE *ioLogFp = NULL;

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
void ws_io_reset(void)
{
   ws_key_start=0;
   ws_key_x4=0;
   ws_key_x2=0;
   ws_key_x1=0;
   ws_key_x3=0;
   ws_key_y4=0;
   ws_key_y2=0;
   ws_key_y1=0;
   ws_key_y3=0;
   ws_key_button_a=0;
   ws_key_button_b=0;
   int i;

   for (i=0; i<0x100; i++)
   {
      ws_ioRam[i]= initialIoValue[i];
   }

   rtcDataRegisterReadCount=0;
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
void ws_io_init(void)
{
   if (ws_ioRam==NULL)
   {
      ws_ioRam=(uint8_t*)malloc(0x100);
   }

   ws_io_reset();
   ws_key_flipped=0;

#ifdef IO_DUMP
   ioLogFp = fopen("iodump.csv", "wt");
#endif
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
void ws_io_flipControls(void)
{
   ws_key_flipped=!ws_key_flipped;
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
void ws_io_done(void)
{
   if (ws_ioRam==NULL)
   {
      free(ws_ioRam);
   }

#ifdef IO_DUMP
   fclose(ioLogFp);
#endif
}

/* Serial port */
#define BDR_9600 (0)
#define BDR_38400 (1)
#define SERIAL_PORT "/dev/tty.USA19H141P1.1"
int serialfd = -1;
int serial_have_data = 0;
unsigned char serial_data = 0;
int serial_speed = BDR_9600;
void open_serial()
{
   if (serialfd < 0)
   {
      serialfd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY);

      //set_baudrate(serial_speed);
      serial_have_data = 0;
   }
}

void set_baudrate(int speed)
{
   struct termios options;

   if (serialfd < 0)
   {
      return;
   }

   tcgetattr(serialfd, &options);

   options.c_cflag &= ~PARENB;
   options.c_cflag &= ~CSTOPB;
   options.c_cflag &= ~CSIZE;
   options.c_cflag |= CS8;

   if (speed == BDR_9600)
   {
      cfsetispeed(&options, B9600);
   }
   else
   {
      cfsetospeed(&options, B38400);
   }

#if 0
   options.c_cflag &= ~CNEW_RTSCTS;
#else
   options.c_cflag &= ~CRTSCTS;
#endif
   options.c_cflag |= (CLOCAL | CREAD);

   options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

   options.c_oflag &= ~OPOST;

   tcsetattr(serialfd, TCSANOW, &options);

   /* Make sure read is not blocking */
   fcntl(serialfd, F_SETFL, FNDELAY);
}

void close_serial()
{
   close(serialfd);
   serialfd = -1;
}

void check_serial_data()
{
   unsigned char buf[10];
   int f;

   if (serialfd < 0)
   {
      return;
   }

   if (serial_have_data == 0)
   {
      f = read(serialfd, buf, 1);

      if (f > 0)
      {
         printf("Ho [%d]!\n", f);
         fflush(stdout);
         serial_have_data = 0x01;
         serial_data = buf[0];
      }
   }

   if(serial_have_data > 0)
   {
      /* Gen an int if enabled */
      if(ws_ioRam[0xB2] & 0x04)
      {
         ws_ioRam[0xb6] &= ~ 0x04;
         printf("SERIAL INNNNNTTTT!!!!!!!");
         nec_int((ws_ioRam[0xb0]+3)*4);
      }
   }
}

unsigned char read_serial()
{
   unsigned char buf[10];
   int f;

   if (serialfd < 0)
   {
      return 0xFF;
   }

   if (serial_have_data > 0)
   {
      serial_have_data = 0;
      return serial_data;
   }

   f = read(serialfd, buf, 1);

   if (f == 1)
   {
      return buf[0];
   }

   return 0x42;
}

void write_serial(unsigned char value)
{
   if (serialfd < 0)
   {
      return;
   }

   write(serialfd, &value, 1);
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
uint8_t cpu_readport(uint8_t port)
{
   int w1,w2;
   uint8_t retVal = 0;

/*
   if (port > 0x100)
   {
      port &= 0xFF;
      if (port > 0x100)
      {
         return 0x00;
      }
   }
*/

   switch (port)
   {
   case 0x4e:
   case 0x4f:
   case 0x50:
   case 0x51:
   case 0x80:
   case 0x81:
   case 0x82:
   case 0x83:
   case 0x84:
   case 0x85:
   case 0x86:
   case 0x87:
   case 0x88:
   case 0x89:
   case 0x8a:
   case 0x8b:
   case 0x8c:
   case 0x8d:
   case 0x8e:
   case 0x8f:
   case 0x90:
   case 0x91:
   case 0x92:
   case 0x93:
   case 0x94:
      retVal = ws_audio_port_read(port);
      break;

   case 0xb5:
      w1=ws_ioRam[0xb5];

      if(w1&0x40)
      {
         w2=0x00;
         w2=(ws_key_start<<1)|(ws_key_button_a<<2)|(ws_key_button_b<<3);
         retVal = (uint8_t)((w1&0xf0)|w2);
         break;
      }

      if(w1&0x20)
      {
         w2=0x00;
         w2=(ws_key_x1<<0)|(ws_key_x2<<1)|(ws_key_x3<<2)|(ws_key_x4<<3);
         retVal = (uint8_t)((w1&0xf0)|w2);
         break;
      }

      if(w1&0x10)
      {
         w2=0x00;
         w2=(ws_key_y1<<0)|(ws_key_y2<<1)|(ws_key_y3<<2)|(ws_key_y4<<3);
         retVal = (uint8_t)((w1&0xf0)|w2);
      }

      break;

   case 0xbe:  // internal eeprom status/command register

      // ack eeprom write
      if(ws_ioRam[0xbe]&0x20)
      {
         retVal = ws_ioRam[0xbe]|2;
         break;
      }

      // ack eeprom read
      if(ws_ioRam[0xbe]&0x10)
      {
         retVal = ws_ioRam[0xbe]|1;
         break;
      }

      // else ack both
      retVal = ws_ioRam[0xbe]|3;
      break;

   case 0xba:  // eeprom even byte read
      retVal = iee_Databuffer & 0x00FF;
      break;

   case 0xbb:  // eeprom odd byte read
      retVal = (iee_Databuffer & 0xFF00) >> 8;
      break;

   case 0xc0 : // ???
      retVal = ((ws_ioRam[0xc0]&0xf)|0x20);
      goto exit;


   case 0xC8:
       // ack eeprom write
       if(ws_ioRam[0xbe]&0x20)
       {
           retVal = ws_ioRam[0xbe]|2;
           break;
       }

       // ack eeprom read
       if(ws_ioRam[0xbe]&0x10)
       {
           retVal = ws_ioRam[0xbe]|1;
           break;
       }

       // else ack both
       retVal = ws_ioRam[0xbe]|3;
       break;

   case 0xC4:  // eeprom even byte read
       return cee_Databuffer & 0x00FF;

   case 0xC5:  // eeprom odd byte read
       return (cee_Databuffer & 0xFF00) >> 8;

   case 0xca : // RTC Command and status register
      // set ack to always 1
      retVal =  (ws_ioRam[0xca]|0x80);
      goto exit;

   case 0xcb : // RTC data register

      if(ws_ioRam[0xca]==0x15)   // get time command
      {
         struct tm *newtime;
         time_t long_time;
         time( &long_time );
         newtime = localtime( &long_time );

#define  BCD(value) ((value/10)<<4)|(value%10)

         switch(rtcDataRegisterReadCount)
         {
         case 0:
            rtcDataRegisterReadCount++;
            retVal =  BCD(newtime->tm_year-100);
            goto exit;

         case 1:
            rtcDataRegisterReadCount++;
            retVal =  BCD(newtime->tm_mon);
            goto exit;

         case 2:
            rtcDataRegisterReadCount++;
            retVal = BCD(newtime->tm_mday);
            goto exit;

         case 3:
            rtcDataRegisterReadCount++;
            retVal = BCD(newtime->tm_wday);
            goto exit;

         case 4:
            rtcDataRegisterReadCount++;
            retVal =  BCD(newtime->tm_hour);
            goto exit;

         case 5:
            rtcDataRegisterReadCount++;
            retVal =  BCD(newtime->tm_min);
            goto exit;

         case 6:
            rtcDataRegisterReadCount=0;
            retVal =  BCD(newtime->tm_sec);
            goto exit;
         }

         return 0;
      }
      else
      {
         // set ack
         retVal = (ws_ioRam[0xcb]|0x80);
         goto exit;
      }

   case 0xD0:
      retVal = 0;
      goto exit;

   /* Serial port link.. */
   case 0xB1:
      retVal = read_serial();
      printf("RS232: Read %02X\n", retVal);
      goto exit;

   case 0xB3:
      check_serial_data();

      if (ws_ioRam[0xB3] & 0x80)
      {
         retVal = (ws_ioRam[0xB3] & ~1) | serial_have_data | 0x04;
      }
      else
      {
         retVal = 0x00;
      }

      printf("<<<<RS232STA: %02X [%c%c%cxx%c%c%c]\n", retVal,
             (retVal & 0x80)?'E':'d',
             (retVal & 0x40)?'3':'9',
             (retVal & 0x20)?'R':'n',
             (retVal & 0x04)?'E':'f',
             (retVal & 0x02)?'V':'n',
             (retVal & 0x01)?'D':'e'
            );
      goto exit;

   case 0xCC:
   case 0xCD:
      retVal = 0;
      break;

   default:
      retVal = ws_ioRam[port];
      if (port > 0xD0)
      {
         printf("ReadIO %02X <= %02X\n", port, retVal);
      }
      break;

   case 0xA0:
   case 0xAA:
   case 0xAB:
   case 0xAC:
   case 0xAD:
      retVal = ws_gpu_port_read(port);
      break;

   }


   if (port >= 0xC4)
   {
      printf("ReadMBCIO(%02X) <= %02X\n", port, retVal);
   }


exit:
   if (ioLogFp)
   {
      fprintf(ioLogFp, "%ld, R, %02X, %02X\n", nec_monotonicCycles, port, retVal);
   }
   return retVal;
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
void cpu_writeport(uint32_t port,uint8_t value)
{
   int unknown_io_port=0;


   if (ioLogFp)
   {
      fprintf(ioLogFp, "%ld, W, %02X, %02X\n", nec_monotonicCycles, port, value);
   }

   if (port > 0x100)
   {
      port &= 0xFF;
      if (port > 0x100)
      {
         return;
      }
   }

   if ((port == 0xA0) && (ws_ioRam[port] & 0x01) && (~value & 0x01))
   {
      value |= 0x01;
   }
 
   ws_ioRam[port]=value;

   switch (port)
   {
   /* GPU IOs */
   case 0x00:
      break;

   case 0x04:
   case 0x07:
   case 0x01:
   case 0x02:
   case 0x03:
   case 0x05:
   case 0x06:
   case 0x08:
   case 0x09:
   case 0x0A:
   case 0x0B:
   case 0x0C:
   case 0x0D:
   case 0x0E:
   case 0x0F:
   case 0x10:
   case 0x11:
   case 0x12:
   case 0x13:
   case 0x14:
      break;

   case 0x15:
      printf("Icons %c %c %c %c %c %c %c %c\n",
             (value>>7)&1?'?':' ',
             (value>>6)&1?'?':' ',
             (value>>5)&1?'3':' ',
             (value>>4)&1?'2':' ',
             (value>>3)&1?'1':' ',
             (value>>2)&1?'H':' ',
             (value>>1)&1?'V':' ',
             (value>>0)&1?'S':' '
            );
      break;

   /* Palettes ? */
   case 0x1C:
   case 0x25:
   case 0x2F:
   case 0x38:
   case 0x1D:
   case 0x26:
   case 0x30:
   case 0x39:
   case 0x1E:
   case 0x27:
   case 0x31:
   case 0x3A:
   case 0x1F:
   case 0x28:
   case 0x32:
   case 0x3B:
   case 0x20:
   case 0x29:
   case 0x33:
   case 0x3C:
   case 0x21:
   case 0x2A:
   case 0x34:
   case 0x3E:
   case 0x22:
   case 0x2B:
   case 0x35:
   case 0x3F:
   case 0x23:
   case 0x2C:
   case 0x36:
   case 0x24:
   case 0x2E:
   case 0x37:
      break;

   /* DMAs */
   case 0x40:
   case 0x41:
   case 0x42:
   case 0x43:
   case 0x44:
   case 0x45:
   case 0x46:
   case 0x47:
      break;

   case 0x48:  // DMA

      // bit 7 set to start dma transfer
      if(value&0x80)
      {
         int dma_start = (((uint32_t)ws_ioRam[0x41])<<8)|(((uint32_t)ws_ioRam[0x40]))|(((uint32_t)ws_ioRam[0x42])<<16);
         int dma_end   = (((uint32_t)ws_ioRam[0x45])<<8)|(((uint32_t)ws_ioRam[0x44]))|(((uint32_t)ws_ioRam[0x43])<<16);
         int dma_size  = (((uint32_t)ws_ioRam[0x47])<<8)|(((uint32_t)ws_ioRam[0x46]));

         for(int ix=0; ix<dma_size; ix++)
         {
            cpu_writemem20(dma_end++,cpu_readmem20(dma_start++));
         }

         ws_ioRam[0x47]=0;
         ws_ioRam[0x46]=0;
         ws_ioRam[0x41]=(uint8_t)(dma_start>>8);
         ws_ioRam[0x40]=(uint8_t)(dma_start&0xff);
         ws_ioRam[0x45]=(uint8_t)(dma_end>>8);
         ws_ioRam[0x44]=(uint8_t)(dma_end&0xff);
         ws_ioRam[0x48]=0;
      }

      break;

   /* Audio */
   case 0x4a:
   case 0x4b:
   case 0x4c:
   case 0x4d:
   case 0x4e:
   case 0x4f:
      ws_audio_port_write(port, value);
      break;

   /* DMA Start! */
   case 0x52:
      break;

   /* GPU (again) */
   case 0x60:
      break;

   /* Audio */
   case 0x80:
   case 0x81:
   case 0x82:
   case 0x83:
   case 0x84:
   case 0x85:
   case 0x86:
   case 0x87:
   case 0x88:
   case 0x89:
   case 0x8a:
   case 0x8b:
   case 0x8c:
   case 0x8d:
   case 0x8e:
   case 0x8f:
   case 0x90:
   case 0x91:
   case 0x92:
   case 0x93:
   case 0x94:
   case 0x95:
   case 0x96:
   case 0x97:
   case 0x98:
   case 0x99:
   case 0x9A:
   case 0x9B:
   case 0x9C:
   case 0x9D:
   case 0x9E:
      ws_audio_port_write(port,value);
      break;
      
   /* Hardware */
   case 0xA0:
      /* Force cart handshake to be set */
      ws_ioRam[port] |= 0x80;
      break;

   /* Timers */
   case 0xA2:
   case 0xA4:
   case 0xA5:
   case 0xA6:
   case 0xA7:
   case 0xA8:
   case 0xA9:
   case 0xAA:
   case 0xAB:
      break;

   
   /* Intc */
   case 0xB0:
   case 0xB2:
   case 0xB4:
   case 0xB6:
      break;

   /* Serial */
   case 0xB1:
      write_serial(value);
      break;
   
   case 0xB3:
      printf(">>>>RS232STA: %02X [%c%c%cxx%c%c%c]\n", value,
             (value & 0x80)?'E':'d',
             (value & 0x40)?'3':'9',
             (value & 0x20)?'R':'n',
             (value & 0x04)?'E':'f',
             (value & 0x02)?'V':'n',
             (value & 0x01)?'D':'e'
            );

      /* Serial status: 7 = Enable, 6 = baudrate, 5 = Overrun reset
         2 = Send Buffer empty
         1 = Overrun
         0 = Data Received
       */
      serial_speed = ((value&040) == 0x00)?BDR_9600:BDR_38400;

      if ((value & 0x80) == 0x80)
      {
         open_serial();
         set_baudrate(serial_speed);
         check_serial_data();
      }

      break;

   /* buttons */
   case 0xB5:
      break;

   /* Internal EEPROM */
   case 0xba: /* DATA Low */
      iee_Databuffer = iee_Databuffer & 0xFF00;
      iee_Databuffer = iee_Databuffer | (value);
      break;

   case 0xbb: /* Data High */
      iee_Databuffer = iee_Databuffer & 0x00FF;
      iee_Databuffer = iee_Databuffer | (value << 8);
      break;

   case 0xBC: /* Address Low */
   case 0xBD: /* Address High */
      break;

   case 0xBE: /* Command / Status */
      {
         uint16_t address, command, subcmd;
         
         iee_SelAddress = (ws_ioRam[0xBD] << 8) | ws_ioRam[0xBC];

         if (ws_gpu_operatingInColor)
         {
            /* 
            13 00
               S CCaa AAAA AAAA
            0001 0011 0000 0000

            */
            /* S CC aaAAAAAAAA */
            command = (iee_SelAddress >> 10) & 0x3;
            address = iee_SelAddress & 0x3FF;
            subcmd = (iee_SelAddress >> 8) & 0x03;
         }
         else
         {
            /* S CC aaAAAA */
            command = (iee_SelAddress >> 6) & 0x3;
            address = iee_SelAddress & 0x3F;
            subcmd = (iee_SelAddress >> 4) & 0x03;
         }
         

         if (command == EEPROM_SUBCOMMAND)
         {
            command = EEPROM_WRITEDISABLE + subcmd;
         }

         printf("IEEP: RA:%04X RD:%04X A:%03X C:%s", iee_SelAddress, iee_Databuffer, address, eii_CommandName[command]);

         if (value & 0x40)
         {
            /* Sub command */
            printf(" - Sub");
            if (command == EEPROM_WRITEENABLE)
            {
               printf(" Write Enable\n");
               iee_WriteEnable = true;
            }
            else if (command == EEPROM_WRITEDISABLE)
            {
               printf(" Write Disable\n");
               iee_WriteEnable = false; 
            }
            else if (command == EEPROM_ERASEALL)
            {
               printf(" Erase All\n");
               if (ws_gpu_operatingInColor)
               {
                  memset(internalEeprom, 0, COLOR_IEEPROM_SIZE);
               }
               else
               {
                  memset(internalEeprom, 0, BW_IEEPROM_SIZE);
               }
            }
            else
            {
               printf(" Write All?\n");
            }
         }
         else if (value & 0x20)
         {
            /* Write */
            printf(" - Write?");
            if (iee_WriteEnable)
            {
               printf(" Yes : %04X\n", iee_Databuffer);
               internalEeprom[address] = iee_Databuffer;
            }
            else
            {
               printf(" No\n");
            }
         }
         else if (value & 0x10)
         {
            /* Read */
            printf(" - Read");
            iee_Databuffer = internalEeprom[address];
            printf(" Data : %04X\n", iee_Databuffer);
         }
         else
         {
            printf(" Unknown value: %02X\n", value);
         }
         fflush(stdout);
      }
      break;


   /* MBC */
   case 0xC0:
   case 0xC1:
   case 0xC2:
   case 0xC3:
      break;

   /* Cart EEPROM */
   case 0xC4: /* Data High */
       cee_Databuffer = cee_Databuffer & 0xFF00;
       cee_Databuffer = cee_Databuffer | (value);
       break;

   case 0xC5: /* Data High */
       cee_Databuffer = cee_Databuffer & 0x00FF;
       cee_Databuffer = cee_Databuffer | (value << 8);
       break;

   case 0xC6: /* Address Low */
   case 0xC7: /* Address High */
       break;

   case 0xC8: /* Command / Status */
   {
       uint16_t address, command, subcmd; /*, start;*/

       cee_SelAddress = (ws_ioRam[0xBD] << 8) | ws_ioRam[0xBC];

       /* S CC aaAAAA */
       command = (cee_SelAddress >> 6) & 0x3;
       address = cee_SelAddress & 0x3F;
       subcmd = (cee_SelAddress >> 4) & 0x03;


       if (command == EEPROM_SUBCOMMAND)
       {
           command = EEPROM_WRITEDISABLE + subcmd;
       }

       printf("CEEP: RA:%04X RD:%04X A:%03X C:%s", cee_SelAddress, cee_Databuffer, address, eii_CommandName[command]);

       if (value & 0x40)
       {
           /* Sub command */
           printf(" - Sub");
           if (command == EEPROM_WRITEENABLE)
           {
               printf(" Write Enable\n");
               cee_WriteEnable = true;
           }
           else if (command == EEPROM_WRITEDISABLE)
           {
               printf(" Write Disable\n");
               cee_WriteEnable = false;
           }
           else if (command == EEPROM_ERASEALL)
           {
               printf(" Erase All\n");
               /* Nothing here at the moment */
           }
           else
           {
               printf(" Write All?\n");
           }
       }
       else if (value & 0x20)
       {
           /* Write */
           printf(" - Write?");
           if (cee_WriteEnable)
           {
               printf(" Yes : %04X\n", cee_Databuffer);
               externalEeprom[address] = cee_Databuffer;
           }
           else
           {
               printf(" No\n");
           }
       }
       else if (value & 0x10)
       {
           /* Read */
           printf(" - Read");
           cee_Databuffer = externalEeprom[address];
           printf(" Data : %04X\n", cee_Databuffer);
       }
       else
       {
           printf(" Unknown value: %02X\n", value);
       }
       fflush(stdout);
   }
    break;

   case 0xca:
      if(value==0x15)
      {
         rtcDataRegisterReadCount=0;
      }

      break;
      break;

   case 0xCB:
      break;

   case 0xF0:
      break;

   case 0xF1:
      printf("%d\n", (signed short)((value << 8) | ws_ioRam[0xF0]));
      break;

   case 0xF2:
      printf("%c", value);
      fflush(stdout);
      break;

   case 0xB7:
      break; /* Somwthing to write there, but what? */

   default:
      unknown_io_port=1;
   }

   if ((ws_gpu_port_write(port,value) == 1) && (unknown_io_port == 1))
   {
      fprintf(log_get(),"WriteIO(%02X, %02X) [%04X:%04Xh];\n",port, value, I.sregs[CS], I.ip);
   }

   if (port >= 0xC4)
   {
     fprintf(log_get(),"WriteMBCIO(%02X, %02X);\n",port, value);
   }
}
