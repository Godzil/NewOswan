/*******************************************************************************
 * NewOswan
 * io.h:
 *
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>

void io_init(void);
void io_cleanup(void);

uint8_t io_readport(uint8_t port);
void io_writeport(uint8_t port, uint8_t value);

typedef uint8_t (*io_read)(void *pdata, uint8_t port);
typedef void (*io_write)(void *pdata, uint8_t port, uint8_t value);

void io_register_hook(uint8_t baseAddress, uint8_t port, io_read readHook, void *pdata, io_write writeHook);
void io_register_hook_array(uint8_t baseAddress, const uint8_t *portList, uint8_t listLen, io_read readHook, io_write writeHook,
                            void *pdata);

#define UNUSED_PARAMETER(_s) (void *)(_s)

#endif
