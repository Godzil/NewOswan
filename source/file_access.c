/*******************************************************************************
 * NewOswan
 * file_access.c: File manipulation functions
 *
 * Created by Manoël Trapier on 27/06/2022.
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 * This file is OS specific and this need to be changed at some point.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <file_access.h>
#include <log.h>

char *load_file(char *filename, bool readOnly)
{
    int fd;
    char *ret_ptr;
    struct stat FileStat;
    int flags = readOnly?MAP_ANONYMOUS:MAP_SHARED;

    fd = open(filename, O_RDWR);

    fstat(fd, &FileStat);

    Log(TLOG_DEBUG, "memory", "Trying to load %s, size = %lu...", filename, (unsigned long)FileStat.st_size);

    ret_ptr = (char *)mmap(NULL, FileStat.st_size, PROT_READ | PROT_WRITE, flags, fd, 0);

    close(fd);

    if (ret_ptr == MAP_FAILED)
    {
        ret_ptr = NULL;
    }

    return ret_ptr;
}

char *create_file(char *filename, uint32_t size)
{
    int fd;
    uint32_t i;
    char *ret_ptr;
    char buf[] = {0};

    Log(TLOG_DEBUG, "memory", "Trying to create %s, size = %u...\n", filename, size);
    fd = open(filename, O_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | O_TRUNC, 0644);
    fchmod(fd, 0644);
    close(fd);
    sync();

    fd = open(filename, O_RDWR);

    for (i = 0 ; i < size ; i++)
    {
        write(fd, buf, 1);
    }

    close(fd);
    sync();

    fd = open(filename, O_RDWR);
    ret_ptr = (char *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    close(fd);

    if (ret_ptr == MAP_FAILED)
    {
        ret_ptr = NULL;
    }

    return ret_ptr;
}