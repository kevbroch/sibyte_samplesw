/* $Id: extrace.c,v 1.3 2004/04/02 19:31:13 cgd Exp $ */

/*
 * Copyright 2004
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

/* extrace.c

   Show an example of a very, very crude stack trace facility.

   Did I mention *very* crude?  (The idea was taken from the MIPS Linux
   kernel crash dump, but the code was not copied in any way.)

   Note that not all entries will necessarily be correct, and if some
   functions don't save RA to the stack (e.g., tail-calls), they
   won't be represented.  Use with caution.  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <exlib.h>

int is_code_address (unsigned long sp);
void extrace (unsigned int code, struct exframe *ef);

int
is_code_address (unsigned long sp)
{
  extern char _ftext[];
  extern char _etext[];

  return (sp >= (unsigned long)_ftext && sp < (unsigned long)_etext);
}

void
extrace (unsigned int code, struct exframe *ef)
{
  unsigned long sp, stack_top;
  int i;

  printf ("exception code (0x%x)", code);
  printf ("\n");

  exframe_dump (ef);

  sp = ef->ef_regs[29];
  if ((long)sp != ef->ef_regs[29])	/* 64-bit SP w/ ILP32 code?  */
    abort ();
  if ((sp & (sizeof(long) - 1)) != 0)	/* Unaligned stack pointer?  */
    abort ();

  /* Guess at the stack top.  In this case, round up to next 4k boundary.  */
  stack_top = (sp + 0xfff) & ~ 0xfff;

  printf ("\n");

  i = 0;
  printf ("possible stack trace:");
  fflush (stdout);

  for (i = 0; sp < stack_top; sp += sizeof (long))
    {
      unsigned long stackval = *(unsigned long *)sp;
      if (is_code_address (stackval))
        {
          printf ("%s [%p]", ((i++ % 4) == 0) ? "\n" : "", (void *)stackval);
          fflush (stdout);
        }
    }
  printf("\n");
  fflush (stdout);

  abort ();
}

static void
crash (void)
{
  printf ("about to crash!\n");
  *(volatile int *)1 = 1;
  printf ("crash!\n");
}

static void
bar (void)
{
  crash ();
  printf ("bar\n");
}

static void
foo (void)
{
  bar ();
  printf ("foo\n");
}

int
main (void)
{
  int i;

  exlib_init ();
  for (i = 0; i < EXCODE_NCODES; i++)
    {
      exlib_set_handler (i, extrace);
    } 

  foo ();
  printf ("survived foo\n");

  exlib_shutdown ();
  exit (EXIT_FAILURE);
}
