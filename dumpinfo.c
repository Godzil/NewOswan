/*******************************************************************************
 * NewOswan
 * dumpinfo.c: Tool to dump the metadata info about a cart rom image.
 *
 * Copyright (c) 2014-2021 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

char *load_file(char *filename, uint32_t *fileSize)
{
    int fd;
    char *ret_ptr;
    struct stat FileStat;

    fd = open(filename, O_RDWR);

    fstat(fd, &FileStat);

    ret_ptr = (char *)mmap(NULL, FileStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    *fileSize = FileStat.st_size;

    close(fd);

    if (ret_ptr == MAP_FAILED)
    {
        ret_ptr = NULL;
    }

    return ret_ptr;
}

#pragma pack(1)
struct cart_metadata
{
    uint8_t farjump[5];
    uint8_t flagExt;
    uint8_t publishId;
    uint8_t gameId[2];
    uint8_t flags2;
    uint8_t romInfo;
    uint8_t saveInfo;
    uint16_t flags;
    uint16_t crc;
};
#pragma pack()


int main(int argc, char *argv[])
{
    char *content;
    uint32_t size;
    struct cart_metadata *data;
    int ret = -1;

    if (argc != 2)
    {
        printf("Usage: %s file.ws[c]\n", argv[0]);
    }
    else
    {
        content = load_file(argv[1], &size);

        if (content != NULL)
        {
            data = (struct cart_metadata *)&(content[size - sizeof(struct cart_metadata)]);

            printf("%s:\n", argv[1]);
            if (data->farjump[0] == 0xEA)
            {
                printf("[%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x]", data->publishId, data->gameId[0],
                       data->gameId[1], data->flags2, data->romInfo, data->saveInfo, data->flags & 0xFF,
                       (data->flags >> 8) & 0xFF, data->crc & 0xFF, (data->crc >> 8) & 0xFF);
                printf(" - Reset @ %02X%02X:%02X%02Xh\n", data->farjump[4], data->farjump[3], data->farjump[2],
                       data->farjump[1]);

                printf(" - publisher: %02X, gameId: %01X%02X\n", data->publishId, data->gameId[0], data->gameId[1]);
                printf(" - %s want to write to EEPROM\n", data->flags2 & 0x80 ? "Do" : "Do not");
                printf(" - %s user defined bootsplash\n", data->flagExt & 0x80 ? "Dissallow" : "Allow");
                printf(" - Is %sbootable on a normal swan\n", data->flagExt & 0x0F ? "not " : "");
                printf(" - ROM Size: %02Xh\n", data->romInfo);
                printf(" - Save type & Size: %02Xh\n", data->saveInfo);
                printf(" - Flags: %d cycles ROM, %d bit ROM bus, %sRTC, %s orientation\n", data->flags & 0x004 ? 1 : 3,
                       data->flags & 0x002 ? 8 : 16, data->flags & 0x100 ? "" : "No ",
                       data->flags & 0x001 ? "Vertical" : "Horizontal");
                printf(" - CRC: %04Xh\n", data->crc);
                ret = 0;
            }
            else
            {
                printf(" - File is not a valid WonderSwan ROM.\n");
            }

        }
        else
        {
            printf("Cannot open '%s'\n", argv[1]);
        }
    }

    return ret;
}
