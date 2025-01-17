/*
 * Jailhouse AArch64 support
 *
 * Copyright (C) 2015 Huawei Technologies Duesseldorf GmbH
 *
 * Authors:
 *  Antonios Motakis <antonios.motakis@huawei.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 * Alternatively, you can use or redistribute this file under the following
 * BSD license:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#define JAILHOUSE_HVC_CODE		0x4a48

#define JAILHOUSE_CALL_INS		"hvc #0x4a48"
#define JAILHOUSE_CALL_NUM_RESULT	"x0"
#define JAILHOUSE_CALL_ARG1		"x1"
#define JAILHOUSE_CALL_ARG2		"x2"
#define JAILHOUSE_CALL_ARG3		"x3"
#define JAILHOUSE_CALL_ARG4		"x4"
#define JAILHOUSE_CALL_ARG5		"x5"

/* CPU statistics */
#define JAILHOUSE_CPU_STAT_VMEXITS_MAINTENANCE	JAILHOUSE_GENERIC_CPU_STATS
#define JAILHOUSE_CPU_STAT_VMEXITS_VIRQ		JAILHOUSE_GENERIC_CPU_STATS + 1
#define JAILHOUSE_CPU_STAT_VMEXITS_VSGI		JAILHOUSE_GENERIC_CPU_STATS + 2
#define JAILHOUSE_CPU_STAT_VMEXITS_PSCI		JAILHOUSE_GENERIC_CPU_STATS + 3
#define JAILHOUSE_NUM_CPU_STATS			JAILHOUSE_GENERIC_CPU_STATS + 4

#ifndef __ASSEMBLY__

struct jailhouse_comm_region {
	COMM_REGION_GENERIC_HEADER;
};

static inline __u64 jailhouse_call(__u64 num)
{
	register __u64 num_result asm(JAILHOUSE_CALL_NUM_RESULT) = num;

	asm volatile(
		JAILHOUSE_CALL_INS
		: "=r" (num_result)
		: "r" (num_result)
		: "memory");
	return num_result;
}

static inline __u64 jailhouse_call_arg1(__u64 num, __u64 arg1)
{
	register __u64 num_result asm(JAILHOUSE_CALL_NUM_RESULT) = num;
	register __u64 __arg1 asm(JAILHOUSE_CALL_ARG1) = arg1;

	asm volatile(
		JAILHOUSE_CALL_INS
		: "=r" (num_result)
		: "r" (num_result), "r" (__arg1)
		: "memory");
	return num_result;
}

static inline __u64 jailhouse_call_arg2(__u64 num, __u64 arg1, __u64 arg2)
{
	register __u64 num_result asm(JAILHOUSE_CALL_NUM_RESULT) = num;
	register __u64 __arg1 asm(JAILHOUSE_CALL_ARG1) = arg1;
	register __u64 __arg2 asm(JAILHOUSE_CALL_ARG2) = arg2;

	asm volatile(
		JAILHOUSE_CALL_INS
		: "=r" (num_result)
		: "r" (num_result), "r" (__arg1), "r" (__arg2)
		: "memory");
	return num_result;
}

static inline __u64 jailhouse_call_arg3(__u64 num, __u64 arg1, __u64 arg2,
					__u64 arg3)
{
	register __u64 num_result asm(JAILHOUSE_CALL_NUM_RESULT) = num;
	register __u64 __arg1 asm(JAILHOUSE_CALL_ARG1) = arg1;
	register __u64 __arg2 asm(JAILHOUSE_CALL_ARG2) = arg2;
	register __u64 __arg3 asm(JAILHOUSE_CALL_ARG3) = arg3;

	asm volatile(
		JAILHOUSE_CALL_INS
		: "=r" (num_result)
		: "r" (num_result), "r" (__arg1), "r" (__arg2),  "r" (__arg3)
		: "memory");
	return num_result;
}

static inline __u64 jailhouse_call_arg5(__u64 num, __u64 arg1, __u64 arg2,
				   __u64 arg3, __u64 arg4, __u64 arg5)
{
   register __u64 num_result asm(JAILHOUSE_CALL_NUM_RESULT) = num;
   register __u64 __arg1 asm(JAILHOUSE_CALL_ARG1) = arg1;
   register __u64 __arg2 asm(JAILHOUSE_CALL_ARG2) = arg2;
   register __u64 __arg3 asm(JAILHOUSE_CALL_ARG3) = arg3;
   register __u64 __arg4 asm(JAILHOUSE_CALL_ARG4) = arg4;
   register __u64 __arg5 asm(JAILHOUSE_CALL_ARG5) = arg5;



   asm volatile(
	   JAILHOUSE_CALL_INS
	   : "=r" (num_result)
	   : "r" (num_result), "r" (__arg1), "r" (__arg2),  "r" (__arg3), "r" (__arg4),
			   "r" (__arg5)
	   : "memory");
   return num_result;
}

static inline void
jailhouse_send_msg_to_cell(struct jailhouse_comm_region *comm_region,
			   __u64 msg)
{
	comm_region->reply_from_cell = JAILHOUSE_MSG_NONE;
	/* ensure reply was cleared before sending new message */
	asm volatile("dmb ishst" : : : "memory");
	comm_region->msg_to_cell = msg;
}

static inline void
jailhouse_send_reply_from_cell(struct jailhouse_comm_region *comm_region,
			       __u64 reply)
{
	comm_region->msg_to_cell = JAILHOUSE_MSG_NONE;
	/* ensure message was cleared before sending reply */
	asm volatile("dmb ishst" : : : "memory");
	comm_region->reply_from_cell = reply;
}

#endif /* !__ASSEMBLY__ */
