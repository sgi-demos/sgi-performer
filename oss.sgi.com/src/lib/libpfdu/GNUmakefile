#-------------------------------------------------------------------------#
#-- Makefile for Performer/src/lib/libpfdu				--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2000/11/21 21:39:35 $					--#
#-------------------------------------------------------------------------#

ifndef PFROOT
PFROOT = $(ROOT)
endif

include $(PFROOT)/usr/share/Performer/src/pfmakedefs.linux


#-- base name of library
LIBRARY	= libpfdu

CFILES = \
	objfnt.c \
	pfdASDGen.c \
	pfdBillboard.c \
	pfdBreakup.c \
	pfdBuildASD.c \
	pfdBuilder.c \
	pfdCallbacks.c \
	pfdCleanTree.c \
	pfdGSet.c \
	pfdGeoBuilder.c \
	pfdImage.c \
	pfdLayer.c \
	pfdLoadFile.c \
	pfdLoadFont.c \
	pfdLoadImage.c \
	pfdLoadShader.c \
	pfdLoadShader_Lex.c \
	pfdLoadShader_Parse.c \
	pfdOpenFile.c \
	pfdShare.c \
	pfdSpatial.c \
	pfdTMesher.c \
	type1rd.c


include $(PFROOT)/usr/share/Performer/src/pfmakerules.linux

GCINCS+=-I.

pfdLoadShader_Lex.c: ../pfdLoadShader_Lex.lxx pfdLoadShader_Parse.c
	flex -P__pfdLoadShader_ -o$@ ../pfdLoadShader_Lex.lxx

pfdLoadShader_Parse.c: ../pfdLoadShader_Parse.yxx
	bison -t -v -d -p__pfdLoadShader_ ../pfdLoadShader_Parse.yxx -o$@

