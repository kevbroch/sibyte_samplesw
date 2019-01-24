$Id: README.txt,v 1.1 2004/05/16 22:00:32 cgd Exp $

This is "libmips_fpu", a library and header file which allows easy
manipulation of the MIPS FPU control registers.

See libmips_fpu/mips_fpu.h for documentation on the facilities
provided by the library.

It provides several features:

  * Accessor functions for the MIPS FPU control registers, and
    definitions of the MIPS FPU control register fields.

  * Constructor functions which can be used to automatically set
    certain FPU control register fields at program startup.

At this time, this library is experimental.  The interfaces
that it provides may change in future releases without notice.


USING THIS LIBRARY
------------------

Support for this library is included in the Specifix "sb1-elf" tools
distribution.  To use this library with the Specifix sb1-elf tools,
in your source code:

	#include <mips_fpu.h>

and include "-lmips_fpu" when linking your program.  (See the
mips_fpu.h header file for more information on how to use the
library.)


BUILDING THIS LIBRARY
---------------------

Assuming you're using sb1-elf tools, you can configure and build this library
like:

	srcpath=/path/to/this/directory
	prefix=/path/in/which/to/install

	mkdir build_dir
	cd build_dir
	CC=sb1-elf-gcc AR=sb1-elf-ar RANLIB=sb1-elf-ranlib \
	    $srcpath/configure --target=sb1-elf --prefix=$prefix
	gmake


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

