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
    case 0x14:
        break;

    case 0x15:
        Log(TLOG_DEBUG, "io", "Icons %c %c %c %c %c %c %c %c", (value >> 7) & 1 ? '?' : ' ', (value >> 6) & 1 ? '?' : ' ',
               (value >> 5) & 1 ? '3' : ' ', (value >> 4) & 1 ? '2' : ' ', (value >> 3) & 1 ? '1' : ' ',
               (value >> 2) & 1 ? 'H' : ' ', (value >> 1) & 1 ? 'V' : ' ', (value >> 0) & 1 ? 'S' : ' ');
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

#endif
