$Id: README.txt,v 1.2 2003/03/26 18:52:02 cgd Exp $

This directory contains header files which implement C wrappers for
the MIPS MDMX instructions provided by the SB-1 CPU, and some sample
test programs to demonstrate MDMX functionality.

These functions and macros are implemented in C, using GCC "asm"
statements, so that the constructions can be easily usable from C code
being compiled by GCC.

The following files are present in this directory:

* mdmx.h

  Master header file to include to pull MDMX support functions/macros
  into your programs.

* mdmx_ob.h

  Header file which defines MDMX ".ob"-format operations.  Do not
  include this file directly, include mdmx.h instead (which includes
  this file).

* mdmx_ob_sb1.h

  Header file which defines SB-1 extensions to the MDMX ".ob"-format
  operations.  Do not include this file directly, include mdmx.h
  instead (which includes this file).

* mdmxtest.c

  A test program which demonstrates all of the MDMX macros provided by
  mdmx.h, and checks their results.

* accswitchtest.c

  A test program which can be used to help verify correct operation of
  an operating system's MDMX accumulator context switch code.  (Run
  multiple copies of this program at once, along with other processes,
  for an effective test.)

* Makefile

  A makefile which can be used to build the 'mdmxtest' and
  'accswitchtest' example programs.
