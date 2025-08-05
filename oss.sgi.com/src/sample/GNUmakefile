#!smake -J 1
#-------------------------------------------------------------------#
#-- Makefile for Performer/src/sample directory                   --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.2 $                                            --#
#--   $Date: 2000/11/21 23:40:13 $                                --#
#-------------------------------------------------------------------#

#-------------------------------------------------------------------#
#-- This makefile builds the programs in OpenGL Performer's sample--#
#-- directories. The default make target makes "opt" versions of  --#
#-- each program and library. Other (actions) targets are:        --#
#--    build debugging versions: dbg (or debug)                   --#
#--    build optimized versions: opt (or optimize)                --#
#--    build dynamic shared object versions: dso                  --#
#--    remove all unneeded files after a build: clean             --#
#--    remove all machine generated files: clobber                --#
#-------------------------------------------------------------------#

#--
#--	definitions
#--

#-- directories that need building
DIRS	= \
	C  \
	C++ 

#--
#--	generic targets
#--

#-- default action is to make optimized non-debug versions
default: ogldso

#-- make designated target in subdirectories
all debug dbg opt dso ddso iglopt igldbg igldso iglddso oglopt ogldbg ogldso oglddso clean clobber:
	@for i in $(DIRS); \
	do \
		echo "\nmaking $$i ($@)"; \
		cd $$i; $(MAKE) -f GNUmakefile $@; cd ..; \
	done
