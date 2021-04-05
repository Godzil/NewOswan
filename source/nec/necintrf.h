/*
 * NewOswan
 * necintrf.h:
 * Based on the original Oswan-unix
 * Copyright (c) 2014-2021 986-Studio. All rights reserved.
 *
 */
/* ASG 971222 -- rewrote this interface */
#ifndef __NECITRF_H_
#define __NECITRF_H_

#include <stdint.h>

enum
{
    NEC_IP = 1,
    NEC_AW,
    NEC_CW,
    NEC_DW,
    NEC_BW,
    NEC_SP,
    NEC_BP,
    NEC_IX,
    NEC_IY,
    NEC_FLAGS,
    NEC_ES,
    NEC_CS,
    NEC_SS,
    NEC_DS,
    NEC_VECTOR,
    NEC_PENDING,
    NEC_NMI_STATE,
    NEC_IRQ_STATE
};

/* Public variables */
extern int nec_ICount;

/* Public functions */
void nec_set_irq_line(int irqline, int state);
void nec_set_reg(int regnum, uint32_t val);
int nec_execute(int cycles);
unsigned nec_get_reg(int regnum);
void nec_reset(void *param);
void nec_int(uint16_t vector);

uint8_t cpu_readport(uint8_t);
void cpu_writeport(uint32_t, uint8_t);
#define cpu_readop cpu_readmem20
#define cpu_readop_arg cpu_readmem20
void cpu_writemem20(uint32_t, uint8_t);
uint8_t cpu_readmem20(uint32_t);

#endif /* __NECITRF_H_ */
