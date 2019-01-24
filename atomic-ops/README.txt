$Id: README.txt,v 1.1 2003/03/26 18:29:28 cgd Exp $

This directory contains examples of implementations of atomic
operations using the MIPS load linked (ll) and store conditional (sc)
instructions.

These functions are implemented in C, using GCC "asm" statements, so
that the constructions can be easily usable from C code being compiled
by GCC.

The following files are present in this directory:

* spinlock.h

  This file implements spinlocks, using ll/sc instructions to
  implement an atomic "test and set" function.

* spinlock.c

  A simple example of use of the functions provided in spinlock.h.

* semaphore.h

  This file implements counting semaphores, using ll/sc to perform
  atomic read/modify/write of the semaphore values.

* semaphore.c

  A simple example of use of the functions provided in semaphore.h.

* Makefile

  A makefile which can be used to build the 'spinlock' and 'semaphore'
  example programs.
