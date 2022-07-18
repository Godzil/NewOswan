/*******************************************************************************
 * NewOswan
 * interrupt_controller.c:
 *
 * Created by Manoël Trapier on 14/03/2022.
 * Copyright (c) 2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>

#include <device.h>
#include <io.h>
#include <interrupt_controller.h>

/* device internal parameters */
typedef struct ic_params_t
{
    uint8_t address_base;
    uint8_t int_enable_mask;
    uint8_t int_status;
} ic_params_t;

static ic_params_t interrupt_controller;

static uint8_t IC_IO_read(void *pdata, uint8_t port)
{
    ic_params_t *params = (ic_params_t *)pdata;

    switch(port)
    {
    case 0x0: return params->address_base;
    case 0x2: return params->int_enable_mask;
    case 0x4: return params->int_status;
    }

    return 0x90;
}

static void IC_IO_write(void *pdata, uint8_t port, uint8_t value)
{
    ic_params_t *params = (ic_params_t *)pdata;

    switch(port)
    {
    case 0x0: params->address_base = value; break;
    case 0x2: params->int_enable_mask = value; break;
    }
}

static void IC_IO_write_ack(void *pdata, uint8_t port, uint8_t value)
{
    ic_params_t *params = (ic_params_t *)pdata;

    // De-assert CPU interrupt accordingly to the mask in value
}


static void IC_init(uint8_t baseAddress, void *params)
{
    UNUSED_PARAMETER(params);

    register_io_hook(baseAddress, 0x0, IC_IO_read, IC_IO_write, &interrupt_controller);
    register_io_hook(baseAddress, 0x2, IC_IO_read, IC_IO_write, &interrupt_controller);
    register_io_hook(baseAddress, 0x4, IC_IO_read, NULL, &interrupt_controller);
    register_io_hook(baseAddress, 0x6, NULL, IC_IO_write_ack, &interrupt_controller);
}

static void IC_reset()
{
    interrupt_controller.int_enable_mask = 0;
    interrupt_controller.address_base = 0;
}

device_t InterruptController =
{
        .init = IC_init,
        .reset = IC_reset,
        .free = NULL,
        .type = DT_INTERRUPT_CONTROLLER,
};

/* Exported functions */
void trigger_interrupt(hw_interrupt_type_t type)
{
    uint16_t int_vector = interrupt_controller.address_base & 0xF8 + (uint8_t)type;

    /* Check that the INT is enabled */
    if ((interrupt_controller.int_enable_mask >> type) & 0x1)
    {
        /* TODO: Do we have to enable that even if the int is not enabled? */
        interrupt_controller.int_status |= (1 << type);

        // Fire the NEC interrupt

    }
}