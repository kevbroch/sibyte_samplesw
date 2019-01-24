$Id: README.txt,v 1.2 2003/05/16 01:57:54 cgd Exp $

This is "libsbprof", a library which layers on top of the
"libexception" library and can be used to gather performance
data about programs running on SiByte SOCs  It is meant to be
used in the sb1-elf CFE-based environment (-Tcfe.ld), and as
sample software for driver writers wishing to implement SiByte
performance monitoring drivers for OSes which are currently not
supported by our performance tools.  For more examples of
such drivers, see the drivers for Linux and NetBSD included in
the performance tools distribution.

See libsbprof/sbprof.h for documentation on the facilities
provided by the library.

This library is included in the SiByte sample software and in the
SiByte cross compilation "sb1-elf" tools environment starting with
version 2.7.1 of the sb1-elf tools.

(Note that this library will not compile as-is with the sb1-elf tools
present in the sbtools 2.5.x releases and earlier versions.  Minimal
modifications are required to make it compile, but those versions are
not supported by this library.)

It provides several features:

  * CPU performance counter data collection.

  * ZBBus trace dump collection.

At this time the library does not include any support for
multiprocessing or multiprocessor operation.  That is expected
to be added in a future library release.

Also, at this time, this library is experimental.  The interfaces
that it provides may change in future releases without notice.


USING THIS LIBRARY
------------------

Support for this library is included in the SiByte "sb1-elf" tools
distribution.  To use this library with the sb1-elf tools, in your
source code:

	#include <sbprof.h>

and include "-lsbprof -lexception" when linking your program.  (See
the sbprof.h header file for more information on how to use the library.)

Example programs (meant for use with the sb1-elf cross-compilation
tools) can be found in the "examples" subdirectory.  See the README.txt
file in that directory for more information about what to expect from
the examples.


BUILDING THIS LIBRARY
---------------------

Assuming you're using the sb1-elf tools to build, configure and build like:

	srcpath=/path/to/this/directory
	prefix=/path/in/which/to/install

	mkdir build_dir
	cd build_dir
	sb1-elf-env $srcpath/configure --target=sb1-elf --prefix=$prefix
	gmake

(See also the SiByte cross compilation tools' "build" script, which
arranges to build this library as part of the tools build process.)


LICENSE
-------

This library is distributed in binary and source code form, and
is distributed under the following license:

    Copyright 2001, 2002, 2003
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

