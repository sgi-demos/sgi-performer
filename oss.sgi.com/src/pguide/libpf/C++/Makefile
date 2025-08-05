#!smake -J 1
#-------------------------------------------------------------------#
#-- Makefile for Performer/src/pguide/libpf/C++ directory         --#
#-------------------------------------------------------------------#
#-- RCS version information                                       --#
#--   $Revision: 1.1 $                                           --#
#--   $Date: 2000/11/21 21:39:37 $                                --#
#-------------------------------------------------------------------#

PFROOT ?= $(ROOT)

# needed for motif.c
PFDOMOTIF ?= 1
#if $(PFDOMOTIF)
NEED_MOTIF_LIBS = 1
#endif

# Needed for video program (siriusvtex.c dmvtex.c)
PFDOVIDEO ?= 1
#if $(PFDOVIDEO)
NEED_VIDEO_LIBS = 1
#endif

include $(PFROOT)/usr/share/Performer/src/pfmakedefs

# C++ programs must use CC as linker
LINKERF=$(CXXF)

.BEGIN:
	@if [ $(PFDOMOTIF) = "1" ]; then \
	echo "Linking with motif libraries. setenv PFDOMOTIF 0 to turn off."; \
	else \
	echo "Not linking with motif libraries can't build motif.c. setenv PFDOMOTIF 1 to enable."; \
	fi

#-- targets are the executables
TARGETS	= \
	anisotropic \
	bench \
        bump \
	blend_engine \
	complex \
	detail \
	earthsky \
	fcs_animate \
	fcs_engine \
	flux_bbox \
	flux_color \
	flux_switch \
	flux_sync_complete \
	fluxed_gset \
	fog \
	hello \
	inherit \
	intersect \
	iostream \
	layer \
	morph_engine \
	morph_LOD \
	motif \
	multichan \
	multipipe \
	multiwin \
	nclops \
	pfi_movie \
        projective \
        shader_blue_and_purple \
        shader_multiple_shaders \
        shader_red_material \
        shader_test \
        shader_wood_texture \
	simple \
	simpleASD \
	stats \
	stringBin \
	strobe_engine \
	sum_engine \
	text \
	text3D \
	texture \
	torusLOD \
	user_engine \
	volume \
	writeascii

OBJECTS = $(TARGETS:=.o) \
	culldl.o


include $(PFROOT)/usr/share/Performer/src/pfmakerules
