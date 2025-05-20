# -*- makefile -*-
#
# Makefile for sample/pguide/libpr/C
#
# $Revision: 1.1 $
# $Date: 2002/12/04 04:31:17 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRALIBS=libpfui.lib libpfdu-util.lib 

PROGRAMS = \
        $(OUTDIR)/testfrags.exe \
	$(OUTDIR)/benchgs.exe \
	$(OUTDIR)/cliptex.exe \
	$(OUTDIR)/cliptex_mwin.exe \
	$(OUTDIR)/fillstats.exe \
	$(OUTDIR)/geomath.exe \
	$(OUTDIR)/gset.exe \
	$(OUTDIR)/hlcube.exe \
	$(OUTDIR)/icache.exe \
        $(OUTDIR)/icache_mwin.exe \
	$(OUTDIR)/linmath.exe \
	$(OUTDIR)/mipmap.exe \
	$(OUTDIR)/packedattrs.exe \
	$(OUTDIR)/queryftr.exe \
	$(OUTDIR)/querysys.exe \
	$(OUTDIR)/queryperf.exe \
	$(OUTDIR)/texlist.exe \
	$(OUTDIR)/texlod.exe \
	$(OUTDIR)/winfbconfig.exe \
	$(OUTDIR)/vchan.exe \
	$(NULL)

#	$(OUTDIR)/pipestats.exe 
#	$(OUTDIR)/queryvchan.exe \
#	$(OUTDIR)/zrgb.exe \

!include $(PFROOT)/pfwin32.rules

.c{$(OUTDIR)}.exe:
	@if exist "$(OUTDIR)/%|fF.obj" erase "$(OUTDIR)\%|fF.obj"
	$(CC) $(CXXFLAGS) -c $<
	@if not exist "$(OUTDIR)/" mkdir "$(OUTDIR)"
	@move %|fF.obj $(OUTDIR)\%|fF.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) "$(OUTDIR)\%|fF.obj"
