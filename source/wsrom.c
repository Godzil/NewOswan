/*******************************************************************************
 * NewOswan
 * wsrom.c:
 *
 *
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <log.h>

#include <file_access.h>
#include <wsrom.h>

static const char *romSizeName[] =
{
    [WSROM_ROMINFO_SIZE_1MBIT] = "1 Mbit",
    [WSROM_ROMINFO_SIZE_2MBIT] = "2 Mbit",
    [WSROM_ROMINFO_SIZE_4MBIT] = "4 Mbit",
    [WSROM_ROMINFO_SIZE_8MBIT] = "8 Mbit",
    [WSROM_ROMINFO_SIZE_16MBIT] = "16 Mbit",
    [WSROM_ROMINFO_SIZE_24MBIT] = "24 Mbit",
    [WSROM_ROMINFO_SIZE_32MBIT] = "32 Mbit",
    [WSROM_ROMINFO_SIZE_48MBIT] = "48 Mbit",
    [WSROM_ROMINFO_SIZE_64MBIT] = "64 Mbit",
    [WSROM_ROMINFO_SIZE_128MBIT] = "128 Mbit",
};

static const char *eepromSizeName[] =
{
    [WSROM_SAVEINFO_EEPROM_SIZE_NONE] = "none",
    [WSROM_SAVEINFO_EEPROM_SIZE_1KBIT] = "1 Kbits",
    [WSROM_SAVEINFO_EEPROM_SIZE_16KBIT] = "16 Kbits",
    [WSROM_SAVEINFO_EEPROM_SIZE_32KBIT] = "32 Kbits",
    [WSROM_SAVEINFO_EEPROM_SIZE_8KBIT] = "8 Kbits",
    [WSROM_SAVEINFO_EEPROM_SIZE_4KBIT] = "4 Kbits",
    [WSROM_SAVEINFO_EEPROM_SIZE_2KBIT] = "2 Kbits",
};

static const char *sramSizeName[] =
{
    [WSROM_SAVEINFO_SRAM_SIZE_NONE] = "none",
    [WSROM_SAVEINFO_SRAM_SIZE_64KBIT] = "64 Kbit",
    [WSROM_SAVEINFO_SRAM_SIZE_256KBIT] = "256 Kkbit",
    [WSROM_SAVEINFO_SRAM_SIZE_1MBIT] = "1 Mbits",
    [WSROM_SAVEINFO_SRAM_SIZE_2MBIT] = "2 Mbits",
    [WSROM_SAVEINFO_SRAM_SIZE_4MBIT] = "4 Mbits",
};

wsrom_game_t *wsrom_loadRom(const char *filepath)
{
    char *savepath;
    char *tmp;
    uint16_t calcChecksum;
    wsrom_game_t *rom = (wsrom_game_t *) calloc(1, sizeof(wsrom_game_t));
    size_t romSize, saveSize, tempSize;

    if (rom == NULL)
    {
        goto exit;
    }

    rom->rom_data = (uint8_t *)load_file(filepath, true, &romSize);
    if (rom->rom_data == NULL)
    {
        Log(TLOG_ERROR, "wsrom", "Loading file '%s' failed..", filepath);
        goto free_and_exit;
    }

    rom->footer = (wsrom_rom_footer_t *)(rom->rom_data + romSize - 16);

    /* A bit of check */
    if (rom->footer->resetOpcode != WSROM_VALID_RESET_OPCODE)
    {
        Log(TLOG_PANIC, "wsrom", "Reset vector is invalid, probably not a WS Game");
        goto unmap_and_exit;
    }

    /* Sanity check */
    if ( wsrom_getRomSize(rom) != romSize)
    {
        Log(TLOG_WARNING, "wsrom", "File size if different from reported ROM size..");
    }

    rom->rom_mask = wsrom_getRomSize(rom) - 1;

    /* Check Checksum */
    if ((calcChecksum = wsrom_getChecksum(rom)) != rom->footer->checksum)
    {
        Log(TLOG_WARNING, "wsrom", "File checksum do not match actual file [%04X : %04X]",
            calcChecksum, rom->footer->checksum);
    }

    /* Open save backing file, create if needed */
    savepath = (char *)malloc(strlen(filepath) + 5);
    strcpy(savepath, filepath);
    tmp = strrchr(savepath, '.');

    if (tmp == NULL)
    {
        Log(TLOG_PANIC, "wsrom", "File '%s' does not have an extension?!", filepath);
        goto save_free_and_exit;
    }

    if ((rom->footer->saveInfo & WSROM_SAVEINFO_SRAM_SIZE_MASK) != WSROM_SAVEINFO_SRAM_SIZE_NONE)
    {
        strcpy(tmp, ".sram");
        rom->saveIsSram = true;
        saveSize = wsrom_getSramSize(rom);
        rom->save_mask = saveSize - 1;
    }
    else if ((rom->footer->saveInfo & WSROM_SAVEINFO_EEPROM_SIZE_MASK) != WSROM_SAVEINFO_EEPROM_SIZE_NONE)
    {
        strcpy(tmp, ".eprom");
        rom->saveIsSram = false;
        saveSize = wsrom_getEepromSize(rom);
        rom->save_mask = saveSize - 1;
    }
    else
    {
        rom->saveIsSram = true;
        saveSize = 0;
        rom->save_mask = 0;
    }

    rom->save_data = NULL;
    tempSize = 0;

    if (saveSize > 0)
    {
        if ((rom->save_data = load_file(savepath, false, &tempSize)) == NULL)
        {
            rom->save_data = create_file(savepath, saveSize);
            tempSize = saveSize;
        }

        if (rom->save_data == NULL)
        {
            Log(TLOG_PANIC, "wsrom", "Cannot open/create save file '%s'", savepath);
            goto save_free_and_exit;
        }
    }

    /* Check save file size just as sanity check */
    if (saveSize != tempSize)
    {
        Log(TLOG_WARNING, "wsrom", "Save file size if different from reported in footer..");
    }

    free(savepath);

    goto exit;

save_free_and_exit:
    free(savepath);

unmap_and_exit:
    close_file(rom->rom_data, romSize);

free_and_exit:
    free(rom);
    rom = NULL;

exit:
    return rom;
}

void wsrom_dumpInfo(wsrom_game_t *rom)
{
    Log(TLOG_NORMAL, "wsrom", "Reset vector: %04X:%04Xh", rom->footer->resetSegment, rom->footer->resetOffset);
    Log(TLOG_NORMAL, "wsrom", "Publisher Id: %04Xh", rom->footer->publisherId);
    Log(TLOG_NORMAL, "wsrom", "Game Id: %02Xh", rom->footer->gameId);
    Log(TLOG_NORMAL, "wsrom", "Rom size: %s", romSizeName[rom->footer->romInfo]);
    Log(TLOG_NORMAL, "wsrom", "Save type: %s", rom->saveIsSram ? "SRAM" : "EEPROM");
    Log(TLOG_NORMAL, "wsrom", "Save size: %s", rom->saveIsSram ? (sramSizeName[rom->footer->saveInfo]) :
                                                                 (eepromSizeName[rom->footer->saveInfo]));
    /* Standard flags */
    Log(TLOG_NORMAL, "wsrom", "ROM access: %s",
        (rom->footer->cartFlags & WSROM_FLAGS_ROMCYCLE_MASK) ? "1 cycles" : "3 cycles");
    Log(TLOG_NORMAL, "wsrom", "ROM data bus: %s",
        (rom->footer->cartFlags & WSROM_FLAGS_DBUSSIZE_MASK) ? "8 bits" : "16 bits");
    Log(TLOG_NORMAL, "wsrom", "RTC is %spresent",
        (rom->footer->cartFlags & WSROM_FLAGS_RTC_MASK) ? "" : "not ");
    Log(TLOG_NORMAL, "wsrom", "Screen orientation: %s",
        (rom->footer->cartFlags & WSROM_FLAGS_DFLT_ORIENTATION_MASK) ? "Vertical" : "Horizontal");

    /* Extended flags cartFlagsExt */
    Log(TLOG_NORMAL, "wsrom", "Want access to internal EEPROM: %s",
        (rom->footer->cartFlagsExt & WSROM_EXTFLAGS_IEEPROM_WRITEENABLE) ? "Yes" : "No");

    /* Boot flags */
    Log(TLOG_NORMAL, "wsrom", "Disable custom bootsplash: %s",
        (rom->footer->cartBootFlags & WSROM_BOOTFLAGS_DISALLOW_BOOTSPLASH) ? "Yes" : "No");
    Log(TLOG_NORMAL, "wsrom", "Can be boot: %s",
        (rom->footer->cartBootFlags & WSROM_BOOTFLAGS_NONBOOTABLE_MASK) ? "No" : "Yes");

    Log(TLOG_NORMAL, "wsrom", "Checksum: %04Xh", rom->footer->checksum);
}

void wsrom_jsonSerialise(FILE *fp, wsrom_game_t *rom)
{
    fprintf(fp, "{\n");
    fprintf(fp, "\"reset\": \"%04X:%04Xh\",\n", rom->footer->resetSegment, rom->footer->resetOffset);
    fprintf(fp, "\"publisher\": %d,\n", rom->footer->publisherId);
    fprintf(fp, "\"title\": %d,\n", rom->footer->gameId);
    fprintf(fp, "\"rom_size\": \"%s\",\n", romSizeName[rom->footer->romInfo]);
    fprintf(fp, "\"sram_save\": %s,\n", rom->saveIsSram ? "true" : "false");
    fprintf(fp, "\"save_size\": \"%s\",\n", rom->saveIsSram ? (sramSizeName[rom->footer->saveInfo]) :
                                               (eepromSizeName[rom->footer->saveInfo]));
    /* Standard flags */
    fprintf(fp, "\"cycle_rom\": %d,\n",
        (rom->footer->cartFlags & WSROM_FLAGS_ROMCYCLE_MASK) ? 1 : 3);
    fprintf(fp, "\"rom_data_bus\": %d,\n",
        (rom->footer->cartFlags & WSROM_FLAGS_DBUSSIZE_MASK) ? 8 : 16);
    fprintf(fp, "\"have_rtc\": %s,\n",
        (rom->footer->cartFlags & WSROM_FLAGS_RTC_MASK) ? "true" : "false");
    fprintf(fp, "\"vertical_screen\": %s,\n",
        (rom->footer->cartFlags & WSROM_FLAGS_DFLT_ORIENTATION_MASK) ? "true" : "false");

    /* Extended flags cartFlagsExt */
    fprintf(fp, "\"access_ieeprom\": %s,\n",
        (rom->footer->cartFlagsExt & WSROM_EXTFLAGS_IEEPROM_WRITEENABLE) ? "true" : "false");

    /* Boot flags */
    fprintf(fp, "\"disable_custom_bootsplash\": %s,\n",
        (rom->footer->cartBootFlags & WSROM_BOOTFLAGS_DISALLOW_BOOTSPLASH) ? "true" : "false");
    fprintf(fp, "\"bootable\": %s,\n",
        (rom->footer->cartBootFlags & WSROM_BOOTFLAGS_NONBOOTABLE_MASK) ? "false" : "true");

    fprintf(fp, "\"Checksum\": %d,\n", rom->footer->checksum);
    fprintf(fp, "\"footer\": \"%02X:%04X:%04X:%02X:%04X:%02X:%02X:%02X:%02X:%04X:%04X\",\n",
            rom->footer->resetOpcode, rom->footer->resetOffset, rom->footer->resetSegment,
            rom->footer->cartBootFlags, rom->footer->publisherId, rom->footer->gameId,
            rom->footer->cartFlagsExt, rom->footer->romInfo, rom->footer->saveInfo,
            rom->footer->cartFlags, rom->footer->checksum);
    // TODO: Add MD5
    printf(fp, "\"md5\": \"\",\n");
    fprintf(fp, "}");
}

uint16_t wsrom_getChecksum(wsrom_game_t *rom)
{
    uint16_t sum = 0;
    size_t size = wsrom_getRomSize(rom) - 2; /* We need to ignore the checksum bytes */
    size_t i;

    for(i = 0; i < size; i++)
    {
        sum = sum + rom->rom_data[i];
    }

    return sum;
}

uint32_t wsrom_getSramSize(wsrom_game_t *rom)
{
    switch (rom->footer->saveInfo & WSROM_SAVEINFO_SRAM_SIZE_MASK)
    {
    case WSROM_SAVEINFO_SRAM_SIZE_NONE:
        return 0;

    case WSROM_SAVEINFO_SRAM_SIZE_64KBIT:
        return (64 * 1024) / 8;

    case WSROM_SAVEINFO_SRAM_SIZE_256KBIT:
        return (256 * 1024) / 8;

    case WSROM_SAVEINFO_SRAM_SIZE_1MBIT:
        return (1 * 1024 * 1024) / 8;

    case WSROM_SAVEINFO_SRAM_SIZE_2MBIT:
        return (2 * 1024 * 1024) / 8;

    case WSROM_SAVEINFO_SRAM_SIZE_4MBIT:
        return (4 * 1024 * 1024) / 8;

    default:
        Log(TLOG_PANIC, "WSROM", "Invalid SRAM size (%02X)! Please check cart metadata!",
            rom->footer->saveInfo);
    }

    return (0);
}

uint32_t wsrom_getEepromSize(wsrom_game_t *rom)
{
    switch (rom->footer->saveInfo & WSROM_SAVEINFO_EEPROM_SIZE_MASK)
    {
    case WSROM_SAVEINFO_EEPROM_SIZE_NONE:
        return 0;

    case WSROM_SAVEINFO_EEPROM_SIZE_1KBIT:
        return (1 * 1024) / 8;

    case WSROM_SAVEINFO_EEPROM_SIZE_16KBIT:
        return (16 * 1024) / 8;

    case WSROM_SAVEINFO_EEPROM_SIZE_32KBIT:
        return (32 * 1024) / 8;

    case WSROM_SAVEINFO_EEPROM_SIZE_8KBIT:
        return (8 * 1024) / 8;

    case WSROM_SAVEINFO_EEPROM_SIZE_4KBIT:
        return (4 * 1024) / 8;

    case WSROM_SAVEINFO_EEPROM_SIZE_2KBIT:
        return (2 * 1024) / 8;

    default:
        Log(TLOG_PANIC, "WSROM", "Invalid EEPROM size (%02X)! Please check cart metadata!",
            rom->footer->saveInfo);
    }

    return (0);
}

uint32_t wsrom_getRomSize(wsrom_game_t *rom)
{
    switch (rom->footer->romInfo)
    {
    case WSROM_ROMINFO_SIZE_1MBIT:
        return (1 * 1024 * 1024) / 8;

    case WSROM_ROMINFO_SIZE_2MBIT:
        return (2 * 1024 * 1024) / 8;

    case WSROM_ROMINFO_SIZE_4MBIT:
        return (4 * 1024 * 1024) / 8;

    case WSROM_ROMINFO_SIZE_8MBIT:
        return (8 * 1024 * 1024) / 8;

    case WSROM_ROMINFO_SIZE_16MBIT:
        return (16 * 1024 * 1024) / 8;

    case WSROM_ROMINFO_SIZE_24MBIT:
        return (24 * 1024 * 1024) / 8;

    case WSROM_ROMINFO_SIZE_32MBIT:
        return (32 * 1024 * 1024) / 8;

    case WSROM_ROMINFO_SIZE_48MBIT:
        return (48 * 1024 * 1024) / 8;

    case WSROM_ROMINFO_SIZE_64MBIT:
        return (64 * 1024 * 1024) / 8;

    case WSROM_ROMINFO_SIZE_128MBIT:
        return (128 * 1024 * 1024) / 8;

    default:
        Log(TLOG_PANIC, "WSROM", "Invalid ROM size (%02X)! Please check cart metadata!",
            rom->footer->saveInfo);
    }

    return (0);
}