#!smake -J 1
#-------------------------------------------------------------------#
#-- Makefile for Performer/src/sample directory                   --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.2 $                                           --#
#--   $Date: 2000/11/21 23:38:53 $                                --#
#-------------------------------------------------------------------#

#--
#--	definitions
#--

#-- directories that are distributed in source form
DIRS	= \
	C C++

#--
#--	generic targets
#--

#-- default action is to make everything
default: ogldso

#-- make designated target in subdirectories
all debug dso ddso opt dbg oglopt ogldbg ogldso oglddso iglopt igldbg igldso iglddso clean clobber:
	@for i in $(DIRS); \
	do \
		echo "\nmaking $$i ($@)"; \
		(cd $$i && $(MAKE) -f GNUmakefile $@) || exit 1; \
	done
