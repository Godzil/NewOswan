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

void *file_create(const char *filename, uint32_t size);
void *file_load(const char *filename, bool readOnly, size_t *fileSize);
void file_close(void *data, uint32_t size);

#endif /* __FILE_ACCESS_H__ */
