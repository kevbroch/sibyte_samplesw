/* $Id: extimer2.c,v 1.6 2003/04/24 23:47:20 cgd Exp $ */

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

/* extimer2.c

   An example of timer interrupt handling using the exception library's
   built-in interrupt handler facilities.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <exlib.h>
#include <sb1250-include/sbmips.h>

#define	TICK_INTERVAL	800000000
volatile int next_compare_value;

volatile int ticks;

struct ihand timer_handler;

static int
timer_intr_handler (void *arg, struct exframe *ef)
{
  unsigned int count;
  unsigned int last, next;
  int wraparound, gt_last, lte_next;

  __asm__ __volatile__ ("mfc0 %0, $%1" : "=r"(count) : "i"(C0_COUNT));

  next = next_compare_value;
  last = next - TICK_INTERVAL;

  /* If the current 'count' register is still between the last count
     value and the current one, we should not have gotten a compare
     interrupt.  (There's currently no other way to tell that we're
     expecting a compare interrupt.)

     Note that the comparisons here have to take into account the
     fact that the last..next range may wrap around 32 bits.  */

  /* No wraparound -> no interrupt if after last and before next.
     Wraparound -> no interrupt if after last _or_ before next.  */
  wraparound = next < last;
  gt_last = count > last;
  lte_next = count <= next;
  if ((!wraparound && gt_last && lte_next)
      || (wraparound && (gt_last || lte_next)))
    return 0;

  /* OK, it looks as if an interval has expired.  (In fact, it's
     possible that multiple intervals have expired if interrupts were
     disabled for a long period of time, but we don't attempt to
     handle that.)  */
  ticks++;
  next += TICK_INTERVAL;

  /* Writing the Compare register clears the interrupt.  */
  __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(next), "i"(C0_COMPARE));
  next_compare_value = next;

  /* Indicate that this function handled an interrupt.  */
  return 1;
}

int
main (void)
{
  unsigned int count;
  int newticks, oldticks;

  exlib_init ();
  exlib_set_common_handlers ();
  intr_init ();

  INIT_IHAND(&timer_handler, timer_intr_handler, NULL, EXFPSAVEMASK_NONE);
  intr_handler_add (7, &timer_handler);

  /* Set timer (count/compare) interrupt to go off some time in the future.
     (The exact amount of time in the future depends on the CPU speed).  */
  __asm__ ("mfc0 %0, $%1" : "=r"(count) : "i"(C0_COUNT));
  count += TICK_INTERVAL;
  __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(count), "i"(C0_COMPARE));
  next_compare_value = count;

  /* Enable IP7 interrupts.  */
  printf ("Enabling interrupts... ");
  intr_set_ie (NULL);
  intr_enable_int (NULL, 7);
  printf ("done.\n");

  oldticks = ticks;
  do
    {
      newticks = ticks;
      if (oldticks != newticks)
        {
          printf ("tick! (%d, next = 0x%08x)\n", newticks,
                  next_compare_value);
          oldticks = newticks;
        }
    }
  while (newticks < 20);

  printf ("got %d ticks!", newticks);

  intr_shutdown ();
  exlib_shutdown ();
  exit (EXIT_SUCCESS);
}
