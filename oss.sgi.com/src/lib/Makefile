#!smake -J 1
#-------------------------------------------------------------------#
#-- Makefile for Performer/src/lib directory                      --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                            --#
#--   $Date: 2000/11/21 21:39:32 $                                --#
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

default: dso

DIRS = libpfdu libpfutil libpfui libpfdb 

all debug dbg opt dso ddso clean clobber: .MAKE
	@for i in $(DIRS); \
	do \
		if test ! -d $$i; then \
			echo "SKIPPING $$i: No such directory."; \
		else \
			echo "\nmaking $$i ($@)"; \
			cd $$i; ${MAKE} -f Makefile $@; cd ..; \
		fi \
	done
