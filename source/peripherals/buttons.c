/*
 * NewOswan
 * buttons.c: 
 *
 * Created by Manoël Trapier on 19/12/2021.
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#if 0

case 0xb5:

read
        w1 = ws_ioRam[0xb5];

        if (w1 & 0x40)
        {
            w2 = 0x00;
            w2 = (ws_key_start << 1) | (ws_key_button_a << 2) | (ws_key_button_b << 3);
            retVal = (uint8_t)((w1 & 0xf0) | w2);
            break;
        }

        if (w1 & 0x20)
        {
            w2 = 0x00;
            w2 = (ws_key_x1 << 0) | (ws_key_x2 << 1) | (ws_key_x3 << 2) | (ws_key_x4 << 3);
            retVal = (uint8_t)((w1 & 0xf0) | w2);
            break;
        }

        if (w1 & 0x10)
        {
            w2 = 0x00;
            w2 = (ws_key_y1 << 0) | (ws_key_y2 << 1) | (ws_key_y3 << 2) | (ws_key_y4 << 3);
            retVal = (uint8_t)((w1 & 0xf0) | w2);
        }

write:



#endif