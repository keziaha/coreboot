/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2009 coresystems GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CONSOLE_VTXPRINTF_H
#define __CONSOLE_VTXPRINTF_H

/* With GCC we use -nostdinc -ffreestanding to keep out system includes.
 * Unfortunately this also gets us rid of the _compiler_ includes, like
 * stdarg.h. To work around the issue, we define varargs directly here.
 * On LLVM we can still just include stdarg.h.
 */
#ifdef __GNUC__
#define va_start(v,l)		__builtin_va_start(v,l)
#define va_end(v)		__builtin_va_end(v)
#define va_arg(v,l)		__builtin_va_arg(v,l)
typedef __builtin_va_list	va_list;
#else
#include <stdarg.h>
#endif

int vtxprintf(void (*tx_byte)(unsigned char byte, void *data),
	const char *fmt, va_list args, void *data);

#endif
