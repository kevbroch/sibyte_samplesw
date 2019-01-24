/* $Id: spinlock.h,v 1.5 2003/05/09 04:34:05 cgd Exp $ */

/*
 * Copyright 2003
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

/* spinlock.h: MIPS spin locks.

   Functions:

   void spinlock32_lock (uint32_t *lock, uint32_t lockval);
   void spinlock64_lock (uint64_t *lock, uint64_t lockval);

     Atomically lock a 32- or 64-bit spinlock, placing "lockval" into
     the lock.  "lockval" must be non-zero.  (Spins until the lock
     can be acquired.)

   uint32_t spinlock32_trylock (uint32_t *lock, uint32_t lockval);
   uint64_t spinlock64_trylock (uint64_t *lock, uint64_t lockval);

     Try to acquire a spinlock.  If successful, "lockval" is placed into
     the lock and zero is returned.  If unsuccessful, the existing lock
     value (at the time of the lock attempt) is returned.

   void spinlock32_release (uint32_t *lock);
   void spinlock64_release (uint64_t *lock);

     Release the specified spinlock (setting it to zero).

   Requirements:

   1. This header assumes that at the time it is included, the types
   uint32_t and uint64_t have been defined.

   2. The 64-bit functions are only provided if compiling in an
   environment which supports 64-bit data in registers.

   Implementation considerations:

   1. It is possible to implement these using asms for only the ll/sc
   ops, having the compiler implement the branches.  That would
   actually potentially shave an instruction or two in simple cases,
   _but_ it can also add instructions in the inner loop and it's
   fragile.  We have to provide implementations like these for the
   unoptimized case (in which case the compiler loads/stores lots of
   extraneous stuf), anyway, so just best to do it once.

   2. We make the asms take the lock pointer in a register and clobber
   memory, rather than modifying the value of "*lock" as an output.
   This is so that we can ".set mips32" / ".set mips64" in the asms,
   without concern for that impacting the current ABI.  (Otherwise,
   the assembler may treat relocations on the ll/sc instructions
   incorrectly.)

   3. It's possible to shave an instruction off if you assume that
   lockval will be a constant.  In practice, it's often useful to
   stuff the lockholder's unique ID in the lock, for debugging, so the
   value to store in the lock is a variable.  */
        
#ifndef _SIBYTE_SPINLOCK_H_
#define _SIBYTE_SPINLOCK_H_

static inline void
spinlock32_lock (volatile uint32_t *lock, uint32_t lockval)
{
  uint32_t tmp;

  __asm__ __volatile__ (
    "	.set push	 \n"
    "	.set noreorder	 \n"
    "   .set mips2       \n"
    "1: ll      %0, (%1) \n"
    "   bnez    %0, 1b   \n"
    "   move	%0, %2	 \n" /* delay slot */
    "   sc      %0, (%1) \n"
    "   beqz    %0, 1b   \n" 
    "	nop		 \n" /* delay slot */
    "	.set pop	 \n"
    : "=&r"(tmp) : "r"(lock), "r"(lockval) : "memory");
}

static inline uint32_t
spinlock32_trylock (volatile uint32_t *lock, uint32_t lockval)
{
  uint32_t tmp;

  __asm__ __volatile__ (
    "	.set push	 \n"
    "	.set noreorder	 \n"
    "   .set mips2       \n"
    "1: ll      %0, (%1) \n"
    "   bnez    %0, 2f   \n"
    "   nop              \n" /* delay slot */
    "   move	%0, %2	 \n"
    "   sc      %0, (%1) \n"
    "   beqz    %0, 1b   \n" 
    "	move    %0, $0   \n" /* delay slot */
    "2:                  \n"
    "	.set pop	 \n"
    : "=&r"(tmp) : "r"(lock), "r"(lockval) : "memory");
  return (tmp);
}

static inline void
spinlock32_release (volatile uint32_t *lock)
{
  *lock = 0;
}

#ifdef __mips64
/* Only provide 64-bit versions if __mips64 is defined, which means
   that we're compiling for an environment which supports 64-bit data
   in registers.  */

static inline void
spinlock64_lock (volatile uint64_t *lock, uint64_t lockval)
{
  uint64_t tmp;

  __asm__ __volatile__ (
    "	.set push	 \n"
    "	.set noreorder	 \n"
    "	.set mips3	 \n"
    "1: lld     %0, (%1) \n"
    "   bnez    %0, 1b   \n"
    "   move	%0, %2	 \n" /* delay slot */
    "   scd     %0, (%1) \n"
    "   beqz    %0, 1b   \n" 
    "	nop		 \n" /* delay slot */
    "	.set pop	 \n"
    : "=&r"(tmp) : "r"(lock), "r"(lockval) : "memory");
}

static inline uint64_t
spinlock64_trylock (volatile uint64_t *lock, uint64_t lockval)
{
  uint64_t tmp;

  __asm__ __volatile__ (
    "	.set push	 \n"
    "	.set noreorder	 \n"
    "   .set mips3       \n"
    "1: lld     %0, (%1) \n"
    "   bnez    %0, 2f   \n"
    "   nop              \n" /* delay slot */
    "   move	%0, %2	 \n"
    "   scd     %0, (%1) \n"
    "   beqz    %0, 1b   \n" 
    "	move    %0, $0   \n" /* delay slot */
    "2:                  \n"
    "	.set pop	 \n"
    : "=&r"(tmp) : "r"(lock), "r"(lockval) : "memory");
  return (tmp);
}

static inline void
spinlock64_release (volatile uint64_t *lock)
{
  *lock = 0;
}
#endif /* __mips64 */
#endif /* _SIBYTE_SPINLOCK_H_ */
