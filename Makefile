#
# Gunderscript-2 Complete Makefile
# (C) 2014 Christian Gunderman
#

# building the CLI automatically builds the library as well
CLIDIR = cli

.PHONY: all-release
all-release:
	$(MAKE) -C $(CLIDIR) all-release

.PHONY: all-debug
all-debug:
	$(MAKE) -C $(CLIDIR) all-debug

.PHONY: clean
clean:
	$(RM) $(SRCDIR)/*~
	$(RM) $(INCDIR)/*~
	$(RM) *~
	$(RM) *.o
	$(MAKE) -C $(CLIDIR) clean
