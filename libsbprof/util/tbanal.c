/* $Id: tbanal.c,v 1.2 2003/06/05 05:16:10 cgd Exp $ */

/*
 * Copyright 2001, 2002, 2003
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BIG_ENDIAN    1234
#define LITTLE_ENDIAN 4321

#if defined(sparc)
#define HOST_ENDIAN   BIG_ENDIAN
#elif defined(i386)
#define HOST_ENDIAN   LITTLE_ENDIAN
#elif defined(mips)
#ifdef __MIPSEB__
#define HOST_ENDIAN   BIG_ENDIAN
#else
#define HOST_ENDIAN   LITTLE_ENDIAN
#endif
#else
#error "unknown platform"
#endif

static int target_endian = BIG_ENDIAN;

typedef int bool;
#define TRUE  (0 == 0)
#define FALSE (0 == 1)

typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned int u4;
typedef unsigned long long u8;

static int soc_pass = 2;
static int print = FALSE;
static int print_raw = FALSE;
static char *progname;

static void fatal(char *msg);

/**********************************************************************
 * Structures for decoding trace bundles
 **********************************************************************/

/* These structures are layed out to compensate for mismatched
   host/target endian */

typedef struct {
  /* these overlap with raw.lo */
  unsigned d_code  :  3;
  unsigned d_mod   :  1;
  unsigned d_rsp   :  4;
  unsigned d_tid   :  6; /*  unsigned d_id    : 10; */
  unsigned d_aid   :  4;
  unsigned r_l2hit :  1;
  unsigned r_exc   :  6;
  unsigned r_shd   :  6;
  unsigned a_l2ca  :  1;
  unsigned a_be    : 32;

  /* these overlap with raw.hi */
  unsigned a_l1ca  :  2;
  unsigned a_cmd   :  3;
  unsigned long long a_ad    : 35;
  unsigned a_tid   :  6; /*   unsigned a_id    : 10; */
  unsigned a_aid   :  4;
  unsigned count   : 12;
  unsigned dtrig   :  1;
  unsigned atrig   :  1;
} rev1_flip_cntrl_bundle_t;

typedef struct {
  /* these overlap with raw.lo */
  unsigned d_code  :  3;
  unsigned d_mod   :  1;
  unsigned d_rsp   :  4;
  unsigned d_tid   :  6; /*  unsigned d_id    : 10; */
  unsigned d_aid   :  4;
  unsigned r_l2hit :  1;
  unsigned r_exc   :  6;
  unsigned r_shd   :  6;
  unsigned a_l2ca  :  1;
  unsigned zero    :  5;
  unsigned trtrig1 :  1;
  unsigned trtrig0 :  1;
  unsigned d_valid :  1;
  unsigned a_valid :  1;
  unsigned a_block : 11;
  unsigned a_byt   :  8;
  unsigned a_dw    :  4;

  /* these overlap with raw.hi */
  unsigned a_l1ca  :  2;
  unsigned a_cmd   :  3;
  unsigned long long a_ad    : 35;
  unsigned a_tid   :  6; /*   unsigned a_id    : 10; */
  unsigned a_aid   :  4;
  unsigned count   : 12;
  unsigned dtrig   :  1;
  unsigned atrig   :  1;
} rev2_flip_cntrl_bundle_t;

/* These structures are layed out for matching host/target endian */

typedef struct {
  /* these overlap with raw.lo */
  unsigned a_be    : 32;
  unsigned a_l2ca  :  1;
  unsigned r_shd   :  6;
  unsigned r_exc   :  6;
  unsigned r_l2hit :  1;
  unsigned d_aid   :  4;
  unsigned d_tid   :  6; /*  unsigned d_id    : 10; */
  unsigned d_rsp   :  4;
  unsigned d_mod   :  1;
  unsigned d_code  :  3;
  
  /* these overlap with raw.hi */
  unsigned atrig   :  1;
  unsigned dtrig   :  1;
  unsigned count   : 12;
  unsigned a_aid   :  4;
  unsigned a_tid   :  6; /*   unsigned a_id    : 10; */
  unsigned long long a_ad    : 35;
  unsigned a_cmd   :  3;
  unsigned a_l1ca  :  2;
} rev1_cntrl_bundle_t;

typedef struct {
  /* these overlap with raw.lo */
  unsigned a_dw    :  4;
  unsigned a_byt   :  8;
  unsigned a_block : 11;
  unsigned a_valid :  1;
  unsigned d_valid :  1;
  unsigned trtrig0 :  1;
  unsigned trtrig1 :  1;
  unsigned zero    :  5;
  unsigned a_l2ca  :  1;
  unsigned r_shd   :  6;
  unsigned r_exc   :  6;
  unsigned r_l2hit :  1;
  unsigned d_aid   :  4;
  unsigned d_tid   :  6; /*  unsigned d_id    : 10; */
  unsigned d_rsp   :  4;
  unsigned d_mod   :  1;
  unsigned d_code  :  3;
  
  /* these overlap with raw.hi */
  unsigned atrig   :  1;
  unsigned dtrig   :  1;
  unsigned count   : 12;
  unsigned a_aid   :  4;
  unsigned a_tid   :  6; /*   unsigned a_id    : 10; */
  unsigned long long a_ad    : 35;
  unsigned a_cmd   :  3;
  unsigned a_l1ca  :  2;
} rev2_cntrl_bundle_t;

typedef union {
  rev1_cntrl_bundle_t      r1;
  rev2_cntrl_bundle_t      r2;
  rev1_flip_cntrl_bundle_t fr1;
  rev2_flip_cntrl_bundle_t fr2;
} cntrl_bundle_t;

typedef struct {
  u8 lo;
  u8 hi;
} raw_t;

typedef union {
  raw_t raw;
  cntrl_bundle_t cb;
} tbsample_t;


/**********************************************************************
 * Code for printing trace buffer data
 **********************************************************************/

const char *a_cmd[8] = {
  "READ_SHD", "READ_EXC", "WRITE", "WR_INV", "INV", "rsrvd5",
  "rsrvd6", "NOP"
};

const char *agent[16] = {
  "cpu0", "cpu1", "br0", "br1", "scd", "tram", "l2", "mc",
  "agnt8", "agnt9", "agnt10", "agnt11", "agnt12", "agnt13", "agnt14",
  "agnt15"
};

const char *blockers[11] = {
  "cpu0", "cpu1", "br0", "br1", "scd", "block[5]", "l2", "mc",
  "scd-all", "mc-all", "mc-br1"
};

const char *dcode[8] = {
  "NOP", "DVal", "DVal_tagECC", "DVal_dataECC", "Bus_error", "Fatal_bus_error",
  "Uncorrectable_ECC_tag_error", "Uncorrectable_ECC_data_error"
};

const char *l1ca[4] = { "CN", "CC", "U0", "U1" };
const char *l2ca[2] = { "U", "C" };

static u4 decode_byen(unsigned dw, unsigned byt)
{
  u4 val = 0;
  int i;

  for (i=0; i<4; i++) {
    val <<= 8;
    if (dw & 8)
      val |= byt;
    dw <<= 1;
  }
  return val;
}

static void print_blockers(unsigned block)
{
  char blockstr[64] = "";
  int i = 0, any = 0;

  while (block) {
    if (block & 1) {
      if (any)
	strcat(blockstr, ", ");
      strcat(blockstr, blockers[i]);
      any = 1;
    }
    i++;
    block >>= 1;
  }
  if (any)
    printf("%s", blockstr);
  else
    printf("(none)");
}

static void print_cntrl(cntrl_bundle_t cb)
{
  printf("delta %d\n", cb.r1.count + 1);

  if (soc_pass == 1) {
    if (cb.r1.atrig) {
      /* In pass 1:
       *   don't know if this is a valid sample
       *   R-phase info is mis-sampled, so don't print it
       */
      printf("A-bus:%10s %6s:%02x 0x%010llx be: %08x %2s/%1s\n",
	     a_cmd[cb.r1.a_cmd],
	     agent[cb.r1.a_aid],
	     cb.r1.a_tid,
	     cb.r1.a_ad << 5,
	     cb.r1.a_be,
	     l1ca[cb.r1.a_l1ca],
	     l2ca[cb.r1.a_l2ca]);
    }
    if (cb.r1.dtrig) {
      /* In pass 1, don't know if this is a valid sample */
      if (cb.r1.d_rsp != cb.r1.d_aid) {
	printf("D-bus: %6s -> %6s:%02x dcode: %s dirty? %s\n",
	       agent[cb.r1.d_rsp],
	       agent[cb.r1.d_aid],
	       cb.r1.d_tid,
	       dcode[cb.r1.d_code],
	       cb.r2.d_mod ? "yes" : "no");
      } else {
	printf("D-bus: WriteData %6s:%02x dcode: %s dirty? %s\n",
	       agent[cb.r1.d_rsp],
	       cb.r1.d_tid,
	       dcode[cb.r1.d_code],
	       cb.r2.d_mod ? "yes" : "no");
      }
    }
  } else {
    assert(cb.r2.zero == 0);
    printf("Blockers: ");
    print_blockers(cb.r2.a_block);
    if (cb.r2.trtrig0 || cb.r2.trtrig1) {
      printf("  Interrupt triggers: %s %s\n",
	     cb.r2.trtrig0 ? "cpu0" : "",
	     cb.r2.trtrig1 ? "cpu1" : "");
    } else {
      printf("\n");
    }
    /* Whichever type of sample this is, A sample may be valid */
    if (cb.r2.a_valid) {
      printf("A-bus:%10s %6s:%02x 0x%010llx be: %08x %2s/%1s l2%-4s shd:%x exc:%x\n",
	     a_cmd[cb.r2.a_cmd],
	     agent[cb.r2.a_aid],
	     cb.r2.a_tid,
	     cb.r2.a_ad << 5,
	     decode_byen(cb.r2.a_dw, cb.r2.a_byt),
	     l1ca[cb.r2.a_l1ca],
	     l2ca[cb.r2.a_l2ca],
	     cb.r2.r_l2hit ? "hit" : "miss",
	     cb.r2.r_shd,
	     cb.r2.r_exc);
    }
    /* Whichever type of sample this is, D-phase control info may be valid */
    if (cb.r2.d_valid) {
      if (cb.r2.d_rsp != cb.r2.d_aid) {
	printf("D-bus: %6s -> %6s:%02x dcode: %s dirty? %s\n",
	       agent[cb.r2.d_rsp],
	       agent[cb.r2.d_aid],
	       cb.r2.d_tid,
	       dcode[cb.r2.d_code],
	       cb.r2.d_mod ? "yes" : "no");
      } else {
	printf("D-bus: WriteData %6s:%02x dcode: %s dirty? %s\n",
	       agent[cb.r2.d_rsp],
	       cb.r2.d_tid,
	       dcode[cb.r2.d_code],
	       cb.r2.d_mod ? "yes" : "no");
      }
    }
  }
}

static void print_data(raw_t r1, raw_t r2)
{
  printf("Data _%08x", (u4) (r2.hi >> 32));
  printf("_%08x", (u4) r2.hi);
  printf("_%08x", (u4) (r2.lo >> 32));
  printf("_%08x", (u4) r2.lo);
  printf("_%08x", (u4) (r1.hi >> 32));
  printf("_%08x", (u4) r1.hi);
  printf("_%08x", (u4) (r1.lo >> 32));
  printf("_%08x_\n", (u4) r1.lo);
}

static int print_tbdata(tbsample_t *tb)
{
  /* Check for end-of-buffer token */
  if (!(tb[0].cb.r1.atrig || tb[0].cb.r1.dtrig)) {
    printf("end of buffer?\n");
    return 1;
  }
  if (tb[0].cb.r1.dtrig) {
    /* D-sample (may have A control data too) */
    print_cntrl(tb[0].cb);
    if ((soc_pass == 1) || tb[0].cb.r2.d_valid) {
      print_data(tb[1].raw, tb[2].raw);
    }
  } else {
    /* A-sample in bundle T0 (rev 2: may have D control data too) */
    print_cntrl(tb[0].cb);
    /* Check T1 and T2 for A-samples (rev 2: and D control) */
    if (tb[1].cb.r1.atrig) {
      print_cntrl(tb[1].cb);
      if (tb[2].cb.r1.atrig)
	print_cntrl(tb[2].cb);
    }
  }
  return 0;
}

static void print_tb(tbsample_t *tb)
{
  int i;
  for (i = 0; i < 256; i++) {
    printf("----------------------------------------------------------------------------\n");
    if (print_raw) {
      int j;
      for (j = 2; j >= 0; j--) {
	printf("t%d hi 0x%016llx\n", j, tb[j].raw.hi);
	printf("t%d lo 0x%016llx\n", j, tb[j].raw.lo);
      }
    }
    if (print_tbdata(tb))
      break;
    tb += 3;
  }
}

/***************************************************************
 * Code for generating histograms from trace buffer data
 ***************************************************************/

#define NHIST 8192
typedef struct {
  unsigned long long sum; /* sum of i * bin[i] */
  unsigned count;	  /* sum of bin[0..NHIST-1] */
  unsigned bin[NHIST];
} hist_t;

/* Modifies: *hist
   Effect:   Makes *hist be empty. */
static void hist_init(hist_t *hist)
{
  int i;
  hist->count = 0;
  hist->sum = 0;
  for (i = 0; i < NHIST; i++) {
    hist->bin[i] = 0;
  }
}

static void hist_insert(hist_t *hist, unsigned value, unsigned count)
{
  assert(value < NHIST);
  hist->bin[value] += count;
  hist->count += count;
  hist->sum += (unsigned long long) value * (unsigned long long) count;
}

static void hist_print(hist_t *hist, char *name)
{
  unsigned i;
  printf("\nhistogram of %s\n", name);
  printf("  Value  Count\n");
  for (i = 0; i < NHIST; i++) {
    if (hist->bin[i] > 0) {
      printf("  %5u  %4u\n", i, hist->bin[i]);
    }
  }
  printf("Mean: %5.1f for %u entries\n",
	 ((double) hist->sum) / (double) hist->count, hist->count);
}

typedef enum {
  TRX_READ_SHD,
  TRX_READ_EXC,
#define TRX_READ_COUNT (TRX_READ_EXC+1)
  TRX_WRITE,
  TRX_WR_INV,
  TRX_INV,
  TRX_RSRVD5,
  TRX_RSRVD6,
  TRX_NOP,
  TRX_COUNT	// for count of real transactions
} trx_t;

typedef enum {
  AGT_CPU0,
  AGT_CPU1,
  AGT_BR0,
  AGT_BR1,
  AGT_SCD,
#define AGT_READER_COUNT (AGT_SCD + 1)
  AGT_TRAM,
  AGT_L2,
  AGT_MC,
  AGT_COUNT	// for count of agents
} agent_t;

/* Read latency histogram for transactions of the form
    [reading agent][read transaction][responder] */
hist_t read_lat[AGT_READER_COUNT][TRX_READ_COUNT][AGT_COUNT];

/* Helper data structure to record info about last use of tid for a read
   by an agent */

typedef struct {
  trx_t	trans;	// TRX_READ_SHD, TRX_READ_EXC, or TRX_COUNT if unknown
  int	time;	// time of last read if trans != TRX_COUNT
} trx_time_t;

#define NTID 64

trx_time_t last_read[AGT_READER_COUNT][NTID];

/* Count of A-bus transactions by each agent */
unsigned int trans_counts[AGT_COUNT][TRX_COUNT];
/* Histogram of idle A-bus phases */
hist_t idle;
/* Histogram of trace buffer lengths, in zclks */
hist_t tb_length;

/* The current timestamp */
unsigned int curtime = 0;

#define MINTIME -9999999

#define VALID_A_PHASE(cb) ((soc_pass == 1) ? ((cb).r1.atrig) : ((cb).r2.a_valid))
#define VALID_D_PHASE(cb) ((soc_pass == 1) ? ((cb).r1.dtrig) : ((cb).r2.d_valid))

/* Assumes that ((atrig || dtrig) == 1) */
static void analyze_cntrl(cntrl_bundle_t cb, bool first)
{
  if (!first) {
    curtime += cb.r1.count + 1;
    hist_insert(&idle, cb.r1.count + 1, 1);
  }
  if (VALID_A_PHASE(cb)) {
    agent_t ag = cb.r1.a_aid;
    unsigned tid = cb.r1.a_tid;
    trx_t trans = cb.r1.a_cmd;
    switch (trans) {
      case TRX_READ_SHD:
      case TRX_READ_EXC:
	/* Update last_read info */
	last_read[ag][tid].trans = trans;
	last_read[ag][tid].time = curtime;
	/* Fall through */
      case TRX_WRITE:
      case TRX_WR_INV:
      case TRX_INV:
      case TRX_NOP:
	/* Increment transaction count */
	trans_counts[ag][trans]++;
	break;
      default: fatal("unexpected transaction");
	break;
    }
  }
  if (VALID_D_PHASE(cb)) {
    if (cb.r1.d_rsp != cb.r1.d_aid) {
      /* A response to a read */
      if (cb.r1.d_code != 1) {
	printf("Warning: D-bus has a %s phase!\n", dcode[cb.r1.d_code]);
      } else {
	agent_t  reader    = cb.r1.d_aid;
	unsigned tid       = cb.r1.d_tid;
	agent_t  responder = cb.r1.d_rsp;
	trx_t    trans     = last_read[reader][tid].trans;
	if (trans != TRX_COUNT) {
	  unsigned delta = curtime - last_read[reader][tid].time;
	  hist_insert(&read_lat[reader][trans][responder], delta, 1);
	  last_read[reader][tid].trans = TRX_COUNT;
	} else {
	  /* We're missing the A-phase for this read response, either
	     because it precedes the trace or because the pass 1 trace
	     buffer failed to capture the A-phase. */
	}
      }
    } else {
      /* This is the write phase for a WRITE or WR_INV */
    }
  }
}

/* Process the next 6x8-byte trace buffer bundle */
static int analyze_bundle(tbsample_t *tb, bool first)
{
  /* End of buffer check */
  if (!(tb[0].cb.r1.atrig || tb[0].cb.r1.dtrig)) {
    return 1;
  }
  /* Must have A and/or D control info */
  analyze_cntrl(tb[0].cb, first);
  if (!tb[0].cb.r1.dtrig) {
    /* For non D-sample, look at T1/T2 bundles */
    if (tb[1].cb.r1.atrig) {
      analyze_cntrl(tb[1].cb, FALSE);
      if (tb[2].cb.r1.atrig)
	analyze_cntrl(tb[2].cb, FALSE);
    }
  }
  return 0;
}

static void tbanal(tbsample_t *tb) {
  int i;
  unsigned int tid;
  
  /* Reset last_read */
  curtime = 0;
  for (i = 0; i < AGT_READER_COUNT; i++) {
    for (tid = 0; tid < NTID; tid++) {
      last_read[i][tid].trans = TRX_COUNT;	// mark as unknown
    }
  }

  for (i = 0; i < 256; i++) {
    if (analyze_bundle(tb, i == 0))
      break;
    tb += 3;
  }
  if (curtime > 8192) {
    printf("not binning tb length %u\n", curtime);
  } else {
    hist_insert(&tb_length, curtime, 1);
  }
}

/**********************************************************************
 * Code for endian mismatch fixup
 **********************************************************************/

static __inline__
void swap(u1 *a, u1 *b)
{
  u1 av = *a;
  u1 bv = *b;
  *a = bv;
  *b = av;
}

static __inline__
void bswap2(u2 *orig)
{
  u1 *p = (u1 *) orig;
  swap(p+0, p+1);
}

static __inline__
void bswap4(u4 *orig)
{
  u1 *p = (u1 *) orig;
  swap(p+0, p+3);
  swap(p+1, p+2);
}

static __inline__
void bswap8(u8 *orig)
{
  u1 *p = (u1 *) orig;
  swap(p+0, p+7);
  swap(p+1, p+6);
  swap(p+2, p+5);
  swap(p+3, p+4);
}

static void cbflip(cntrl_bundle_t *cb)
{
  cntrl_bundle_t tmp;

  tmp = *cb;
  if (soc_pass == 1) {
    cb->r1.d_code  = tmp.fr1.d_code;
    cb->r1.d_mod   = tmp.fr1.d_mod;
    cb->r1.d_rsp   = tmp.fr1.d_rsp;
    cb->r1.d_tid   = tmp.fr1.d_tid;
    cb->r1.d_aid   = tmp.fr1.d_aid;
    cb->r1.r_l2hit = tmp.fr1.r_l2hit;
    cb->r1.r_exc   = tmp.fr1.r_exc;
    cb->r1.r_shd   = tmp.fr1.r_shd;
    cb->r1.a_l2ca  = tmp.fr1.a_l2ca;
    cb->r1.a_be    = tmp.fr1.a_be;
    cb->r1.a_l1ca  = tmp.fr1.a_l1ca;
    cb->r1.a_ad    = tmp.fr1.a_ad;
    cb->r1.a_cmd   = tmp.fr1.a_cmd;
    cb->r1.a_tid   = tmp.fr1.a_tid;
    cb->r1.a_aid   = tmp.fr1.a_aid;
    cb->r1.count   = tmp.fr1.count;
    cb->r1.atrig   = tmp.fr1.atrig;
    cb->r1.dtrig   = tmp.fr1.dtrig;
  } else {
    cb->r2.d_code  = tmp.fr2.d_code;
    cb->r2.d_mod   = tmp.fr2.d_mod;
    cb->r2.d_rsp   = tmp.fr2.d_rsp;
    cb->r2.d_tid   = tmp.fr2.d_tid;
    cb->r2.d_aid   = tmp.fr2.d_aid;
    cb->r2.r_l2hit = tmp.fr2.r_l2hit;
    cb->r2.r_exc   = tmp.fr2.r_exc;
    cb->r2.r_shd   = tmp.fr2.r_shd;
    cb->r2.a_l2ca  = tmp.fr2.a_l2ca;
    cb->r2.zero    = tmp.fr2.zero;
    cb->r2.trtrig1 = tmp.fr2.trtrig1;
    cb->r2.trtrig0 = tmp.fr2.trtrig0;
    cb->r2.d_valid = tmp.fr2.d_valid;
    cb->r2.a_valid = tmp.fr2.a_valid;
    cb->r2.a_block = tmp.fr2.a_block;
    cb->r2.a_byt   = tmp.fr2.a_byt;
    cb->r2.a_dw    = tmp.fr2.a_dw;
    cb->r2.a_l1ca  = tmp.fr2.a_l1ca;
    cb->r2.a_cmd   = tmp.fr2.a_cmd;
    cb->r2.a_ad    = tmp.fr2.a_ad;
    cb->r2.a_tid   = tmp.fr2.a_tid;
    cb->r2.a_aid   = tmp.fr2.a_aid;
    cb->r2.count   = tmp.fr2.count;
    cb->r2.atrig   = tmp.fr2.atrig;
    cb->r2.dtrig   = tmp.fr2.dtrig;
  }
}

/**********************************************************************
 * Main code
 **********************************************************************/

static void fatal(char *msg)
{
  if (msg)
    fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}  

static void usage(void)
{
  fprintf(stderr, "Usage: %s [-1] [-d] [-D] <filename>\n", progname);
  fprintf(stderr, "       -1        Pass 1 BCM1250\n");
  fprintf(stderr, "       -d        Print decoded buffer contents\n");
  fprintf(stderr, "       -D        Print raw buffer contents\n");
  fprintf(stderr, "       -eb       Target is big endian (default)\n");
  fprintf(stderr, "       -el       Target is little endian\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
  char c;
  int fd;
  int i;
  u8 *base;
  struct stat sb;
  int samples;
  agent_t ag, rsp;
  trx_t trans;
  u4 sum_aphases;
  
  assert(sizeof(tbsample_t) == 16);

  progname = strrchr(argv[0], '/');
  progname = (progname == NULL) ? argv[0] : (progname + 1);

  while ((c = getopt(argc, argv, "1dDhe:")) != -1) {
    switch (c) {
    case 'd':
      print = TRUE;
      break;
    case 'D':
      print_raw = TRUE;
      break;
    case '1':
      soc_pass = 1;
      break;
    case 'e':
      if (optarg && ((*optarg == 'l') || (*optarg == 'b')))
	target_endian = (*optarg == 'b') ? BIG_ENDIAN : LITTLE_ENDIAN;
      else
	usage();
      break;
    case 'h':
    default:
      usage();
      break;
    }
  }
  argc -= optind;
  argv += optind;

  if (argc != 1)
    usage();

  fd = open(argv[0], O_RDONLY);
  if (fd < 0) {
    perror(progname);
    fatal(NULL);
  }
  if (fstat(fd, &sb) != 0) fatal("fstat failed");
  if ((sb.st_size % (256*6*8)) != 0)
    fatal("File size not a multiple of trace buffer size");
  samples = sb.st_size / (256*6*8);
  base = (u8 *) mmap(NULL, sb.st_size, PROT_READ|PROT_WRITE,
		     MAP_PRIVATE, fd, 0);
  if (base == MAP_FAILED) fatal("mmap failed");
  printf("Processing %s endian trace of %d samples (each sample is a full trace buffer)\n",
	 target_endian == BIG_ENDIAN ? "big" : "little", samples);
  hist_init(&idle);
  hist_init(&tb_length);
  for (ag = 0; ag < AGT_READER_COUNT; ag++) {
    for (trans = 0; trans < TRX_READ_COUNT; trans++) {
      for (rsp = 0; rsp < AGT_COUNT; rsp++) {
	hist_init(&read_lat[ag][trans][rsp]);
      }
    }
  }

  while (samples--) {
    /* byte-swap each doubleword if the host and target have different endian */
    if (target_endian != HOST_ENDIAN) {
      for (i = 0; i < 256*6; i++) {
	bswap8(&base[i]);
      }
    }
    /* little-endian hosts need to switch the fields around with cbflip */
    if (HOST_ENDIAN == LITTLE_ENDIAN) {
      cntrl_bundle_t *cb = (cntrl_bundle_t *)base;
      for (i = 0; i < 256; i++, cb+=3) {
	cbflip(&cb[0]);
	if (!cb[0].r1.dtrig) {
	  cbflip(&cb[1]);
	  cbflip(&cb[2]);
	}
      }
    }
    if (print) {
      print_tb((tbsample_t *) base);
      printf("\n########################################################\n\n");
    }
    tbanal((tbsample_t *) base);
    base += 256*6;
  }

  /* Now print out interesting histograms and other data */
  hist_print(&tb_length, "Trace Buffer Length (Zclks)");
  hist_print(&idle, "Consecutive Idle A-phase Zclks");
  for (ag = 0; ag < AGT_READER_COUNT; ag++) {
    for (trans = 0; trans < TRX_READ_COUNT; trans++) {
      for (rsp = 0; rsp < AGT_COUNT; rsp++) {
	if (read_lat[ag][trans][rsp].count > 0) {
	  char name[256];
	  sprintf(name, "%s latency %s <- %s",
		  a_cmd[trans], agent[ag], agent[rsp]);
	  hist_print(&read_lat[ag][trans][rsp], name);
	}
      }
    }
  }
  printf("\nTransaction counts\n");
  /* First pass: compute total bus cycles seen */
  sum_aphases = 0;
  for (ag = 0; ag < AGT_COUNT; ag++) {
    for (trans = 0; trans < TRX_COUNT; trans++) {
      sum_aphases += trans_counts[ag][trans];
    }
  }
  sum_aphases += idle.sum;

  /* Second pass: output with percentages */
  for (ag = 0; ag < AGT_COUNT; ag++) {
    for (trans = 0; trans < TRX_COUNT; trans++) {
      unsigned int count = trans_counts[ag][trans];
      if (count > 0)
	printf("%8s %8s %8u %5.1f %%\n",
	       agent[ag], a_cmd[trans], count,
	       count * 100.0 / (float) sum_aphases);
    }
  }
  printf("%8s %8s %8llu %5.1f %%\n", "idle", "NONE", idle.sum,
	 idle.sum * 100.0 / (float) sum_aphases);
  return 0;
}
