/*******************************************************************************
 * NewOswan
 * eeprom.c:
 *
 * Created by Manoël Trapier on 19/12/2021.
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <log.h>

#if 0

extern uint8_t *externalEeprom;
extern uint16_t *internalEeprom;

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

char *eii_CommandName[] = {
    "SUB", "WRI", "RED", "ERA", "WRD", "WRA", "ERL", "WRE",
};

uint8_t iee_WriteEnable = false;
uint16_t iee_SelAddress = 0;
uint16_t iee_Databuffer = 0;
uint8_t iee_Mode = EEPROM_READ;

uint8_t cee_WriteEnable = true;
uint16_t cee_SelAddress = 0;
uint16_t cee_Databuffer = 0;
uint8_t cee_Mode = EEPROM_READ;

// TODO: temporary
extern uint8_t *ws_ioRam;

uint8_t rs232_io_read(void *pdata, uint8_t port)
{
    uint8_t retVal;
    switch (port)
    {
    case 0xba:  // eeprom even byte read
        retVal = iee_Databuffer & 0x00FF;
        break;

    case 0xbb:  // eeprom odd byte read
        retVal = (iee_Databuffer & 0xFF00) >> 8;
        break;

    case 0xbe:  // internal eeprom status/command register
        // ack eeprom write
        if (ws_ioRam[0xbe] & 0x20)
        {
            retVal = ws_ioRam[0xbe] | 2;
            break;
        }

        // ack eeprom read
        if (ws_ioRam[0xbe] & 0x10)
        {
            retVal = ws_ioRam[0xbe] | 1;
            break;
        }

        // else ack both
        retVal = ws_ioRam[0xbe] | 3;
        break;
    case 0xC8:
        // ack eeprom write
        if (ws_ioRam[0xbe] & 0x20)
        {
            retVal = ws_ioRam[0xbe] | 2;
            break;
        }

        // ack eeprom read
        if (ws_ioRam[0xbe] & 0x10)
        {
            retVal = ws_ioRam[0xbe] | 1;
            break;
        }

        // else ack both
        retVal = ws_ioRam[0xbe] | 3;
        break;
    case 0xC4:  // eeprom even byte read
        return cee_Databuffer & 0x00FF;
    case 0xC5:  // eeprom odd byte read
        return (cee_Databuffer & 0xFF00) >> 8;
    }
}

void rs232_io_write(void *pdata, uint8_t port, uint8_t value)
{
    uint8_t retVal;
    switch (port)
    {
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
#ifdef EEPROM_DEBUG
        printf("IEEP: RA:%04X RD:%04X A:%03X C:%s", iee_SelAddress, iee_Databuffer, address, eii_CommandName[command]);
#endif
        if (value & 0x40)
        {
            /* Sub command */
#ifdef EEPROM_DEBUG
            printf(" - Sub");
#endif
            if (command == EEPROM_WRITEENABLE)
            {
#ifdef EEPROM_DEBUG
                printf(" Write Enable\n");
#endif
                iee_WriteEnable = true;
            }
            else if (command == EEPROM_WRITEDISABLE)
            {
#ifdef EEPROM_DEBUG
                printf(" Write Disable\n");
#endif
                iee_WriteEnable = false;
            }
            else if (command == EEPROM_ERASEALL)
            {
#ifdef EEPROM_DEBUG
                printf(" Erase All\n");
#endif
                if (ws_gpu_operatingInColor)
                {
                    //memset(internalEeprom, 0, COLOR_IEEPROM_SIZE);
                }
                else
                {
                    //memset(internalEeprom, 0, BW_IEEPROM_SIZE);
                }
            }
#ifdef EEPROM_DEBUG
            else
            {
                printf(" Write All?\n");
            }
#endif
        }
        else if (value & 0x20)
        {
            /* Write */
#ifdef EEPROM_DEBUG
            printf(" - Write?");
#endif
            if (iee_WriteEnable)
            {
#ifdef EEPROM_DEBUG
                printf(" Yes : %04X\n", iee_Databuffer);
#endif
                internalEeprom[address] = iee_Databuffer;
            }
#ifdef EEPROM_DEBUG
            else
            {
                printf(" No\n");
            }
#endif
        }
        else if (value & 0x10)
        {
            /* Read */
#ifdef EEPROM_DEBUG
            printf(" - Read");
#endif
            iee_Databuffer = internalEeprom[address];
#ifdef EEPROM_DEBUG
            printf(" Data : %04X\n", iee_Databuffer);
#endif
        }
#ifdef EEPROM_DEBUG
        else
        {
            printf(" Unknown value: %02X\n", value);
        }
#endif
        fflush(stdout);
    }
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
            printf(" Unknown value: %02X@", value);
        }
        fflush(stdout);
    }
        break;

    case 0xCB:
        break;

    }
}

#endif