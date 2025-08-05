#-------------------------------------------------------------------#
#-- Makefile for Performer/src/lib directory                      --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.3 $                                            --#
#--   $Date: 2000/11/21 23:53:49 $                                --#
#-------------------------------------------------------------------#

#-------------------------------------------------------------------#
#-- This makefile builds the programs in IRIS Performer's source  --#
#-- directories. The default make target makes "opt" versions of  --#
#-- each program and library. Other (actions) targets are:        --#
#--    build debugging versions: dbg (or debug)                   --#
#--    build optimized versions: opt (or optimize)                --#
#--    build dynamic shared object versions: dso                  --#
#--    remove all unneeded files after a build: clean             --#
#--    remove all machine generated files: clobber                --#
#-------------------------------------------------------------------#

default: oglddso

DIRS = libpfdu libpfutil libpfui libpfdb

all debug dbg opt dso ddso iglopt igldbg igldso iglddso oglopt ogldbg ogldso oglddso clean clobber:
	@for i in $(DIRS); \
	do \
		if test ! -d $$i; then \
			echo "SKIPPING $$i: No such directory."; \
		else \
			echo "\nmaking $$i ($@)"; \
			(cd $$i && ${MAKE} -f GNUmakefile $@) || exit 1; \
		fi \
	done
