/* ###*B*###
 * Erika Enterprise, version 3
 * 
 * Copyright (C) 2017 - 2019 Evidence s.r.l.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License, version 2, for more details.
 * 
 * You should have received a copy of the GNU General Public License,
 * version 2, along with this program; if not, see
 * <www.gnu.org/licenses/old-licenses/gpl-2.0.html >.
 * 
 * This program is distributed to you subject to the following
 * clarifications and special exceptions to the GNU General Public
 * License, version 2.
 * 
 * THIRD PARTIES' MATERIALS
 * 
 * Certain materials included in this library are provided by third
 * parties under licenses other than the GNU General Public License. You
 * may only use, copy, link to, modify and redistribute this library
 * following the terms of license indicated below for third parties'
 * materials.
 * 
 * In case you make modified versions of this library which still include
 * said third parties' materials, you are obligated to grant this special
 * exception.
 * 
 * The complete list of Third party materials allowed with ERIKA
 * Enterprise version 3, together with the terms and conditions of each
 * license, is present in the file THIRDPARTY.TXT in the root of the
 * project.
 * ###*E*### */

/** \file   ee_x86_64_system_timer.c
 *  \brief  System timer implementation (Supported device: x86_64 APIC Timer).
 *
 *  \author  Errico Gudiieri
 *  \date    2018
 */

#include "ee_internal.h"

/* This file is needed only if System Timer is defined with a device */
#if (defined(OSEE_SYSTEM_TIMER_DEVICE))
/****************************************************************
                    System Timer Support
 ****************************************************************/
/* Check the supported system timer */
#if (OSEE_SYSTEM_TIMER_DEVICE != OSEE_X86_64_APIC_TIMER)
#error Unsupported Device as System Timer!
#endif /* OSEE_SYSTEM_TIMER_DEVICE != OSEE_X86_64_APIC_TIMER */

/* System timer: device type and interrupt id are the same */
#define SYSTEM_TIMER_INT_ID OSEE_SYSTEM_TIMER_DEVICE

void osEE_x86_64_system_timer_init(void) {
  /* This returns the frequency in KiloHertz */
  osEE_x86_64_oneshot_timer_init(SYSTEM_TIMER_INT_ID);

  /* This will fire after OSTICKDURATION ns */
  osEE_x86_64_timer_set_deadline(OSTICKDURATION);
  return;
}

void osEE_x86_64_system_timer_handler(void)
{
  OsEE_CDB * p_cdb;
  /* This will fire after OSTICKDURATION ns */
  osEE_x86_64_timer_set_deadline(OSTICKDURATION);

  p_cdb = osEE_get_curr_core();
  osEE_counter_increment(p_cdb->p_sys_counter_db);
}

#endif /* OSEE_SYSTEM_TIMER_DEVICE */
