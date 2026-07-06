# -*- makefile -*-
#
# Makefile for sample/pguide/libpr/C++
#
# $Revision: 1.1 $
# $Date: 2002/11/20 02:04:08 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRALIBS=libpfui.lib libpfdu-util.lib 

PROGRAMS = \
	$(OUTDIR)/fillstats.exe \
	$(OUTDIR)/geomath.exe \
	$(OUTDIR)/gset.exe \
	$(OUTDIR)/linmath.exe \
	$(OUTDIR)/winfbconfig.exe \
	$(NULL)
#	$(OUTDIR)/hlcube.exe  X loop\
#	$(OUTDIR)/iostream.exe \
#	$(OUTDIR)/zrgb.exe 

!include $(PFROOT)/pfwin32.rules

.cxx{$(OUTDIR)}.exe:
	@if exist "$(OUTDIR)/%|fF.obj" erase "$(OUTDIR)\%|fF.obj"
	$(CC) $(CXXFLAGS) -c $<
	@if not exist "$(OUTDIR)/" mkdir "$(OUTDIR)"
	@move %|fF.obj $(OUTDIR)\%|fF.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) "$(OUTDIR)\%|fF.obj"
