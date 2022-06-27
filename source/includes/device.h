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

typedef void (*device_init)(void);
typedef void (*device_reset)(void);
typedef void (*device_free)(void);

typedef struct device_t
{
    device_init *init;
    device_reset *reset;
    device_free *free;
} device_t;

#endif /* NEWOSWAN_DEVICE_H */
