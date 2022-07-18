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
#include <peripherals/interrupt_controller.h>

/* device internal parameters */
typedef struct intc_params_t
{
    uint8_t address_base;
    uint8_t int_enable_mask;
    uint8_t int_status;
} intc_params_t;

static intc_params_t interrupt_controller;

static uint8_t intc_io_read(void *pdata, uint8_t port)
{
    intc_params_t *params = (intc_params_t *)pdata;

    switch(port)
    {
    case 0x0: return params->address_base;
    case 0x2: return params->int_enable_mask;
    case 0x4: return params->int_status;
    }

    return 0x90;
}

static void intc_io_write(void *pdata, uint8_t port, uint8_t value)
{
    intc_params_t *params = (intc_params_t *)pdata;

    switch(port)
    {
    case 0x0: params->address_base = value; break;
    case 0x2: params->int_enable_mask = value; break;
    }
}

static void intc_io_write_ack(void *pdata, uint8_t port, uint8_t value)
{
    intc_params_t *params = (intc_params_t *)pdata;

    // De-assert CPU interrupt accordingly to the mask in value
}


static void intc_init(uint8_t baseAddress, void *params)
{
    UNUSED_PARAMETER(params);

    io_register_hook(baseAddress, 0x0, intc_io_read, intc_io_write, &interrupt_controller);
    io_register_hook(baseAddress, 0x2, intc_io_read, intc_io_write, &interrupt_controller);
    io_register_hook(baseAddress, 0x4, intc_io_read, NULL, &interrupt_controller);
    io_register_hook(baseAddress, 0x6, NULL, intc_io_write_ack, &interrupt_controller);
}

static void intc_reset()
{
    interrupt_controller.int_enable_mask = 0;
    interrupt_controller.address_base = 0;
}

/* Exported device */
device_t InterruptController =
{
        .init = intc_init,
        .reset = intc_reset,
        .free = NULL,
        .type = DT_INTERRUPT_CONTROLLER,
};

/* Exported functions */
void intc_trigger_interrupt(hw_interrupt_type_t type)
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