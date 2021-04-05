////////////////////////////////////////////////////////////////////////////////
// GPU
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
// 7.04.2002: Fixed sprites order
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include "log.h"
#include "rom.h"
#include "./nec/nec.h"
#include "io.h"
#include "gpu.h"
#include "ws.h"

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
extern uint8_t *internalRam;

enum VideoModes
{
    DISPLAY_MODE_GRAY = 0, DISPLAY_MODE_2BPP = 4, DISPLAY_MODE_P_4BPP = 7, DISPLAY_MODE_L_4BPP = 6,
};

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
#define RGB555(R, G, B) ((((int)(R))<<10)|(((int)(G))<<5)|((int)(B)))

uint8_t ws_gpu_operatingInColor = 0;
uint8_t ws_videoMode = DISPLAY_MODE_GRAY;
uint8_t ws_gpu_scanline = 0;
int16_t ws_palette[16 * 4];
int8_t ws_paletteColors[8];
int16_t wsc_palette[16 * 16];

// white
#define SHADE_COLOR_RED        1.00
#define SHADE_COLOR_GREEN    1.00
#define SHADE_COLOR_BLUE    1.00

int16_t ws_shades[16] = {
    RGB555(SHADE_COLOR_RED * 30, SHADE_COLOR_GREEN * 30, SHADE_COLOR_BLUE * 30),
    RGB555(SHADE_COLOR_RED * 28, SHADE_COLOR_GREEN * 28, SHADE_COLOR_BLUE * 28),
    RGB555(SHADE_COLOR_RED * 26, SHADE_COLOR_GREEN * 26, SHADE_COLOR_BLUE * 26),
    RGB555(SHADE_COLOR_RED * 24, SHADE_COLOR_GREEN * 24, SHADE_COLOR_BLUE * 24),
    RGB555(SHADE_COLOR_RED * 22, SHADE_COLOR_GREEN * 22, SHADE_COLOR_BLUE * 22),
    RGB555(SHADE_COLOR_RED * 20, SHADE_COLOR_GREEN * 20, SHADE_COLOR_BLUE * 20),
    RGB555(SHADE_COLOR_RED * 18, SHADE_COLOR_GREEN * 18, SHADE_COLOR_BLUE * 18),
    RGB555(SHADE_COLOR_RED * 16, SHADE_COLOR_GREEN * 16, SHADE_COLOR_BLUE * 16),
    RGB555(SHADE_COLOR_RED * 14, SHADE_COLOR_GREEN * 14, SHADE_COLOR_BLUE * 14),
    RGB555(SHADE_COLOR_RED * 12, SHADE_COLOR_GREEN * 12, SHADE_COLOR_BLUE * 12),
    RGB555(SHADE_COLOR_RED * 10, SHADE_COLOR_GREEN * 10, SHADE_COLOR_BLUE * 10),
    RGB555(SHADE_COLOR_RED * 8, SHADE_COLOR_GREEN * 8, SHADE_COLOR_BLUE * 8),
    RGB555(SHADE_COLOR_RED * 6, SHADE_COLOR_GREEN * 6, SHADE_COLOR_BLUE * 6),
    RGB555(SHADE_COLOR_RED * 4, SHADE_COLOR_GREEN * 4, SHADE_COLOR_BLUE * 4),
    RGB555(SHADE_COLOR_RED * 2, SHADE_COLOR_GREEN * 2, SHADE_COLOR_BLUE * 2),
    RGB555(SHADE_COLOR_RED * 0, SHADE_COLOR_GREEN * 0, SHADE_COLOR_BLUE * 0)
};

uint8_t *ws_tile_cache;
uint8_t *ws_hflipped_tile_cache;

uint8_t *wsc_tile_cache;
uint8_t *wsc_hflipped_tile_cache;

uint8_t *ws_modified_tile;
uint8_t *wsc_modified_tile;

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void ws_gpu_init(void)
{
    ws_tile_cache = (uint8_t *)malloc(1024 * 8 * 8);
    wsc_tile_cache = (uint8_t *)malloc(1024 * 8 * 8);

    ws_hflipped_tile_cache = (uint8_t *)malloc(1024 * 8 * 8);
    wsc_hflipped_tile_cache = (uint8_t *)malloc(1024 * 8 * 8);

    ws_modified_tile = (uint8_t *)malloc(1024);
    wsc_modified_tile = (uint8_t *)malloc(1024);

    memset(ws_tile_cache, 0x00, 1024 * 8 * 8);
    memset(wsc_tile_cache, 0x00, 1024 * 8 * 8);

    memset(ws_hflipped_tile_cache, 0x00, 1024 * 8 * 8);
    memset(wsc_hflipped_tile_cache, 0x00, 1024 * 8 * 8);

    memset(ws_modified_tile, 0x01, 1024);
    memset(wsc_modified_tile, 0x01, 1024);
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void ws_gpu_done(void)
{
    free(ws_tile_cache);
    free(wsc_tile_cache);
    free(ws_hflipped_tile_cache);
    free(wsc_hflipped_tile_cache);
    free(ws_modified_tile);
    free(wsc_modified_tile);
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void ws_gpu_changeVideoMode(uint8_t value)
{
    if (ws_videoMode != (value >> 5))
    {
        ws_videoMode = value >> 5;
        memset(ws_modified_tile, 0x01, 1024);
    }
    ws_gpu_operatingInColor = 0;
    if (value & 0x80)
    {
        ws_gpu_operatingInColor = 1;
    }
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void ws_gpu_reset(void)
{
    memset(ws_modified_tile, 0x01, 1024);
    ws_gpu_scanline = 0;
    ws_gpu_changeVideoMode(0x00);
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void ws_gpu_clearCache(void)
{
    memset(ws_modified_tile, 0x01, 1024);
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
uint8_t *ws_tileCache_getTileRow(uint32_t tileIndex, uint32_t line, uint32_t vFlip, uint32_t hFlip, uint32_t bank)
{
    if (ws_gpu_operatingInColor)
    {
        if (bank)
        {
            tileIndex += 512;
        }

        // need to update tile cache ?
        // 4 colors tiles
        if ((ws_videoMode == DISPLAY_MODE_2BPP) && (ws_modified_tile[tileIndex]))
        {
            uint8_t *tileInCachePtr = &wsc_tile_cache[tileIndex << 6];
            uint8_t *hflippedTileInCachePtr = &wsc_hflipped_tile_cache[tileIndex << 6];
            uint16_t *tileInRamPtr = (uint16_t *)&internalRam[0x2000 + (tileIndex << 4)];
            uint16_t tileLine;

            for (int line = 0 ; line < 8 ; line++)
            {
                tileLine = *tileInRamPtr++;

                tileInCachePtr[0] = ((tileLine & 0x80) >> 7) | ((tileLine & 0x8000) >> 14);
                hflippedTileInCachePtr[7] = tileInCachePtr[0];
                tileInCachePtr[1] = ((tileLine & 0x40) >> 6) | ((tileLine & 0x4000) >> 13);
                hflippedTileInCachePtr[6] = tileInCachePtr[1];
                tileInCachePtr[2] = ((tileLine & 0x20) >> 5) | ((tileLine & 0x2000) >> 12);
                hflippedTileInCachePtr[5] = tileInCachePtr[2];
                tileInCachePtr[3] = ((tileLine & 0x10) >> 4) | ((tileLine & 0x1000) >> 11);
                hflippedTileInCachePtr[4] = tileInCachePtr[3];
                tileInCachePtr[4] = ((tileLine & 0x08) >> 3) | ((tileLine & 0x0800) >> 10);
                hflippedTileInCachePtr[3] = tileInCachePtr[4];
                tileInCachePtr[5] = ((tileLine & 0x04) >> 2) | ((tileLine & 0x0400) >> 9);
                hflippedTileInCachePtr[2] = tileInCachePtr[5];
                tileInCachePtr[6] = ((tileLine & 0x02) >> 1) | ((tileLine & 0x0200) >> 8);
                hflippedTileInCachePtr[1] = tileInCachePtr[6];
                tileInCachePtr[7] = ((tileLine & 0x01) >> 0) | ((tileLine & 0x0100) >> 7);
                hflippedTileInCachePtr[0] = tileInCachePtr[7];

                tileInCachePtr += 8;
                hflippedTileInCachePtr += 8;
            }

            ws_modified_tile[tileIndex] = 0;
        }
        else if (wsc_modified_tile[tileIndex])
        {
            // 16 colors by tile layered mode
            if (ws_videoMode == DISPLAY_MODE_L_4BPP)
            {
                uint8_t *tileInCachePtr = &wsc_tile_cache[tileIndex << 6];
                uint8_t *hflippedTileInCachePtr = &wsc_hflipped_tile_cache[tileIndex << 6];
                uint32_t *tileInRamPtr = (uint32_t *)&internalRam[0x4000 + (tileIndex << 5)];
                uint32_t tileLine;

                for (int line = 0 ; line < 8 ; line++)
                {
                    tileLine = *tileInRamPtr++;

                    tileInCachePtr[0] = ((tileLine & 0x00000080) >> 7) | ((tileLine & 0x00008000) >> 14) |
                                        ((tileLine & 0x00800000) >> 21) | ((tileLine & 0x80000000) >> 28);
                    hflippedTileInCachePtr[7] = tileInCachePtr[0];

                    tileInCachePtr[1] = ((tileLine & 0x00000040) >> 6) | ((tileLine & 0x00004000) >> 13) |
                                        ((tileLine & 0x00400000) >> 20) | ((tileLine & 0x40000000) >> 27);
                    hflippedTileInCachePtr[6] = tileInCachePtr[1];

                    tileInCachePtr[2] = ((tileLine & 0x00000020) >> 5) | ((tileLine & 0x00002000) >> 12) |
                                        ((tileLine & 0x00200000) >> 19) | ((tileLine & 0x20000000) >> 26);
                    hflippedTileInCachePtr[5] = tileInCachePtr[2];

                    tileInCachePtr[3] = ((tileLine & 0x00000010) >> 4) | ((tileLine & 0x00001000) >> 11) |
                                        ((tileLine & 0x00100000) >> 18) | ((tileLine & 0x10000000) >> 25);
                    hflippedTileInCachePtr[4] = tileInCachePtr[3];

                    tileInCachePtr[4] = ((tileLine & 0x00000008) >> 3) | ((tileLine & 0x00000800) >> 10) |
                                        ((tileLine & 0x00080000) >> 17) | ((tileLine & 0x08000000) >> 24);
                    hflippedTileInCachePtr[3] = tileInCachePtr[4];

                    tileInCachePtr[5] = ((tileLine & 0x00000004) >> 2) | ((tileLine & 0x00000400) >> 9) |
                                        ((tileLine & 0x00040000) >> 16) | ((tileLine & 0x04000000) >> 23);
                    hflippedTileInCachePtr[2] = tileInCachePtr[5];

                    tileInCachePtr[6] = ((tileLine & 0x00000002) >> 1) | ((tileLine & 0x00000200) >> 8) |
                                        ((tileLine & 0x00020000) >> 15) | ((tileLine & 0x02000000) >> 22);
                    hflippedTileInCachePtr[1] = tileInCachePtr[6];

                    tileInCachePtr[7] = ((tileLine & 0x00000001) >> 0) | ((tileLine & 0x00000100) >> 7) |
                                        ((tileLine & 0x00010000) >> 14) | ((tileLine & 0x01000000) >> 21);
                    hflippedTileInCachePtr[0] = tileInCachePtr[7];

                    tileInCachePtr += 8;
                    hflippedTileInCachePtr += 8;
                }
            }
            else

                // 16 colors by tile packed mode
            if (ws_videoMode == DISPLAY_MODE_P_4BPP)
            {
                uint8_t *tileInCachePtr = &wsc_tile_cache[tileIndex << 6];
                uint8_t *hflippedTileInCachePtr = &wsc_hflipped_tile_cache[tileIndex << 6];
                uint32_t *tileInRamPtr = (uint32_t *)&internalRam[0x4000 + (tileIndex << 5)];
                uint32_t tileLine;

                for (int line = 0 ; line < 8 ; line++)
                {
                    tileLine = *tileInRamPtr++;

                    tileInCachePtr[0] = (tileLine >> 4) & 0x0f;
                    hflippedTileInCachePtr[7] = tileInCachePtr[0];
                    tileInCachePtr[1] = (tileLine >> 0) & 0x0f;
                    hflippedTileInCachePtr[6] = tileInCachePtr[1];
                    tileInCachePtr[2] = (tileLine >> 12) & 0x0f;
                    hflippedTileInCachePtr[5] = tileInCachePtr[2];
                    tileInCachePtr[3] = (tileLine >> 8) & 0x0f;
                    hflippedTileInCachePtr[4] = tileInCachePtr[3];
                    tileInCachePtr[4] = (tileLine >> 20) & 0x0f;
                    hflippedTileInCachePtr[3] = tileInCachePtr[4];
                    tileInCachePtr[5] = (tileLine >> 16) & 0x0f;
                    hflippedTileInCachePtr[2] = tileInCachePtr[5];
                    tileInCachePtr[6] = (tileLine >> 28) & 0x0f;
                    hflippedTileInCachePtr[1] = tileInCachePtr[6];
                    tileInCachePtr[7] = (tileLine >> 24) & 0x0f;
                    hflippedTileInCachePtr[0] = tileInCachePtr[7];

                    tileInCachePtr += 8;
                    hflippedTileInCachePtr += 8;

                }
            }
            else
            {
                // unknown mode
            }

            // tile cache updated
            wsc_modified_tile[tileIndex] = 0;
        }

        if (vFlip)
        {
            line = 7 - line;
        }

        if (hFlip)
        {
            return (&wsc_hflipped_tile_cache[(tileIndex << 6) + (line << 3)]);
        }
        else
        {
            return (&wsc_tile_cache[(tileIndex << 6) + (line << 3)]);
        }

    }
    else
    {
        // need to update tile cache ?
        if (ws_modified_tile[tileIndex])
        {
            uint8_t *tileInCachePtr = &ws_tile_cache[tileIndex << 6];
            uint8_t *hflippedTileInCachePtr = &ws_hflipped_tile_cache[(tileIndex << 6) + 7];
            uint32_t *tileInRamPtr = (uint32_t *)&internalRam[0x2000 + (tileIndex << 4)];
            uint32_t tileLine;

            for (int line = 0 ; line < 4 ; line++)
            {
                tileLine = *tileInRamPtr++;

                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x80) >> 7) | ((tileLine & 0x8000) >> 14);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x40) >> 6) | ((tileLine & 0x4000) >> 13);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x20) >> 5) | ((tileLine & 0x2000) >> 12);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x10) >> 4) | ((tileLine & 0x1000) >> 11);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x08) >> 3) | ((tileLine & 0x0800) >> 10);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x04) >> 2) | ((tileLine & 0x0400) >> 9);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x02) >> 1) | ((tileLine & 0x0200) >> 8);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x01) >> 0) | ((tileLine & 0x0100) >> 7);
                hflippedTileInCachePtr += 16;
                tileLine >>= 16;
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x80) >> 7) | ((tileLine & 0x8000) >> 14);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x40) >> 6) | ((tileLine & 0x4000) >> 13);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x20) >> 5) | ((tileLine & 0x2000) >> 12);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x10) >> 4) | ((tileLine & 0x1000) >> 11);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x08) >> 3) | ((tileLine & 0x0800) >> 10);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x04) >> 2) | ((tileLine & 0x0400) >> 9);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x02) >> 1) | ((tileLine & 0x0200) >> 8);
                *hflippedTileInCachePtr-- = *tileInCachePtr++ = ((tileLine & 0x01) >> 0) | ((tileLine & 0x0100) >> 7);
                hflippedTileInCachePtr += 16;
            }

            // tile cache updated
            ws_modified_tile[tileIndex] = 0;
        }

        if (vFlip)
        {
            line = 7 - line;
        }

        if (hFlip)
        {
            return (&ws_hflipped_tile_cache[(tileIndex << 6) + (line << 3)]);
        }
        else
        {
            return (&ws_tile_cache[(tileIndex << 6) + (line << 3)]);
        }
    }

    return (NULL);
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void ws_drawClippedSpriteLine(int16_t *framebuffer, uint16_t scanline, uint32_t x, uint32_t y, uint32_t tileIndex,
                              uint32_t paletteIndex, uint32_t vFlip, uint32_t hFlip, uint32_t clip_x0, uint32_t clip_y0,
                              uint32_t clip_x1, uint32_t clip_y1)
{

    if ((scanline < y) || (scanline > (y + 7)))
    {
        return;
    }

    if (((x + 7 < clip_x0) || (x >= clip_x1)))
    {
        return;
    }

    if (scanline > clip_y0 && scanline < clip_y1)
    {
        return;
    }

    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileIndex, (scanline - y) & 0x07, hFlip, vFlip, 0);
    uint16_t nbPixels = 8;

    if (x < clip_x0)
    {
        ws_tileRow += clip_x0 - x;
        nbPixels -= clip_x0 - x;
        x = clip_x0;
    }

    if (x + nbPixels > clip_x1)
    {
        nbPixels = (clip_x1 - x);
    }

    framebuffer += x;

    if (ws_gpu_operatingInColor)
    {
        int16_t *wsc_paletteAlias = &wsc_palette[paletteIndex << 4];

        while (nbPixels)
        {
            if (*ws_tileRow)
            {
                *framebuffer = wsc_paletteAlias[*ws_tileRow];
            }

            framebuffer++;
            ws_tileRow++;
            nbPixels--;
        }
    }
    else
    {
        int16_t *ws_paletteAlias = &ws_palette[paletteIndex << 2];

        if (paletteIndex & 0x04)
        {
            while (nbPixels)
            {
                if (*ws_tileRow)
                {
                    *framebuffer = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                }

                framebuffer++;
                ws_tileRow++;
                nbPixels--;
            }
        }
        else
        {
            while (nbPixels)
            {
                *framebuffer = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                framebuffer++;
                ws_tileRow++;
                nbPixels--;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void ws_gpu_renderScanline(int16_t *framebuffer)
{

    if (ws_gpu_scanline > 143)
    {
        return;
    }
    framebuffer += (224 * ws_gpu_scanline);

    // fill with background color
    int16_t backgroundColor;

    if (ws_gpu_operatingInColor)
    {
        backgroundColor = wsc_palette[ws_ioRam[0x01]];
    }
    else
    {
        backgroundColor = ws_shades[ws_paletteColors[ws_palette[((ws_ioRam[0x01] & 0xf0) >> 2) +
                                                                (ws_ioRam[0x01] & 0x03)]]];
    }

    for (int i = 0 ; i < 224 ; i++)
    {
        framebuffer[i] = backgroundColor;
    }

    // render background layer
    if (ws_ioRam[0x00] & 0x01)
    {
        int ws_bgScroll_x = ws_ioRam[0x10];
        int ws_bgScroll_y = ws_ioRam[0x11];

        // seek to the first tile
        ws_bgScroll_y = (ws_bgScroll_y + ws_gpu_scanline) & 0xff;

        // note: byte ordering assumptions!
        int ws_currentTile = (ws_bgScroll_x >> 3);
        uint16_t *ws_bgScrollRamBase = (uint16_t *)(internalRam + (((uint32_t)ws_ioRam[0x07] & 0x0f) << 11) +
                                                    ((ws_bgScroll_y & 0xfff8) << 3));

        int lineInTile = ws_bgScroll_y & 0x07;
        int columnInTile = ws_bgScroll_x & 0x07;

        int16_t *scanlinePtr = framebuffer;

        if (ws_gpu_operatingInColor)
        {
            // render the first clipped tile
            if (columnInTile)
            {
                uint16_t tileInfo = ws_bgScrollRamBase[ws_currentTile & 0x1f];
                ws_currentTile++;
                uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                              tileInfo & 0x4000, tileInfo & 0x2000);
                int16_t *wsc_paletteAlias = &wsc_palette[((tileInfo >> 9) & 0x0f) << 4];
                ws_tileRow += columnInTile;

                for (int i = columnInTile ; i < 8 ; i++)
                {
                    if (*ws_tileRow)
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                }
            }

            // render the tiles between them
            int nbTiles = 28;

            if (columnInTile)
            {
                nbTiles = 27;
            }

            for (int i = 0 ; i < nbTiles ; i++)
            {
                uint16_t tileInfo = ws_bgScrollRamBase[ws_currentTile & 0x1f];
                ws_currentTile++;
                uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                              tileInfo & 0x4000, tileInfo & 0x2000);
                int16_t *wsc_paletteAlias = &wsc_palette[((tileInfo >> 9) & 0x0f) << 4];

                if (*ws_tileRow)
                {
                    *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                }

                scanlinePtr++;
                ws_tileRow++;

                if (*ws_tileRow)
                {
                    *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                }

                scanlinePtr++;
                ws_tileRow++;

                if (*ws_tileRow)
                {
                    *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                }

                scanlinePtr++;
                ws_tileRow++;

                if (*ws_tileRow)
                {
                    *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                }

                scanlinePtr++;
                ws_tileRow++;

                if (*ws_tileRow)
                {
                    *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                }

                scanlinePtr++;
                ws_tileRow++;

                if (*ws_tileRow)
                {
                    *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                }

                scanlinePtr++;
                ws_tileRow++;

                if (*ws_tileRow)
                {
                    *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                }

                scanlinePtr++;
                ws_tileRow++;

                if (*ws_tileRow)
                {
                    *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                }

                scanlinePtr++;
                ws_tileRow++;
            }

            // render the last clipped tile
            if (columnInTile)
            {
                uint16_t tileInfo = ws_bgScrollRamBase[ws_currentTile & 0x1f];
                ws_currentTile++;
                uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                              tileInfo & 0x4000, tileInfo & 0x2000);
                int16_t *wsc_paletteAlias = &wsc_palette[((tileInfo >> 9) & 0x0f) << 4];


                for (int i = 0 ; i < columnInTile ; i++)
                {
                    if (*ws_tileRow)
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                }
            }
        }
        else
        {
            // render the first clipped tile
            if (columnInTile)
            {
                uint16_t tileInfo = ws_bgScrollRamBase[ws_currentTile & 0x1f];
                ws_currentTile++;
                uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                              tileInfo & 0x4000, tileInfo & 0x2000);

                int16_t *ws_paletteAlias = &ws_palette[((tileInfo >> 9) & 0x0f) << 2];
                ws_tileRow += columnInTile;

                if ((tileInfo >> 9) & 0x04)
                {
                    for (int i = columnInTile ; i < 8 ; i++)
                    {
                        if (*ws_tileRow)
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }
                else
                {
                    for (int i = columnInTile ; i < 8 ; i++)
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }
            }

            int nbTiles = 28;

            if (columnInTile)
            {
                nbTiles = 27;
            }

            for (int i = 0 ; i < nbTiles ; i++)
            {
                uint16_t tileInfo = ws_bgScrollRamBase[ws_currentTile & 0x1f];
                ws_currentTile++;
                uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                              tileInfo & 0x4000, tileInfo & 0x2000);
                int16_t *ws_paletteAlias = &ws_palette[((tileInfo >> 9) & 0x0f) << 2];

                if ((tileInfo >> 9) & 0x04)
                {
                    if (*ws_tileRow)
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                }
                else
                {
                    *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    scanlinePtr++;
                    ws_tileRow++;
                    *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    scanlinePtr++;
                    ws_tileRow++;
                    *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    scanlinePtr++;
                    ws_tileRow++;
                    *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    scanlinePtr++;
                    ws_tileRow++;
                    *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    scanlinePtr++;
                    ws_tileRow++;
                    *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    scanlinePtr++;
                    ws_tileRow++;
                    *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    scanlinePtr++;
                    ws_tileRow++;
                    *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    scanlinePtr++;
                    ws_tileRow++;
                }

            }

            // render the last clipped tile
            if (columnInTile)
            {
                uint16_t tileInfo = ws_bgScrollRamBase[ws_currentTile & 0x1f];
                ws_currentTile++;
                uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                              tileInfo & 0x4000, tileInfo & 0x2000);
                int16_t *ws_paletteAlias = &ws_palette[((tileInfo >> 9) & 0x0f) << 2];


                if ((tileInfo >> 9) & 0x04)
                {
                    for (int i = 0 ; i < columnInTile ; i++)
                    {
                        if (*ws_tileRow)
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }
                else
                {
                    for (int i = 0 ; i < columnInTile ; i++)
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }
            }
        }
    }

    // render sprites which are between both layers
    if (ws_ioRam[0x00] & 0x04)
    {
        int ws_sprWindow_x0 = ws_ioRam[0x0c];
        int ws_sprWindow_y0 = ws_ioRam[0x0d];
        int ws_sprWindow_x1 = ws_sprWindow_x0 + ws_ioRam[0x0e];
        int ws_sprWindow_y1 = ws_sprWindow_y0 + ws_ioRam[0x0f];
        uint32_t *ws_sprRamBase = (uint32_t *)(internalRam + (((uint32_t)ws_ioRam[0x04]) << 9));

        // seek to first sprite
        ws_sprRamBase += ws_ioRam[0x06] - 1;

        for (int i = ws_ioRam[0x06] ; i > ws_ioRam[0x05] ; i--)
        {
            uint32_t spr = *ws_sprRamBase--;

            if (!(spr & 0x2000))
            {
                // sprite window on ?
                if ((ws_ioRam[0x00] & 0x08) && (spr & 0x1000) && (ws_sprWindow_x0 != ws_sprWindow_x1))
                {
                    ws_drawClippedSpriteLine(framebuffer, ws_gpu_scanline, (spr & 0xff000000) >> 24,
                                             (spr & 0x00ff0000) >> 16, spr & 0x1ff, 8 + ((spr & 0xe00) >> 9),
                                             spr & 0x4000, spr & 0x8000, ws_sprWindow_x0, ws_sprWindow_y0,
                                             ws_sprWindow_x1, ws_sprWindow_y1);
                }
                else
                {
                    ws_drawClippedSpriteLine(framebuffer, ws_gpu_scanline, (spr & 0xff000000) >> 24,
                                             (spr & 0x00ff0000) >> 16, spr & 0x1ff, 8 + ((spr & 0xe00) >> 9),
                                             spr & 0x4000, spr & 0x8000,
                        //0,0,224,144);
                                             0, 0, 224, 0);
                }
            }
        }
    }

    // render foreground layer
    if (ws_ioRam[0x00] & 0x02)
    {
        int ws_fgWindow_x0 = ws_ioRam[0x08];
        int ws_fgWindow_x1 = ws_ioRam[0x0a];
        int ws_fgScroll_x = ws_ioRam[0x12];
        int ws_fgScroll_y = ws_ioRam[0x13];

        int windowMode = ws_ioRam[0x00] & 0x30;

        // seek to the first tile
        ws_fgScroll_y = (ws_fgScroll_y + ws_gpu_scanline) & 0xff;

        // note: byte ordering assumptions!
        int ws_currentTile = (ws_fgScroll_x >> 3);
        uint16_t *ws_fgScrollRamBase = (uint16_t *)(internalRam + (((uint32_t)ws_ioRam[0x07] & 0xf0) << 7) +
                                                    ((ws_fgScroll_y & 0xfff8) << 3));

        int lineInTile = ws_fgScroll_y & 0x07;
        int columnInTile = ws_fgScroll_x & 0x07;

        int16_t *scanlinePtr = framebuffer;


        // window disabled
        if (!windowMode)
        {
            if (ws_gpu_operatingInColor)
            {
                // render the first clipped tile
                if (columnInTile)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);

                    int16_t *wsc_paletteAlias = &wsc_palette[((tileInfo >> 9) & 0x0f) << 4];

                    ws_tileRow += columnInTile;

                    for (int i = columnInTile ; i < 8 ; i++)
                    {
                        if (*ws_tileRow)
                        {
                            *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                        }

                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }

                // render the tiles between them
                int nbTiles = 28;

                if (columnInTile)
                {
                    nbTiles = 27;
                }

                for (int i = 0 ; i < nbTiles ; i++)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *wsc_paletteAlias = &wsc_palette[((tileInfo >> 9) & 0x0f) << 4];


                    if (*ws_tileRow)
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;

                    if (*ws_tileRow)
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                }

                // render the last clipped tile
                if (columnInTile)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *wsc_paletteAlias = &wsc_palette[((tileInfo >> 9) & 0x0f) << 4];


                    for (int i = 0 ; i < columnInTile ; i++)
                    {
                        if (*ws_tileRow)
                        {
                            *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                        }

                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }
            }
            else
            {
                // render the first clipped tile
                if (columnInTile)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);

                    int16_t *ws_paletteAlias = &ws_palette[((tileInfo >> 9) & 0x0f) << 2];
                    ws_tileRow += columnInTile;

                    if ((tileInfo >> 9) & 0x04)
                    {
                        for (int i = columnInTile ; i < 8 ; i++)
                        {
                            if (*ws_tileRow)
                            {
                                *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                            }

                            scanlinePtr++;
                            ws_tileRow++;
                        }
                    }
                    else
                    {
                        for (int i = columnInTile ; i < 8 ; i++)
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                            scanlinePtr++;
                            ws_tileRow++;
                        }
                    }
                }

                int nbTiles = 28;

                if (columnInTile)
                {
                    nbTiles = 27;
                }

                for (int i = 0 ; i < nbTiles ; i++)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *ws_paletteAlias = &ws_palette[((tileInfo >> 9) & 0x0f) << 2];

                    if ((tileInfo >> 9) & 0x04)
                    {
                        if (*ws_tileRow)
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        scanlinePtr++;
                        ws_tileRow++;

                        if (*ws_tileRow)
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        scanlinePtr++;
                        ws_tileRow++;

                        if (*ws_tileRow)
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        scanlinePtr++;
                        ws_tileRow++;

                        if (*ws_tileRow)
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        scanlinePtr++;
                        ws_tileRow++;

                        if (*ws_tileRow)
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        scanlinePtr++;
                        ws_tileRow++;

                        if (*ws_tileRow)
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        scanlinePtr++;
                        ws_tileRow++;

                        if (*ws_tileRow)
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        scanlinePtr++;
                        ws_tileRow++;

                        if (*ws_tileRow)
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        scanlinePtr++;
                        ws_tileRow++;
                    }
                    else
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        scanlinePtr++;
                        ws_tileRow++;
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        scanlinePtr++;
                        ws_tileRow++;
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        scanlinePtr++;
                        ws_tileRow++;
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        scanlinePtr++;
                        ws_tileRow++;
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        scanlinePtr++;
                        ws_tileRow++;
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        scanlinePtr++;
                        ws_tileRow++;
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        scanlinePtr++;
                        ws_tileRow++;
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }

                // render the last clipped tile
                if (columnInTile)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *ws_paletteAlias = &ws_palette[((tileInfo >> 9) & 0x0f) << 2];


                    if ((tileInfo >> 9) & 0x04)
                    {
                        for (int i = 0 ; i < columnInTile ; i++)
                        {
                            if (*ws_tileRow)
                            {
                                *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                            }

                            scanlinePtr++;
                            ws_tileRow++;
                        }
                    }
                    else
                    {
                        for (int i = 0 ; i < columnInTile ; i++)
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                            scanlinePtr++;
                            ws_tileRow++;
                        }
                    }
                }
            }
        }
        else

            // foreground layer displayed only inside the window
        if (windowMode == 0x20)
        {
            int column = 0;

            if (ws_gpu_operatingInColor)
            {
                // render the first clipped tile
                if (columnInTile)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *wsc_paletteAlias = &wsc_palette[((tileInfo >> 9) & 0x0f) << 4];

                    ws_tileRow += columnInTile;

                    for (int i = columnInTile ; i < 8 ; i++)
                    {
                        if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                        {
                            *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                        }

                        column++;
                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }

                // render the tiles between them
                int nbTiles = 28;

                if (columnInTile)
                {
                    nbTiles = 27;
                }

                for (int i = 0 ; i < nbTiles ; i++)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *wsc_paletteAlias = &wsc_palette[((tileInfo >> 9) & 0x0f) << 4];


                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;
                }

                // render the last clipped tile
                if (columnInTile)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *wsc_paletteAlias = &wsc_palette[((tileInfo >> 9) & 0x0f) << 4];


                    for (int i = 0 ; i < columnInTile ; i++)
                    {
                        if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                        {
                            *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                        }

                        column++;
                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }
            }
            else
            {
                // render the first clipped tile
                if (columnInTile)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);

                    int16_t *ws_paletteAlias = &ws_palette[((tileInfo >> 9) & 0x0f) << 2];
                    ws_tileRow += columnInTile;

                    for (int i = columnInTile ; i < 8 ; i++)
                    {
                        if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        column++;
                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }

                int nbTiles = 28;

                if (columnInTile)
                {
                    nbTiles = 27;
                }

                for (int i = 0 ; i < nbTiles ; i++)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *ws_paletteAlias = &ws_palette[((tileInfo >> 9) & 0x0f) << 2];


                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;
                }

                // render the last clipped tile
                if (columnInTile)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *ws_paletteAlias = &ws_palette[((tileInfo >> 9) & 0x0f) << 2];


                    for (int i = 0 ; i < columnInTile ; i++)
                    {
                        if ((*ws_tileRow) && (column >= ws_fgWindow_x0) && (column <= ws_fgWindow_x1))
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        column++;
                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }
            }
        }
        else

            // foreground layer displayed only outside the window
        if (windowMode == 0x30)
        {
            int column = 0;

            if (ws_gpu_operatingInColor)
            {
                // render the first clipped tile
                if (columnInTile)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *wsc_paletteAlias = &wsc_palette[((tileInfo >> 9) & 0x0f) << 4];

                    ws_tileRow += columnInTile;

                    for (int i = columnInTile ; i < 8 ; i++)
                    {
                        if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                        {
                            *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                        }

                        column++;
                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }

                // render the tiles between them
                int nbTiles = 28;

                if (columnInTile)
                {
                    nbTiles = 27;
                }

                for (int i = 0 ; i < nbTiles ; i++)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *wsc_paletteAlias = &wsc_palette[((tileInfo >> 9) & 0x0f) << 4];


                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;
                }

                // render the last clipped tile
                if (columnInTile)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *wsc_paletteAlias = &wsc_palette[((tileInfo >> 9) & 0x0f) << 4];


                    for (int i = 0 ; i < columnInTile ; i++)
                    {
                        if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                        {
                            *scanlinePtr = wsc_paletteAlias[*ws_tileRow];
                        }

                        column++;
                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }
            }
            else
            {
                // render the first clipped tile
                if (columnInTile)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);

                    int16_t *ws_paletteAlias = &ws_palette[((tileInfo >> 9) & 0x0f) << 2];
                    ws_tileRow += columnInTile;

                    for (int i = columnInTile ; i < 8 ; i++)
                    {
                        if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        column++;
                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }

                int nbTiles = 28;

                if (columnInTile)
                {
                    nbTiles = 27;
                }

                for (int i = 0 ; i < nbTiles ; i++)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *ws_paletteAlias = &ws_palette[((tileInfo >> 9) & 0x0f) << 2];

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;

                    if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                    {
                        *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                    }

                    scanlinePtr++;
                    ws_tileRow++;
                    column++;
                }

                // render the last clipped tile
                if (columnInTile)
                {
                    uint16_t tileInfo = ws_fgScrollRamBase[ws_currentTile & 0x1f];
                    ws_currentTile++;
                    uint8_t *ws_tileRow = ws_tileCache_getTileRow(tileInfo & 0x1ff, lineInTile, tileInfo & 0x8000,
                                                                  tileInfo & 0x4000, tileInfo & 0x2000);
                    int16_t *ws_paletteAlias = &ws_palette[((tileInfo >> 9) & 0x0f) << 2];


                    for (int i = 0 ; i < columnInTile ; i++)
                    {
                        if ((*ws_tileRow) && ((column < ws_fgWindow_x0) || (column > ws_fgWindow_x1)))
                        {
                            *scanlinePtr = ws_shades[ws_paletteColors[ws_paletteAlias[*ws_tileRow]]];
                        }

                        column++;
                        scanlinePtr++;
                        ws_tileRow++;
                    }
                }
            }
        }
        else
        {
            // unknown
        }
    }

    // render sprites
    if (ws_ioRam[0x00] & 0x04)
    {
        int ws_sprWindow_x0 = ws_ioRam[0x0c];
        int ws_sprWindow_y0 = ws_ioRam[0x0d];
        int ws_sprWindow_x1 = ws_ioRam[0x0e];
        int ws_sprWindow_y1 = ws_ioRam[0x0f];
        uint32_t *ws_sprRamBase = (uint32_t *)(internalRam + (((uint32_t)ws_ioRam[0x04]) << 9));

        // seek to first sprite
        ws_sprRamBase += ws_ioRam[0x06] - 1;

        for (int i = ws_ioRam[0x06] ; i > ws_ioRam[0x05] ; i--)
        {
            uint32_t spr = *ws_sprRamBase--;

            if (spr & 0x2000)
            {
                // sprite window on ?
                if ((ws_ioRam[0x00] & 0x08) && (spr & 0x1000) && (ws_sprWindow_x0 != ws_sprWindow_x1))
                {
                    ws_drawClippedSpriteLine(framebuffer, ws_gpu_scanline, (spr & 0xff000000) >> 24,
                                             (spr & 0x00ff0000) >> 16, spr & 0x1ff, 8 + ((spr & 0xe00) >> 9),
                                             spr & 0x4000, spr & 0x8000, ws_sprWindow_x0, ws_sprWindow_y0,
                                             ws_sprWindow_x1, ws_sprWindow_y1);
                }
                else
                {
                    ws_drawClippedSpriteLine(framebuffer, ws_gpu_scanline, (spr & 0xff000000) >> 24,
                                             (spr & 0x00ff0000) >> 16, spr & 0x1ff, 8 + ((spr & 0xe00) >> 9),
                                             spr & 0x4000, spr & 0x8000,
                        //						 0,0,224,144);
                                             0, 0, 224, 0);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void ws_gpu_write_byte(uint32_t offset, uint8_t value)
{
    // ws 4 color tiles
    if ((offset >= 0x2000) && (offset < 0x4000))
    {
        if (internalRam[offset] != value)
        {
            ws_modified_tile[(offset & 0x1fff) >> 4] = 1;
        }

        // update the ram
        internalRam[offset] = value;
    }

    if (ws_gpu_operatingInColor)
    {
        // wsc 16 color tiles bank 0
        if ((offset >= 0x4000) && (offset < 0x8000))
        {
            if (internalRam[offset] != value)
            {
                wsc_modified_tile[(offset & 0x3fff) >> 5] = 1;
            }
        }
        else

            // wsc 16 color tiles bank 1
        if ((offset >= 0x8000) && (offset < 0xC000))
        {
            if (internalRam[offset] != value)
            {
                wsc_modified_tile[512 + ((offset & 0x3fff) >> 5)] = 1;
            }
        }

        // update the ram
        internalRam[offset] = value;

        // palette ram
        if (offset >= 0xfe00)
        {
            // RGB444 format
            uint16_t color = (internalRam[(offset & 0xfffe) + 1]);
            color <<= 8;
            color |= (internalRam[(offset & 0xfffe)]);

            wsc_palette[(offset & 0x1ff) >> 1] = RGB555(((color >> 8) & 0x0f) << 1, ((color >> 4) & 0x0f) << 1,
                                                        (color & 0x0f) << 1);
        }
    }
    else if (offset < 0x4000)
    {
        internalRam[offset] = value;
    }
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
unsigned int ws_gpu_unknownPort;

int ws_gpu_port_write(uint32_t port, uint8_t value)
{
    ws_gpu_unknownPort = 0;

    switch (port)
    {
    case 0x60:
        if (ws_get_system() != WS_SYSTEM_MONO)
        {
            ws_gpu_changeVideoMode(value);
        }
        return 0;

    case 0x1C:
        ws_paletteColors[0] = value & 0xf;
        ws_paletteColors[1] = (value >> 4) & 0xf;
        return 0;

    case 0x1D:
        ws_paletteColors[2] = value & 0xf;
        ws_paletteColors[3] = (value >> 4) & 0xf;
        return 0;

    case 0x1E:
        ws_paletteColors[4] = value & 0xf;
        ws_paletteColors[5] = (value >> 4) & 0xf;
        return 0;

    case 0x1F:
        ws_paletteColors[6] = value & 0xf;
        ws_paletteColors[7] = (value >> 4) & 0xf;
        return 0;

    default:ws_gpu_unknownPort = 1;
    }

    if ((port >= 0x20) && (port <= 0x3f))
    {
        ws_gpu_unknownPort = 0;
        port -= 0x20;
        int paletteIndex = port >> 1;

        if (port & 0x01)
        {
            ws_palette[(paletteIndex << 2) + 2] = value & 0x7;
            ws_palette[(paletteIndex << 2) + 3] = (value >> 4) & 0x7;
        }
        else
        {
            ws_palette[(paletteIndex << 2) + 0] = value & 0x7;
            ws_palette[(paletteIndex << 2) + 1] = (value >> 4) & 0x7;
        }
    }

    return ws_gpu_unknownPort;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
uint8_t ws_gpu_port_read(uint8_t port)
{
    switch (port)
    {
    case 0xa0:
        ws_ioRam[0xA0] |= 0x80;
        if (ws_get_system() == WS_SYSTEM_MONO)
        {
            return ws_ioRam[0xa0] & (~0x2);
        }
        else
        {
            return ws_ioRam[0xa0] | 2;
        }
        break;

    case 0xaa:
        return vblank_count & 0xff;

    case 0xab:
        return (vblank_count >> 8) & 0xff;

    case 0xac:
        return (vblank_count >> 16) & 0xff;

    case 0xad:
        return (vblank_count >> 24) & 0xff;
    }

    return (ws_ioRam[port]);
}
