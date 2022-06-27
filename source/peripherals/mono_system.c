/*******************************************************************************
 * NewOswan
 * mono_system.c: IOs specific to the original WonderSwan
 *
 * Created by Manoël Trapier on 19/12/2021.
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#if 0
READ


 case 0x62:
        switch (ws_get_system())
        {
        case WS_SYSTEM_AUTODETECT:
        case WS_SYSTEM_MONO:
        case WS_SYSTEM_COLOR:
            retVal = 0x00;
            break;

        case WS_SYSTEM_CRYSTAL:
            retVal = 0x80;
            break;
        }
        break;

WRITE

    if ((port == 0xA0) && (ws_ioRam[port] & 0x01) && (~value & 0x01))
    {
        value |= 0x01;
    }

        case 0xA0:

        /* Force cart handshake to be set */
        ws_ioRam[port] |= 0x80;

        if (value & 0x01)
        {
            Log(TLOG_WARNING, "A0", "Oh yeah %02X BABY", value);
#ifdef USE_PAGED_MEMORY_ACCESS
            uint32_t romSize;
            uint8_t *rom = getRom(&romSize);
            set_memory_bank(0xF, ws_get_page_ptr(rom, romSize, (ws_ioRam[0xC0] & 0x0F << 4) + 0x0F));
#endif
        }
        break;



#endif

#if 0

case 0x62:
        switch (ws_get_system())
        {
        case WS_SYSTEM_AUTODETECT:
        case WS_SYSTEM_MONO:
        case WS_SYSTEM_COLOR:
            retVal = 0x00;
            break;

        case WS_SYSTEM_CRYSTAL:
            retVal = 0x80;
            break;
        }
        break;


   case 0xA0:

        /* Force cart handshake to be set */
        ws_ioRam[port] |= 0x80;

        if (value & 0x01)
        {
            Log(TLOG_WARNING, "A0", "Oh yeah %02X BABY", value);
#ifdef USE_PAGED_MEMORY_ACCESS
            uint32_t romSize;
            uint8_t *rom = getRom(&romSize);
            set_memory_bank(0xF, ws_get_page_ptr(rom, romSize, (ws_ioRam[0xC0] & 0x0F << 4) + 0x0F));
#endif
        }
        break;


#endif