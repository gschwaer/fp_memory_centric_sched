/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) ARM Limited, 2014
 * Copyright (c) Siemens AG, 2014-2017
 *
 * Authors:
 *  Jean-Philippe Brucker <jean-philippe.brucker@arm.com>
 *  Jan Kiszka <jan.kiszka@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <mach.h>
#include <inmate.h>

static void get_tag(u32, u32, u32 *);
static void get_data(u32, u32, u8, u32 *);
//static void get_dbit(u32, u32, u32 *);
static void get_cacheline(u32, u32, u32 *);
static inline void asm_ramindex_mcr(u32);
static inline void asm_ramindex_mrc(u32 *, u8);
static inline void cisw(u32, u32, u8);
static inline void civac(u64);
//static void print_cacheline(u32 *);
//static void dump_cache(void);
static void dump_index(int);
//static void print_index_tags(int);
static inline void cache_size_id(u8);
static inline void disable_prefetch(void);
// Cache size information (see 4.3.22 in ARM DDI0488H)
static inline void cache_size_id(u8 level)
{
	u32 volatile csselr, ccsidr;
	csselr=((level-1)<<1);
	asm volatile (
		"msr csselr_el1, %x1\n\t"
		"isb\n\t"
		"mrs %x0, ccsidr_el1":"=r"(ccsidr): "r" (csselr));

	printk("CCSIDR: 0x%llx\n",ccsidr);
}


// Disable L2 prefetch
// BUG: The cpuectlr value is restored after a while
static inline void disable_prefetch(void)
{
	u64 volatile cpuectlr;

	asm volatile(
		"mrs %x0,S3_1_c15_c2_1\n\t"
		"bic %x0,%x0, #3<<32\n\t"  //L2 load/store prefetch distance to 0
		"bic %x0,%x0, #3<<35\n\t"  //L2 instruction fetch prefetch distance to 0
		"bic %x0,%x0, #1<<38\n\t"  //disable table walk descriptor access L2 prefetch
		"msr S3_1_c15_c2_1,%x0\n\t"
		"isb\n\t"
		"dsb sy\n\t"
		:"=r"(cpuectlr));

	//printk("cpuectlr: 0x%llx\n",cpuectlr);
}



//static void dump_cache(void)
//{
//	int index;
//
//	for(index=0;index<2048;index++)
//		dump_index(index);
//}

static void dump_index(int index)
{
	int way, w;
	u32 cache_set[16][16];
	u32 tag;

	printk("index: 0x%08x\n",index);

	for(way=0;way<16;way++)
        	get_cacheline(index,way,&cache_set[way][0]);
        for(way=0;way<16;way++) {
		get_tag(index,way,&tag);

		printk("%d (TAG 0x%08x)\n",way,(tag>>4));
		for(w=0;w<16;w++) printk(" 0x%08x",cache_set[way][w]);
                printk("\n");
        }
}

//static void print_index_tags(int index)
//{
//	int way;
//	u32 tags[16];
//	for(way=0;way<16;way++)
//		get_tag(index,way,&tags[way]);
//	for(way=0;way<16;way++)
//		printk("%d (TAG 0x%08x)\n",way,(tags[way]>>4));
//}


//static inline void sync(void)
//{
//	asm volatile(
//		"dsb sy\t\n"
//		"isb");
//}


// Ramindex operation
// 4.3.64 in ARM Cortex-A57 MPCore Processor Technical Reference Manual
static inline void asm_ramindex_mcr(u32 ramindex)
{
	asm volatile(
		"sys #0, c15, c4, #0, %0\t\n"
		"dsb sy\t\n"
		"isb" :: "r" (ramindex));
}


// reading from DL1DATA0_EL1
// 4.3.63 in ARM Cortex-A57 MPCore Processor Technical Reference Manual
static inline void asm_ramindex_mrc(u32 *dl1data, u8 sel)
{
	if(sel&0x01) asm volatile("mrs %0,S3_0_c15_c1_0" : "=r"(dl1data[0]));
        if(sel&0x02) asm volatile("mrs %0,S3_0_c15_c1_1" : "=r"(dl1data[1]));
        if(sel&0x04) asm volatile("mrs %0,S3_0_c15_c1_2" : "=r"(dl1data[2]));
        if(sel&0x08) asm volatile("mrs %0,S3_0_c15_c1_3" : "=r"(dl1data[3]));
}


// Get Tag of L2 cache entry at (index,way)
// Tag bank select ignored, 2MB L2 cache assumed
static void get_tag(u32 index, u32 way, u32 * dl1data)
{
	u32 ramid    = 0x10;
	u32 ramindex = (ramid<<24) + (way<<18) + (index<<6);
	asm_ramindex_mcr(ramindex);
	asm_ramindex_mrc(dl1data,0x01);
}


// Get data from cache entry at (index,way,bank)
// returns bytes from 4*bank to 4*bank+4 in the cache line at (index,way)
// Ther are 4 banks; each bank has 4 words (16 bytes)
static void get_data(u32 index, u32 way, u8 bank, u32 * dl1data)
{
	u32 ramid    = 0x11;
	u32 ramindex = (ramid<<24) + (way<<18) + (index<<6) + (bank<<4);
	asm_ramindex_mcr(ramindex);
	asm_ramindex_mrc(dl1data,0x0F);
}


// Get the cache line for (index,way)
// cache line has 16 words (64 bytes)
static void get_cacheline(u32 index, u32 way, u32 * cache_line)
{
	u8 bank;
	for(bank=0;bank<4;bank++)
		get_data(index,way,bank,&cache_line[4*bank]);

}

// Get Dirty Bit of L2 cache entry at (index,way)
// Tag bank select ignored, 2MB L2 cache assumed
//static void get_dbit(u32 index, u32 way, u32 * dl1data)
//{
//	u32 ramid 	= 0x14;
//	u32 ramindex 	= (ramid<<24) + (way<<18) + (index<<6);
//	asm_ramindex_mcr(ramindex);
//	asm_ramindex_mrc(dl1data,0x01);
//}


// Clear and Invalidate by Set/Way cache level l
static inline void cisw(u32 index, u32 way, u8 l)
{
	u64 ws = (way<<28) + (index<<6) + ((l-1)<<1);
	asm volatile(
		"dc cisw,%0\n\t"
		"dsb sy\n\t"
		"isb" :: "r"(ws) : "memory");

}

//static inline void fast_cisw(u32 index, u32 way, u8 l)
//{
//	u64 ws = (way<<28) + (index<<6) + ((l-1)<<1);
//	asm volatile(
//		"dc cisw,%0\n\t"
//		:: "r"(ws) : "memory");
//
//}


// Clear and Invalidate by Vritual Address
static inline void civac(u64 va)
{
	asm volatile (
		"dc civac, %0\n\t"
//		"isb\n\t"
//		"dsb sy\n\t"
		:: "r" (va) : "memory");
}

// Print the cache line
//static void print_cacheline(u32 * cache_line)
//{
//	int w=0; // word in cache line
//        for(w=0;w<16;w++)
//                printk("0x%08x ", cache_line[w]);
//        printk("\n");
//}

static volatile u8 __attribute__((aligned(131072))) aligned_data[128*1024];

void inmate_main(void)
{
	printk("Initializing...\n");

	arch_mmu_enable();

	printk("Writing...\n");

	for(u32 i=0;i<((128*1024)/sizeof(u32));i++) {
		u32* ptr = (((u32*)aligned_data)+i);
		*ptr = 0x01234567;
	}

	dump_index(0);

	halt();
}
