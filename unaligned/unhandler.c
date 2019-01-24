/* $Id: unhandler.c,v 1.15 2004/03/20 06:39:58 cgd Exp $ */

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

/* unhandler.c: Unaligned load and store handlers.  */

/* NOTE (FIXME): this code DOES NOT handle unaligned FP loads/stores.  */

#ifndef UNHANDLER_HEADER
# define UNHANDLER_HEADER	"unhandler.h"
#endif
#include UNHANDLER_HEADER

#ifdef BUILD_HANDLERS

/* Set this to non-zero if you want to see debugging printfs.  Note that
   if you enable this, and for some reason you take an unaligned access
   inside the C library (e.g. when printing something out), things might
   become ... very unpredictable.  So, use it only to debug the test code,
   or to observe what's going on in your own sample code.  */
#ifndef UNHANDLER_DEBUG
# define UNHANDLER_DEBUG	0
#endif


#define INSN_TYPE			UINT32_TYPE

#define INSN_OPCODE(insn)		(((insn) >> 26) & 0x3f)

#define INSN_I_IMMEDIATE(insn)		(((insn) >>  0) & 0xffff)
#define INSN_I_RT(insn)			(((insn) >> 16) & 0x1f)
#define INSN_I_RS(insn)			(((insn) >> 21) & 0x1f)

#define INSN_J_INSTR_INDEX(insn)	(((insn) >>  0) & 0x3ffffff)

#define INSN_R_FUNCTION(insn)		(((insn) >>  0) & 0x3f)
#define INSN_R_SA(insn)			(((insn) >>  6) & 0x1f)
#define INSN_R_RD(insn)			(((insn) >> 11) & 0x1f)
#define INSN_R_RT(insn)			(((insn) >> 16) & 0x1f)
#define INSN_R_RS(insn)			(((insn) >> 21) & 0x1f)

static int
calculate_next_pc (EXC_ARGS, EXC_INTPTR_TYPE old_pc, int bd,
                   EXC_INTPTR_TYPE *new_pc,
		   int *need_new_ra)
{
  INSN_TYPE insn;
  REGISTER_TYPE rs_val, rt_val;
  int opcode, condition, cond_need_new_ra;
  int ccmask, ccodes;

  /* Assume not a "link" instruction.  */
  *need_new_ra = 0;
  cond_need_new_ra = 0;

  if (! bd)
    {
      *new_pc = old_pc + 4;
      goto done;
    }

#if UNHANDLER_DEBUG
  printf ("\t... ugh, op in branch delay slot.\n");
#endif

  if (COPYIN (&insn, (INSN_TYPE *)old_pc, sizeof insn))
    goto fail;
#if UNHANDLER_DEBUG
  printf ("\t... branch insn is 0x%08x\n", insn);
#endif

  /* This code handles the following branch/jump instructions:

       J, JAL
       BEQ, BGTZ, BLEZ, BNE,
       BEQL, BGTZL, BLEZL, BNEL,
       JR, JALR
       BGEZ, BGEZAL, BLTZ, BLTZAL
       BC1[TF], BC1[TF]L
       BC1ANY2[TF]
       BC1ANY4[TF]

     We assume that the branch-likely instructions will always be taken.
     If not, their delay-slot instructions should have been nullified,
     which means that we shouldn't ever be here for an unaligned access
     in a not-taken branch-likely branch delay slot!  */

  opcode = INSN_OPCODE (insn);
  switch (opcode)
    {
    case 0x00:	/* SPECIAL opcode */
      switch (INSN_R_FUNCTION (insn))
	{
	case 0x08:	/* JR */
          rs_val = GET_EXC_REG (EXC_FRAME, INSN_R_RS (insn));
          *new_pc = rs_val;
	  goto done;

	case 0x09:	/* JALR */
          rs_val = GET_EXC_REG (EXC_FRAME, INSN_R_RS (insn));
          *new_pc = rs_val;
          *need_new_ra = 1;
	  goto done;

	default:
	  goto fail;
	}
      /* NOTREACHED */

    case 0x01:	/* REGIMM opcode */
      switch (INSN_I_RT (insn))
	{
        case 0x00:	/* BLTZ */
          rs_val = GET_EXC_REG (EXC_FRAME, INSN_I_RS (insn));
          condition = rs_val < 0;
 	  goto cond_branch;

        case 0x01:	/* BGEZ */
          rs_val = GET_EXC_REG (EXC_FRAME, INSN_I_RS (insn));
          condition = rs_val >= 0;
 	  goto cond_branch;

	case 0x02:	/* BLTZL */
	case 0x03:	/* BGEZL */
	  condition = 1;
 	  goto cond_branch;

        case 0x10:	/* BLTZAL */
          rs_val = GET_EXC_REG (EXC_FRAME, INSN_I_RS (insn));
          condition = rs_val < 0;
	  cond_need_new_ra = 1;
 	  goto cond_branch;

        case 0x11:	/* BGEZAL */
          rs_val = GET_EXC_REG (EXC_FRAME, INSN_I_RS (insn));
          condition = rs_val >= 0;
	  cond_need_new_ra = 1;
 	  goto cond_branch;

	case 0x12:	/* BLTZALL */
	case 0x13:	/* BGEZALL */
	  condition = 1;
	  cond_need_new_ra = 1;
 	  goto cond_branch;
	  
	default:
	  goto fail;
	}
      /* NOTREACHED */

    case 0x02:	/* J */
    case 0x03:	/* JAL */
      *new_pc = (((old_pc + 4) & ~(EXC_INTPTR_TYPE)0xfffffff)
		 | INSN_J_INSTR_INDEX (insn) << 2);
      *need_new_ra = (opcode == 0x03);
      goto done;

    case 0x04:	/* BEQ */
    case 0x05:	/* BNE */
    case 0x06:	/* BLEZ -- rt must be zero, but we don't check */
    case 0x07:	/* BGTZ -- rt must be zero, but we don't check */
      rs_val = GET_EXC_REG (EXC_FRAME, INSN_I_RS (insn));
      rt_val = GET_EXC_REG (EXC_FRAME, INSN_I_RT (insn));
      condition = ((opcode == 0x04 && rs_val == rt_val)
		   || (opcode == 0x05 && rs_val != rt_val)
		   || (opcode == 0x06 && rs_val <= 0)
		   || (opcode == 0x07 && rs_val > 0));
      goto cond_branch;

    case 0x11:	/* COP1 opcode */
      ccmask = 1;
      switch (INSN_I_RS (insn))
	{
	case 0xa:	/* BC1ANY4[TF] */
	  ccmask <<= 2;
	case 0x9:	/* BC1ANY2[TF] */
	  ccmask <<= 1;
	case 0x8:	/* BC1[TF] and BC1[TF]L */
	  ccmask <<= 1;
	  ccmask -= 1;		/* select number of bits based on opcode */

	  /* Check ND bit.  If set, it's a branch-likely insn, and so
	     we assume it will be taken.  */
	  if ((INSN_I_RT (insn) & 0x02) != 0)
	    {
	      /* There are no BC1ANY[24][TF] 'likely' instructions.  */
	      if (ccmask != 1)
		goto fail;
              condition = 1;
	      goto cond_branch;
	    }

	  /* Shift CC mask into right position based on CC to check.  */
	  ccmask <<= (INSN_I_RT (insn) >> 2) & 0x7;
	  ccodes = GET_EXC_FP_COND_CODES (EXC_FRAME);

	  /* If the 'TF' bit is 0, we're looking for any of the bits of
	     ccmask to be *zero* in ccodes.  If it's 1, we're looking
	     for them to be 1.  So, if TF is zero, invert the bits
	     in ccodes, so that masking with ccmask will produce correct
	     result.  */
	  if ((INSN_I_RT (insn) & 0x01) == 0)
	     ccodes = ~ccodes;

	  condition = (ccodes & ccmask) != 0;
	  goto cond_branch;

	default:
	  goto fail;
	}
      /* NOTREACHED */
      

    case 0x14:	/* BEQL */
    case 0x15:	/* BNEL */
    case 0x16:	/* BLEZL -- rt must be zero, but we don't check */
    case 0x17:	/* BGTZL -- rt must be zero, but we don't check */
      condition = 1;
 cond_branch:
#if UNHANDLER_DEBUG
      printf ("\t... cond branch, codition = %d\n", condition);
#endif
      if (!condition)
        *new_pc = old_pc + 8;
      else
        {
          *new_pc = ((old_pc + 4)
		     + (((EXC_INTPTR_TYPE)(INT16_TYPE)INSN_I_IMMEDIATE (insn))
		        << 2));
	  *need_new_ra = cond_need_new_ra;
	}
      goto done;

    default:
      goto fail;
    }
  /* NOTREACHED */

done:
#if UNHANDLER_DEBUG
  printf ("\t... new_pc = %#llx, need_new_ra = %d\n",
	  (long long)*new_pc, *need_new_ra);
#endif
  return 0;

fail:
  return 1;
}

EXC_RETURN_TYPE
unaligned_load_handler (EXC_ARGS)
{
  EXC_INTPTR_TYPE pc, new_pc, base, ea;
  INSN_TYPE insn;
  int bd, basereg, tgtreg, need_new_ra;

#if UNHANDLER_DEBUG
  printf ("handling unaligned load exception\n");
  printf ("\t... EPC = %#llx, BD = %d\n",
          (long long)GET_EXC_PC (EXC_FRAME),
          ((GET_EXC_CAUSE (EXC_FRAME) & M_CAUSE_BD) != 0));
#endif

  pc = GET_EXC_PC (EXC_FRAME);
  bd = ((GET_EXC_CAUSE (EXC_FRAME) & M_CAUSE_BD) != 0);

  /* pc might be truncated if EXC_INTPTR_TYPE is smaller than reg size.  */
  if (pc != GET_EXC_PC (EXC_FRAME))
    goto fatal;

  /* If PC was unaligned, it was an ADEL on ifetch.  Can't fix.  */
  if ((GET_EXC_PC (EXC_FRAME) & 0x3) != 0)
    goto fatal;

  /* Calculate the PC that will be used on return from the exception.
     We can't re-run the instruction (since it'll just cause another
     unaligned access exception).  If it was in a branch delay slot,
     we must compute the effect of the branch!  (UGH!)  We have to
     do this *before* emulating the load, because the branch computation
     takes effect before the delay-slot instruction.  */
  if (calculate_next_pc (EXC_ARGS_CALL, pc, bd, &new_pc, &need_new_ra))
    goto fatal;

  /* If Cause.BD was set, exception PC saves the address of a branch
     and the faulting instruction is in the delay slot.  Therefore,
     use the saved PC + 4.  */
  if (bd)
    pc += 4;
#if UNHANDLER_DEBUG
  printf ("\t... instruction @ %#llx\n", (long long)pc);
#endif

  /* Fetch the instruction that caused the fault.  */
  if (COPYIN (&insn, (INSN_TYPE *)pc, sizeof insn))
    goto fatal;
#if UNHANDLER_DEBUG
  printf ("\t... insn is 0x%08x\n", insn);
#endif

  /* Calculate target effective address.

     All operations that we handle are of the form:
       value(rt) = memory[value(rs) + sext16(offset)]

     As before, 'base' may differ from its correct value because
     this was an unaligned deref of a 64-bit pointer, when this code
     has been compiled to use only 32-bit pointers.  */
  basereg = INSN_I_RS (insn);
  tgtreg = INSN_I_RT (insn);
#if UNHANDLER_DEBUG
  printf ("\t... OP $%d, %d($%d)\n", tgtreg,
          (INT16_TYPE)INSN_I_IMMEDIATE (insn), basereg);
#endif
  base = GET_EXC_REG (EXC_FRAME, basereg);
  if (base != GET_EXC_REG (EXC_FRAME, basereg))
    goto fatal;
  ea = base + (INT16_TYPE)INSN_I_IMMEDIATE (insn);
#if UNHANDLER_DEBUG
  printf ("\t... ea = %#llx\n", (long long)ea);
#endif
  
  /* Process based on instruction opcode field and handle.  The cases here
     are for OPCODEs 0x2[0-7] and 0x3[0-7], i.e., the load opcodes.  The
     rest are definitely fatal.  */
  switch ((insn >> 26) & 0x3f)
    {
    case 0x20:	/* LB	 -- never unaligned */
      goto fatal;

    case 0x21:	/* LH */
      {
	INT16_TYPE val;
	if (COPYIN (&val, (void *)ea, sizeof val))
	  goto fatal;
  	SET_EXC_REG (EXC_FRAME, tgtreg, val);
	goto handled;
      }

    case 0x22:	/* LWL	 -- never unaligned */
      goto fatal;

    case 0x23:	/* LW */
      {
	INT32_TYPE val;
	if (COPYIN (&val, (void *)ea, sizeof val))
	  goto fatal;
  	SET_EXC_REG (EXC_FRAME, tgtreg, val);
	goto handled;
      }

    case 0x24:	/* LBU	 -- never unaligned */
      goto fatal;

    case 0x25:	/* LHU */
      {
	UINT16_TYPE val;
	if (COPYIN (&val, (void *)ea, sizeof val))
	  goto fatal;
  	SET_EXC_REG (EXC_FRAME, tgtreg, val);
	goto handled;
      }

    case 0x26:	/* LWR	 -- never unaligned */
      goto fatal;

    case 0x27:	/* LWU	 -- 64bit */
#ifdef __mips64
      {
	UINT32_TYPE val;
	if (COPYIN (&val, (void *)ea, sizeof val))
	  goto fatal;
  	SET_EXC_REG (EXC_FRAME, tgtreg, val);
	goto handled;
      }
#else
      goto fatal;
#endif

    case 0x30:	/* LL	 -- not handled */
    case 0x31:	/* LWC1	 -- not handled */
    case 0x32:	/* LWC2	 -- not handled */
    case 0x33:	/* PREF	 -- never unaligned */
    case 0x34:	/* LLD	 -- 64bit, not handled */
    case 0x35:	/* LDC1	 -- 64bit, not handled */
    case 0x36:	/* LDC2	 -- 64bit, not handled */
      goto fatal;

    case 0x37:	/* LD	 -- 64bit */
#ifdef __mips64
      {
	INT64_TYPE val;
	if (COPYIN (&val, (void *)ea, sizeof val))
	  goto fatal;
  	SET_EXC_REG (EXC_FRAME, tgtreg, val);
	goto handled;
      }
#else
      goto fatal;
#endif

    default:
      goto fatal;
    }
  /* NOTREACHED */

handled:
  if (need_new_ra)
    {
      SET_EXC_REG (EXC_FRAME, 31, GET_EXC_PC (EXC_FRAME) + 8);
#if UNHANDLER_DEBUG
      printf ("\t... set RA to = %#llx\n",
              (long long)GET_EXC_REG (EXC_FRAME, 31));
#endif
    }
  SET_EXC_PC (EXC_FRAME, new_pc);
#if UNHANDLER_DEBUG
  printf ("\t... returning to PC = %#llx\n",
          (long long)GET_EXC_PC (EXC_FRAME));
#endif
  EXC_RETURN (EXC_FRAME);
  /* NOTREACHED */

fatal:
#if UNHANDLER_DEBUG
  printf ("\t... not handled\n");
#endif
  EXC_FATAL (EXC_ARGS_CALL_MACRO);
  /* NOTREACHED */
}

EXC_RETURN_TYPE
unaligned_store_handler (EXC_ARGS)
{
  EXC_INTPTR_TYPE pc, new_pc, base, ea;
  INSN_TYPE insn;
  int bd, basereg, tgtreg, need_new_ra;

#if UNHANDLER_DEBUG
  printf ("handling unaligned store exception\n");
  printf ("\t... EPC = %#llx, BD = %d\n",
          (long long)GET_EXC_PC (EXC_FRAME),
          ((GET_EXC_CAUSE (EXC_FRAME) & M_CAUSE_BD) != 0));
#endif

  pc = GET_EXC_PC (EXC_FRAME);
  bd = ((GET_EXC_CAUSE (EXC_FRAME) & M_CAUSE_BD) != 0);

  /* pc might be truncated if EXC_INTPTR_TYPE is smaller than reg size.  */
  if (pc != GET_EXC_PC (EXC_FRAME))
    goto fatal;

  /* Calculate the PC that will be used on return from the exception.
     We can't re-run the instruction (since it'll just cause another
     unaligned access exception).  If it was in a branch delay slot,
     we must compute the effect of the branch!  (UGH!)  */
  if (calculate_next_pc (EXC_ARGS_CALL, pc, bd, &new_pc, &need_new_ra))
    goto fatal;

  /* If Cause.BD was set, exception PC saves the address of a branch
     and the faulting instruction is in the delay slot.  Therefore,
     use the saved PC + 4.  */
  if (bd)
    pc += 4;
#if UNHANDLER_DEBUG
  printf ("\t... instruction @ %#llx\n", (long long)pc);
#endif

  /* Fetch the instruction that caused the fault.  */
  if (COPYIN (&insn, (INSN_TYPE *)pc, sizeof insn))
    goto fatal;
#if UNHANDLER_DEBUG
  printf ("\t... insn is 0x%08x\n", insn);
#endif

  /* Calculate target effective address.

     All operations that we handle are of the form:
       memory[value(rs) + sext16(offset)] = value(rt);

     As before, 'base' may differ from its correct value because
     this was an unaligned deref of a 64-bit pointer, when this code
     has been compiled to use only 32-bit pointers.  */
  basereg = INSN_I_RS (insn);
  tgtreg = INSN_I_RT (insn);
#if UNHANDLER_DEBUG
  printf ("\t... OP $%d, %d($%d)\n", tgtreg,
          (INT16_TYPE)INSN_I_IMMEDIATE (insn), basereg);
#endif
  base = GET_EXC_REG (EXC_FRAME, basereg);
  if (base != GET_EXC_REG (EXC_FRAME, basereg))
    goto fatal;
  ea = base + (INT16_TYPE)INSN_I_IMMEDIATE (insn);
#if UNHANDLER_DEBUG
  printf ("\t... ea = %#llx\n", (long long)ea);
  printf ("\t... val = %#llx\n", (long long)GET_EXC_REG (EXC_FRAME, tgtreg));
#endif

  /* Process based on instruction opcode field and handle.  The cases here
     are for OPCODEs 0x2[8-f] and 0x3[8-f], i.e., the store opcodes.  The
     rest are definitely fatal.  */
  switch ((insn >> 26) & 0x3f)
    {
    case 0x28:	/* SB	 -- never unaligned */
      goto fatal;

    case 0x29:	/* SH */
      {
	INT16_TYPE val;
  	val = GET_EXC_REG (EXC_FRAME, tgtreg);
	if (COPYOUT ((void *)ea, &val, sizeof val))
	  goto fatal;
	goto handled;
      }

    case 0x2a:	/* SWL	 -- never unaligned */
      goto fatal;

    case 0x2b:	/* SW */
      {
	INT32_TYPE val;
  	val = GET_EXC_REG (EXC_FRAME, tgtreg);
	if (COPYOUT ((void *)ea, &val, sizeof val))
	  goto fatal;
	goto handled;
      }

    case 0x2c:	/* SDL	 -- 64bit, never unaligned */
    case 0x2d:	/* SDR	 -- 64bit, never unaligned */
    case 0x2e:	/* SWR	 -- never unaligned */
    case 0x2f:	/* CACHE -- not handled */
      goto fatal;

    case 0x38:	/* SC	 -- not handled */
    case 0x39:	/* SWC1	 -- not handled */
    case 0x3a:	/* SWC2	 -- not handled */
    case 0x3b:	/* rsvd	 -- not handled*/
    case 0x3c:	/* SCD	 -- 64bit, not handled */
    case 0x3d:	/* SDC1	 -- 64bit, not handled */
    case 0x3e:	/* SDC2	 -- 64bit, not handled */
      goto fatal;

    case 0x3f:	/* SD	 -- 64bit */
#ifdef __mips64
      {
	INT64_TYPE val;
  	val = GET_EXC_REG (EXC_FRAME, tgtreg);
	if (COPYOUT ((void *)ea, &val, sizeof val))
	  goto fatal;
	goto handled;
      }
#else
      goto fatal;
#endif

    default:
      goto fatal;
    }
  /* NOTREACHED */

handled:
  if (need_new_ra)
    {
      SET_EXC_REG (EXC_FRAME, 31, GET_EXC_PC (EXC_FRAME) + 8);
#if UNHANDLER_DEBUG
      printf ("\t... set RA to = %#llx\n",
              (long long)GET_EXC_REG (EXC_FRAME, 31));
#endif
    }
  SET_EXC_PC (EXC_FRAME, new_pc);
#if UNHANDLER_DEBUG
  printf ("\t... returning to PC = %#llx\n",
          (long long)GET_EXC_PC (EXC_FRAME));
#endif
  EXC_RETURN (EXC_FRAME);
  /* NOTREACHED */

fatal:
#if UNHANDLER_DEBUG
  printf ("\t... not handled\n");
#endif
  EXC_FATAL (EXC_ARGS_CALL_MACRO);
  /* NOTREACHED */
}

#endif /* BUILD_HANDLERS */
