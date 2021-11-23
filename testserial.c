/******************************************************************************
 * NewOswan
 * testserial.c: A simple tool to test serial in/out
 *
 * Copyright (c) 2014-2021 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

/*
 * The code, as is, was part of a fuzzer for the WonderSwan Tetris game.
 */

/* Serial port */
#define BDR_9600 (0)
#define BDR_38400 (1)
#define SERIAL_PORT "/dev/cu.usbserial-FTE3AXGN"
int serialfd = -1;
int serial_have_data = 0;
unsigned char serial_data = 0;
int serial_speed = BDR_38400;
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
      return;
   
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
   
//   options.c_cflag &= ~CNEW_RTSCTS;
   options.c_cflag |= (CLOCAL | CREAD);
   options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
   
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
      return;
   
   if (serial_have_data == 0)
   {
      f = read(serialfd, buf, 1);
      if (f > 0)
      {
         //printf("Ho [%d]!\n", f);fflush(stdout);
         serial_have_data = 0x01;
         serial_data = buf[0];
      }
   }
}

unsigned char read_serial()
{
   unsigned char buf[10];
   int f;
   if (serialfd < 0)
      return 0xFF;
   if (serial_have_data > 0)
   {
      serial_have_data = 0;
      return serial_data;
   }
   f = read(serialfd, buf, 1);
   
   if (f == 1)
      return buf[0];
   
   return 0x42;
}

void write_serial(unsigned char value)
{
   if (serialfd < 0)
      return;
   write(serialfd, &value, 1);
}

void fuzz(unsigned char *buf)
{
   if ((rand() % 200) < 5)
   {
      switch(rand() % 8)
      {
         case 0: write_serial(0xF6); *buf = rand()%0x10; printf("x"); break;
         case 1: write_serial(0xFB); *buf = rand()%0x10; printf("a"); break;
         //case 2: write_serial(0xF5); *buf = rand()%0xFF; printf("b"); break;
         //case 3: *buf ^= 0x08; printf("c"); break;
         //case 4: *buf ^= 0x10; printf("d"); break;
         //case 5: *buf ^= 0x20; printf("e"); break;
         //case 6: *buf ^= 0x40; printf("f"); break;
         //case 7: *buf ^= 0x80; printf("g"); break;
      }
      fflush(stdout);
   }
  /* else
      printf(" ");*/
}

int main(int argc, char *argv[])
{
   int i = 0;
   unsigned char buf;
   open_serial();
   if (argc > 1)
      set_baudrate(atoi(argv[1]));
   else
      set_baudrate(0);
   
   printf("Run!\n");
   while(1)
   {
      while(serial_have_data == 0) check_serial_data();
      buf = read_serial();
     // if (buf != 0xFC)
      //{
         printf("%02X ", buf); fflush(stdout);
         i++;
         //}
      //fuzz(&buf);  
      if (i >= 32)
      {
         i = 0;
         printf("\n");
      }
      write_serial(buf);
   }
}
