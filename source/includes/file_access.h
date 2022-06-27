/*******************************************************************************
 * NewOswan
 * memory.h:
 *
 * Based on the original Oswan-unix
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#ifndef __FILE_ACCESS_H__
#define __FILE_ACCESS_H__

#include <stdint.h>
#include <stdbool.h>

char *create_file(char *filename, uint32_t size);
char *load_file(char *filename, bool readOnly);

#endif /* __FILE_ACCESS_H__ */
