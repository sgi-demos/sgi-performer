#
#-------------------------------------------------------------------#
#-- Makefile for Src/apps/C directory                             --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.2 $                                           --#
#--   $Date: 2002/11/21 21:44:57 $                                --#
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
