/*******************************************************************************
 * NewOswan
 * audio.h:
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

#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <stdint.h>

void ws_audio_init();
void ws_audio_reset();
void ws_audio_port_write(uint32_t port, uint8_t value);
uint8_t ws_audio_port_read(uint8_t port);
void ws_audio_done();

unsigned int ws_audio_mrand(unsigned int Degree);
int ws_audio_seal_init();
void ws_audio_seal_done();
int ws_audio_play_channel(int Channel);
int ws_audio_stop_channel(int Channel);
void ws_audio_clear_channel(int Channel);
void ws_audio_set_channel_frequency(int Channel,int Period);
void ws_audio_set_channel_volume(int Channel,int Vol);
void ws_audio_set_channel_pan(int Channel,int Left,int Right);
void ws_audio_set_channel_pdata(int Channel,int Index);
void ws_audio_set_channels_pbuf(int Addr,int Data);
void ws_audio_rst_channel(int Channel);
int ws_audio_int();
void ws_audio_set_pcm(int Data);
void ws_audio_flash_pcm();
void ws_audio_write_byte(uint32_t offset, uint8_t value);
void ws_audio_process();
void ws_audio_readState(int fp);
void ws_audio_writeState(int fp);
uint32_t timer_callupdate(uint32_t, void*);

#endif

