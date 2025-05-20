# -*- makefile -*-
#
# Makefile for sample/pguide/libpf/C++
#
# $Revision: 1.3 $
# $Date: 2002/11/20 02:04:08 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRACXXFLAGS=$(EXTRACFLAGS)
EXTRALIBS=libpfui.lib libpfdu-util.lib 

PROGRAMS =  \
	$(OUTDIR)/anisotropic.exe \
	$(OUTDIR)/blend_engine.exe \
        $(OUTDIR)/bump.exe \
	$(OUTDIR)/cullPgmSimple.exe \
	$(OUTDIR)/cullPgmMultipass.exe \
	$(OUTDIR)/drawOrder.exe \
	$(OUTDIR)/earthsky.exe \
	$(OUTDIR)/fcs_animate.exe \
	$(OUTDIR)/fcs_engine.exe \
	$(OUTDIR)/flux_bbox.exe \
	$(OUTDIR)/flux_color.exe \
	$(OUTDIR)/flux_switch.exe \
	$(OUTDIR)/flux_sync_complete.exe \
	$(OUTDIR)/fluxed_gset.exe \
	$(OUTDIR)/fog.exe \
	$(OUTDIR)/gsetCB.exe \
	$(OUTDIR)/hello.exe \
	$(OUTDIR)/inherit.exe \
	$(OUTDIR)/intersect.exe \
	$(OUTDIR)/layer.exe \
	$(OUTDIR)/makeSequence.exe \
	$(OUTDIR)/morph_engine.exe \
	$(OUTDIR)/morph_LOD.exe \
	$(OUTDIR)/multichan.exe \
	$(OUTDIR)/multiwin.exe \
	$(OUTDIR)/occlusionCull.exe \
	$(OUTDIR)/occlusionCullConcave.exe \
	$(OUTDIR)/pfi_movie.exe \
	$(OUTDIR)/pickdemo.exe \
	$(OUTDIR)/projective.exe \
	$(OUTDIR)/shadowsNew.exe \
	$(OUTDIR)/simple.exe \
	$(OUTDIR)/strobe_engine.exe \
	$(OUTDIR)/sum_engine.exe \
	$(OUTDIR)/text.exe \
	$(OUTDIR)/texture.exe \
	$(OUTDIR)/user_engine.exe \
	$(OUTDIR)/volume.exe \
	$(OUTDIR)/writeascii.exe \
	$(OUTDIR)/IBRnode.exe \
        $(NULL)
#	$(OUTDIR)/bench \
#	$(OUTDIR)/complex \
#	$(OUTDIR)/detail \
#	$(OUTDIR)/multipipe \
#	$(OUTDIR)/regCmbTexBlend \
#	$(OUTDIR)/simpleASD \
#	$(OUTDIR)/stats \
#	$(OUTDIR)/stringBin \
#	$(OUTDIR)/text3D \
#	$(OUTDIR)/torusLOD \

!include $(PFROOT)/pfwin32.rules

.cxx{$(OUTDIR)}.exe:
	@if exist "$(OUTDIR)/%|fF.obj" erase "$(OUTDIR)\%|fF.obj"
	$(CC) $(CXXFLAGS) -c $<
	@if not exist "$(OUTDIR)/" mkdir "$(OUTDIR)"
	@move %|fF.obj $(OUTDIR)\%|fF.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) "$(OUTDIR)\%|fF.obj"


