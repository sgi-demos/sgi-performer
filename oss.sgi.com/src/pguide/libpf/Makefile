#!smake -J 1
#-------------------------------------------------------------------#
#-- Makefile for Performer/src/sample directory                   --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                           --#
#--   $Date: 2000/11/21 21:39:36 $                                --#
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

#-- default action is to make DSOs
default: dso

#-- make designated target in subdirectories
all debug dso ddso opt dbg oglopt ogldbg ogldso oglddso iglopt igldbg igldso iglddso clean clobber: .MAKE
	@for i in $(DIRS); \
	do \
		echo "\nmaking $$i ($@)"; \
		cd $$i; $(MAKE) -f Makefile $@; cd ..; \
	done
