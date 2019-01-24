$Id: README.txt,v 1.1 2004/05/20 21:27:15 cgd Exp $

This is a demonstration of MIPS unaligned access handling.  It
includes handlers for unaligned reads and writes, as well as
a demonstration program.  It includes adaptations for use
in a standalone (sb1-elf + libexception) environment, for use in
NetBSD, and for use in VxWorks.

CONTENTS
--------

The following files are provided:

* unhandler.c

	Unaligned access handlers.

* unhandler_exlib.h

	Adaptation to sb1-elf environment using libexception.

* unhandler_netbsd.h, unhandler_netbsd.patch

	Adaptation to NetBSD/mips.  Apply the patch to the NetBSD
	kernel, place the files in the location described in the patch,
	build a new kernel.  Unaligned accesses in userland programs
	will be handled.)

* unhandler_vxworks.h, unhandler_vxworks_init.c

	Adaptation to VxWorks.  Compile as normal for your BSP, and
	either link into your kernel or load the object file at run-time.
	To install the handler, invoke the sysExcAdeHandleSet function
	(for instance, during BSP init).

* undemo.c

	Demo and test program.  This program can also be used
	to calculate the overhead of unaligned accesses when used
	in the standalone environment.

* Makefile

	Makefile which can build the test program for several environments.

LICENSE
-------

This library is distributed in binary and source code form, and
is distributed under the following license:

    Copyright 2004
    Broadcom Corporation. All rights reserved.
    
    This software is furnished under license and may be used and copied only
    in accordance with the following terms and conditions.  Subject to these
    conditions, you may download, copy, install, use, modify and distribute
    modified or unmodified copies of this software in source and/or binary
    form. No title or ownership is transferred hereby.
    
    1) Any source code used, modified or distributed must reproduce and
       retain this copyright notice and list of conditions as they appear in
       the source file.
    
    2) No right is granted to use any trade name, trademark, or logo of
       Broadcom Corporation.  The "Broadcom Corporation" name may not be
       used to endorse or promote products derived from this software
       without the prior written permission of Broadcom Corporation.
    
    3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR IMPLIED
       WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED WARRANTIES OF
       MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR
       NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT SHALL BROADCOM BE LIABLE
       FOR ANY DAMAGES WHATSOEVER, AND IN PARTICULAR, BROADCOM SHALL NOT BE
       LIABLE FOR DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
       CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
       SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
       BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
       WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
       OR OTHERWISE), EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

