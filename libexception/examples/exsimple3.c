/* $Id: exsimple3.c,v 1.6 2003/04/03 17:33:15 cgd Exp $ */

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

/* exsimple3.c

   Show exception library initialization and installation of common
   handlers, tweak the handlers to save FP registers, and then cause
   an exception.  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <exlib.h>

/* Not static so that GCC is less likely to inline the call away.  */
double f (double foo);

double
f (double foo)
{
  printf ("foo = %f\n", foo);

  return foo / 3.0;
}

int
main (void)
{
  int i;
  int a[2];

  exlib_init ();
  exlib_set_common_handlers ();

  /* Make _sure_ that FP regs are saved on exception.  */
  for (i = 0; i < EXCODE_NCODES; i++)
    exlib_set_fpsavemask (i, EXFPSAVEMASK_ALL);

  /* Cause something to be done with FP regs.  */
  f (9.0);

  /* Cause an AdES.  */
  *(int *)((char *)a + 1) = 1;

  printf ("survived AdES?!");

  exlib_shutdown ();
  exit (EXIT_FAILURE);
}
