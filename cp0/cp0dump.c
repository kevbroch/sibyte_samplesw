/* $Id: cp0dump.c,v 1.1 2003/05/09 21:25:21 tbroch Exp $

   cp0dump.c: tests read/write functions of mips cp0 registers

   returns non-negative integer if success, otherwise failure
*/

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

   In case of cp0 registers which are writeable, code below attempts
   to test that write simply by reading current value and re-writing it.
   This test is only meant to illustrate use of the 'cp0_set_*' function
   and show any difference that may occur as a result.  Users should
   consult the sbmips.h header and the mips architectural specification
   for further information.
   
   In addition, the following sample software includes calls to some of
   the 'cp0_set_*' functions:
   
   samplesw/tlb/tlballoc.c */

#include <stdio.h>

#include <sb1250-include/sbmips.h>

void dump_and_chk_reg (unsigned int get_reg, 
		  unsigned int set_reg, char *name);
void dump_and_chk_reg_ull (unsigned long long get_reg, 
		      unsigned long long set_reg, char *name);

void
dump_and_chk_reg (unsigned int get_reg, 
	     unsigned int set_reg, char *name)
{
  if (get_reg != set_reg)
    printf ("DIFFERENT : %16s read: %8s0x%08x wrote: %8s0x%08x\n",
	    name, "", get_reg, "", set_reg);
  else
    printf ("SAME      : %16s     : %8s0x%08x\n",
	    name, "", get_reg);

}
                                                                                
void
dump_and_chk_reg_ull (unsigned long long get_reg, 
		 unsigned long long set_reg, char *name)
{
  if (get_reg != set_reg)
    printf ("DIFFERENT : %16s read: 0x%016llx wrote: 0x%016llx\n",
	    name, get_reg, set_reg);
  else
    printf ("SAME      : %16s     : 0x%016llx\n",
	    name, get_reg);
  
}

int
main (void)
{
  unsigned int reg_int;
  unsigned long long reg_ull;

  /* register 0 */
  reg_int = cp0_get_index();
  cp0_set_index(reg_int);
  dump_and_chk_reg (cp0_get_index(), reg_int, "index");

  /* register 1 */
  reg_int = cp0_get_random();
  dump_and_chk_reg (cp0_get_random(), reg_int, "random");

  /* register 2 */
  reg_ull = cp0_get_entrylo0();
  cp0_set_entrylo0(reg_ull);
  dump_and_chk_reg_ull (cp0_get_entrylo0(), reg_ull, "entrylo0");

  /* register 3 */
  reg_ull = cp0_get_entrylo1();
  cp0_set_entrylo1(reg_ull);
  dump_and_chk_reg_ull (cp0_get_entrylo1(), reg_ull, "entrylo1");

  /* register 4 */
  reg_ull = cp0_get_context();
  cp0_set_context(reg_ull);
  dump_and_chk_reg_ull (cp0_get_context(), reg_ull, "context");

  /* register 5 */
  reg_int = cp0_get_pagemask();
  cp0_set_pagemask(reg_int);
  dump_and_chk_reg (cp0_get_pagemask(), reg_int, "pagemask");

  /* register 6 */
  reg_int = cp0_get_wired();
  cp0_set_wired(reg_int);
  dump_and_chk_reg (cp0_get_wired(), reg_int, "wired");

  /* register 7 reserved */

  /* register 8 */
  reg_ull = cp0_get_badvaddr();
  dump_and_chk_reg_ull (cp0_get_badvaddr(), reg_ull, "badvaddr");

  /* register 9 */
  reg_int = cp0_get_count();
  cp0_set_count(reg_int);
  dump_and_chk_reg (cp0_get_count(), reg_int, "count");

  /* register 10 */
  reg_ull = cp0_get_entryhi();
  cp0_set_entryhi(reg_ull);
  dump_and_chk_reg_ull (cp0_get_entryhi(), reg_ull, "entryhi");

  /* register 11 */
  reg_int = cp0_get_compare();
  cp0_set_compare(reg_int);
  dump_and_chk_reg (cp0_get_compare(), reg_int, "compare");

  /* register 12 */
  reg_int = cp0_get_status();
  cp0_set_status(reg_int);
  dump_and_chk_reg (cp0_get_status(), reg_int, "status");

  /* register 13 */
  reg_int = cp0_get_cause();
  cp0_set_cause(reg_int);
  dump_and_chk_reg (cp0_get_cause(), reg_int, "cause");

  /* register 14 */
  reg_ull = cp0_get_epc();
  cp0_set_epc(reg_ull);
  dump_and_chk_reg_ull (cp0_get_epc(), reg_ull, "epc");

  /* register 15 */
  reg_int = cp0_get_prid();
  dump_and_chk_reg (cp0_get_prid(), reg_int, "prid");

  /* register 16 */
  reg_int = cp0_get_config();
  cp0_set_config (reg_int);
  dump_and_chk_reg (cp0_get_config(), reg_int, "config");

  reg_int = cp0_get_config2();
  dump_and_chk_reg (cp0_get_config2(), reg_int, "config2");
  reg_int = cp0_get_config3();
  dump_and_chk_reg (cp0_get_config3(), reg_int, "config3");

  /* register 17 */
  reg_ull = cp0_get_lladdr();
  dump_and_chk_reg_ull (cp0_get_lladdr(), reg_ull, "lladdr");

  /* register 18 */
  reg_ull = cp0_get_watchlo();
  cp0_set_watchlo(reg_ull);
  dump_and_chk_reg_ull (cp0_get_watchlo(), reg_ull, "watchlo");

  reg_ull = cp0_get_watchlo1();
  cp0_set_watchlo1(reg_ull);
  dump_and_chk_reg_ull (cp0_get_watchlo1(), reg_ull, "watchlo1");

  /* register 19 */
  reg_int = cp0_get_watchhi();
  cp0_set_watchhi(reg_int);
  dump_and_chk_reg (cp0_get_watchhi(), reg_int, "watchhi");

  reg_int = cp0_get_watchhi1();
  cp0_set_watchhi1(reg_int);
  dump_and_chk_reg (cp0_get_watchhi1(), reg_int, "watchhi1");

  /* register 20 */
  reg_ull = cp0_get_xcontext();
  cp0_set_xcontext(reg_ull);
  dump_and_chk_reg_ull (cp0_get_xcontext(), reg_ull, "xcontext");

  /* register 21 reserved */

  /* register 22 implementation independent use */

  /* register 23 */
  reg_int = cp0_get_debug();
  cp0_set_debug(reg_int);
  dump_and_chk_reg (cp0_get_debug(), reg_int, "debug");

  reg_int = cp0_get_edebug();
  cp0_set_edebug(reg_int);
  dump_and_chk_reg (cp0_get_edebug(), reg_int, "edebug");

  /* register 24 */
  reg_int = cp0_get_depc();
  cp0_set_depc(reg_int);
  dump_and_chk_reg (cp0_get_depc(), reg_int, "depc");

  /* register 25 */
  reg_int = cp0_get_perfcnt();
  cp0_set_perfcnt(reg_int);
  dump_and_chk_reg (cp0_get_perfcnt(), reg_int, "perfcnt");
				  
  reg_int = cp0_get_perfcnt1();
  cp0_set_perfcnt1(reg_int);	  
  dump_and_chk_reg (cp0_get_perfcnt1(), reg_int, "perfcnt1");
				  
  reg_int = cp0_get_perfcnt2();
  cp0_set_perfcnt2(reg_int);	  
  dump_and_chk_reg (cp0_get_perfcnt2(), reg_int, "perfcnt2");
				  
  reg_int = cp0_get_perfcnt3();
  cp0_set_perfcnt3(reg_int);	  
  dump_and_chk_reg (cp0_get_perfcnt3(), reg_int, "perfcnt3");
				  
  reg_int = cp0_get_perfcnt4();
  cp0_set_perfcnt4(reg_int);	  
  dump_and_chk_reg (cp0_get_perfcnt4(), reg_int, "perfcnt4");
				  
  reg_int = cp0_get_perfcnt5();
  cp0_set_perfcnt5(reg_int);	  
  dump_and_chk_reg (cp0_get_perfcnt5(), reg_int, "perfcnt5");
				  
  reg_int = cp0_get_perfcnt6();
  cp0_set_perfcnt6(reg_int);	  
  dump_and_chk_reg (cp0_get_perfcnt6(), reg_int, "perfcnt6");

  reg_int = cp0_get_perfcnt7();
  cp0_set_perfcnt7(reg_int);
  dump_and_chk_reg (cp0_get_perfcnt7(), reg_int, "perfcnt7");

  /* register 26 */
  reg_int = cp0_get_errctl();
  dump_and_chk_reg (cp0_get_errctl(), reg_int, "errctl");
  reg_int = cp0_get_buserr_pa();
  dump_and_chk_reg (cp0_get_buserr_pa(), reg_int, "buserr_pa");

  /* register 27 */
  reg_int = cp0_get_cacheerr_i();
  dump_and_chk_reg (cp0_get_cacheerr_i(), reg_int, "cacheerr_i");
  reg_int = cp0_get_cacheerr_d();
  dump_and_chk_reg (cp0_get_cacheerr_d(), reg_int, "cacheerr_d");
  reg_int = cp0_get_cacheerr_d_pa();
  dump_and_chk_reg (cp0_get_cacheerr_d_pa(), reg_int, "cacheerr_d_pa");

  /* register 28 */
  reg_ull = cp0_get_taglo_i();
  cp0_set_taglo_i(reg_ull);
  dump_and_chk_reg_ull (cp0_get_taglo_i(), reg_ull, "taglo_i");

  reg_ull = cp0_get_datalo_i();
  dump_and_chk_reg_ull (cp0_get_datalo_i(), reg_ull, "datalo_i");
  reg_ull = cp0_get_datalo_d();
  dump_and_chk_reg_ull (cp0_get_datalo_d(), reg_ull, "datalo_d");

  reg_ull = cp0_get_taglo_d();
  cp0_set_taglo_d(reg_ull);
  dump_and_chk_reg_ull (cp0_get_taglo_d(), reg_ull, "taglo_d");


  /* register 29 */
  reg_ull = cp0_get_taghi_i();
  cp0_set_taghi_i(reg_ull);
  dump_and_chk_reg_ull (cp0_get_taghi_i(), reg_ull, "taghi_i");

  reg_ull = cp0_get_datahi_i();
  dump_and_chk_reg_ull (cp0_get_datahi_i(), reg_ull, "datahi_i");
  reg_ull = cp0_get_datahi_d();
  dump_and_chk_reg_ull (cp0_get_datahi_d(), reg_ull, "datahi_d");

  reg_ull = cp0_get_taghi_d();
  cp0_set_taghi_d(reg_ull);
  dump_and_chk_reg_ull (cp0_get_taghi_d(), reg_ull, "taghi_d");

  /* register 30 */
  reg_ull = cp0_get_errorepc();
  cp0_set_errorepc(reg_ull);
  dump_and_chk_reg_ull (cp0_get_errorepc(), reg_ull, "errorepc");

  /* register 31 */
  reg_ull = cp0_get_desave();
  cp0_set_desave(reg_ull);
  dump_and_chk_reg_ull (cp0_get_desave(), reg_ull, "desave");
  return 0;
}
