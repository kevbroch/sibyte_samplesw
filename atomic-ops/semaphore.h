/* $Id: semaphore.h,v 1.4 2003/05/09 04:34:05 cgd Exp $ */

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

/* semaphore.h: MIPS counting semaphores.

   Types:

   typedef int semaphore;

   Functions:

   void sem_take (semaphore *sem);       (P)
   void sem_release (semaphore *sem);    (V)

   These routines implement counting semaphores, as described by
   Dijkstra.

   sem_take() implements Dijkstra's "P" operation: the semaphore's
   value is checked and if found to be non-zero is atomically
   decremented.  If the semaphore's values is already zero, the
   routine blocks execution (by spinning on the semaphore) until the
   value is non-zero, at which point the atomic check and decrement is
   attempted again.

   sem_release() implements Dijstra's "V" operation: the semaphore's
   value is atomically incremented.

   (You should initialize a new semaphore to the number of threads of
   execution allowed in the critical section.  Often, that'll be 1.)

   Implementation considerations:

   1. It is possible to implement these using asms for only the ll/sc
   ops, having the compiler implement the branches.  That would
   actually potentially shave an instruction or two in simple cases,
   _but_ it can also add instructions in the inner loop and it's
   fragile.  We have to provide implementations like these for the
   unoptimized case (in which case the compiler loads/stores lots of
   extraneous stuf), anyway, so just best to do it once.

   2. We make the asms take the semaphore pointer in a register and
   clobber memory, rather than modifying the value of "*sem" as an
   output.  This is so that we can ".set mips32" / ".set mips64" in
   the asms, without concern for that impacting the current ABI.
   (Otherwise, the assembler may treat relocations on the ll/sc
   instructions incorrectly.)

   3. Trapping adds/subtracts are used, to help catch errors
   (decrement below zero, or an extra release which if executed many
   times, would cause the semaphore to overflow).

   4. No 64-bit versions are provided.  They shouldn't be needed.  */

#ifndef _SIBYTE_SEMAPHORE_H_
#define _SIBYTE_SEMAPHORE_H_

typedef int semaphore;

static inline void
sem_take (volatile semaphore *sem)
{
  int tmp;

  __asm__ __volatile__ (
    "	.set push	   \n"
    "	.set noreorder	   \n"
    "   .set mips2         \n"
    "1: ll      %0, (%1)   \n"
    "   beqz    %0, 1b     \n" 
    "   addi    %0, %0, -1 \n" /* delay slot */
    "   sc      %0, (%1)   \n"
    "   beqz    %0, 1b     \n" 
    "	nop		   \n" /* delay slot */
    "	.set pop	   \n"
    : "=&r"(tmp) : "r"(sem) : "memory");
}

static inline void
sem_release (volatile semaphore *sem)
{
  int tmp;

  __asm__ __volatile__ (
    "	.set push	   \n"
    "	.set noreorder	   \n"
    "   .set mips2         \n"
    "1: ll      %0, (%1)   \n"
    "   addi    %0, %0, 1  \n"
    "   sc      %0, (%1)   \n"
    "   beqz    %0, 1b     \n" 
    "	nop		   \n" /* delay slot */
    "	.set pop	   \n"
    : "=&r"(tmp) : "r"(sem) : "memory");
}
#endif /* _SIBYTE_SEMAPHORE_H_ */
