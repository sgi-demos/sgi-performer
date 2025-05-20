#
#-------------------------------------------------------------------#
#-- Makefile for Src/pguide/libpfv directory                             --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.3 $                                           --#
#--   $Date: 2002/11/29 00:17:11 $                                --#
#-------------------------------------------------------------------#

#--
#--	definitions
#--

#-- directories that are distributed in source form
SUBDIRS	= picker.dir viewer.dir

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
