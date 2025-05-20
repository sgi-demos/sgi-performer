#
#-------------------------------------------------------------------#
#-- Makefile for Src/sample/C directory                             --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.2 $                                           --#
#--   $Date: 2002/11/20 15:38:04 $                                --#
#-------------------------------------------------------------------#

#--
#--	definitions
#--

#-- directories that are distributed in source form
SUBDIRS	= perfly.dir clipfly.dir fogfly.dir #asdfly.dir fakeclipfly.dir

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
