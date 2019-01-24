/* $Id: tlbtest.c,v 1.7 2003/10/10 06:02:54 cgd Exp $ */

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

/* tlbtest.c: test a CPU's TLB.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tlb.h"
#include "tlb_utils.h"

void check_tlb_entry (int entry, const char *msg, int bit,
                      const struct mips64_tlb_entry *expected,
                      const struct mips64_tlb_entry *got);
void check_tlb_invalid (void);
void set_expectations (const struct mips64_tlb_entry *sp,
                       struct mips64_tlb_entry *ep);
void test_tlb_entry (int i);

int errors;
int verbose = 1;

/* PFN, C, D, and V should mirror the written value.  G is special,
   and so is note included here.  Note that this uses SB-1's
   extra-wide PFN field.  */
const unsigned long long writable_entrylo = 0x00000003fffffffe;

/* R, VPN2, and ASID should mirror the written value.  Note that this
   uses SB-1's extra-wide VPN2 field.  */
const unsigned long long writable_entryhi = 0xc0000fffffffe0ff;

/* SB-1 only supports 64MB pages.  This assumes that all page masks
   up to the maximum supported size are also supported.  */
const unsigned int writable_pagemask = 0x07ffe000;

const struct {
  const char *name;
  unsigned int pagemask;
  unsigned int tlb_ent_size;
} pagemasks[] = {
  {   "4KB pagemask", 0x00000000, (2 *   4 * 1024) },
  {  "16KB pagemask", 0x00006000, (2 *  16 * 1024) },
  {  "64KB pagemask", 0x0001e000, (2 *  64 * 1024) },
  { "256KB pagemask", 0x0007e000, (2 * 256 * 1024) },
  {   "1MB pagemask", 0x001fe000, (2 *   1 * 1024 * 1024) },
  {   "4MB pagemask", 0x007fe000, (2 *   4 * 1024 * 1024) },
  {  "16MB pagemask", 0x01ffe000, (2 *  16 * 1024 * 1024) },
  {  "64MB pagemask", 0x07ffe000, (2 *  64 * 1024 * 1024) },
  { "256MB pagemask", 0x1fffe000, (2 * 256 * 1024 * 1024) }
};

#define CHECK_TLB_BIT(field, bit, ep, gp, entry, msg)			\
  do									\
    {									\
      if ((((ep)->field ^ (gp)->field) & (1ULL << (bit))) != 0)		\
        check_tlb_entry ((entry), (msg), (bit), (ep), (gp));		\
    }									\
  while (0)

void
check_tlb_entry (int entry, const char *msg, int bit,
                 const struct mips64_tlb_entry *expected,
                 const struct mips64_tlb_entry *got)
{
  if (expected->entrylo0 == got->entrylo0
      && expected->entrylo1 == got->entrylo1
      && expected->entryhi == got->entryhi
      && expected->pagemask == got->pagemask)
    return;

  errors++;

  printf ("*** MISMATCH checking entry %d, %s", entry, msg);
  if (bit != -1)
    printf (" bit %d", bit);
  printf ("\n");

  if (verbose > 0)
    {
      printf ("    Expected:\n");
      dump_tlb_entry (6, expected);
      printf ("    Got:\n");
      dump_tlb_entry (6, got);
    }
}

void
set_expectations (const struct mips64_tlb_entry *sp,
                  struct mips64_tlb_entry *ep)
{
      ep->entrylo0 = sp->entrylo0 & writable_entrylo;
      ep->entrylo1 = sp->entrylo1 & writable_entrylo;
      ep->entryhi = sp->entryhi & writable_entryhi;
      ep->pagemask = sp->pagemask & writable_pagemask;

      /* Expect 'G' bits to be set only if if both G bits
         are being set.  */
      if ((sp->entrylo0 & sp->entrylo1 & 0x1) != 0)
        {
          ep->entrylo0 |= 0x1;
          ep->entrylo1 |= 0x1;
        }
}

void
check_tlb_invalid (void)
{
  struct mips64_tlb_entry got, expected;
  int i, size;

  size = tlb_size ();

  for (i = 0; i < size; i++)
    {
      memset (&expected, 0, sizeof expected);
      expected.entryhi = TLB_INVALID_RANGE + (8192 * i) + TLB_INVALID_ASID;
      expected.entryhi &= writable_entryhi;

      tlb_read (i, &got);

      check_tlb_entry (i, "invalidated", -1, &expected, &got);
    }
}

void
test_tlb_entry (int i)
{
  struct mips64_tlb_entry set, expected, got;
  int bit, pmi, pmj, npm;

  if (verbose > 1)
    printf ("* Testing entry %d\n", i);

  /* As a quick test, write all-bits-set to the entry, and check
     that all of the expected bits were set.  */
  if (verbose > 2)
    printf ("** Testing %d all-bits-set\n", i);

  memset (&set, 0xff, sizeof set);
  set_expectations (&set, &expected);

  tlb_write_index (i, &set);
  tlb_read (i, &got);

  check_tlb_entry (i, "all-bits-set", -1, &expected, &got);


  /* As a quick test, write all-bits-clear to the entry, and check
     that all bits were clear.  */
  if (verbose > 2)
    printf ("** Testing %d all-bits-clear\n", i);

  memset (&set, 0, sizeof set);
  set_expectations (&set, &expected);

  tlb_write_index (i, &set);
  tlb_read (i, &got);

  check_tlb_entry (i, "all-bits-clear", -1, &expected, &got);


  /* To test the TLB entries, we walk a single set bit through all
     bit positions, checking to see that the bit which we expect to
     be set really are (or, are not), then a single clear bit.

     It's fairly lucky that this test avoids all addresses used in
     invalid TLB entries.

     This (mostly) works for EntryHi, EntryLo0, and EntryLo1.  The
     EntryLo0/EntryLo1 'G' bits must be tested specially, and PageMask
     must be tested specially as well.

     Writing a marching one-bit-set pattern will cause EntryHi to use
     a reserved value in the 'R' field, but that's OK.  */

  /* Walk 1 through all entrylo0 bits (except 'G').  */
  for (bit = 0; bit < 64; bit++)
    {
      if (verbose > 2)
        printf ("** Testing %d entrylo0 bit %d -> 1\n", i, bit);

      memset (&set, 0, sizeof set);
      set.entrylo0 = (1ULL << bit);
      set_expectations (&set, &expected);

      tlb_write_index (i, &set);
      tlb_read (i, &got);

      CHECK_TLB_BIT (entrylo0, bit, &expected, &got, i, "entrylo0 walking 1");
    }

  /* Walk 1 through all entrylo1 bits except 'G'.  */
  for (bit = 0; bit < 64; bit++)
    {
      if (verbose > 2)
        printf ("** Testing %d entrylo1 bit %d -> 1\n", i, bit);

      memset (&set, 0, sizeof set);
      set.entrylo1 = (1ULL << bit);
      set_expectations (&set, &expected);

      tlb_write_index (i, &set);
      tlb_read (i, &got);

      CHECK_TLB_BIT (entrylo1, bit, &expected, &got, i, "entrylo1 walking 1");
    }

  /* Walk 1 through entryhi bits.  */
  for (bit = 0; bit < 64; bit++)
    {
      if (verbose > 2)
        printf ("** Testing %d entryhi bit %d -> 1\n", i, bit);

      memset (&set, 0, sizeof set);
      set.entryhi = (1ULL << bit);
      set_expectations (&set, &expected);

      tlb_write_index (i, &set);
      tlb_read (i, &got);

      CHECK_TLB_BIT (entryhi, bit, &expected, &got, i, "entryhi walking 1");
    }

  /* Walk 0 through all entrylo0 bits (except 'G').  */
  for (bit = 0; bit < 64; bit++)
    {
      if (verbose > 2)
        printf ("** Testing %d entrylo0 bit %d -> 0\n", i, bit);

      memset (&set, 0xff, sizeof set);
      set.entrylo0 &= ~(1ULL << bit);
      set_expectations (&set, &expected);

      tlb_write_index (i, &set);
      tlb_read (i, &got);

      CHECK_TLB_BIT (entrylo0, bit, &expected, &got, i, "entrylo0 walking 0");
    }

  /* Walk 0 through all entrylo1 bits (except 'G').  */
  for (bit = 0; bit < 64; bit++)
    {
      if (verbose > 2)
        printf ("** Testing %d entrylo1 bit %d -> 0\n", i, bit);

      memset (&set, 0xff, sizeof set);
      set.entrylo1 &= ~(1ULL << bit);
      set_expectations (&set, &expected);

      tlb_write_index (i, &set);
      tlb_read (i, &got);

      CHECK_TLB_BIT (entrylo1, bit, &expected, &got, i, "entrylo1 walking 0");
    }

  /* Walk 0 through all entryhi bits (except 'G').  */
  for (bit = 0; bit < 64; bit++)
    {
      if (verbose > 2)
        printf ("** Testing %d entryhi bit %d -> 0\n", i, bit);

      memset (&set, 0xff, sizeof set);
      set.entryhi &= ~(1ULL << bit);
      set_expectations (&set, &expected);

      tlb_write_index (i, &set);
      tlb_read (i, &got);

      CHECK_TLB_BIT (entryhi, bit, &expected, &got, i, "entryhi walking 0");
    }


  /* Test entrylo0/entrylo1 'G' bits.  */
  if (verbose > 2)
    printf ("** Testing %d 'G' bits\n", i);

  memset (&set, 0, sizeof set);
  set.entrylo0 = 1;					/* G = 1; */
  set.entrylo1 = 1;					/* G = 1; */
  set_expectations (&set, &expected);

  tlb_write_index (i, &set);
  tlb_read (i, &got);

  check_tlb_entry (i, "G bits", -1, &expected, &got);


  /* Test accepted PageMask values.  */
  npm = sizeof pagemasks / sizeof pagemasks[0];
  for (pmi = 0; pmi < npm; pmi++)
    {
      /* Set pagemask (and entryhi) and read it back.  */
      if (verbose > 2)
        printf ("** Testing %d %s\n", i, pagemasks[pmi].name);

      memset (&set, 0, sizeof set);
      set.entryhi = pagemasks[pmi].tlb_ent_size;
      set.pagemask = pagemasks[pmi].pagemask;
      set_expectations (&set, &expected);

      tlb_write_index (i, &set);
      tlb_read (i, &got);

      check_tlb_entry (i, pagemasks[pmi].name, -1, &expected, &got);

      /* Don't test unsupported page masks.  */
      if (set.pagemask != expected.pagemask)
        continue;

      /* Probe at VA - 1.  Should miss.  */
      if (tlb_probe (pagemasks[pmi].tlb_ent_size - 1, 0) >= 0)
        {
          errors++;
          printf ("*** PROBE ERROR checking entry %d, size 0x%x, addr 0x%x, expected %s got %s\n",
	          i, pagemasks[pmi].tlb_ent_size,
                  pagemasks[pmi].tlb_ent_size - 1,
                  0 ? "hit" : "miss", 
                  1 ? "hit" : "miss");
        }

      /* Probe at VA.  Should hit.  */
      if (tlb_probe (pagemasks[pmi].tlb_ent_size, 0) < 0)
        {
          errors++;
          printf ("*** PROBE ERROR checking entry %d, size 0x%x, addr 0x%x, expected %s got %s\n",
                  i, pagemasks[pmi].tlb_ent_size,
                  pagemasks[pmi].tlb_ent_size,
                  1 ? "hit" : "miss", 
                  0 ? "hit" : "miss");
        }

      for (pmj = 0; pmj < npm; pmj++)
        {
	  int hit, exp_hit;

          /* Probe at VA + each entry size.  */
          exp_hit = pmj < pmi;		/* Testing offset < page size. */
          hit = (tlb_probe (pagemasks[pmi].tlb_ent_size +
                            pagemasks[pmj].tlb_ent_size, 0) >= 0);

          if (hit ^ exp_hit)
            {
              errors++;
              printf ("*** PROBE ERROR checking entry %d, size 0x%x, addr 0x%x, expected %s got %s\n",
                      i, pagemasks[pmi].tlb_ent_size,
                      pagemasks[pmi].tlb_ent_size + pagemasks[pmj].tlb_ent_size,
                      exp_hit ? "hit" : "miss", 
                      hit ? "hit" : "miss");
            }
        }
    }
}

int
main (void)
{
  int i, size;
  unsigned int prid;

  /* To see what all this talk about safely invalidating TLBs after
     a transfer of control is about, use the tlbdump demo to show you
     how CFE sets up TLB entries, then change 'safe' from 1 to 0 in
     this call to init_tlb(), compile, and run.  Note that a machine
     check exception (TLB Shutdown) results.  */
  init_tlb (1, TLB_INVALID_RANGE);
  check_tlb_invalid ();

  __asm__ ("mfc0 %0, $15" : "=r"(prid));
  if ((prid & 0x00ffff00) != 0x00040100)
    {
      printf ("ERROR: this test can only be run on SB-1 CPUs.\n");
      errors++;
      goto done;
    }

  size = tlb_size ();
  for (i = 0; i < size; i++)
    {
      test_tlb_entry (i);
      invalidate_tlb_entry (i, TLB_INVALID_RANGE);
    }

done:
  if (errors)
    printf ("FAILED: %d errors detected.\n", errors);
  else
    printf ("ALL TESTS PASSED\n");

  printf ("About to exit to firmware...\n");
  exit (errors ? EXIT_FAILURE : EXIT_SUCCESS);
}
