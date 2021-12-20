/*
 * NewOswan
 * rs232.c: 
 *
 * Created by Manoël Trapier on 19/12/2021.
 * Copyright (c) 2014-2021 986-Studio. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdint.h>

#include <unistd.h>  /* UNIX standard function definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <sys/mman.h>
#include <fcntl.h>

#include <nec.h>
#include <log.h>

/* Temporary */
extern uint8_t *ws_ioRam;

/* Serial port */
#define BDR_9600 (0)
#define BDR_38400 (1)
#define SERIAL_PORT "/dev/tty.USA19H141P1.1"
static int serialfd = -1;
static int serial_have_data = 0;
static unsigned char serial_data = 0;
static int serial_speed = BDR_9600;

static void open_serial()
{
    if (serialfd < 0)
    {
        serialfd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY);

        //set_baudrate(serial_speed);
        serial_have_data = 0;
    }
}

static void set_baudrate(int speed)
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

static void close_serial()
{
    close(serialfd);
    serialfd = -1;
}

static void check_serial_data()
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
            Log(TLOG_DEBUG, "serial", "Have data from serial [%d]!", f);
            fflush(stdout);
            serial_have_data = 0x01;
            serial_data = buf[0];
        }
    }

    if (serial_have_data > 0)
    {
        /* Gen an int if enabled */
        if (ws_ioRam[0xB2] & 0x04)
        {
            ws_ioRam[0xb6] &= ~0x04;
            Log(TLOG_DEBUG, "serial", "SERIAL INNNNNTTTT!!!!!!!");
            nec_int((ws_ioRam[0xb0] + 3) * 4);
        }
    }
}

static unsigned char read_serial()
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

static void write_serial(unsigned char value)
{
    if (serialfd < 0)
    {
        return;
    }

    write(serialfd, &value, 1);
}

uint8_t rs232_io_read(void *pdata, uint8_t port)
{
    uint8_t retVal;
    switch(port)
    {
    case 0xB1:
        retVal = read_serial();
        Log(TLOG_DEBUG, "serial", "Read %02X", retVal);
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

        Log(TLOG_DEBUG, "serial", "<<<<RS232STA: %02X [%c%c%cxx%c%c%c]", retVal, (retVal & 0x80) ? 'E' : 'd',
            (retVal & 0x40) ? '3' : '9', (retVal & 0x20) ? 'R' : 'n', (retVal & 0x04) ? 'E' : 'f',
            (retVal & 0x02) ? 'V' : 'n', (retVal & 0x01) ? 'D' : 'e');
        goto exit;
    }

exit:
    return retVal;
}

void rs232_io_write(void *pdata, uint8_t port, uint8_t value)
{
    switch(port)
    {
    case 0xB1:
        write_serial(value);
        break;

    case 0xB3:
        Log(TLOG_DEBUG, "serial", ">>>>RS232STA: %02X [%c%c%cxx%c%c%c]", value, (value & 0x80) ? 'E' : 'd', (value & 0x40) ? '3' : '9',
            (value & 0x20) ? 'R' : 'n', (value & 0x04) ? 'E' : 'f', (value & 0x02) ? 'V' : 'n',
            (value & 0x01) ? 'D' : 'e');

        /* Serial status: 7 = Enable, 6 = baudrate, 5 = Overrun reset
           2 = Send Buffer empty
           1 = Overrun
           0 = Data Received
         */
        serial_speed = ((value & 040) == 0x00) ? BDR_9600 : BDR_38400;

        if ((value & 0x80) == 0x80)
        {
            open_serial();
            set_baudrate(serial_speed);
            check_serial_data();
        }

        break;
    }
}

void rs232_init()
{

}