#
#-------------------------------------------------------------------#
#-- Makefile for Src/pguide/libpf directory                             --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.3 $                                           --#
#--   $Date: 2002/11/19 01:22:38 $                                --#
#-------------------------------------------------------------------#

#--
#--	definitions
#--

#-- directories that are distributed in source form
SUBDIRS	= C.dir C++.dir

#--
#--	generic targets
#--

#-- default action is to make everything
default: $(SUBDIRS)


$(SUBDIRS):
	@echo making in $* subdirectory
	@cd $*
	@$(MAKE)
	@cd ..
