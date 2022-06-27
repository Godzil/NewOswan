/*******************************************************************************
 * NewOswan
 * rtc.c: 
 *
 * Created by Manoël Trapier on 19/12/2021.
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdint.h>
#include <time.h>

static int rtcDataRegisterReadCount = 0;

// TODO: Temporary to let build for now
static uint8_t ws_ioRam[0x100];

uint8_t rtc_io_read(void *pdata, uint8_t port)
{
    uint8_t retVal = 0;

    switch (port)
    {
    case 0xca : // RTC Command and status register
        // set ack to always 1
        retVal = (ws_ioRam[0xca] | 0x80);
        goto exit;

    case 0xcb : // RTC data register

        if (ws_ioRam[0xca] == 0x15)   // get time command
        {
            struct tm *newtime;
            time_t long_time;
            time(&long_time);
            newtime = localtime(&long_time);

#define  BCD(value) ((value/10)<<4)|(value%10)

            switch (rtcDataRegisterReadCount)
            {
            case 0:
                rtcDataRegisterReadCount++;
                retVal = BCD(newtime->tm_year - 100);
                goto exit;

            case 1:
                rtcDataRegisterReadCount++;
                retVal = BCD(newtime->tm_mon);
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
                retVal = BCD(newtime->tm_hour);
                goto exit;

            case 5:
                rtcDataRegisterReadCount++;
                retVal = BCD(newtime->tm_min);
                goto exit;

            case 6:
                rtcDataRegisterReadCount = 0;
                retVal = BCD(newtime->tm_sec);
                goto exit;
            }

            return 0;
        }
        else
        {
            // set ack
            retVal = (ws_ioRam[0xcb] | 0x80);
            goto exit;
        }
    }

exit:
    return retVal;
}

void rtc_io_write(void *pdata, uint8_t port, uint8_t value)
{
    switch(port)
    {
    case 0xca:
        if (value == 0x15)
        {
            rtcDataRegisterReadCount = 0;
        }
        break;
    }
}