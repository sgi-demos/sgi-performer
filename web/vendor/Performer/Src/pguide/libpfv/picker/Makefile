#
#-------------------------------------------------------------------#
#-- Makefile for Src/pguide/libpfv/picker directory                             --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.4 $                                           --#
#--   $Date: 2002/11/07 02:32:16 $                                --#
#-------------------------------------------------------------------#

#--
#--	definitions
#--

#-- directories that are distributed in source form


SUBDIRS = \
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
	sample13.dir	\
	sample14.dir	\
	sample15.dir
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
