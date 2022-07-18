/*******************************************************************************
 * NewOswan
 * interrupt_controller.h:
 * Based on the original Oswan-unix
 * Copyright (c) 2022-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#ifndef __P_INTERRUPT_CONTROLLER_H__
#define __P_INTERRUPT_CONTROLLER_H__

#include <device.h>

typedef enum hw_interrupt_type_t
{
    /* They are in the same order as the hardware */
    HWI_SERIAL_TX = 0,
    HWI_KEY,
    HWI_CART,
    HWI_SERIAL_RX,
    HWI_LINE_COMPARE,
    HWI_VBLANK_TIMER,
    HWI_VBLANK,
    HWI_HBLANK_TIMER,
} hw_interrupt_type_t;

/* Exported device infos */
extern device_t InterruptController;

/* Exported functions */
void intc_trigger_interrupt(hw_interrupt_type_t type);

#endif /* __P_INTERRUPT_CONTROLLER_H__ */
