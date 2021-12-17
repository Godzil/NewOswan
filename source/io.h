/*******************************************************************************
 * NewOswan
 * io.h:
 * Based on the original Oswan-unix
 * Copyright (c) 2014-2021 986-Studio. All rights reserved.
 *
 ******************************************************************************/
 
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>

extern uint8_t *ws_ioRam;
extern uint8_t ws_key_start;
extern uint8_t ws_key_x4;
extern uint8_t ws_key_x2;
extern uint8_t ws_key_x1;
extern uint8_t ws_key_x3;
extern uint8_t ws_key_y4;
extern uint8_t ws_key_y2;
extern uint8_t ws_key_y1;
extern uint8_t ws_key_y3;
extern uint8_t ws_key_button_a;
extern uint8_t ws_key_button_b;

void io_init(void);
void io_reset(void);
void io_flipControls(void);
void io_done(void);

uint8_t io_readport(uint8_t port);
void io_writeport(uint32_t port, uint8_t value);

#endif

