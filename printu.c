/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) Siemens AG, 2013
 *
 * Authors:
 *  Jan Kiszka <jan.kiszka@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */
#include <stdarg.h>
#include <jailhouse/string.h>

#include "printu.h"
#include "uart-driver.h"

/* This function is used in the code of "printk-core.c" (see below). */
static void console_write(const char *msg) // @suppress("Unused static function")
{
	UART_write_str(UART_C, msg);
}

#include "printk-core.c"

void printu(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	__vprintk(fmt, ap);
	va_end(ap);
}
