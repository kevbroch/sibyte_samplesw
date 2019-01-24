$Id: README.txt,v 1.9 2004/12/11 08:39:22 cgd Exp $

This is a collection of sample software related to the Broadcom
(SiByte) BCM1125/H and BCM1250 MIPS64 SOCs.  For information about
changes in this distribution, see the "version" file in this
directory.

Below is a list of the sample software directories which
are provided, with a brief description of the contents of each.  Most
of them also contain a README file which contains more information.

* atomic-ops

  Examples of implementations of some atomic or read-modify-write
  operations (spin locks, semaphores, etc.) for the MIPS architecture.

* cache-err

  Example SB-1 cache error handler, and sample code to provoke cache
  errors for testing.

* cp0

  Examples of use of MIPS CP0 registers, including use of some
  of the CP0 access functions provided by the file
  sb1250/include/sbmips.h relative to this directory (which can be
  found as <sb1250-include/sbmips.h> in the sb1-elf tools).

* include

  Various headers and example programs related to the SiByte SOCs
  Special attention should be paid to "include/sibyte", which contains
  header files that define constants and macros relating to the SiByte
  SOC peripheral registers.

* libexception

  A simple MIPS64 exception handling library.  (This library is also
  included in the SiByte System Software tools "sb1-elf"
  cross-compilation environment starting with version 2.7.1, for
  ease of use when developing embedded applications with those tools.)

* libmips_fpu

  A library and header file which allows easy access to the MIPS FPU
  control registers.

* libsbprof

  A library which can profile programs using the SiByte CPU performance
  counters and SOC trace buffer.  This library layers on top of the
  libexception library.  (This library is also included in the SiByte
  System Software tools "sb1-elf" cross-compilation environment starting
  with version 2.7.1, for ease of use when analyzing the performance of
  embedded applications built with those tools.)

  This is also an example of what one has to do to build a minimal
  profiling environment for an operating system that we don't
  currently support.  For more examples, see the NetBSD and Linux
  profiling drivers, included with the performance tools distribution.

* mdmx

  Headers to facilitate use of the MIPS MDMX Application Specific
  Extension provided by the SB-1 CPU core, and some example programs.
  (Since the SB-1 provides only ".ob"-format MDMX operations, only
  those are included here.)

* psingle

  Headers to facilitate use of of the MIPS "Paired Single" floating
  point format (provided by the SB-1 CPU core), and some example
  programs.

* sb1-opt

  Sources for certain standard C library functions which have been
  optimized for the SB-1 CPU core.

* tlb

  Headers to facilitate MIPS64 TLB access, and some example programs.

* unaligned

  Example of a MIPS unaligned access exception handler.
