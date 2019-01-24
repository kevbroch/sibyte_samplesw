/* $Id: tlbconvert.c,v 1.11 2003/05/16 01:16:39 cgd Exp $ */

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

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef HOST_UTILITY 
#include "../sb1250/include/sbmips.h"
#else
#error "no way to get arguments to target program."
#include <sb1250-include/sbmips.h>
#endif

struct mips64_tlb_entry {
  unsigned long long entrylo0;
  unsigned long long entrylo1;
  unsigned long long entryhi;
  unsigned int pagemask;
};

#define ASID_DEFAULT		0
#define PAGE_SHIFT_DEFAULT	12
#define CCA_DEFAULT		5

void usage (void);
void dump_tlb_entry (const struct mips64_tlb_entry *ep);
unsigned long long string_to_ull (const char * str, unsigned long long min,
			          unsigned long long max, const char *name);
void decode (const char *entryhi_str, const char *entrylo0_str,
             const char *entrylo1_str, const char *pagemask_str);
void encode (const char *va_str, const char *pa0_str, const char *pa1_str,
	     int page_shift, int cca, int asid, int dirty, int global,
	     int valid);

static const char *progname;

/* Maximum pagesize index (64MB) allowed by sb1 and mips architecture.
   User may change. 
   Allowable ranges: 12 <= MAX_PAGESIZE_INDEX <= 26
   XXX: MIPS actually allows up to 28 (256MB).  */
#define MAX_PAGESIZE_INDEX 26

/* Minimum pagesize index (4KB) allowed by sb1 and mips architecture.
   DO NOT CHANGE
   XXX: MIPS actually allows down to 10 (1kB) in MIPS32r2.  */
#define MIN_PAGESIZE_INDEX 12

/* Convert VA to EntryHi:VPN2 and vice versa.  */
#define VA_TO_VPN2(x) ((x) >> S_TLBHI_VPN2)
#define VPN2_TO_VA(x) ((x) << S_TLBHI_VPN2)

/* Convert PA to EntryLoN:PFN and vice versa.  */
#define PA_TO_PFN(x)  ((x) >> MIN_PAGESIZE_INDEX)
#define PFN_TO_PA(x)  ((x) << MIN_PAGESIZE_INDEX)

/* Convert page size to pagemask value and vice versa.  */
#define PGSIZE_TO_PGMASK(x)      (V_TLB_PGMSK (((x) - 1) \
					       >> MIN_PAGESIZE_INDEX))
#define PGMASK_TO_PGSIZE(x)      (((G_TLB_PGMSK (x) << MIN_PAGESIZE_INDEX) \
				    | ((1 << (MIN_PAGESIZE_INDEX + 1)) - 1)) \
				  + 1)

/* Create register value for tlb entryLo */
#define C0_TLBLO_REG(pfn,cca,d,v,g) (V_TLBLO_PFNMASK (pfn) \
				     | V_TLBLO_CALG (cca) \
				     | V_TLBLO_D (d) \
				     | V_TLBLO_V (v) \
				     | V_TLBLO_G (g))

/* Create register value for tlb entryHi */
#define C0_TLBHI_REG(va,asid)       (V_TLBHI_VPN2 (va) | V_TLBHI_ASID (asid))

void
dump_tlb_entry (const struct mips64_tlb_entry *ep)
{
  printf ("EntryHi    = 0x%016llx\n", ep->entryhi);
  printf ("EntryLo0   = 0x%016llx\n", ep->entrylo0);
  printf ("EntryLo1   = 0x%016llx\n", ep->entrylo1);
  printf ("PageMask   = 0x%08x\n", ep->pagemask);

  printf ("\n");

  printf ("VA         = 0x%016llx\n", VPN2_TO_VA (G_TLBHI_VPN2 (ep->entryhi)));
  printf ("ASID       = 0x%02llx\n", G_TLBHI_ASID (ep->entryhi));
  printf ("G          = %d\n", G_TLBLO_G (ep->entrylo0) != 0
			       && G_TLBLO_G (ep->entrylo1) != 0);
  printf ("Page size  = 0x%x\n", PGMASK_TO_PGSIZE (ep->pagemask));

  printf ("\n");

  printf ("Page 0 PA  = 0x%016llx (PFN = 0x%llx)\n",
	  PFN_TO_PA (G_TLBLO_PFNMASK (ep->entrylo0)),
	  G_TLBLO_PFNMASK (ep->entrylo0));
  printf ("Page 0 CCA = %d\n", (int)G_TLBLO_CALG (ep->entrylo0));
  printf ("Page 0 D   = %d\n", (int)G_TLBLO_D (ep->entrylo0));
  printf ("Page 0 V   = %d\n", (int)G_TLBLO_V (ep->entrylo0));

  printf ("\n");

  printf ("Page 1 PA  = 0x%016llx (PFN = 0x%llx)\n",
	  PFN_TO_PA (G_TLBLO_PFNMASK (ep->entrylo1)),
	  G_TLBLO_PFNMASK (ep->entrylo1));
  printf ("Page 1 CCA = %d\n", (int)G_TLBLO_CALG (ep->entrylo1));
  printf ("Page 1 D   = %d\n", (int)G_TLBLO_D (ep->entrylo1));
  printf ("Page 1 V   = %d\n", (int)G_TLBLO_V (ep->entrylo1));
}

unsigned long long
string_to_ull (const char *str, unsigned long long min,
	       unsigned long long max, const char *name)
{
  unsigned long long val;
  char *end;

  errno = 0;
  val = strtoull (str, &end, 0);
  if (errno == EINVAL
      || (val == ~(unsigned long long)0 && errno == ERANGE)
      || *end != '\0')
    {
      fprintf (stderr, "%s: %s value \"%s\" is invalid\n", progname, name,
	       str);
      exit (EXIT_FAILURE);
    }
  if (val < min || val > max)
    {
      fprintf (stderr,
	       "%s: %s value %#llx is out of range.  Must be in the\n"
	       "%*s range [",
	       progname, name, val, strlen (progname) + 1, "");
      if (min <= 256 && max <= 256)
	fprintf (stderr, "%llu, %llu", min, max);
      else
	fprintf (stderr, "%#llx, %#llx", min, max);
      fprintf (stderr, "]\n");
      exit (EXIT_FAILURE);
    }
  return val; 
}

void 
usage (void)
{
  
  fprintf (stderr, "usage:\n");

  fprintf (stderr, "\n");

  fprintf (stderr,
	   "  %s -e [-DVG] [-a asid] [-c cca] [-s page_shift] va pa0 [pa1]\n",
	   progname);
  fprintf (stderr, "\n");
  fprintf (stderr,
	   "    Encode given virtual address (va) and physical addresses\n"
	   "    (pa0 and pa1), and other attributes into TLB EntryLo0,\n"
	   "    EntryLo1, EntryHi, and PageMask registers.\n");
  fprintf (stderr, "\n");
  fprintf (stderr,
	   "    va, pa0, and pa1 are the virtual and physical addresses to\n"
	   "    be used when constructing th TLB entry.  If pa1 is omitted\n"
	   "    it is assumed to be the next page after pa0.\n");
  fprintf (stderr, "\n");
  fprintf (stderr,
	   "    The following additional arguments may be provided:\n");
  fprintf (stderr, "\n");
  fprintf (stderr,
	   "    -D               Clear the TLB entry's EntryLo0/1:D (dirty)\n"
	   "                     bits.  They are set by default.\n"
	   "    -G               Clear the TLB entry's EntryLo0/1:G (global)\n"
	   "                     bits.  They are set by default.\n"
	   "    -V               Clear the TLB entry's EntryLo0/1:V (valid)\n"
	   "                     bits.  They are set by default.\n");
  fprintf (stderr,
	   "    -a asid          Set the TLB entry's EntryHi:ASID field.\n"
	   "                     Default ASID is %d.\n", ASID_DEFAULT);
  fprintf (stderr,
	   "    -c cca           Set the TLB entry's EntryLo0/1:CCA fields.\n"
	   "                     Default is %d.\n", CCA_DEFAULT);
  fprintf (stderr,
	   "    -s page_shift    Use 2^page_shift as the page size.  Default\n"
	   "                     is %d (%d byte pages).\n",
				 PAGE_SHIFT_DEFAULT,
				 (1 << PAGE_SHIFT_DEFAULT));
  fprintf (stderr, "\n");

  fprintf (stderr,
	   "  %s -d entryhi entrylo0 entrylo1 [pagemask]\n", progname);
  fprintf (stderr, "\n");
  fprintf (stderr,
	   "    Print the attributes of the TLB entry that the given TLB\n"
           "    registers encode.\n");
  fprintf (stderr, "\n");
  fprintf (stderr,
	   "    entryhi, entrylo0, entrylo1, and pagemask are the TLB\n"
	   "    entry's CP0 register values.  If pagemask is omitted, a\n"
	   "    value of 0 (minimum page size, 4K) is assumed.\n");

  fprintf (stderr, "\n");

  fprintf (stderr,
           "  All numerical values (register values, argument values, etc.)\n"
	   "  may be provided in decimal, hexadecimal (if prefixed by 0x),\n"
           "  or octal (if prefixed by 0).\n");

  fprintf (stderr, "\n");

  exit(EXIT_FAILURE);
}

void
decode (const char *entryhi_str, const char *entrylo0_str,
        const char *entrylo1_str, const char *pagemask_str)
{
  struct mips64_tlb_entry e;
  unsigned long long entryhi, entrylo0, entrylo1, pagemask;

  entryhi = string_to_ull (entryhi_str, 0x0, 0xffffffffffffffffULL,
			   "entryhi");
  entrylo0 = string_to_ull (entrylo0_str, 0x0, 0xffffffffffffffffULL,
			    "entrylo0");
  entrylo1 = string_to_ull (entrylo1_str, 0x0, 0xffffffffffffffffULL,
			    "entrylo1");
  if (pagemask_str == NULL)
    pagemask = 0;
  else
    pagemask = string_to_ull (pagemask_str, 0x0, 0x00000000ffffffffULL,
			      "pagemask");

  e.entryhi = entryhi;
  e.entrylo0 = entrylo0;
  e.entrylo1 = entrylo1;
  e.pagemask = pagemask;

  dump_tlb_entry (&e);
}

void
encode (const char *va_str, const char *pa0_str, const char *pa1_str,
        int page_shift, int cca, int asid, int dirty, int global, int valid)
{
  struct mips64_tlb_entry e;
  unsigned long long page_size, pa_mask, va_mask;
  unsigned long long va, pa0, pa1;

  page_size = 1ULL << page_shift;
  pa_mask = ~(page_size - 1);
  va_mask = ~((2 * page_size) - 1);

  va = string_to_ull (va_str, 0x0, 0xffffffffffffffffULL, "va");
  if ((va & va_mask) != va)
    {
      fprintf (stderr,
	       "%s: warning: va 0x%llx is not aligned to a two-page\n"
	       "%*s          boundary, using 0x%llx instead\n",
	       progname, va, strlen (progname) + 1, "", (va & va_mask));
      va &= va_mask;
    }

  pa0 = string_to_ull (pa0_str, 0x0, 0xffffffffffffffffULL, "pa0");
  if ((pa0 & pa_mask) != pa0)
    {
      fprintf (stderr,
	       "%s: warning: pa0 0x%llx is not aligned to a page\n"
	       "%*s          boundary, using 0x%llx instead\n",
	       progname, pa0, strlen (progname) + 1, "", (pa0 & pa_mask));
      pa0 &= pa_mask;
    }

  if (pa1_str != NULL)
    pa1 = string_to_ull (pa1_str, 0x0, 0xffffffffffffffffULL, "pa1");
  else
    pa1 = pa0 + page_size;
  if ((pa1 & pa_mask) != pa1)
    {
      fprintf (stderr,
	       "%s: warning: pa1 0x%llx is not aligned to a page\n"
	       "%*s          boundary, using 0x%llx instead\n",
	       progname, pa1, strlen (progname) + 1, "", (pa1 & pa_mask));
      pa1 &= pa_mask;
    }

  e.entryhi = C0_TLBHI_REG (VA_TO_VPN2 (va), asid);
  e.entrylo0 = C0_TLBLO_REG (PA_TO_PFN (pa0), cca, dirty, valid, global);
  e.entrylo1 = C0_TLBLO_REG (PA_TO_PFN (pa1), cca, dirty, valid, global);
  e.pagemask = PGSIZE_TO_PGMASK (page_size);

  dump_tlb_entry (&e);
}

int main (int argc, char **argv)
{
  /* Decode or encode flags.  */
  int dflag, eflag;
  /* Optional encoding flags.  */
  int Dflag, Gflag, Vflag, aflag, cflag, sflag;
  /* Encoding values set by options.  */
  int page_shift, cca, asid, valid, global, dirty;
  int c;

  progname = strrchr(argv[0], '/');
  progname = (progname == NULL) ? argv[0] : (progname + 1);

  dflag = eflag = 0;
  Dflag = Gflag = Vflag = aflag = cflag = sflag = 0;

  cca = CCA_DEFAULT;
  asid = ASID_DEFAULT;
  page_shift = PAGE_SHIFT_DEFAULT;
  valid = global = dirty = 1;
  
  while ((c = getopt(argc, argv, "DGVa:c:des:")) != -1) 
    {
      switch (c) 
	{
	case 'D':
	  Dflag = 1;
	  dirty = 0;
	  break;

	case 'G':
	  Gflag = 1;
	  global = 0;
	  break;

	case 'V':
	  Vflag = 1;
	  valid = 0;
	  break;

	case 'a':
	  aflag = 1;
	  asid = string_to_ull (optarg, 0, 0xff, "asid");
	  break;

	case 'c':
	  cflag = 1;
	  cca = string_to_ull (optarg, 0, 7, "cca");
          if (cca == 6)
	    fprintf (stderr,
		     "%s: warning: cca %d is reserved on SB-1.\n",
		     progname, cca);
	  break;

	case 'd':
	  dflag = 1;
	  break;

	case 'e':
	  eflag = 1;
	  break;

	case 's':
	  sflag = 1;
	  page_shift = string_to_ull (optarg, MIN_PAGESIZE_INDEX,
				      MAX_PAGESIZE_INDEX, "page_shift");
          if ((page_shift % 2) != 0)
	    {
	      fprintf (stderr, "%s: page_shift was %d, must be even.\n",
		       progname, page_shift);
	      exit (EXIT_FAILURE);
	    }
	  break;

	default:
	  usage();
	  break;
	}
    }
  argc -= optind;
  argv += optind;

  /* Exactly one of decode or encode must be specified.  */
  if ((!dflag && !eflag) || (dflag && eflag))
    usage ();

  /* If decoding, none of the encoding options may be specified.  */
  if (dflag &&
      (Dflag || Gflag || Vflag || aflag || cflag || sflag))
    usage ();

  /* Check for correct number of arguments and handle the request.  */
  if (dflag)
    {
      if (argc < 3 || argc > 4)
        usage ();
      decode (argv[0], argv[1], argv[2], (argc == 4) ? argv[3] : NULL);
      exit (EXIT_SUCCESS);
    }
  else if (eflag)
    {
      if (argc < 2 || argc > 3)
        usage ();
      encode (argv[0], argv[1], (argc == 3) ? argv[2] : NULL,
	      page_shift, cca, asid, dirty, global, valid);
      exit (EXIT_SUCCESS);
    }
  else
    exit (EXIT_FAILURE);		/* Should never happen.  */
}
