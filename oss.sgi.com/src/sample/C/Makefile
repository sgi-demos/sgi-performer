#!smake -J 1
#-------------------------------------------------------------------#
#-- Makefile for Performer/src/sample directory                   --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                           --#
#--   $Date: 2000/11/21 21:39:43 $                                --#
#-------------------------------------------------------------------#

#--
#--	definitions
#--

#-- directories that are distributed in source form
DIRS	= \
	perfly\
	asdfly \
	clipfly \
	fakeclipfly \
	fogfly

#--
#--	generic targets
#--

#-- default action is to make everything
default: ogldso

#-- make designated target in subdirectories
all debug dso ddso opt dbg oglopt ogldbg ogldso oglddso iglopt igldbg igldso iglddso clean clobber: .MAKE
	@for i in $(DIRS); \
	do \
		echo "\nmaking $$i ($@)"; \
		cd $$i; $(MAKE) -f Makefile $@; cd ..; \
	done
