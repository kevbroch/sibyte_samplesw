$Id: README.txt,v 1.5 2003/10/21 03:31:06 cgd Exp $

This directory contains a header file which implements C wrappers for
common MIPS64 TLB operations and TLB-related CP0 register access,
and some sample programs which demonstrate reading and writing the TLB.

These functions and macros are implemented in C, using GCC "asm"
statements, so that the constructions can be easily usable from C code
being compiled by GCC.

The following files are present in this directory:

* tlb_inval.txt

  A document that explains some of the intracacies related to
  flushing MIPS64-compatible TLBs.

* tlb.h

  Include file which provide the TLB access (inline) functions.

* tlb_utils.h

  Include file which provides definitions shared between test
  programs, and prototypes for shared functions in tlb_utils.c.

* tlb_utils.c

  Some common TLB-related utility functions.  These aren't in
  tlb.h because they're not "necessary" for use of the TLB, and
  because they include system dependencies (e.g. some use printf).

* tlballoc.c

  A replacement for the low-level memory allocation routines provided
  by newlib and libgloss.  If you compile and link this into your
  program that uses newlib and libgloss (i.e., uses -Tcfe.ld), it
  will map up to 2GB of your physical memory using the TLB, and make
  that TLB-mapped memory available to malloc().  More generally, this
  is an example of programming the TLB to do something useful.

* tlbconvert.c

  A program (meant to be compiled for and run on host systems) which
  will encode TLB registers given virtual and physical addresses to
  map, and which will decode TLB registers into addresses and bitfields.

* tlbdump.c

  A sample program which dumps the contents of the TLB in the CPU on
  which it is run.

* tlbtest.c

  A sample program which tests the TLB in the CPU on which it is run.
  (Note: as of CFE 1.0.37, this code will actually cause the firmware
  to crash after it exits.)

* tlbtestalloc.c

  A program which tests the allocator provided by tlballoc.c.

* Makefile

  A makefile which can be used to build the 'tlbdump' and
  'tlbtest' example programs.
