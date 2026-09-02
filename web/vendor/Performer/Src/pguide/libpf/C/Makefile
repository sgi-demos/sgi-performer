# -*- makefile -*-
#
# Makefile for sample/pguide/libpf/C
#
# $Revision: 1.4 $
# $Date: 2002/11/20 02:04:08 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRALIBS=libpfui.lib libpfdu-util.lib 

PROGRAMS = \
        $(OUTDIR)/ASD_align.exe \
	$(OUTDIR)/ASD_decal.exe \
	$(OUTDIR)/ASD_decal2.exe \
	$(OUTDIR)/anisotropic.exe \
	$(OUTDIR)/buildarcinfo.exe \
	$(OUTDIR)/buildbw.exe \
	$(OUTDIR)/builddted.exe \
	$(OUTDIR)/builddem.exe \
	$(OUTDIR)/calcdc.exe \
        $(OUTDIR)/callig.exe \
	$(OUTDIR)/cliptex.exe \
	$(OUTDIR)/convasd.exe \
	$(OUTDIR)/deleteTBox.exe \
	$(OUTDIR)/deleteGraph.exe \
	$(OUTDIR)/doubleDCS.exe \
	$(OUTDIR)/doubleDCS2.exe \
	$(OUTDIR)/earthsky.exe \
	$(OUTDIR)/evSimple.exe \
	$(OUTDIR)/flux_chase.exe \
	$(OUTDIR)/fog.exe \
	$(OUTDIR)/hello.exe \
	$(OUTDIR)/inherit.exe \
	$(OUTDIR)/intersect.exe \
	$(OUTDIR)/layer.exe \
	$(OUTDIR)/lod_func.exe \
	$(OUTDIR)/lpstate.exe \
	$(OUTDIR)/morph_engine.exe \
	$(OUTDIR)/multichan.exe \
	$(OUTDIR)/multiTexBox.exe \
	$(OUTDIR)/multiTexBox_flux.exe \
	$(OUTDIR)/multiwin.exe \
	$(OUTDIR)/page.exe \
	$(OUTDIR)/pagedASD.exe \
	$(OUTDIR)/pickdemo.exe \
	$(OUTDIR)/pvchan.exe \
	$(OUTDIR)/shadows.exe \
	$(OUTDIR)/simple.exe \
	$(OUTDIR)/starfield.exe \
	$(OUTDIR)/texture.exe \
	$(OUTDIR)/torusLOD.exe \
	$(OUTDIR)/virtcliptex.exe \
	$(OUTDIR)/writeascii.exe \
	$(NULL)
#	$(OUTDIR)/bench.exe \
#	$(OUTDIR)/bins.exe \
#	$(OUTDIR)/complex.exe \
#	$(OUTDIR)/detail.exe \
#	$(OUTDIR)/gldlist.exe \
#	$(OUTDIR)/motif.exe \
#	$(OUTDIR)/multipipe.exe \
#	$(OUTDIR)/pbuffer.exe \
#	$(OUTDIR)/pipewin.exe \
#	$(OUTDIR)/stats.exe \
#	$(OUTDIR)/stereo.exe \
#	$(OUTDIR)/stereotb.exe \
#	$(OUTDIR)/text.exe \
#	$(OUTDIR)/text3D.exe \



!include $(PFROOT)/pfwin32.rules

.c{$(OUTDIR)}.exe:
	@if exist "$(OUTDIR)/%|fF.obj" erase "$(OUTDIR)\%|fF.obj"
	$(CC) $(CXXFLAGS) -c $<
	@if not exist "$(OUTDIR)/" mkdir "$(OUTDIR)"
	@move %|fF.obj $(OUTDIR)\%|fF.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) "$(OUTDIR)\%|fF.obj"
