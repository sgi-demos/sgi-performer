#
#-------------------------------------------------------------------#
#-- Makefile for Src/pguide/libpfv/viewer directory               --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.24 $                                           --#
#--   $Date: 2002/12/05 01:25:02 $                                --#
#-------------------------------------------------------------------#

#--
#--	definitions
#--

#-- directories that are distributed in source form


SUBDIRS = \
	myModule.dir	\
	pfvmNavigator.dir	\
	pfvmTrackball.dir	\
	pfvmLoader.dir	\
	pfvmDrawStyle.dir	\
	pfvmPicker.dir	\
	pfvmEarthSky.dir \
	pfvmLogo.dir \
	pfvmWorldSwitcher.dir \
	pfvmStats.dir \
	pfvmSnapshot.dir

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
