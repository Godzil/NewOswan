//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

// this file is a travesty. [SU]int(8|16|32) should be used instead.

#ifndef __TYPES_H__
#define __TYPES_H__

#include "SDL.h"

#define UINT8	Uint8
#define UINT16	Uint16
#define INT8	Sint8
#define INT16	Sint16
#define INT32	Sint32
#define UINT32	Uint32

#define uint8	UINT8
#define uint16	UINT16
#define uint32	UINT32
#define int8	INT8
#define int16	INT16
#define int32	INT32

#define u8		uint8
#define u16		uint16
#define u32		uint32
#define s8		int8
#define s16		int16
#define s32		int32

#define BYTE Uint8
#define WORD Uint16
#define DWORD Uint32

#endif
