$Id: README.txt,v 1.2 2004/12/07 06:47:44 cgd Exp $

This directory contains header files which implement C wrappers for
MIPS Paired Single format floating point instructions, and some sample
test programs to demonstrate Paired Single functionality.

These functions and macros are implemented in C, so that the
constructions can be easily usable from C code being compiled by GCC.
To access the Paired-Single instructions, they either use GCC "asm"
statements or GCC builtins, depending the version of GCC and whether
Paired-Single code generation is enabled.

The following files are present in this directory:

* psingle.h

  Master header file to include to pull Paired Single support
  functions/macros into your programs.

* psingle_sb1.h

  Header file which defines SB-1 extensions to the Paired Single
  operations.  Do not include this file directly, include psingle.h
  instead (which includes this file).

* psingletest.c

  A test program which demonstrates all of the macros provided by
  psingle.h, and checks their results.

* Makefile

  A makefile which can be used to build the 'psingletest' program.

