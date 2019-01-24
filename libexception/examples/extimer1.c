/* $Id: extimer1.c,v 1.9 2003/04/24 23:47:20 cgd Exp $ */

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

/* extimer1.c

   An example of timer interrupt handling _without_ using the exception
   library's built-in exception handler facilities.  (To see an example
   of use of those facilities, see extimer2.c.)  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <exlib.h>
#include <sb1250-include/sbmips.h>

#define	TICK_INTERVAL	800000000

volatile int ticks;

static void
intr_handler (unsigned int code, struct exframe *ef)
{
  unsigned int count;

#if 0
  /* Really, you should try to avoid doing "lots of stuff" in an
     interrupt handler.  There may be, for instance, library
     reentrancy/locking issues to consider in your system.
     I.e., DON'T DO THIS IN REAL CODE!  Also, if there's any
     chance that your interrupt handler may invoke floating
     point code, don't forget to arrange for it to save the FP
     state!  */
  exframe_dump (ef);
#endif

  ticks++;

  /* Note that this will _not_ tick at a regular interval, due to
     slip at each interrupt.  Don't copy this code for precise
     timekeeping.  Writing the Compare register clears the interrupt.  */
  __asm__ ("mfc0 %0, $%1" : "=r"(count) : "i"(C0_COUNT));
  count += TICK_INTERVAL;
  __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(count), "i"(C0_COMPARE));

  /* Of course, if it was some other kind of interrupt, this might
     lose... */
}

int
main (void)
{
  unsigned int status;
  unsigned int count;
  int newticks, oldticks;

  exlib_init ();
  exlib_set_common_handlers ();

  /* Set up a custom interrupt handler, and arrange to save no FP
     regs when entering it for speed.  */
  exlib_set_handler (K_CAUSE_EXC_INT, intr_handler);
  exlib_set_fpsavemask (K_CAUSE_EXC_INT, EXFPSAVEMASK_NONE);

  /* Set timer (count/compare) interrupt to go off some time in the future.
     (The exact amount of time in the future depends on the CPU speed).  */
  __asm__ ("mfc0 %0, $%1" : "=r"(count) : "i"(C0_COUNT));
  count += TICK_INTERVAL;
  __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(count), "i"(C0_COMPARE));

  /* Enable IP7 interrupts (only).
     *sigh*  The sbmips.h names for these bits don't actually match the
     MIPS64 names!  */
  __asm__ ("mfc0 %0, $%1" : "=r"(status) : "i"(C0_STATUS));
  status |= M_SR_IE | M_SR_IMASK;
  status ^= M_SR_IBIT7 | M_SR_IBIT6 | M_SR_IBIT5 | M_SR_IBIT4 |
	    M_SR_IBIT3 | M_SR_IBIT2 | M_SR_IBIT1;
  printf ("Setting CP0 Status reg to 0x%x (enabling interrupts)...\n", status);
  __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(status), "i"(C0_STATUS));

  oldticks = ticks;
  do
    {
      newticks = ticks;
      if (oldticks != newticks)
        {
          printf ("tick! (%d)\n", newticks);
          oldticks = newticks;
        }
    }
  while (newticks < 5);

  printf ("got 5 ticks!");

  exlib_shutdown ();
  exit (EXIT_SUCCESS);
}
