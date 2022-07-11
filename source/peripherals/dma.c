/*******************************************************************************
 * NewOswan
 * dma.c: 
 *
 * Created by Manoël Trapier on 19/12/2021.
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#if 0

Need to check how to differenciate all the DMA types.


WRITE:

 case 0x48:  // DMA

        // bit 7 set to start dma transfer
        if (value & 0x80)
        {
            uint32_t dma_start = (ws_ioRam[0x41] << 8) | (ws_ioRam[0x40]) | (ws_ioRam[0x42] << 16);
            uint32_t dma_dest = (ws_ioRam[0x45] << 8) | (ws_ioRam[0x44]) | (ws_ioRam[0x43] << 16);
            uint32_t dma_size = (ws_ioRam[0x47] << 8) | (ws_ioRam[0x46]);

            uint8_t dma_inc = (value & 0x01) ? -1: 1;

            Log(TLOG_VERBOSE, "DMA", "Starting DMA from %08X to %08X (len: %08X, inc: %d)",
                dma_start, dma_dest, dma_size, dma_inc);



            for (uint32_t ix = 0 ; ix < dma_size ; ix++)
            {
                mem_write(dma_dest, mem_read(dma_start));
                dma_start += dma_inc;
                dma_dest += dma_inc;
            }

            ws_ioRam[0x47] = 0;
            ws_ioRam[0x46] = 0;
            ws_ioRam[0x41] = (uint8_t)(dma_start >> 8);
            ws_ioRam[0x40] = (uint8_t)(dma_start & 0xff);
            ws_ioRam[0x45] = (uint8_t)(dma_dest >> 8);
            ws_ioRam[0x44] = (uint8_t)(dma_dest & 0xff);
            ws_ioRam[0x48] = 0;
        }

        break;

#endif



#if 0

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
        if (value & 0x80)
        {
            uint32_t dma_start = (ws_ioRam[0x41] << 8) | (ws_ioRam[0x40]) | (ws_ioRam[0x42] << 16);
            uint32_t dma_dest = (ws_ioRam[0x45] << 8) | (ws_ioRam[0x44]) | (ws_ioRam[0x43] << 16);
            uint32_t dma_size = (ws_ioRam[0x47] << 8) | (ws_ioRam[0x46]);

            uint8_t dma_inc = (value & 0x01) ? -1: 1;

            Log(TLOG_VERBOSE, "DMA", "Starting DMA from %08X to %08X (len: %08X, inc: %d)",
                dma_start, dma_dest, dma_size, dma_inc);



            for (uint32_t ix = 0 ; ix < dma_size ; ix++)
            {
                mem_write(dma_dest, mem_read(dma_start));
                dma_start += dma_inc;
                dma_dest += dma_inc;
            }

            ws_ioRam[0x47] = 0;
            ws_ioRam[0x46] = 0;
            ws_ioRam[0x41] = (uint8_t)(dma_start >> 8);
            ws_ioRam[0x40] = (uint8_t)(dma_start & 0xff);
            ws_ioRam[0x45] = (uint8_t)(dma_dest >> 8);
            ws_ioRam[0x44] = (uint8_t)(dma_dest & 0xff);
            ws_ioRam[0x48] = 0;
        }

        break;

        /* DMA Start! */
    case 0x52:
        break;

#endif