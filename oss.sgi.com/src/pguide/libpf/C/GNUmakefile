#-------------------------------------------------------------------#
#-- Makefile for Performer/src/pguide/libpf/C directory           --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                           --#
#--   $Date: 2000/11/21 21:39:36 $                                --#
#-------------------------------------------------------------------#

ifndef PFROOT
PFROOT = $(ROOT)
endif


# needed for motif.c
PFDOMOTIF = 1
#if $(PFDOMOTIF)
NEED_MOTIF_LIBS = 1
#endif

# Needed for video program (divotex.c siriusvtex.c movietex.c dmvtO2.c)
#PFDOVIDEO ?= 1
#if $(PFDOVIDEO)
#NEED_VIDEO_LIBS = 1
#VIDEOCMDS = divotex siriusvtex movietex dmvtO2
#endif

include $(PFROOT)/usr/share/Performer/src/pfmakedefs.linux


.BEGIN:
	@if [ $(PFDOMOTIF) = "1" ]; then \
	echo "Linking with motif libraries. setenv PFDOMOTIF 0 to turn off."; \
	else \
	echo "Not linking with motif libraries can't build motif.c. setenv PFDOMOTIF 1 to enable."; \
	fi
	@if [ $(PFDOVIDEO) = "1" ]; then \
	echo "Linking with video libraries " $(VIDEOLIBS) ". setenv PFDOVIDEO 0 to turn off."; \
	else \
	echo "Not linking with video libraries can't build video programs. setenv PFDOVIDEO 1 to enable."; \
	fi


#-- targets are the executables
TARGETS	= \
	ASD_align \
	ASD_decal \
	ASD_decal2 \
	anisotropic \
	bench \
	bins \
	calcdc \
	cliptex \
	complex \
	detail \
	doubleDCS \
	doubleDCS2 \
	earthsky \
	evSimple \
	flux_chase \
	gldlist \
	fog \
	hello \
	inherit \
	intersect \
	layer \
	lod_func \
	lpstate \
	morph_engine \
	multiTexBox \
	multiTexBox_flux \
	multichan \
	multipipe \
	multiwin \
	page \
	pipewin \
	pvchan \
	simple \
	shader_test \
	shadows \
	stats \
	starfield \
	stereo \
	text \
	text3D \
	texture \
	torusLOD \
	virtcliptex \
	motif \
	writeascii

OBJECTS = $(TARGETS:=.o) \
	culldl.o \
	billboard.o


include $(PFROOT)/usr/share/Performer/src/pfmakerules.linux
