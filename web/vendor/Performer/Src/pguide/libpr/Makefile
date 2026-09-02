#
#-------------------------------------------------------------------#
#-- Makefile for Src/pguide/libpr directory                       --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                           --#
#--   $Date: 2002/11/20 02:04:08 $                                --#
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
