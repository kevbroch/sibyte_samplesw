/* $Id: undemo.c,v 1.16 2004/05/16 23:46:51 cgd Exp $ */

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

/* undemo.c: Demonstrate and test unaligned access handler.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef UNHANDLER_HEADER
# define UNHANDLER_HEADER	"unhandler.h"
#endif
#include UNHANDLER_HEADER

#define MARKER_INSN() \
  __asm__ __volatile__ (".set push; .set mips32 ; ssnop ; .set pop");

#define DO_LOAD_OP(op, ptr, off)					\
  do									\
    {									\
      REGISTER_TYPE val;						\
      MARKER_INSN ();							\
      __asm__ __volatile__ (op " %0, %1(%2)"				\
                            : "=r"(val)					\
			    : "I"(off), "r"(ptr));			\
      MARKER_INSN ();							\
      printf ("%-10s %p -> 0x%08x%08x\n", (op), (ptr) + (off),		\
              (unsigned int)((((long long)val) >> 32) & 0xffffffff),	\
              (unsigned int)(((long long)val) & 0xffffffff));		\
    }									\
  while (0)

#define DO_STORE_OP(op, ptr, off)					\
  do									\
    {									\
      REGISTER_TYPE val = ~(REGISTER_TYPE)0;				\
      memset (scratch, 0, sizeof scratch);				\
      MARKER_INSN ();							\
      __asm__ __volatile__ (op " %0, %1(%2)"				\
                            :						\
			    : "r"(val), "I"(off), "r"(ptr)		\
			    : "memory");				\
      MARKER_INSN ();							\
      printf ("%-10s %p -> ", (op), (ptr) + (off));			\
      printarray (scratch);						\
      printf ("\n");							\
    }									\
  while (0)

#define ARRAY_SIZE 16
unsigned char data[ARRAY_SIZE] =
  {
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f
  };
unsigned char scratch[ARRAY_SIZE];

void printarray (const unsigned char *data);

void
printarray (const unsigned char *data)
{
  int i;
  for (i = 0; i < ARRAY_SIZE; i++)
    printf ("%s%02x", (i != 0 && (i % 4) == 0) ? "_" : "", data[i]);
}

#ifndef DEMO_FUNCTION
#define DEMO_FUNCTION main
#endif

int
DEMO_FUNCTION (void)
{
  char *datap = data;
  char *dataendp = data + sizeof data;
  char *scratchp = scratch;
  char *scratchendp = scratch + sizeof scratch;

#ifdef	SET_EXLIB_HANDLERS
  /* Set up exception library, and exception handlers that we want.  */
  exlib_init ();
  exlib_set_common_handlers ();
  exlib_set_handler (K_CAUSE_EXC_ADEL, unaligned_load_handler);
  exlib_set_handler (K_CAUSE_EXC_ADES, unaligned_store_handler);
#endif

  printf ("data    @ %p = ", data);
  printarray (data);
  printf ("\n");
  printf ("scratch @ %p = ", scratch);
  printarray (scratch);
  printf ("\n");

  printf ("\n");

  /* Show off unaligned loads.  */
  DO_LOAD_OP ("lh",  datap,    1);
  DO_LOAD_OP ("lh",  dataendp, -9);
  DO_LOAD_OP ("lw",  datap,    1);
  DO_LOAD_OP ("lw",  dataendp, -9);
  DO_LOAD_OP ("lhu", datap,    1);
  DO_LOAD_OP ("lhu", dataendp, -9);
#ifdef __mips64
  DO_LOAD_OP ("lwu", datap,    1);
  DO_LOAD_OP ("lwu", dataendp, -9);
  DO_LOAD_OP ("ld",  datap,    1);
  DO_LOAD_OP ("ld",  dataendp, -9);
#endif

  printf("\n");

  DO_STORE_OP ("sh", scratchp,    1);
  DO_STORE_OP ("sh", scratchendp, -9);
  DO_STORE_OP ("sw", scratchp,    1);
  DO_STORE_OP ("sw", scratchendp, -9);
#ifdef __mips64
  DO_STORE_OP ("sd", scratchp,    1);
  DO_STORE_OP ("sd", scratchendp, -9);
#endif

  printf("\n");

  /* Test ops in branch delay slots.  These could use a **LOT** more
     testing.  */

  /* LH in taken branch delay slot.  */
#define ptr datap
#define	off 1
  {
    REGISTER_TYPE val;
    MARKER_INSN ();
    __asm__ __volatile__ ("\n"
			  "	.set push		\n"
			  "	.set noreorder		\n"
			  "	beq	$0, $0, 1f	\n"
			  "	lh	%0, %1(%2)	\n"
			  "	addiu	%0, %0, 1	\n"
			  "	.set pop		\n"
			  "1:"
                          : "=r"(val)
			  : "I"(off), "r"(ptr));
    MARKER_INSN ();
    printf ("%-10s %p -> 0x%08x%08x\n", "lh (bd-t)", (ptr) + (off),
            (unsigned int)((((long long)val) >> 32) & 0xffffffff),
            (unsigned int)(((long long)val) & 0xffffffff));
  }
#undef ptr
#undef off
  
  /* LH in not-taken branch delay slot.  */
#define ptr datap
#define	off 1
  {
    REGISTER_TYPE val;
    MARKER_INSN ();
    __asm__ __volatile__ ("\n"
			  "	.set push		\n"
			  "	.set noreorder		\n"
			  "	bne	$0, $0, 1f	\n"
			  "	lh	%0, %1(%2)	\n"
			  "	addiu	%0, %0, 1	\n"
			  "	.set pop		\n"
			  "1:"
                          : "=r"(val)
			  : "I"(off), "r"(ptr));
    MARKER_INSN ();
    printf ("%-10s %p -> 0x%08x%08x\n", "lh (bd-n)", (ptr) + (off),
            (unsigned int)((((long long)val) >> 32) & 0xffffffff),
            (unsigned int)(((long long)val) & 0xffffffff));
  }
#undef ptr
#undef off
  
  printf ("\n");

#ifdef DO_TIMINGS
  /* Timing tests.  */

#define LOOP_COUNT 10000
  {
    int iter, a_count, ma_count;
    struct { INT32_TYPE val; } *normal_ptr;
    struct { INT32_TYPE val __attribute__ ((packed)); } *packed_ptr;
    INT32_TYPE val;

    normal_ptr = (void *)&scratch[0];
    __asm__ __volatile__ (".align 5");
    cp0_set_count (0);
    for (iter = 0; iter < LOOP_COUNT; iter++)
      {
        val = normal_ptr->val;
        __asm__ __volatile__ ("" : : "r"(val) : "memory");
      }
    a_count = cp0_get_count ();

    normal_ptr = (void *)&scratch[1];
    __asm__ __volatile__ (".align 5");
    cp0_set_count (0);
    for (iter = 0; iter < LOOP_COUNT; iter++)
      {
        val = normal_ptr->val;
        __asm__ __volatile__ ("" : : "r"(val) : "memory");
      }
    ma_count = cp0_get_count ();

    printf ("count for %d    aligned %18s: %d (%d per iter)\n",
	    iter, "normal-word loads", a_count, a_count / iter);
    printf ("count for %d misaligned %18s: %d (%d per iter)\n",
	    iter, "normal-word loads", ma_count, ma_count / iter);
    printf("\tmisalignment cost: %d count ticks per iter\n",
	   (ma_count - a_count) / iter);

    printf ("\n");

    packed_ptr = (void *)&scratch[0];
    __asm__ __volatile__ (".align 5");
    cp0_set_count (0);
    for (iter = 0; iter < LOOP_COUNT; iter++)
      {
        val = packed_ptr->val;
        __asm__ __volatile__ ("" : : "r"(val) : "memory");
      }
    a_count = cp0_get_count ();

    packed_ptr = (void *)&scratch[1];
    __asm__ __volatile__ (".align 5");
    cp0_set_count (0);
    for (iter = 0; iter < LOOP_COUNT; iter++)
      {
        val = packed_ptr->val;
        __asm__ __volatile__ ("" : : "r"(val) : "memory");
      }
    ma_count = cp0_get_count ();

    printf ("count for %d    aligned %18s: %d (%d per iter)\n",
	    iter, "packed-word loads", a_count, a_count / iter);
    printf ("count for %d misaligned %18s: %d (%d per iter)\n",
	    iter, "packed-word loads", ma_count, ma_count / iter);
    printf("\tmisalignment cost: %d count ticks per iter\n",
	   (ma_count - a_count) / iter);

    printf ("\n");

    normal_ptr = (void *)&scratch[0];
    __asm__ __volatile__ (".align 5");
    cp0_set_count (0);
    for (iter = 0; iter < LOOP_COUNT; iter++)
      __asm__ __volatile__ ("\n"
			    "	.set push		\n"
			    "	.set noreorder		\n"
			    "	beq	$2, $2, 1f	\n"
			    "	lw	%0, 0(%1)	\n"
			    "1:				\n"
			    "	.set pop		\n"
			    : "=r"(val) : "r"(normal_ptr));
    a_count = cp0_get_count ();

    normal_ptr = (void *)&scratch[1];
    __asm__ __volatile__ (".align 5");
    cp0_set_count (0);
    for (iter = 0; iter < LOOP_COUNT; iter++)
      __asm__ __volatile__ ("\n"
			    "	.set push		\n"
			    "	.set noreorder		\n"
			    "	beq	$2, $2,	1f		\n"
			    "	lw	%0, 0(%1)	\n"
			    "1:				\n"
			    "	.set pop		\n"
			    : "=r"(val) : "r"(normal_ptr));
    ma_count = cp0_get_count ();

    printf ("count for %d    aligned %18s: %d (%d per iter)\n",
	    iter, "beq/lw", a_count, a_count / iter);
    printf ("count for %d misaligned %18s: %d (%d per iter)\n",
	    iter, "beq/lw", ma_count, ma_count / iter);
    printf("\tmisalignment cost: %d count ticks per iter\n",
	   (ma_count - a_count) / iter);

    printf ("\n");

    normal_ptr = (void *)&scratch[0];
    __asm__ __volatile__ (".align 5");
    cp0_set_count (0);
    for (iter = 0; iter < LOOP_COUNT; iter++)
      {
        normal_ptr->val = 0;
        __asm__ __volatile__ ("" : : "r"(val) : "memory");
      }
    a_count = cp0_get_count ();

    normal_ptr = (void *)&scratch[1];
    __asm__ __volatile__ (".align 5");
    cp0_set_count (0);
    for (iter = 0; iter < LOOP_COUNT; iter++)
      {
        normal_ptr->val = 0;
        __asm__ __volatile__ ("" : : "r"(val) : "memory");
      }
    ma_count = cp0_get_count ();

    printf ("count for %d    aligned %18s: %d (%d per iter)\n",
	    iter, "normal-word stores", a_count, a_count / iter);
    printf ("count for %d misaligned %18s: %d (%d per iter)\n",
	    iter, "normal-word stores", ma_count, ma_count / iter);
    printf("\tmisalignment cost: %d count ticks per iter\n",
	   (ma_count - a_count) / iter);

    printf ("\n");

    packed_ptr = (void *)&scratch[0];
    __asm__ __volatile__ (".align 5");
    cp0_set_count (0);
    for (iter = 0; iter < LOOP_COUNT; iter++)
      {
        packed_ptr->val = 0;
        __asm__ __volatile__ ("" : : "r"(val) : "memory");
      }
    a_count = cp0_get_count ();

    packed_ptr = (void *)&scratch[1];
    __asm__ __volatile__ (".align 5");
    cp0_set_count (0);
    for (iter = 0; iter < LOOP_COUNT; iter++)
      {
        packed_ptr->val = 0;
        __asm__ __volatile__ ("" : : "r"(val) : "memory");
      }
    ma_count = cp0_get_count ();

    printf ("count for %d    aligned %18s: %d (%d per iter)\n",
	    iter, "packed-word stores", a_count, a_count / iter);
    printf ("count for %d misaligned %18s: %d (%d per iter)\n",
	    iter, "packed-word stores", ma_count, ma_count / iter);
    printf("\tmisalignment cost: %d count ticks per iter\n",
	   (ma_count - a_count) / iter);

    printf ("\n");

    normal_ptr = (void *)&scratch[0];
    __asm__ __volatile__ (".align 5");
    cp0_set_count (0);
    for (iter = 0; iter < LOOP_COUNT; iter++)
      __asm__ __volatile__ ("\n"
			    "	.set push		\n"
			    "	.set noreorder		\n"
			    "	beq	$2, $2, 1f	\n"
			    "	sw	$0, 0(%0)	\n"
			    "1:				\n"
			    "	.set pop		\n"
			    : : "r"(normal_ptr));
    a_count = cp0_get_count ();

    normal_ptr = (void *)&scratch[1];
    __asm__ __volatile__ (".align 5");
    cp0_set_count (0);
    for (iter = 0; iter < LOOP_COUNT; iter++)
      __asm__ __volatile__ ("\n"
			    "	.set push		\n"
			    "	.set noreorder		\n"
			    "	beq	$2, $2,	1f		\n"
			    "	sw	$0, 0(%0)	\n"
			    "1:				\n"
			    "	.set pop		\n"
			    : : "r"(normal_ptr));
    ma_count = cp0_get_count ();

    printf ("count for %d    aligned %18s: %d (%d per iter)\n",
	    iter, "beq/sw", a_count, a_count / iter);
    printf ("count for %d misaligned %18s: %d (%d per iter)\n",
	    iter, "beq/sw", ma_count, ma_count / iter);
    printf("\tmisalignment cost: %d count ticks per iter\n",
	   (ma_count - a_count) / iter);
  }
#undef LOOP_COUNT

  printf ("\n");
#endif /* DO_TIMINGS */

  printf ("About to return...\n");
  return 0;
}
