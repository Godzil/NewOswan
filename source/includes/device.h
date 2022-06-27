/*******************************************************************************
 * NewOswan
 * device.h: 
 *
 * Created by Manoël Trapier on 19/12/2021.
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#ifndef __DEVICE_H__
#define __DEVICE_H__

typedef void (*device_init)(void *param);
typedef void (*device_reset)(void);
typedef void (*device_free)(void);

typedef enum device_type_t
{
    DT_INTERRUPT_CONTROLLER,
    DT_GPU,
    DT_BUTTONS,
    DT_DMA,
    DT_LUXOR,
    DT_AUDIO,
    DT_SYSTEM,
    DT_RTC,
    DT_RS232,
    DT_EEPROM,
    DT_DEBUG,
} device_type_t;

typedef struct device_t
{
    device_init init;           /***< Function called to init the device */
    device_reset reset;         /***< Function called to reset the device */
    device_free free;           /***< Function called to deinit the device */
    device_type_t deviceType;   /***< Used to tell the type of device, could be useful to pass the
                                 * right parameters to init */
} device_t;

#endif /* __DEVICE_H__ */
