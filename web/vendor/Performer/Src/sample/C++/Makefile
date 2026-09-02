#
#-------------------------------------------------------------------#
#-- Makefile for Src/sample/C++ directory                         --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.2 $                                           --#
#--   $Date: 2002/12/06 21:55:53 $                                --#
#-------------------------------------------------------------------#

#--
#--	definitions
#--

#-- directories that are distributed in source form
SUBDIRS	= pfview.dir rotorwash.dir perfly.dir

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
