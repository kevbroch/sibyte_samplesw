/*
 * Copyright 2002, 2004
 * Broadcom Corporation. All rights reserved.
 *
 * This software is furnished under license and may be used and copied only
 * in accordance with the following terms and conditions.  Subject to these
 * conditions, you may download, copy, install, use, modify and distribute
 * modified or unmodified copies of this software in source and/or binary
 * form. No title or ownership is transferred hereby.
 *
 * 1) Any source code used, modified or distributed must reproduce and
 *    retain this copyright notice and list of conditions as they appear in
 *    the source file.
 *
 * 2) No right is granted to use any trade name, trademark, or logo of
 *    Broadcom Corporation.  The "Broadcom Corporation" name may not be
 *    used to endorse or promote products derived from this software
 *    without the prior written permission of Broadcom Corporation.
 *
 * 3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR IMPLIED
 *    WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED WARRANTIES OF
 *    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR
 *    NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT SHALL BROADCOM BE LIABLE
 *    FOR ANY DAMAGES WHATSOEVER, AND IN PARTICULAR, BROADCOM SHALL NOT BE
 *    LIABLE FOR DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *    OR OTHERWISE), EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * The functions implemented in this file (csum_partial,
 * csum_partial_copy_user) are intended to be compatible with MIPS
 * Linux.  They were not coded based on Linux sources, however, and the
 * expectation is that they may be useful in other environments as well.
 */

/*
 * XXX FIXME: this file needs work to exploit 64-bit registers.  So,
 * for now, we always use a 32-bit version.
 */
#undef __mips64

#include "sb1-opt-common.h"

#ifdef __linux__
#include <linux/config.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/errno.h>
#endif

#include <limits.h>
#if INT_MAX != LONG_MAX		/* If long != int, assume 64bit ptrs.  */
#  define LOADADDR dla
#else
#  define LOADADDR la
#endif

#ifndef PTR
# if INT_MAX != LONG_MAX
#  define PTR .dword
# else
#  define PTR .word
# endif
#endif

#ifndef EFAULT
# define EFAULT 14		/* Typical UN*X value.  */
#endif

#if defined(USE_DOUBLE)
#define LDHALFU lwu
#else
#define LDHALFU lhu
#endif /* USE_DOUBLE */
	
#ifdef __MIPSEB__
#define SHIFT_DISCARD SRLV
#define UNSHIFT_DISCARD SLLV
#endif
#ifdef __MIPSEL__
#define SHIFT_DISCARD SLLV
#define UNSHIFT_DISCARD SRLV
#endif

#define FIRST(unit) ((unit)*NBYTES)
#define REST(unit)  (FIRST(unit)+NBYTES-1)
#define UNIT(unit)  FIRST(unit)

#define ADDRMASK (NBYTES-1)
	
### N.B. In kernel mode, a prefetch may cause a bus error, and bus
### errors are not precise, so we can't attach a handler.
### The kernel must arrange to reserve, say, 1 page of RAM at the end of
### each cacheable, unmapped (i.e., KSEG0) ram region so that kernel 
### prefetches won't trigger bogus bus errors.
### 

#ifndef PREF
#ifdef CONFIG_CPU_HAS_PREFETCH
#define PREF(hint,addr)			\
	.set push;			\
	.set mips64;			\
	pref	hint, addr;		\
	.set pop;
#else
#define PREF(hint,addr)
#endif /* CONFIG_CPU_HAS_PREFETCH */
#endif /* PREF */

### Use PREFD instead of PREF when filling a delay slot.

#ifdef CONFIG_CPU_HAS_PREFETCH
#define PREFD(hint,addr) PREF(hint,addr)
#else
#define PREFD(hint,addr) nop
#endif
	
### N.B. The danger of using streaming prefetches is that one stream
### may evict the other before the cpu consumes it.  Hence, non-streaming
### hints are used below.

	.text
	.set	noreorder
	.set	noat
	
### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### 
### unsigned int
### csum_partial(const char *src,
###		 int len,
###		 unsigned int partial_sum)
### 
### Compute checksum of len bytes at src.  Result is a 32-bit value
### that needs to be folded down to 16 bits.
### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### 
	
### Register linkage (standard o32 ABI)

#define src a0
#define len a1
#define partial_sum a2
#define sum0 v0
	
###  Temps

#define limithw t6
#define count t7
#define limit t8
#define byteswap t9

	.align	5
LEAF(csum_partial)
	## Note: src may be unaligned, len may be 0.
	## The "issue break"s below are very approximate.
	## Issue delays for dcache fills will perturb the schedule, as will
	## load queue full replay traps, etc.
	
	## issue break
	PREF(	PREF_LOAD,  0(src) )
	move	t0, zero		# default to no initial odd byte
	beqz	len, zero_len
	 andi	byteswap, src, 0x1	# byteswap to agree with partial_sum
	
	## issue break
	PREF(	PREF_LOAD,  1*32(src) )
	PREF(	PREF_LOAD,  2*32(src) )
	beqz	byteswap, 1f
	 ADD	limit, src, len
	## issue break
	lbu	t0, 0(src)		# get initial odd byte
	ADD	src, src, 1
	SRL	t2, partial_sum, 8	# byteswap partial_sum
	SLL	t3, partial_sum, (NBYTES-1)*8
	## issue break
	or	partial_sum, t2, t3
	SUB	len, len, 1
#ifdef __MIPSEL__
	## issue break
	SLL	t0, t0, 8
#endif
1:	
	## Fold partial_sum down to a HALF UNIT to prevent overflowing a
	## UNIT in main loops below.  Low half goes into sum0; hi half into
	## sum1.
#ifdef USE_DOUBLE
	fixme
#else
	andi	sum0, partial_sum, 0xffff
#endif
#define sum1 partial_sum
	SRL	sum1, partial_sum, 8*NBYTES/2
	ADD	sum0, sum0, t0			# initial odd byte, or zero
	
	## issue break
	PREF(	PREF_LOAD,  3*32(src) )
	PREF(	PREF_LOAD,  4*32(src) )
	SRL	count, len, LOG_NBYTES+3	# +3 for 8 UNITS/iter
	li	limithw, (-1) << (LOG_NBYTES-1)	# limithw is just past
	## issue break
	and	limithw, limit, limithw		# last hw-aligned addr to csum
	beqz	count, cleanup
	 SUB	count, count, 1
	## issue break
	.align 4
big_csum:
#define CSUM_UNIT(n)				\
	LDHALFU	t0, UNIT(n)(src);		\
	LDHALFU	t1, UNIT(n)+(NBYTES/2)(src);	\
	ADD	sum0, sum0, t0;			\
	ADD	sum1, sum1, t1

	## issue break
	CSUM_UNIT(0)
	## issue break
	CSUM_UNIT(1)
	## issue break
	CSUM_UNIT(2)
	## issue break
	CSUM_UNIT(3)
	## issue break
	CSUM_UNIT(4)
	## issue break
	CSUM_UNIT(5)
	## issue break
	CSUM_UNIT(6)
	## issue break
	CSUM_UNIT(7)
	## issue break
	PREF(	PREF_LOAD,  9*32(src))
	ADD	src, src, 8*NBYTES	# need short pipe
	beqz	count, cleanup
	 SUB	count, count, 1
	## issue break
	CSUM_UNIT(0)
	## issue break
	CSUM_UNIT(1)
	## issue break
	CSUM_UNIT(2)
	## issue break
	CSUM_UNIT(3)
	## issue break
	CSUM_UNIT(4)
	## issue break
	CSUM_UNIT(5)
	## issue break
	CSUM_UNIT(6)
	## issue break
	CSUM_UNIT(7)
	## issue break
	PREF(	PREF_LOAD,  9*32(src))
	ADD	src, src, 8*NBYTES	# need short pipe
	bnez	count, big_csum
	 SUB	count, count, 1
cleanup:
	beq	src, limithw, cleanup_quarter
	 nop
	
	.align 4
1:
	LDHALFU	t0, UNIT(0)(src)
	ADD	src, src, NBYTES/2	# need short pipe
	beq	src, limithw, cleanup_quarter
	 ADD	sum0, sum0, t0
	LDHALFU	t0, UNIT(0)(src)
	ADD	src, src, NBYTES/2	# need short pipe
	bne	src, limithw, 1b
	 ADD	sum0, sum0, t0
cleanup_quarter:
	## less than a HALF UNIT to go
#ifdef USE_DOUBLE
	may need to do a lhu
#endif
cleanup_byte:
	## at most a byte to go
	## assume common case is no trailing byte
	bne	src, limit, last_byte
	 ADD	sum0, sum0, sum1	# merge two sums; carryout == 0 'cause
					# max packet size and initially 16-bit
	## issue break
	bnez	byteswap, final_swap
	 nop
	## issue break
done:
	jr	ra
	 nop

last_byte:
	lbu	t0, 0(src)
#ifndef CONFIG_CPU_SB1
	 nop
#endif
#ifdef __MIPSEB__
	SLL	t0, t0, 8
#endif
	beqz	byteswap, done
	 ADD	sum0, sum0, t0

	.align 4
final_swap:
	## byteswap result
	SRL	t1, sum0, 8
	SLL	t0, sum0, (NBYTES-1)*8
	## issue break
	jr	ra
	 or	sum0, t0, t1

zero_len:
	jr	ra
	 move	sum0, partial_sum
	END(csum_partial)
#undef src
#undef len
#undef partial_sum
#undef sum0
#undef sum1
#undef limithw
#undef limit
#undef byteswap


### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### 
### unsigned int
### csum_partial_copy_user(const char *src,
###		      	   char *dst,
###		      	   int len,
###		      	   unsigned int partial_sum,
###     	      	   int *err_ptr)
### 
### Copy to/from userspace and compute checksum.  If an access from src
### causes an exception, zero the rest of the dst buffer and store
### -EFAULT in *err_ptr.  The checksum is undefined if an exception
### happens.
### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### 
	
### Register linkage (standard o32 ABI)

#define src  a0
#define dst  a1
#define len  a2
#define partial_sum a3
#define err_ptr_addr 16(sp)
	
### Some temp variables

#define sum0   	 v0
#define sum1   	 v1
#define carry0   t6
#define carry1   t7
#define rem      t8
#define byteswap t9

#define EXC(inst_reg,addr,handler)		\
9:	inst_reg, addr;				\
	.section __ex_table,"a";		\
	PTR	9b, handler;			\
	.previous
	
### Implementation Note
### 
### Except for lengths < NBYTES, sums are done a UNIT (word or dword) at
### a time.  The alignment of the ith byte within its UNIT is determined by
### the low order bits of the initial dst pointer.  This alignment is the
### opposite of the partial_sum argument exactly when dst is odd, in which
### case partial_sum is byteswapped initially, and the final sum is 
### byteswapped just before returning.

	.align	5
LEAF(csum_partial_copy_user)
	## Note: dst & src may be unaligned, len may be 0
	## The "issue break"s below are very approximate.
	## Issue delays for dcache fills will perturb the schedule, as will
	## load queue full replay traps, etc.
	
	sltu	t2, len, NBYTES 	# long pipe
	andi	byteswap, dst, 0x1	# byteswap to agree with partial_sum
	PREF(	PREF_LOAD,  0(src) )		#  unless < NBYTES
	PREF(	PREF_STORE, 0(dst) )
	
	move	carry0, zero
	move	carry1, zero
	bnez	t2, small_csum		# If len < NBYTES use byte operations
	 move	sum0, zero
	
	## issue break
	PREF(	PREF_LOAD,  1*32(src) )
	PREF(	PREF_STORE, 1*32(dst) )
	move	sum1, partial_sum
	and	t1, dst, ADDRMASK

	## issue break
	PREF(	PREF_LOAD,  2*32(src) )
	PREF(	PREF_STORE, 2*32(dst) )
	and	t0, src, ADDRMASK
	beqz	byteswap, 1f
	
	## issue break
	 SRL	t3, partial_sum, 8	# byteswap partial_sum into sum1
	SLL	t2, partial_sum, (NBYTES-1)*8
	
	or	sum1, t2, t3
1:	
	## issue break
	bnez	t1, dst_unaligned
	 PREFD(	PREF_LOAD,  3*32(src) )
	## issue break
	bnez	t0, src_unaligned_dst_aligned
	 PREFD(	PREF_STORE, 3*32(dst) )
	## use delay slot for fall-through
	## src and dst are aligned; need to compute rem
both_aligned:
	 SRL	t0, len, LOG_NBYTES+3    # +3 for 8 units/iter
	## issue break
	beqz	t0, cleanup_both_aligned # len < 8*NBYTES
	 and	rem, len, (8*NBYTES-1)	 # rem = len % (8*NBYTES)
	.align	4
1:
#define c0 t4
#define c1 t5
	## issue break
EXC(	LOAD	t0, UNIT(0)(src),	l_exc)
EXC(	LOAD	t1, UNIT(1)(src),	l_exc_copy)
	ADD	sum0, sum0, t0
	ADD	sum1, sum1, t1
	## issue break
EXC(	LOAD	t2, UNIT(2)(src),	l_exc_copy)
EXC(	LOAD	t3, UNIT(3)(src),	l_exc_copy)
	sltu	c0, sum0, t0
	ADD	sum0, sum0, t2
	## issue break
	sltu	c1, sum1, t1
	ADD	sum1, sum1, t3
	## issue break
#ifdef USE_DOUBLE
	PREF(	PREF_LOAD,  9*32(src) )
	PREF(	PREF_STORE, 9*32(dst) )
#endif
	ADD	carry0, carry0, c0
	sltu	c0, sum0, t2	
	## issue break
	ADD	carry1, carry1, c1
	sltu	c1, sum1, t3
	## issue break
	ADD	carry0, carry0, c0
	ADD	carry1, carry1, c1
	## issue break
EXC(	STORE	t0, UNIT(0)(dst),	s_exc)
EXC(	STORE	t1, UNIT(1)(dst),	s_exc)
	## issue break
EXC(	STORE	t2, UNIT(2)(dst),	s_exc)
EXC(	STORE	t3, UNIT(3)(dst),	s_exc)

	## issue break
EXC(	LOAD	t0, UNIT(4)(src),	l_exc_copy)
EXC(	LOAD	t1, UNIT(5)(src),	l_exc_copy)
	ADD	sum0, sum0, t0
	ADD	sum1, sum1, t1
	## issue break
EXC(	LOAD	t2, UNIT(6)(src),	l_exc_copy)
EXC(	LOAD	t3, UNIT(7)(src),	l_exc_copy)
	sltu	c0, sum0, t0
	ADD	sum0, sum0, t2
	## issue break
	ADD	dst, dst, 8*NBYTES   # need short pipe
	sltu	c1, sum1, t1
	ADD	sum1, sum1, t3
	## issue break
	ADD	src, src, 8*NBYTES   # need short pipe
	ADD	carry0, carry0, c0
	sltu	c0, sum0, t2	
	## issue break
	SUB	len, len, 8*NBYTES   # use short pipe
	ADD	carry1, carry1, c1
	sltu	c1, sum1, t3
	## issue break
	ADD	carry0, carry0, c0
	ADD	carry1, carry1, c1
	## issue break
EXC(	STORE	t0, UNIT(4-8)(dst),	s_exc)
EXC(	STORE	t1, UNIT(5-8)(dst),	s_exc)
	## issue break
EXC(	STORE	t2, UNIT(6-8)(dst),	s_exc)
EXC(	STORE	t3, UNIT(7-8)(dst),	s_exc)
	## issue break
	PREF(	PREF_LOAD,  8*32(src) )
	bne	len, rem, 1b
	 PREFD(	PREF_STORE, 8*32(dst) )
	## len == rem == the number of bytes left to copy < 8*NBYTES
cleanup_both_aligned:
	and	rem, len, (NBYTES-1)	# rem = len % NBYTES
	beq	len, rem, cleanup_bytes_both_aligned
	 nop
1:	
EXC(	LOAD	t0, UNIT(0)(src),	l_exc)
	 ADD	dst, dst, NBYTES
	ADD	sum0, sum0, t0
	SUB	len, len, NBYTES
	## issue break
	sltu	t2, sum0, t0
	ADD	src, src, NBYTES
	## issue break
	ADD	carry0, carry0, t2
	bne	len, rem, 1b
	## issue break
EXC(	 STORE	t0, UNIT(0-1)(dst),	s_exc)
cleanup_bytes_both_aligned:
	## src and dst are aligned, need to copy len bytes (len < NBYTES)
	## A loop would do only a byte at a time with possible branch 
	## mispredicts.  Can't do an explicit LOAD dst,mask,or,STORE
	## because can't assume read-access to dst.  Instead, use
	## STREST dst, which doesn't require read access to dst.
	
	## This code should perform better than a simple loop on modern,
	## wide-issue mips processors because the code has fewer branches and
	## more instruction-level parallelism.
#define bits t2
	beqz	len, finish
	 ADD	t1, dst, len	# t1 is just past last byte of dst
	li	bits, 8*NBYTES
	SLL	rem, len, 3	# rem = number of bits to keep
EXC(	LOAD	t0, 0(src),		l_exc)
	SUB	bits, bits, rem	# bits = number of bits to discard
	SHIFT_DISCARD t3, t0, bits
	
	UNSHIFT_DISCARD t0, t3, bits
	
	ADD	sum0, sum0, t0
	
	sltu	t0, sum0, t0
EXC(	STREST	t3, -1(t1),		s_exc)
	
	ADD	carry0, carry0, t0

finish:
	## combine carry0, carry1, the upper and lower halves of sum0 & sum1
#ifdef USE_DOUBLE
	dsll32	t0, sum0, 0
	dsll32	t1, sum1, 0
	ADD	sum0, sum0, t0
	ADD	sum1, sum1, t1
	sltu	t0, sum0, t0
	sltu	t1, sum1, t1
	dsrl32	sum0, sum0, 0
	dsrl32	sum1, sum1, 1
	ADD	carry0, carry0, t0
	ADD	carry1, carry1, t1
#endif
	## combine sum0 += sum1 with overflow into sum1; carry0 += carry1
	ADD	sum0, sum0, sum1
	ADD	carry0, carry0, carry1	# carries don't overflow NBYTES
	sltu	sum1, sum0, sum1	# sum1 = carryout(sum0 + sum1)
	
	ADD	sum1, carry0, sum1	# sum1 = sum of all carries
	
	ADD	sum0, sum0, sum1	# sum0 = everything, but may have
					#  overflowed
	bnez	byteswap, 1f
	 sltu	sum1, sum0, sum1	# sum1 = overflowed?
					# sum1 == 1 implies sum0 < 0xff..ff
	jr	ra
	 ADD	sum0, sum0, sum1	# replicated below

	.align 4
1:
	## replicate final inst in delay slot above
	ADD	sum0, sum0, sum1	# replicated 
	## byteswap result
	SRL	t1, sum0, 8
	SLL	t0, sum0, (NBYTES-1)*8
	jr	ra
	 or	sum0, t0, t1

dst_unaligned:
	## dst is unaligned
	## t0 = src & ADDRMASK
	## t1 = dst & ADDRMASK; T1 > 0
	## len >= NBYTES
	
	## Copy enough bytes to align dst
	## Set match = (src and dst have same alignment)
#define match rem
	## issue break
EXC(	LDFIRST	t3, FIRST(0)(src),	l_exc)
	ADD	t2, zero, NBYTES
	xor	match, t0, t1
	SUB	t2, t2, t1	# t2 = number of bytes copied
	## issue break
EXC(	LDREST	t3, REST(0)(src),	l_exc_copy)
	SLL	t4, t1, 3	# t4 = number of bits discarded
	PREF(	PREF_STORE, 3*32(dst) )
	ADD	src, src, t2
	## issue break
	SHIFT_DISCARD t4, t3, t4
	SUB	len, len, t2
	## issue break
	ADD	sum0, sum0, t4
	PREF(	PREF_LOAD,  4*32(dst) )
	PREF(	PREF_STORE, 4*32(dst) )
	## issue break
	sltu	t4, sum0, t4
EXC(	STFIRST t3, FIRST(0)(dst),	s_exc)
	## issue break
	beqz	len, finish
	 ADD	carry0, carry0, t4
	## issue break
	beqz	match, both_aligned
	 ADD	dst, dst, t2
src_unaligned_dst_aligned:
	## Update len, src, and dst for number of bytes copied
	and	rem, len, (8*NBYTES-1)   # rem = len % 8*NBYTES
	beq	rem, len, cleanup_src_unaligned
#define c2 partial_sum
#define c3 c0
	 move	c3, zero		# clear carry bit for 1st iter
	.align	4
1:
### Avoid consecutive LD*'s to the same register since some mips 
### implementations can't issue them in the same cycle.
### It's OK to load FIRST(N+1) before REST(N) because the two addresses
### are to the same word (unless src is aligned, but it's not).
	## why doesn't this issue with backward branch of loop on pass1?
EXC(	LDFIRST	t0, FIRST(0)(src),	l_exc)
EXC(	LDFIRST	t1, FIRST(1)(src),	l_exc_copy)
	## issue break
	LDREST	t0, REST(0)(src)	# can't fault
EXC(	LDREST	t1, REST(1)(src),	l_exc_copy)
	ADD	sum0, sum0, t0
	ADD	sum1, sum1, t1
	## issue break
	LDFIRST	t2, FIRST(2)(src)	# can't fault
EXC(	LDFIRST	t3, FIRST(3)(src),	l_exc_copy)
	ADD	carry1, carry1, c3	# keep lifetimes of c3 and c0 disjoint
	sltu	c0, sum0, t0
	## issue break
	LDREST	t2, REST(2)(src)	# can't fault
EXC(	LDREST	t3, REST(3)(src),	l_exc_copy)
	sltu	c1, sum1, t1
	ADD	sum0, sum0, t2
	## issue break
	sltu	c2, sum0, t2
	ADD	sum1, sum1, t3
	PREF(	PREF_LOAD,  8*32(src))
	PREF(	PREF_STORE, 8*32(dst))
	## issue break
	ADD	carry0, carry0, c0	# keep lifetimes of c3 and c0 disjoint
	sltu	c3, sum1, t3
#ifdef USE_DOUBLE
	PREF(	PREF_LOAD,  UNIT(9)(src))
	PREF(	PREF_STORE, UNIT(9)(dst))
#endif
	## issue break
EXC(	STORE	t0, UNIT(0)(dst),	s_exc)
EXC(	STORE	t1, UNIT(1)(dst),	s_exc)
	## issue break
	LDFIRST	t0, FIRST(4)(src)	# can't fault
EXC(	LDFIRST	t1, FIRST(5)(src),	l_exc_copy)
	ADD	carry1, carry1, c1
	ADD	carry0, carry0, c2
	## issue break
EXC(	STORE	t2, UNIT(2)(dst),	s_exc)
EXC(	STORE	t3, UNIT(3)(dst),	s_exc)

	## issue break
	LDREST	t0, REST(4)(src)	# can't fault
EXC(	LDREST	t1, REST(5)(src),	l_exc_copy)
	ADD	sum0, sum0, t0
	ADD	sum1, sum1, t1
	## issue break
	LDFIRST	t2, FIRST(6)(src)	# can't fault
EXC(	LDFIRST	t3, FIRST(7)(src),	l_exc_copy)
	ADD	carry1, carry1, c3	# keep lifetimes of c3 and c0 disjoint
	sltu	c0, sum0, t0
	## issue break
	LDREST	t2, REST(6)(src)	# can't fault
EXC(	LDREST	t3, REST(7)(src),	l_exc_copy)
	sltu	c1, sum1, t1
	ADD	sum0, sum0, t2
	## issue break
	sltu	c2, sum0, t2
	ADD	src, src, 8*NBYTES	# need short pipe
	ADD	sum1, sum1, t3
	## issue break
	SUB	len, len, 8*NBYTES	# use short pipe
	ADD	carry0, carry0, c0	# keep lifetimes of c3 and c0 disjoint
	sltu	c3, sum1, t3
	## issue break
EXC(	STORE	t0, UNIT(4)(dst),	s_exc)
EXC(	STORE	t1, UNIT(5)(dst),	s_exc)
	## issue break
	ADD	dst, dst, 8*NBYTES	# need short pipe
	ADD	carry1, carry1, c1
	ADD	carry0, carry0, c2
	## issue break
EXC(	STORE	t2, UNIT(6-8)(dst),	s_exc)
EXC(	STORE	t3, UNIT(7-8)(dst),	s_exc)

	## issue break
	bne	len, rem, 1b
	 nop
	ADD	carry1, carry1, c3	# add c3 carry bit for last iter

cleanup_src_unaligned:
	beqz	len, finish
	 and	rem, len, NBYTES-1  # rem = len % NBYTES
	beq	rem, len, csum_bytes
1:
EXC(	 LDFIRST t0, FIRST(0)(src),	l_exc)
EXC(	LDREST	t0, REST(0)(src),	l_exc_copy)
	 ADD	src, src, NBYTES
	ADD	sum0, sum0, t0
	SUB	len, len, NBYTES
	sltu	t2, sum0, t0
	ADD	dst, dst, NBYTES
	ADD	carry0, carry0, t2
	bne	len, rem, 1b
EXC(	 STORE	t0, UNIT(0-1)(dst),	s_exc)

csum_bytes_checklen:
	beqz	len, finish
	 nop
csum_bytes:
	## need to csum and copy len bytes
	## 0 < len <= NBYTES  (len = NBYTES possible when called from handler)
	## src may be unaligned
	## dst may be unaligned
	## an even number of bytes have been csum'd

#ifdef __MIPSEB__
	
#define SHIFT_EVEN_CSUM(reg,tmp,sum,carry)	\
	SLL	tmp, reg, 8;			\
	ADD	sum, sum, tmp;			\
	sltu	tmp, sum, tmp;			\
	ADD	carry, carry, tmp

#define SHIFT_ODD_CSUM(reg,tmp,sum,carry)	\
	ADD	sum, sum, reg;			\
	sltu	tmp, sum, reg;			\
	ADD	carry, carry, tmp
	
#endif
	
#ifdef __MIPSEL__
	
#define SHIFT_EVEN_CSUM(reg,tmp,sum,carry)	\
	ADD	sum, sum, reg;			\
	sltu	tmp, sum, reg;			\
	ADD	carry, carry, tmp
	
#define SHIFT_ODD_CSUM(reg,tmp,sum,carry)	\
	SLL	tmp, reg, 8;			\
	ADD	sum, sum, tmp;			\
	sltu	tmp, sum, tmp;			\
	ADD	carry, carry, tmp
#endif

EXC(	lbu	t0, 0(src),		l_exc)
	 SUB	len, len, 1
	SHIFT_EVEN_CSUM(t0,t1,sum0,carry0)
	beqz	len, finish
EXC(	 sb	t0, 0(dst),		s_exc)
EXC(	lbu	t0, 1(src),		l_exc_copy)
	 SUB	len, len, 1
	SHIFT_ODD_CSUM(t0,t1,sum0, carry0)
	beqz	len, finish
EXC(	 sb	t0, 1(dst),		s_exc)
EXC(	lbu	t0, 2(src),		l_exc_copy)
	 SUB	len, len, 1
	SHIFT_EVEN_CSUM(t0,t1,sum0,carry0)
	beqz	len, finish
EXC(	 sb	t0, 2(dst),		s_exc)
EXC(	lbu	t0, 3(src),		l_exc_copy)
	 SUB	len, len, 1
	SHIFT_ODD_CSUM(t0,t1,sum0,carry0)
#ifdef USE_DOUBLE
	fixme -- 4 more bytes
#endif
	j	finish
EXC(	 sb	t0, 3(dst),		s_exc)
	
small_csum:
	## called with len < NBYTES; need to finish initialization
	move	sum1, partial_sum
	j	csum_bytes_checklen
	 move	byteswap, zero
	
##########################
### Exception Handlers ###
##########################

### Copy from src to dst a byte at a time.  The lb will eventually get an
### exception.  We could avoid this second exception at the cost of writing
### testing, and debugging many more exception handlers.
	
	.align	4
l_exc_copy:
EXC(	lb	t0, 0(src),		l_exc)
	ADD	src, src, 1
	sb	t0, 0(dst)
	SUB	len, len, 1
	bnez	len, l_exc_copy
	 ADD	dst, dst, 1
l_exc:
	li	t0, -EFAULT
	LOAD	t1, err_ptr_addr
	ADD	a0, zero, dst
	move	a1, zero
	## len is already in a2
#ifdef __PIC__
	LOADADDR t9, memset
	jr	t9
#else
	j	memset
#endif
	 STORE	t0, 0(t1)
	## Note: returned checksum is garbage

s_exc:
	li	t0, -EFAULT
	LOAD	t1, err_ptr_addr
	jr	ra
	 STORE	t0, 0(t1)
	## Note: returned checksum is garbage
	
	END(csum_partial_copy_user)
