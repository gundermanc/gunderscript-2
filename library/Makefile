#
# Gunderscript-2 Library Makefile
# (C) 2014 Christian Gunderman
#
SRCDIR = src
CC = g++
AR = ar
ARFLAGS = rcs
INCDIR = include
CFLAGS = -fPIC -std=c++11 -Wall -I $(INCDIR)
TESTDIR = test
# Debug flags
DFLAGS = -g -DDEBUG

# Release flags
RFLAGS = 

FFLAGS =
GSLIB = ../libgunderscript.a

# Files to build
SOURCES = symbol_table

.PHONY: all-release
all-release: FFLAGS+=$(RFLAGS)
all-release: gslib

.PHONY: run-all-test
run-all-test: all-test
	$(TESTDIR)/*_unittest

.PHONY: all-test
all-test:
	$(MAKE) -C $(TESTDIR) all

.PHONY: all-debug
all-debug: FFLAGS+=$(DFLAGS)
all-debug: gslib

.PHONY: gslib
gslib: $(SOURCES)
	$(AR) $(ARFLAGS) $(GSLIB) *.o

.PHONY: clean
clean:
	$(RM) $(SRCDIR)/*~
	$(RM) $(INCDIR)/*~
	$(RM) *~
	$(RM) *.o
	$(RM) $(GSLIB)
	$(MAKE) -C $(TESTDIR) clean

# Individual File Targets:

symbol_table: $(SRCDIR)/symbol_table.cc
	$(CC) $(CFLAGS) $(FFLAGS) -c $(SRCDIR)/symbol_table.cc

#standard_map: $(SRCDIR)/standard_map.cc
#	$(CC) $(CFLAGS) $(FFLAGS) -c $(SRCDIR)/standard_map.cc
