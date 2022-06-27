/*******************************************************************************
 * NewOswan
 * mono_gpu.c: Implementation of the monochrome GPU
 *
 * Created by Manoël Trapier on 14/03/2022.
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#if 0

READ

    case 0xA0:
    case 0xAA:
    case 0xAB:
    case 0xAC:
    case 0xAD:
        retVal = ws_gpu_port_read(port);

WRITE

case 0x00:
        Log(TLOG_DEBUG, "GPU", "Screen enabled: W2:%c W2M:%c, SW:%c, S:%c, S2:%c, S1:%c",
            (value & 0x20)?'Y':'N',
            (value & 0x10)?'I':'O',
            (value & 0x08)?'Y':'N',
            (value & 0x04)?'Y':'N',
            (value & 0x02)?'Y':'N',
            (value & 0x01)?'Y':'N');
        break;

    case 0x04:
        if (ws_gpu_operatingInColor)
        {
            Log(TLOG_DEBUG, "GPU", "Sprite base: %04X", (value & 0x1F) << 9);
        }
        else
        {
            Log(TLOG_DEBUG, "GPU", "Sprite base: %04X", (value & 0x3F) << 9);
        }
        break;
    case 0x07:
        if (ws_gpu_operatingInColor)
        {
            Log(TLOG_DEBUG, "GPU", "Sprite Screen1 base: %04X", (value & 0x7) << 11);
            Log(TLOG_DEBUG, "GPU", "Sprite Screen2 base: %04X", (value & 0x70) << (11-4));
        }
        else
        {
            Log(TLOG_DEBUG, "GPU", "Sprite Screen1 base: %04X", (value & 0xF) << 11);
            Log(TLOG_DEBUG, "GPU", "Sprite Screen2 base: %04X", (value & 0xF0) << (11-4));
        }
        break;
    case 0x10:
        //Log(TLOG_DEBUG, "GPU", "Sprite Screen1 X scroll: %d", value);
        break;
    case 0x11:
        //Log(TLOG_DEBUG, "GPU", "Sprite Screen1 T scroll: %d", value);
        break;
    case 0x12:
        //Log(TLOG_DEBUG, "GPU", "Sprite Screen2 X scroll: %d", value);
        break;
    case 0x13:
        //Log(TLOG_DEBUG, "GPU", "Sprite Screen2 Y scroll: %d", value);
        break;

    case 0x15:
        Log(TLOG_DEBUG, "io", "Icons %c %c %c %c %c %c %c %c", (value >> 7) & 1 ? '?' : ' ', (value >> 6) & 1 ? '?' : ' ',
               (value >> 5) & 1 ? '3' : ' ', (value >> 4) & 1 ? '2' : ' ', (value >> 3) & 1 ? '1' : ' ',
               (value >> 2) & 1 ? 'H' : ' ', (value >> 1) & 1 ? 'V' : ' ', (value >> 0) & 1 ? 'S' : ' ');
        break;




#endif
