/* $Id: intr.c,v 1.5 2003/05/14 21:56:46 cgd Exp $ */

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

#include <string.h>
#include <stdlib.h>
#include "exlib_int.h"

static int intr_initted;

static exhandler_func orig_handler;
static unsigned int orig_fpsavemask;

static struct ihand *handler_chains[8];
static unsigned int intr_fpsavemask = EXFPSAVEMASK_NONE;
static unsigned int orig_ie_bits;

static void exhandler_intr (unsigned int excode, struct exframe *ef);
static int mod_cause_bits (int set, unsigned int bit);
static int mod_status_bits (struct exframe *ef, int set, unsigned int bit);
static void run_intr_chain (unsigned int n, struct exframe *ef);

void
intr_init (void)
{
  if (intr_initted)
    return;
  intr_initted = 1;

  exlib_init();

  orig_handler = exlib_get_handler (K_CAUSE_EXC_INT);
  orig_fpsavemask = exlib_get_fpsavemask (K_CAUSE_EXC_INT);
  orig_ie_bits = mod_status_bits (NULL, 0, M_SR_IE | M_SR_IMASK);

  exlib_set_handler (K_CAUSE_EXC_INT, exhandler_intr);
  exlib_set_fpsavemask (K_CAUSE_EXC_INT, intr_fpsavemask);
}

void
intr_shutdown (void)
{
  if (!intr_initted)
    return;
  intr_initted = 0;

  exlib_set_handler (K_CAUSE_EXC_INT, orig_handler);
  exlib_set_fpsavemask (K_CAUSE_EXC_INT, orig_fpsavemask);

  mod_status_bits (NULL, 0, M_SR_IE | M_SR_IMASK);
  mod_status_bits (NULL, 1, orig_ie_bits);
}

int
intr_set_ie (struct exframe *ef)
{
  return (mod_status_bits (ef, 1, M_SR_IE) != 0);
}

int
intr_clear_ie (struct exframe *ef)
{
  return (mod_status_bits (ef, 0, M_SR_IE) != 0);
}

int
intr_enable_int (struct exframe *ef, unsigned int i)
{
  if (i >= 8)
    return 0;
  return (mod_status_bits (ef, 1, (1 << (i + S_SR_IMASK))) != 0);
}

int
intr_disable_int (struct exframe *ef, unsigned int i)
{
  if (i >= 8)
    return 0;
  return (mod_status_bits (ef, 0, (1 << (i + S_SR_IMASK))) != 0);
}

int
intr_set_softint (unsigned int i)
{
  if (i >= 2)
    return 0;
  return (mod_cause_bits (1, (1 << (i + S_CAUSE_IPMASK))) != 0);
}

int
intr_clear_softint (unsigned int i)
{
  if (i >= 2)
    return 0;
  return (mod_cause_bits (0, (1 << (i + S_CAUSE_IPMASK))) != 0);
}

void
intr_handler_add (unsigned int intr, struct ihand *ih)
{
  unsigned int old_ie;
  struct ihand **ihp;

  old_ie = mod_status_bits (NULL, 0, M_SR_IE);

  for (ihp = &handler_chains[intr]; *ihp != NULL; ihp = &(*ihp)->ih_next)
    ;
  ih->ih_next = NULL;
  *ihp = ih;

  /* If the set of FP regs that this handler needs saved isn't already
     being saved, add the required regs to the set.  */
  if ((ih->ih_fpsavemask & intr_fpsavemask) != ih->ih_fpsavemask)
    {
      intr_fpsavemask |= ih->ih_fpsavemask;
      exlib_set_fpsavemask(K_CAUSE_EXC_INT, intr_fpsavemask);
    }

  mod_status_bits (NULL, 1, old_ie);
}

void
intr_handler_remove (unsigned int intr, struct ihand *ih)
{
  unsigned int old_ie;
  struct ihand **ihp;
  int i;

  old_ie = mod_status_bits (NULL, 0, M_SR_IE);

  for (ihp = &handler_chains[intr];
       *ihp != ih && *ihp != NULL;
       ihp = &(*ihp)->ih_next)
    ;
  if (*ihp == NULL)
    goto done;

  *ihp = ih->ih_next;
  ih->ih_next = NULL;

  /* If this handler needed FP regs saved, removing the handler may result
     in fewer regs needing to be saved, so recalculate the save mask.  */
  if (ih->ih_fpsavemask != EXFPSAVEMASK_NONE)
    {
      intr_fpsavemask = EXFPSAVEMASK_NONE;
      for (i = 0; i < 8; i++)
        for (ih = handler_chains[i]; ih != NULL; ih = ih->ih_next)
          intr_fpsavemask |= ih->ih_fpsavemask;
      exlib_set_fpsavemask(K_CAUSE_EXC_INT, intr_fpsavemask);
    }

done:
  mod_status_bits (NULL, 1, old_ie);
}

static void
exhandler_intr (unsigned int excode, struct exframe *ef)
{
  int status_enabled, cause_ready, intrs_to_process, i;

  /* Run through the interrupt handler chains for interrupts asserted
     in the exception frame's copy of Cause.  Note that we dispatch
     only those asserted interrupts which are actually enabled!  */

  cause_ready = (ef->ef_cause & M_CAUSE_IPMASK) >> S_CAUSE_IPMASK;
  status_enabled = (ef->ef_status & M_SR_IMASK) >> S_SR_IMASK;
  intrs_to_process = cause_ready & status_enabled;

  for (i = 7; i >= 0; i--)
    if ((intrs_to_process & (1 << i)) != 0)
      run_intr_chain (i, ef);
}

static int
mod_cause_bits (int set, unsigned int bit)
{
  unsigned int cause, old;

  __asm__ ("mfc0 %0, $%1" : "=r"(cause) : "i"(C0_CAUSE));
  old = cause & bit;
  if (set)
    cause |= bit;
  else
    cause &= ~bit;
  __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(cause), "i"(C0_CAUSE));

  /* If clearing bits, avoid hazard.  */
  if (!set)
    {
      __asm__ __volatile__ (".set push ; "
                            ".set mips64 ; "
                            "ssnop ; "
                            "ssnop ; "
                            "ssnop ; "
                            ".set pop");
    }

  return old;
}

static int
mod_status_bits (struct exframe *ef, int set, unsigned int bit)
{
  unsigned int status, old;

  if (ef != NULL)
    {
      old = ef->ef_status & bit;
      if (set)
        ef->ef_status |= bit;
      else
        ef->ef_status &= ~bit;
      return old;
    }

  __asm__ ("mfc0 %0, $%1" : "=r"(status) : "i"(C0_STATUS));
  old = status & bit;
  if (set)
    status |= bit;
  else
    status &= ~bit;
  __asm__ __volatile__ ("mtc0 %0, $%1" : : "r"(status), "i"(C0_STATUS));

  /* If clearing bits, avoid hazard.  */
  if (!set)
    {
      __asm__ __volatile__ (".set push ; "
                            ".set mips64 ; "
                            "ssnop ; "
                            "ssnop ; "
                            "ssnop ; "
                            ".set pop");
    }

  return old;
}

static void
run_intr_chain (unsigned int i, struct exframe *ef)
{
  struct ihand *ih, *next_ih;
  int handled;

  handled = 0;
  for (ih = handler_chains[i]; ih != NULL; ih = next_ih)
    {
      next_ih = ih->ih_next;
      handled |= (*ih->ih_func) (ih->ih_arg, ef);
    }
  if (!handled)
    exlib_panic (ef, "unhandled interrupt %d", i);
}
