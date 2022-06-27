/*******************************************************************************
 * NewOswan
 * universal_luxor.c: Implement a "universal" version of both known version
 *     of Luxor (Bandai 2001 and Bandai 2003) as there is no way from the ROM
 *     to really know which version is on the original cart.
 *
 * Created by Manoël Trapier on 19/12/2021.
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#if 0

READ

    case 0xc0 : // ???
        retVal = ((ws_ioRam[0xc0] & 0xf) | 0x20);
        goto exit;

    case 0xD0:
        retVal = 0;
        goto exit;

    case 0xCC:
    case 0xCD:
        retVal = 0;
        break;



WRITE

    case 0xC0:
    {
        /* page 4 to F */
        uint32_t romSize;
        uint8_t *rom = getRom(&romSize);
        for (int i = 0x04 ; i < 0x10 ; i++)
        {
            set_memory_bank(i, ws_get_page_ptr(rom, romSize, (value << 4) + i));
        }

        if (!(ws_ioRam[0xA0] & 0x01))
        {
            set_irom_overlay();
        }
        break;
    }

    case 0xC1:
        /* Sram bank */
        if (sramSize > 0)
        {
            uint32_t sramSize;
            uint8_t *sram = getSram(&sramSize);
            set_memory_bank(0x1, ws_get_page_ptr(sram, sramSize, value));
        }
        break;

    case 0xC2:
    {
        /* page 4 to F */
        uint32_t romSize;
        uint8_t *rom = getRom(&romSize);
        /* Page 2 */
        set_memory_bank(0x2, ws_get_page_ptr(rom, romSize, value));
        break;
    }

    case 0xC3:
    {
        /* page 4 to F */
        uint32_t romSize;
        uint8_t *rom = getRom(&romSize);
        /* Page 3 */
        set_memory_bank(0x3, ws_get_page_ptr(rom, romSize, value));
        break;
    }


#endif