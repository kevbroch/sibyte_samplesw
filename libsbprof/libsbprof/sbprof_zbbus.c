/* $Id: sbprof_zbbus.c,v 1.8 2003/05/16 02:08:37 cgd Exp $ */

/*
 * Copyright 2001, 2003
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

#include <exlib.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sb1250-include/sb1250_defs.h>
#include <sb1250-include/sb1250_regs.h>
#include <sb1250-include/sb1250_scd.h>

#include "sbprof_int.h"

typedef uint64_t tb_sample_t[6*256];

static tb_sample_t *sbprof_tbbuf;
static int next_tb_sample;
static int max_tb_samples;

static struct ihand zbprof_intr_handle;

static int zbprof_running;
static int tb_int_was_enabled;


/************************************************************************
 * Support for ZBbus sampling using the trace buffer
 *
 * We use the SCD performance counter interrupt, caused by a Zclk counter
 * overflow, to trigger the start of tracing.
 *
 * We set the trace buffer to sample everything and freeze on
 * overflow.
 *
 * We map the interrupt for trace_buffer_freeze to handle it on CPU 0.
 * 
 ************************************************************************/

#define SCD_PERFCNT_INT  14
#define TRACE_FREEZE_INT 15
#define SCD_PERFCNT_INT_MAP_0  (0x10020200 + SCD_PERFCNT_INT*8)
#define TRACE_FREEZE_INT_MAP_0 (0x10020200 + TRACE_FREEZE_INT*8)
//#define INTERRUPT_MASK_0 (A_IMR_REGISTER(0,R_IMR_INTERRUPT_MASK))
#define INTERRUPT_MASK_0  0x10020028
#define INTERRUPT_TRACE_0 0x10020038
#define INTERRUPT_SOURCE_STATUS_0 0x10020040
#define A_PERF_CNT_CFG    0x100204c0
#define A_PERF_CNT_1      0x100204d8


#define V_SCD_TRACE_CFG_RESET       M_SCD_TRACE_CFG_RESET
#define V_SCD_TRACE_CFG_START_READ  M_SCD_TRACE_CFG_START_READ
#define V_SCD_TRACE_CFG_START	    M_SCD_TRACE_CFG_START	
#define V_SCD_TRACE_CFG_STOP	    M_SCD_TRACE_CFG_STOP	
#define V_SCD_TRACE_CFG_FREEZE	    M_SCD_TRACE_CFG_FREEZE	
#define V_SCD_TRACE_CFG_FREEZE_FULL M_SCD_TRACE_CFG_FREEZE_FULL
#define V_SCD_TRACE_CFG_DEBUG_FULL  M_SCD_TRACE_CFG_DEBUG_FULL
#define V_SCD_TRACE_CFG_FULL	    M_SCD_TRACE_CFG_FULL	
#define V_SCD_TRACE_CFG_CUR_ADDR(x) _SB_MAKEVALUE(x,S_SCD_TRACE_CFG_CUR_ADDR)

#define G_SCD_TRACE_CFG_CUR_ADDR(x) _SB_GETVALUE(x,S_SCD_TRACE_CFG_CUR_ADDR,M_SCD_TRACE_CFG_CUR_ADDR)


int
sbprof_zb_init (long bufsize)
{
  char *buf;

  if (bufsize == SBPROF_BUFSIZE_DEFAULT)
    bufsize = SBPROF_ZB_BUFSIZE_DEFAULT;

  max_tb_samples = bufsize / sizeof (tb_sample_t);

  /* Allocate the buffer plus a little bit, so we can make sure it's cache
     line aligned.  */
  buf = malloc (bufsize + 31);
  if (buf == NULL)
    return -1;

  /* Align to a cache line boundary.  */
  sbprof_tbbuf = (void *)(((long)buf + 31) & ~31);

  return 0;
}

void
sbprof_get_zb_buf (void **bufpp, size_t *sizep)
{

  *bufpp = sbprof_tbbuf;
  if (sbprof_tbbuf == NULL)
    *sizep = 0;
  else
    *sizep = (next_tb_sample * sizeof (tb_sample_t));
}

/* 100 trace buffer samples per second on a 500 Mhz 1250 */
#define TB_PERIOD 2500000ULL

static void
arm_tb (void)
{
  unsigned long long next = (1ULL << 40) - TB_PERIOD;
  /* Generate an SCD_PERFCNT interrupt in TB_PERIOD Zclks to trigger start
     of trace.
     XXX vary sampling period */
  SBPROF_WRITE_CSR(A_PERF_CNT_1, 0);
  /* Unfortunately, in Pass 2 we must clear all counters to knock down
     a previous interrupt request.  This means that bus profiling
     writes ALL of the SCD perf counter values. */
  SBPROF_WRITE_CSR(A_PERF_CNT_CFG,
	     M_SPC_CFG_ENABLE | // enable counting
	     M_SPC_CFG_CLEAR |	// clear all counters
	     V_SPC_CFG_SRC1(1));// counter 1 counts cycles
  SBPROF_WRITE_CSR(A_PERF_CNT_1, next);
  /* Reset the trace buffer */
  SBPROF_WRITE_CSR(A_SCD_TRACE_CFG, V_SCD_TRACE_CFG_RESET);
  SBPROF_WRITE_CSR(A_SCD_TRACE_CFG, V_SCD_TRACE_CFG_FREEZE_FULL);
  /* Read back so TB frozen interrupt is _PROBABLY_ knocked down before
     CPU enables interrupts.  Not a guarantee. */
  {
    volatile unsigned long long dummy __attribute__ ((unused))
      = SBPROF_READ_CSR(A_SCD_TRACE_CFG);
  }
}

static int
zbprof_trapfunc (void *arg, struct exframe *ef)
{
  int i;
  /* Interrupt is spurious if trace buffer is not frozen */
  unsigned long long trace_cfg = SBPROF_READ_CSR(A_SCD_TRACE_CFG);
  unsigned long long src_stat =  SBPROF_READ_CSR(INTERRUPT_SOURCE_STATUS_0);
  if (!(trace_cfg & M_SCD_TRACE_CFG_FREEZE)) {
    /* If TB is really interrupting, we should knock down the TB
       interrupt.  This shouldn't happen. */
    if (src_stat & (1 << TRACE_FREEZE_INT)) {
      printf("ZBbus profiling failure\n");
      arm_tb(); // Knocks down the interrupt
    }
    return 0;
  }
  if (next_tb_sample < max_tb_samples) {
    /* XXX should use XKPHYS to make writes bypass L2 */
    unsigned long long *p = sbprof_tbbuf[next_tb_sample++];
    /* Turn off the SCD perf counter (disable counting and clear interrupts) */
    SBPROF_WRITE_CSR(A_PERF_CNT_CFG, M_SPC_CFG_CLEAR);
    /* Read out trace */
    SBPROF_WRITE_CSR(A_SCD_TRACE_CFG, V_SCD_TRACE_CFG_START_READ);
    SYNC ();
    /* Loop runs backwards because bundles are read out in reverse order */
    for (i = 256 * 6; i > 0; i -= 6) {
      // Subscripts decrease to put bundle in the order
      //   t0 lo, t0 hi, t1 lo, t1 hi, t2 lo, t2 hi
      p[i-1] = SBPROF_READ_CSR(A_SCD_TRACE_READ);	// read t2 hi
      p[i-2] = SBPROF_READ_CSR(A_SCD_TRACE_READ);	// read t2 lo
      p[i-3] = SBPROF_READ_CSR(A_SCD_TRACE_READ);	// read t1 hi
      p[i-4] = SBPROF_READ_CSR(A_SCD_TRACE_READ);	// read t1 lo
      p[i-5] = SBPROF_READ_CSR(A_SCD_TRACE_READ);	// read t0 hi
      p[i-6] = SBPROF_READ_CSR(A_SCD_TRACE_READ);	// read t0 lo
    }
    arm_tb();	// knock down current interrupt and get another one later
  } else {
    /* No more trace buffer samples */
    SBPROF_WRITE_CSR(A_SCD_TRACE_CFG, V_SCD_TRACE_CFG_RESET);
  }
  return 1;
}

void
sbprof_zbprof_start (void)
{

  if (!sbprof_tbbuf)
    return;

  disable_intr();

  if (zbprof_running)
    goto out;
  zbprof_running = 1;

  /* Map trace_freeze_int to TRACEBUF_INTR (I4/IM6) on CPU 0 */
  SBPROF_WRITE_CSR(TRACE_FREEZE_INT_MAP_0, 4);
  /* Map scd_perfcnt_int to (I3/IM5) on CPU 0, which will ignore it */
  SBPROF_WRITE_CSR(SCD_PERFCNT_INT_MAP_0, 3);
  /* Allow the scd_perfcnt_int to trigger trace events */
  SBPROF_WRITE_CSR(INTERRUPT_TRACE_0, (1ULL << SCD_PERFCNT_INT));
  
  /* Unmask the interrupts at SCD level */
  SBPROF_WRITE_CSR(INTERRUPT_MASK_0,
	     SBPROF_READ_CSR(INTERRUPT_MASK_0)
	     & ~((1ULL << TRACE_FREEZE_INT) |
		 (1ULL << SCD_PERFCNT_INT)));

  INIT_IHAND(&zbprof_intr_handle, zbprof_trapfunc, NULL, EXFPSAVEMASK_NONE);
  intr_handler_add(6, &zbprof_intr_handle);

  /* Unmask I4/IM6 interrupts at CPU level */
  tb_int_was_enabled = intr_enable_int(NULL, 6);

  /* Initialize address traps */
  SBPROF_WRITE_CSR(A_ADDR_TRAP_UP_0, 0);
  SBPROF_WRITE_CSR(A_ADDR_TRAP_UP_1, 0);
  SBPROF_WRITE_CSR(A_ADDR_TRAP_UP_2, 0);
  SBPROF_WRITE_CSR(A_ADDR_TRAP_UP_3, 0);
	      
  SBPROF_WRITE_CSR(A_ADDR_TRAP_DOWN_0, 0);
  SBPROF_WRITE_CSR(A_ADDR_TRAP_DOWN_1, 0);
  SBPROF_WRITE_CSR(A_ADDR_TRAP_DOWN_2, 0);
  SBPROF_WRITE_CSR(A_ADDR_TRAP_DOWN_3, 0);
	      
  SBPROF_WRITE_CSR(A_ADDR_TRAP_CFG_0, 0);
  SBPROF_WRITE_CSR(A_ADDR_TRAP_CFG_1, 0);
  SBPROF_WRITE_CSR(A_ADDR_TRAP_CFG_2, 0);
  SBPROF_WRITE_CSR(A_ADDR_TRAP_CFG_3, 0);

  /* Initialize Trace Event 0-7 */
  //                              when interrupt
  SBPROF_WRITE_CSR(A_SCD_TRACE_EVENT_0, (1 << 7));
  SBPROF_WRITE_CSR(A_SCD_TRACE_EVENT_1, 0);
  SBPROF_WRITE_CSR(A_SCD_TRACE_EVENT_2, 0);
  SBPROF_WRITE_CSR(A_SCD_TRACE_EVENT_3, 0);
  SBPROF_WRITE_CSR(A_SCD_TRACE_EVENT_4, 0);
  SBPROF_WRITE_CSR(A_SCD_TRACE_EVENT_5, 0);
  SBPROF_WRITE_CSR(A_SCD_TRACE_EVENT_6, 0);
  SBPROF_WRITE_CSR(A_SCD_TRACE_EVENT_7, 0);

  /* Initialize Trace Sequence 0-7 */
  //                                   Start on event 0 (interrupt)
  SBPROF_WRITE_CSR(A_SCD_TRACE_SEQUENCE_0, (1 << 16)|0x0fff);
  //                        dsamp when d used | asamp when a used
  SBPROF_WRITE_CSR(A_SCD_TRACE_SEQUENCE_1, (1 << 19)| (1 << 18)|0xffff);
  SBPROF_WRITE_CSR(A_SCD_TRACE_SEQUENCE_2, 0);
  SBPROF_WRITE_CSR(A_SCD_TRACE_SEQUENCE_3, 0);
  SBPROF_WRITE_CSR(A_SCD_TRACE_SEQUENCE_4, 0);
  SBPROF_WRITE_CSR(A_SCD_TRACE_SEQUENCE_5, 0);
  SBPROF_WRITE_CSR(A_SCD_TRACE_SEQUENCE_6, 0);
  SBPROF_WRITE_CSR(A_SCD_TRACE_SEQUENCE_7, 0);

  arm_tb();
  
  //printf("start: trace_cfg is 0x%llx\n", SBPROF_READ_CSR(A_SCD_TRACE_CFG));

out:
  enable_intr();
}

void
sbprof_zbprof_stop (void)
{
  disable_intr();

  if (!zbprof_running)
    goto out;

  /* Turn of the SCD perf counter (disable counting and clear interrupts) */
  SBPROF_WRITE_CSR(A_PERF_CNT_CFG, M_SPC_CFG_CLEAR);
  /* mask the trace freeze interrupt at SCD level */
  SBPROF_WRITE_CSR(INTERRUPT_MASK_0,
	     SBPROF_READ_CSR(INTERRUPT_MASK_0) | (1ULL << TRACE_FREEZE_INT));

  enable_intr();

  /* Wait for any last interrupts */
  PAUSE();
  PAUSE();

  disable_intr();

  if (!tb_int_was_enabled)
    intr_disable_int(NULL, 6);
  intr_handler_remove(6, &zbprof_intr_handle);

out:
  enable_intr();
}
