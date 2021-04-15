/*
 * NewOswan
 * nec_debugger.h: 
 * Based on the original Oswan-unix
 * Created by Manoël Trapier on 14/04/2021.
 * Copyright (c) 2014-2021 986-Studio. All rights reserved.
 *
 */

#ifndef NEWOSWAN_SOURCE_NEC_NEC_DEBUGGER_H
#define NEWOSWAN_SOURCE_NEC_NEC_DEBUGGER_H


int nec_decode_instruction(uint16_t segment, uint16_t offset, char *buffer, unsigned int bufferSize);

#endif /* NEWOSWAN_SOURCE_NEC_NEC_DEBUGGER_H */
