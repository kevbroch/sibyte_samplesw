/* $Id: accswitchtest.c,v 1.8 2003/05/09 04:48:06 cgd Exp $ */

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

/*
 * accswitchtest.c: simple accumulator context switch test.
 *
 * Run multiple copies of this test simultaneously on an OS that
 * support MDMX, to help test its ability to context switch the MDMX
 * accumulator correctly.  (You should run at least 2 per CPU core
 * on which the OS allows execution.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mdmx.h"

/* (1<<30) iterations takes approximately 20 seconds on a 700MHz
   BCM1250.  */
#define	ITER_COUNT	(1 << 30)

int
main (void)
{
  mdmxreg r1, r2, r3;
  pid_t p;
  int i, mm1, mm2, mm3;

  /* Enable use of MDMX resources.  */
  mdmx_enable ();


  /* Place our PID into the accumulator.  */
  p = getpid ();
  mdmx_acc_write_ob (mdmx_reg_from_int (0x1000000000000000ULL | p),
                     mdmx_reg_from_int (0x2000000000000000ULL | p),
                     mdmx_reg_from_int (0x3000000000000000ULL | p));

  printf ("%d: testing\n", p);

  /* Read the accumulator values repeatedly, and check them.  */
  for (i = 0; i < ITER_COUNT; i++)
    {
      mdmx_acc_read_ob (&r1, &r2, &r3);

      
      mm1 = mdmx_int_from_reg (r1) != (mdmxint)(0x1000000000000000ULL | p);
      mm2 = mdmx_int_from_reg (r2) != (mdmxint)(0x2000000000000000ULL | p);
      mm3 = mdmx_int_from_reg (r3) != (mdmxint)(0x3000000000000000ULL | p);
      if (mm1 || mm2 || mm3)
        {
	  printf ("%d: mismatch on iteration %d:\n", p, i);
	  printf ("%d: h: expected 0x%016" PRIxMDMXINT ", got 0x%016"
                  PRIxMDMXINT " [%s]\n", p,
		  (mdmxint)(0x1000000000000000ULL | p), mdmx_int_from_reg (r1),
		  mm1 ? "BAD" : "OK");
	  printf ("%d: m: expected 0x%016" PRIxMDMXINT ", got 0x%016"
                  PRIxMDMXINT " [%s]\n", p,
		  (mdmxint)(0x2000000000000000ULL | p), mdmx_int_from_reg (r2),
		  mm2 ? "BAD" : "OK");
	  printf ("%d: l: expected 0x%016" PRIxMDMXINT ", got 0x%016"
                  PRIxMDMXINT " [%s]\n", p,
		  (mdmxint)(0x3000000000000000ULL | p), mdmx_int_from_reg (r3),
		  mm3 ? "BAD" : "OK");
	  abort ();
        }
    }

  printf ("%d: passed\n", p);

  exit (EXIT_SUCCESS);
}
