# -*- makefile -*-
#
# Makefile for sample/tools
#
# $Revision: 1.1 $
# $Date: 2002/11/20 19:45:12 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRALIBS=libpfui.lib libpfdu-util.lib 

PROGRAMS = \
	$(OUTDIR)/rgb2raw.exe \
	$(OUTDIR)/rsets.exe \
	$(OUTDIR)/shrink.exe \
	$(OUTDIR)/to5551.exe \
	$(OUTDIR)/to888.exe \
	$(OUTDIR)/to8888.exe \
	$(OUTDIR)/toL16.exe \
	$(OUTDIR)/generatePhongMap.exe \
        $(NULL)
#	$(OUTDIR)/dioinfo.exe \
#	$(OUTDIR)/viewtile.exe \

!include $(PFROOT)/pfwin32.rules

.c{$(OUTDIR)}.exe:
	@if exist "$(OUTDIR)/%|fF.obj" erase "$(OUTDIR)\%|fF.obj"
	$(CC) $(CXXFLAGS) -c $<
	@if not exist "$(OUTDIR)/" mkdir "$(OUTDIR)"
	@move %|fF.obj $(OUTDIR)\%|fF.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) "$(OUTDIR)\%|fF.obj"

.cxx{$(OUTDIR)}.exe:
	@if exist "$(OUTDIR)/%|fF.obj" erase "$(OUTDIR)\%|fF.obj"
	$(CC) $(CXXFLAGS) -c $<
	@if not exist "$(OUTDIR)/" mkdir "$(OUTDIR)"
	@move %|fF.obj $(OUTDIR)\%|fF.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) "$(OUTDIR)\%|fF.obj"
