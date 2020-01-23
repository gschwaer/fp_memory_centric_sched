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

/** \brief	AUTOSAR BSW ICU Driver Schedule Manager Header File.
 *
 *  \author	Giuseppe Serano
 *  \date	2018
 */

#ifndef	SCHM_ICU_17_GTMCCU6_H
#define	SCHM_ICU_17_GTMCCU6_H

/******************************************************************************
 *			Includes					      *
 ******************************************************************************/

#include "Std_Types.h"
#include "McalOsConfig.h"
#include MCAL_OS_HEADER

/******************************************************************************
 *			Global Function Definitions			      *
 ******************************************************************************/
 
/** \brief	Icu_17_GtmCcu6_EnableWakeup() exclusive area entering.
 *  \return	Nothing.
 *
 *  \note	Synchronous.
 *  \note	Non - Reentrant
 **/
LOCAL_INLINE FUNC(void, OS_CODE) SchM_Enter_Icu_17_GtmCcu6_EnableWakeup(void);

/** \brief	Icu_17_GtmCcu6_EnableWakeup() exclusive area exiting.
 *  \return	Nothing.
 *
 *  \note	Synchronous.
 *  \note	Non - Reentrant
 **/
LOCAL_INLINE FUNC(void, OS_CODE) SchM_Exit_Icu_17_GtmCcu6_EnableWakeup(void);

/** \brief	Icu_17_GtmCcu6_EnableNotification() exclusive area entering.
 *  \return	Nothing.
 *
 *  \note	Synchronous.
 *  \note	Non - Reentrant
 **/
LOCAL_INLINE FUNC(void, OS_CODE) SchM_Enter_Icu_17_GtmCcu6_EnableNotification(void);

/** \brief	Icu_17_GtmCcu6_EnableNotification() exclusive area exiting.
 *  \return	Nothing.
 *
 *  \note	Synchronous.
 *  \note	Non - Reentrant
 **/
LOCAL_INLINE FUNC(void, OS_CODE) SchM_Exit_Icu_17_GtmCcu6_EnableNotification(void);

/** \brief	Icu_17_GtmCcu6_ResetEdgeCount() exclusive area entering.
 *  \return	Nothing.
 *
 *  \note	Synchronous.
 *  \note	Non - Reentrant
 **/
LOCAL_INLINE FUNC(void, OS_CODE) SchM_Enter_Icu_17_GtmCcu6_ResetEdgeCount(void);

/** \brief	Icu_17_GtmCcu6_ResetEdgeCount() exclusive area exiting.
 *  \return	Nothing.
 *
 *  \note	Synchronous.
 *  \note	Non - Reentrant
 **/
LOCAL_INLINE FUNC(void, OS_CODE) SchM_Exit_Icu_17_GtmCcu6_Ccu6IenUpdate(void);

/** \brief	Icu_17_GtmCcu6_Ccu6IenUpdate() exclusive area entering.
 *  \return	Nothing.
 *
 *  \note	Synchronous.
 *  \note	Non - Reentrant
 **/
LOCAL_INLINE FUNC(void, OS_CODE) SchM_Enter_Icu_17_GtmCcu6_Ccu6IenUpdate(void);

/** \brief	Icu_17_GtmCcu6_Ccu6IenUpdate() exclusive area exiting.
 *  \return	Nothing.
 *
 *  \note	Synchronous.
 *  \note	Non - Reentrant
 **/
LOCAL_INLINE FUNC(void, OS_CODE) SchM_Exit_Icu_17_GtmCcu6_Ccu6IenUpdate(void);

/** \brief	Icu_17_GtmCcu6_CcuInterruptHandle() exclusive area entering.
 *  \return	Nothing.
 *
 *  \note	Synchronous.
 *  \note	Non - Reentrant
 **/
LOCAL_INLINE FUNC(void, OS_CODE) SchM_Enter_Icu_17_GtmCcu6_CcuInterruptHandle(void);

/** \brief	Icu_17_GtmCcu6_CcuInterruptHandle() exclusive area exiting.
 *  \return	Nothing.
 *
 *  \note	Synchronous.
 *  \note	Non - Reentrant
 **/
LOCAL_INLINE FUNC(void, OS_CODE) SchM_Exit_Icu_17_GtmCcu6_CcuInterruptHandle(void);

/** \brief	Icu_17_GtmCcu6_CcuVariableupdate() exclusive area entering.
 *  \return	Nothing.
 *
 *  \note	Synchronous.
 *  \note	Non - Reentrant
 **/
LOCAL_INLINE FUNC(void, OS_CODE) SchM_Enter_Icu_17_GtmCcu6_CcuVariableupdate(void);

/** \brief	Icu_17_GtmCcu6_CcuVariableupdate() exclusive area exiting.
 *  \return	Nothing.
 *
 *  \note	Synchronous.
 *  \note	Non - Reentrant
 **/
LOCAL_INLINE FUNC(void, OS_CODE) SchM_Exit_Icu_17_GtmCcu6_CcuVariableupdate(void);

#endif	/* SCHM_ICU_17_GTMCCU6_H */
