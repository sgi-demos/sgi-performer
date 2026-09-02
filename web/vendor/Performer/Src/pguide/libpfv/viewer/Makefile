#
#-------------------------------------------------------------------#
#-- Makefile for Src/pguide/libpfv/viewer directory               --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.3 $                                           --#
#--   $Date: 2002/12/05 02:51:18 $                                --#
#-------------------------------------------------------------------#

#--
#--	definitions
#--

#-- directories that are distributed in source form

#modules.dir
SUBDIRS = \
	modules.dir \
	sample01.dir	\
	sample02.dir	\
	sample03.dir	\
	sample04.dir	\
	sample05.dir	\
	sample06.dir	\
	sample07.dir	\
	sample08.dir  	\
	sample09.dir	\
	sample10.dir 	\
	sample11.dir	\
	sample12.dir  	\
	sample13.dir

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
