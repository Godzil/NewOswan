/******************************************************************************
 * NewOswan
 * nec_debugger.h:
 *
 * Created by Manoël Trapier on 14/04/2021.
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#ifndef __NEC_DEBUGGER_H__
#define __NEC_DEBUGGER_H__

int nec_decode_instruction(uint16_t segment, uint16_t offset, char *buffer, unsigned int bufferSize);

#endif /* __NEC_DEBUGGER_H__ */
