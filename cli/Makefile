#
# Gunderscript-2 CLI Makefile
# (C) 2014 Christian Gunderman
#
SRCDIR = src
GSLIB = ../libgunderscript.a
GSLIBDIR = ../library
GSEXE = ../gunderscript

CC = g++
AR = ar
ARFLAGS = rcs
INCDIR = $(GSLIBDIR)/include
CFLAGS = -Wall -I $(INCDIR)

# Debug flags
DFLAGS = -g

# Release flags
RFLAGS = 
FFLAGS = 

# Files to build
SOURCES = entry cli

.PHONY: all-release
all-release: FFLAGS+=$(RFLAGS)
all-release: climodule

.PHONY: all-debug
all-debug: FFLAGS+=$(DFLAGS)
all-debug: climodule

# makes the release code for gslib, regardless of the target.
.PHONY: gslib
gslib:
	$(MAKE) -C $(GSLIBDIR) all-release

.PHONY: cli
climodule: gslib $(SOURCES)
	$(CC) $(CFLAGS) $(RFLAGS) -o $(GSEXE) *.o $(GSLIB)


.PHONY: clean
clean:
	$(MAKE) -C $(GSLIBDIR) clean
	$(RM) $(SRCDIR)/*~
	$(RM) *~
	$(RM) *.o
	$(RM) $(GSEXE)

# Individual File Targets:

entry: $(SRCDIR)/entry.cc
	$(CC) $(CFLAGS) $(FFLAGS) -c $(SRCDIR)/entry.cc

cli: $(SRCDIR)/cli.cc
	$(CC) $(CFLAGS) $(FFLAGS) -c $(SRCDIR)/cli.cc
