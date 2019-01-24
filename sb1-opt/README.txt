$Id: README.txt,v 1.1 2003/09/05 17:59:46 cgd Exp $

This directory contains implementations of several standard C
library functions which have been optimized for the SB-1 CPU core.

These functions are implemented in (preprocessed) assembly code, but
should drop in to most build environments fairly easily.

See the comments in the Makefile about how to build these functions
into a library.

The following files are present in this directory:

* memcpy.S

  An optimized implementation of memcpy().

* memset.S

  An optimized implementation of memset().

* strcmp.S

  An optimized implementation of strcmp().

* sb1-opt-common.h

  Common preprocessor definitions used by the assembler sources in this
  directory.

* Makefile

  A makefile which can be used to build a small library containing
  the optimized functions.

