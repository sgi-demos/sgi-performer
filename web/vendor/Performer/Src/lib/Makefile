#
#-------------------------------------------------------------------#
#-- Makefile for Src/lib directory                             --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                           --#
#--   $Date: 2002/11/20 06:39:48 $                                --#
#-------------------------------------------------------------------#

#--
#--	definitions
#--

#-- directories that are distributed in source form
SUBDIRS	= libpfdu.dir libpfui.dir libpfuiD.dir libpfdb.dir

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
