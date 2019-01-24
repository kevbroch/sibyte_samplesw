/* $Id: mdmxtest.c,v 1.27 2003/05/09 04:48:06 cgd Exp $ */

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
 * mdmxtest.c: simple MDMX test program and demonstration.
 *
 * This program serves to test the MDMX access macro/asm implementation
 * provided by mdmx.h, and to provide a short demonstration of the MDMX
 * oeprations.
 */

#include <stdio.h>
#include <stdlib.h>

#include "mdmx.h"

int errors;

#define CHECK_I(regi, val)						\
  do {									\
    if (regi != (mdmxint)(val))						\
      {									\
        printf ("%s:%d: mismatch (expected 0x%016"			\
                PRIxMDMXINT ", got 0x%016" PRIxMDMXINT ")\n",		\
                __FILE__, __LINE__, (mdmxint)(val), regi);		\
        errors++;							\
      }									\
  } while (0)

#define CHECK(reg, val)		CHECK_I (mdmx_int_from_reg (reg), (val))

#define CHECK_ACC(f, h, m, l)						\
  do {									\
    mdmxreg _ha, _ma, _la;						\
    mdmxint _hai, _mai, _lai;						\
    f (&_ha, &_ma, &_la);						\
    _hai = mdmx_int_from_reg (_ha);					\
    _mai = mdmx_int_from_reg (_ma);					\
    _lai = mdmx_int_from_reg (_la);					\
    if (_hai != (mdmxint)(h))						\
      {									\
        printf ("%s:%d: high-part mismatch (expected 0x%016"		\
                PRIxMDMXINT ", got 0x%016" PRIxMDMXINT ")\n",		\
                __FILE__, __LINE__, (mdmxint)(h), _hai);		\
        errors++;							\
      }									\
    if (_mai != (mdmxint)(m))						\
      {									\
        printf ("%s:%d: middle-part mismatch (expected 0x%016"		\
                PRIxMDMXINT ", got 0x%016" PRIxMDMXINT ")\n",		\
                __FILE__, __LINE__, (mdmxint)(m), _mai);		\
        errors++;							\
      }									\
    if (_lai != (mdmxint)(l))						\
      {									\
        printf ("%s:%d: low-part mismatch (expected 0x%016"		\
                PRIxMDMXINT ", got 0x%016" PRIxMDMXINT ")\n",		\
                __FILE__, __LINE__, (mdmxint)(l), _lai);		\
        errors++;							\
      }									\
  } while (0)

mdmxint load_ob_val = 0xa1a2a3a4a5a6a7a8ULL;
mdmxint store_ob_val;

static void
check_ob (void)
{
  mdmxreg m1, m2, m3;
  mdmxreg r, r1, r2, r3;
  mdmxreg z;
  mdmxint i;

  /*
   * Enable use of MDMX resources.
   */

  mdmx_enable ();		/* This does it if running standalone.  */
  mdmx_sb1_enable ();
  z = mdmx_zero_ob ();		/* This does it if running under an OS.  */


  /*
   * Set up some .ob format values in registers.
   */

  m1 = mdmx_reg_from_int (0x1122334455667788ULL);
  m2 = mdmx_reg_from_int (0x66778899aabbccddULL);
  m3 = mdmx_reg_from_int (0x0001020304050607ULL);


  /*
   * Test and demonstrate non-accumulator .ob format vector ops.
   */

  /* ADD.OB */
  r = mdmx_add_ob_v (m1, m2);		CHECK (r, 0x7799bbddffffffffULL);
  r = mdmx_add_ob_ev (m1, m2, 6);	CHECK (r, 0x8899aabbccddeeffULL);
  r = mdmx_add_ob_cv (m1, 0x10);	CHECK (r, 0x2132435465768798ULL);

  /* ALNI.OB */
  r = mdmx_alni_ob (m1, m2, 3);		CHECK (r, 0x4455667788667788ULL);

  /* ALNV.OB */
  r = mdmx_alnv_ob (m1, m2, (mdmx_int_from_reg (m3) >> 16) & 0xff);
					CHECK (r, 0x66778866778899aaULL);

  /* AND.OB */
  r = mdmx_and_ob_v (m1, m2);		CHECK (r, 0x0022000000224488ULL);
  r = mdmx_and_ob_ev (m1, m2, 4);	CHECK (r, 0x1100110011001188ULL);
  r = mdmx_and_ob_cv (m1, 0x1e);	CHECK (r, 0x1002120414061608ULL);

  /* MAX.OB */
  r = mdmx_max_ob_v (m1, m2);		CHECK (r, 0x66778899aabbccddULL);
  r = mdmx_max_ob_ev (m1, m2, 7);	CHECK (r, 0x6666666666667788ULL);
  r = mdmx_max_ob_cv (m1, 0x15);	CHECK (r, 0x1522334455667788ULL);

  /* MIN.OB */
  r = mdmx_min_ob_v (m1, m2);		CHECK (r, 0x1122334455667788ULL);
  r = mdmx_min_ob_ev (m1, m2, 7);	CHECK (r, 0x1122334455666666ULL);
  r = mdmx_min_ob_cv (m1, 0x15);	CHECK (r, 0x1115151515151515ULL);

  /* MUL.OB */
  r = mdmx_mul_ob_v (m1, m3);		CHECK (r, 0x002266ccffffffffULL);
  r = mdmx_mul_ob_ev (m1, m3, 4);	CHECK (r, 0x336699ccffffffffULL);
  r = mdmx_mul_ob_cv (m1, 2);		CHECK (r, 0x22446688aacceeffULL);

  /* NOR.OB */
  r = mdmx_nor_ob_v (m1, m2);		CHECK (r, 0x8888442200000022ULL);
  r = mdmx_nor_ob_ev (m1, m2, 6);	CHECK (r, 0x8888888888888800ULL);
  r = mdmx_nor_ob_cv (m1, 0x08);	CHECK (r, 0xe6d5c4b3a2918077ULL);

  /* OR.OB */
  r = mdmx_or_ob_v (m1, m2);		CHECK (r, 0x7777bbddffffffddULL);
  r = mdmx_or_ob_ev (m1, m2, 6);	CHECK (r, 0x77777777777777ffULL);
  r = mdmx_or_ob_cv (m1, 0x08);		CHECK (r, 0x192a3b4c5d6e7f88ULL);

  /* SHFL.op.OB */
  r = mdmx_shfl_mixh_ob (m1, m2);	CHECK (r, 0x1166227733884499ULL);
  r = mdmx_shfl_mixl_ob (m1, m2);	CHECK (r, 0x55aa66bb77cc88ddULL);
  r = mdmx_shfl_pach_ob (m1, m2);	CHECK (r, 0x113355776688aaccULL);
  r = mdmx_shfl_upsl_ob (m1);		CHECK (r, 0x005500660077ff88ULL);

  /* SLL.OB */
  r = mdmx_sll_ob_v (m1, m3);		CHECK (r, 0x1144cc2050c0c000ULL);
  r = mdmx_sll_ob_ev (m1, m3, 3);	CHECK (r, 0x1020304050607080ULL);
  r = mdmx_sll_ob_cv (m1, 1);		CHECK (r, 0x22446688aaccee10ULL);

  /* SRL.OB */
  r = mdmx_srl_ob_v (m1, m3);		CHECK (r, 0x11110c0805030101ULL);
  r = mdmx_srl_ob_ev (m1, m3, 3);	CHECK (r, 0x0102030405060708ULL);
  r = mdmx_srl_ob_cv (m1, 1);		CHECK (r, 0x081119222a333b44ULL);

  /* SUB.OB */
  r = mdmx_sub_ob_v (m1, m3);		CHECK (r, 0x1121314151617181ULL);
  r = mdmx_sub_ob_ev (m1, m2, 7);	CHECK (r, 0x0000000000001122ULL);
  r = mdmx_sub_ob_cv (m1, 0x10);	CHECK (r, 0x0112233445566778ULL);

  /* XOR.OB */
  r = mdmx_xor_ob_v (m1, m2);		CHECK (r, 0x7755bbddffddbb55ULL);
  r = mdmx_xor_ob_ev (m1, m2, 6);	CHECK (r, 0x66554433221100ffULL);
  r = mdmx_xor_ob_cv (m1, 0x08);	CHECK (r, 0x192a3b4c5d6e7f80ULL);

  /* PAVG.OB (SB-1 Extension) */
  r = mdmx_pavg_ob_v (m1, m2);		CHECK (r, 0x3c4d5e6f8091a2b3ULL);
  r = mdmx_pavg_ob_ev (m1, m2, 6);	CHECK (r, 0x444d555e666f7780ULL);
  r = mdmx_pavg_ob_cv (m1, 0x10);	CHECK (r, 0x1119222a333b444cULL);

  /* PABSDIFF.OB (SB-1 Extension) */
  r = mdmx_pabsdiff_ob_v (m1, m2);	CHECK (r, 0x5555555555555555ULL);
  r = mdmx_pabsdiff_ob_ev (m1, m2, 7);	CHECK (r, 0x5544332211001122ULL);
  r = mdmx_pabsdiff_ob_cv (m3, 0x04);	CHECK (r, 0x0403020100010203ULL);


  /*
   * Test and demonstrate convenience macros/functions.
   *
   * These are tested here, rather than at the beginning, because
   * the registers are more likely to contain non-zero values at this
   * point.
   */

  i = mdmx_int_from_reg (m1);		CHECK_I (i, 0x1122334455667788ULL);
  r = mdmx_zero_ob ();			CHECK (r, 0x0000000000000000ULL);
  r = mdmx_reg_load (&load_ob_val);	CHECK (r, 0xa1a2a3a4a5a6a7a8ULL);
  mdmx_reg_store (&store_ob_val, m1);
  r = mdmx_reg_from_int (store_ob_val);	CHECK (r, 0x1122334455667788ULL);


  /*
   * Test and demonstrate accumulator .ob format vector ops.
   */

  /* WACL.OB, WACH.OB then RACL.OB, RACM.OB, RACH.OB */
  mdmx_acc_write_ob (mdmx_reg_from_int (0x1000000230000004ULL),
                     mdmx_reg_from_int (0x5000000670000008ULL),
                     mdmx_reg_from_int (0x9000000ab000000cULL));
  mdmx_acc_read_ob (&r, NULL, NULL);	CHECK (r, 0x1000000230000004ULL);
  mdmx_acc_read_ob (NULL, &r, NULL);	CHECK (r, 0x5000000670000008ULL);
  mdmx_acc_read_ob (NULL, NULL, &r);	CHECK (r, 0x9000000ab000000cULL);

  /* WACL.OB then RACL.OB, RACM.OB, RACH.OB */
  mdmx_acc_write_low_ob (mdmx_reg_from_int (0x1000000230000004ULL),
                         mdmx_reg_from_int (0x5000000670000008ULL));
  mdmx_acc_read_ob (&r1, &r2, &r3);
  CHECK (r1, 0x0000000000000000ULL);
  CHECK (r2, 0x1000000230000004ULL);
  CHECK (r3, 0x5000000670000008ULL);

  /* ADDA.OB */
  mdmx_acc_write_ob (m3, z, z);
  mdmx_adda_ob_v (m1, m2);
  CHECK_ACC (mdmx_acc_read_ob, 0x0001020304050607ULL,
             0x0000000000010101ULL, 0x7799bbddff214365ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_adda_ob_ev (m1, m2, 2);
  CHECK_ACC (mdmx_acc_read_ob, 0x0001020304050607ULL,
             0x0000000001010101ULL, 0xccddeeff10213243ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_adda_ob_cv (m1, 0x1f);
  CHECK_ACC (mdmx_acc_read_ob, 0x0001020304050607ULL,
             0x0000000000000000ULL, 0x30415263748596a7ULL);

  /* ADDL.OB */
  mdmx_acc_write_ob (m3, z, z);
  mdmx_addl_ob_v (m1, m2);
  CHECK_ACC (mdmx_acc_read_ob, 0x0000000000000000ULL,
             0x0000000000010101ULL, 0x7799bbddff214365ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_addl_ob_ev (m1, m2, 2);
  CHECK_ACC (mdmx_acc_read_ob, 0x0000000000000000ULL,
             0x0000000001010101ULL, 0xccddeeff10213243ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_addl_ob_cv (m1, 0x1f);
  CHECK_ACC (mdmx_acc_read_ob, 0x0000000000000000ULL,
             0x0000000000000000ULL, 0x30415263748596a7ULL);

  /* MULA.OB */
  mdmx_acc_write_ob (m3, z, z);
  mdmx_mula_ob_v (m1, m2);
  CHECK_ACC (mdmx_acc_read_ob, 0x0001020304050607ULL,
             0x060f1b28384a5e75ULL, 0xc6ce18a47282d468ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_mula_ob_ev (m1, m2, 2);
  CHECK_ACC (mdmx_acc_read_ob, 0x0001020304050607ULL,
             0x0c1825313e4a5663ULL, 0x6bd641ac1782ed58ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_mula_ob_cv (m1, 0x1f);
  CHECK_ACC (mdmx_acc_read_ob, 0x0001020304050607ULL,
             0x020406080a0c0e10ULL, 0x0f1e2d3c4b5a6978ULL);

  /* MULL.OB */
  mdmx_acc_write_ob (m3, z, z);
  mdmx_mull_ob_v (m1, m2);
  CHECK_ACC (mdmx_acc_read_ob, 0x0000000000000000ULL,
             0x060f1b28384a5e75ULL, 0xc6ce18a47282d468ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_mull_ob_ev (m1, m2, 2);
  CHECK_ACC (mdmx_acc_read_ob, 0x0000000000000000ULL,
             0x0c1825313e4a5663ULL, 0x6bd641ac1782ed58ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_mull_ob_cv (m1, 0x1f);
  CHECK_ACC (mdmx_acc_read_ob, 0x0000000000000000ULL,
             0x020406080a0c0e10ULL, 0x0f1e2d3c4b5a6978ULL);

  /* MULS.OB */
  mdmx_acc_write_ob (m3, z, z);
  mdmx_muls_ob_v (m1, m2);
  CHECK_ACC (mdmx_acc_read_ob, 0xff00010203040506ULL,
             0xf9f0e4d7c7b5a18aULL, 0x3a32e85c8e7e2c98ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_muls_ob_ev (m1, m2, 2);
  CHECK_ACC (mdmx_acc_read_ob, 0xff00010203040506ULL,
             0xf3e7dacec1b5a99cULL, 0x952abf54e97e13a8ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_muls_ob_cv (m1, 0x1f);
  CHECK_ACC (mdmx_acc_read_ob, 0xff00010203040506ULL,
             0xfdfbf9f7f5f3f1efULL, 0xf1e2d3c4b5a69788ULL);

  /* MULSL.OB */
  mdmx_acc_write_ob (m3, z, z);
  mdmx_mulsl_ob_v (m1, m2);
  CHECK_ACC (mdmx_acc_read_ob, 0xffffffffffffffffULL,
             0xf9f0e4d7c7b5a18aULL, 0x3a32e85c8e7e2c98ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_mulsl_ob_ev (m1, m2, 2);
  CHECK_ACC (mdmx_acc_read_ob, 0xffffffffffffffffULL,
             0xf3e7dacec1b5a99cULL, 0x952abf54e97e13a8ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_mulsl_ob_cv (m1, 0x1f);
  CHECK_ACC (mdmx_acc_read_ob, 0xffffffffffffffffULL,
             0xfdfbf9f7f5f3f1efULL, 0xf1e2d3c4b5a69788ULL);

  /* SUBA.OB */
  mdmx_acc_write_ob (m3, z, z);
  mdmx_suba_ob_v (m1, m2);
  CHECK_ACC (mdmx_acc_read_ob, 0xff00010203040506ULL,
             0xffffffffffffffffULL, 0xababababababababULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_suba_ob_ev (m1, m2, 2);
  CHECK_ACC (mdmx_acc_read_ob, 0xff00010203040506ULL,
             0xffffffffffffffffULL, 0x566778899aabbccdULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_suba_ob_cv (m1, 0x1f);
  CHECK_ACC (mdmx_acc_read_ob, 0xff01020304050607ULL,
             0xff00000000000000ULL, 0xf203142536475869ULL);

  /* SUBL.OB */
  mdmx_acc_write_ob (m3, z, z);
  mdmx_subl_ob_v (m1, m2);
  CHECK_ACC (mdmx_acc_read_ob, 0xffffffffffffffffULL,
             0xffffffffffffffffULL, 0xababababababababULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_subl_ob_ev (m1, m2, 2);
  CHECK_ACC (mdmx_acc_read_ob, 0xffffffffffffffffULL,
             0xffffffffffffffffULL, 0x566778899aabbccdULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_subl_ob_cv (m1, 0x1f);
  CHECK_ACC (mdmx_acc_read_ob, 0xff00000000000000ULL,
             0xff00000000000000ULL, 0xf203142536475869ULL);

  /* PABSDIFFC.OB (SB-1 Extension) */
  mdmx_acc_write_ob (m3, z, z);
  mdmx_pabsdiffc_ob_v (m1, m2);
  CHECK_ACC (mdmx_acc_read_ob, 0x0001020304050607ULL,
             0x0000000000000000ULL, 0x5555555555555555ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_pabsdiffc_ob_ev (m1, m2, 7);
  CHECK_ACC (mdmx_acc_read_ob, 0x0001020304050607ULL,
             0x0000000000000000ULL, 0x5544332211001122ULL);
  mdmx_acc_write_ob (m3, z, z);
  mdmx_pabsdiffc_ob_cv (m3, 0x04);
  CHECK_ACC (mdmx_acc_read_ob, 0x0001020304050607ULL,
             0x0000000000000000ULL, 0x0403020100010203ULL);

  /* Set up accumulator to contain:
       0x40, 0x42, 0x44, 0x46, 0x3f8, 0x3fa, 0x3fc, 0x3fe
     for use by the scale/round/clamp ops below.  */
  mdmx_acc_write_ob (mdmx_reg_from_int (0x0000000000000000ULL),
                     mdmx_reg_from_int (0x0000000003030303ULL),
                     mdmx_reg_from_int (0x40424446f8fafcfeULL));

  /* RNAU.OB */
  r = mdmx_rnau_ob_v (m3);		CHECK (r, 0x4021110940201008ULL);
  r = mdmx_rnau_ob_ev (m3, 4);		CHECK (r, 0x080809097f7f8080ULL);
  r = mdmx_rnau_ob_cv (2);		CHECK (r, 0x10111112feffffffULL);

  /* RNEU.OB */
  r = mdmx_rneu_ob_v (m3);		CHECK (r, 0x4021110940201008ULL);
  r = mdmx_rneu_ob_ev (m3, 4);		CHECK (r, 0x080808097f7f8080ULL);
  r = mdmx_rneu_ob_cv (2);		CHECK (r, 0x10101112fefeffffULL);

  /* RZU.OB */
  r = mdmx_rzu_ob_v (m3);		CHECK (r, 0x402111083f1f0f07ULL);
  r = mdmx_rzu_ob_ev (m3, 4);		CHECK (r, 0x080808087f7f7f7fULL);
  r = mdmx_rzu_ob_cv (2);		CHECK (r, 0x10101111fefeffffULL);


  /*
   * These and demonstrate comparision ops, and other ops that use FP
   * condition codes.
   */

  /* XXX: C.EQ.OB */

  /* XXX: C.LE.OB */

  /* XXX: C.LT.OB */

  /* XXX: PICKF.OB */

  /* XXX: PICKT.OB */
}

int
main (void)
{
  int saveerrors;

  saveerrors = errors;
  check_ob ();
  printf ("OB checks %s!\n", (saveerrors == errors) ? "passed" : "failed");

  exit (errors == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
