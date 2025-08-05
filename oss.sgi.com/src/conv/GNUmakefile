#-------------------------------------------------------------------#
#-- Makefile for Performer/src/conv directory                     --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $						  --#
#--   $Date: 2000/11/21 21:39:32 $				  --#
#-------------------------------------------------------------------#

ifndef PFROOT
PFROOT = $(ROOT)
endif

include $(PFROOT)/usr/share/Performer/src/pfmakedefs.linux


#-- targets are the executables
TARGETS	= \
	pfconv \
	pficonv \
	pfpalettize

OBJECTS = $(TARGETS:=.o)


include $(PFROOT)/usr/share/Performer/src/pfmakerules.linux
