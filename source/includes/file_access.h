/*******************************************************************************
 * NewOswan
 * file_access.h:
 *
 * Created by Manoël Trapier on 26/06/2022.
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#ifndef __FILE_ACCESS_H__
#define __FILE_ACCESS_H__

#include <stdint.h>
#include <stdbool.h>

void *create_file(const char *filename, uint32_t size);
void *load_file(const char *filename, bool readOnly, size_t *fileSize);
void close_file(void *data, uint32_t size);

#endif /* __FILE_ACCESS_H__ */
