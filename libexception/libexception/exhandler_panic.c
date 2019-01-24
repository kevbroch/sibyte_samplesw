/* $Id: exhandler_panic.c,v 1.14 2003/04/03 17:21:03 cgd Exp $ */

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

#include "exlib.h"

static const char * const codestrs[EXCODE_NCODES] = {
  "Int",      "Mod",      "TLBL",     "TLBS",
  "AdEL",     "AdES",     "IBE",      "DBE",
  "Sys",      "Bp",       "RI",       "CpU",
  "Ov",       "Tr",       "Rsvd14",   "FPE",
  "Rsvd16",   "Rsvd17",   "C2E",      "Rsvd19",
  "Rsvd20",   "Rsvd20",   "MDMX",     "WATCH",
  "MCheck",   "Rsvd25",   "Rsvd26",   "Rsvd27",
  "Rsvd28",   "Rsvd29",   "CacheErr", "Rsvd31"
};

void
exhandler_panic(unsigned int code, struct exframe *ef)
{
  const char *codestr;

  if (code < EXCODE_NCODES)
    codestr = codestrs[code];
  else
    codestr = "UNKNOWN";

  exlib_panic (ef, "fatal %s exception (0x%x)", codestr, code);  
}
