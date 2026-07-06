# -*- makefile -*-
#
# Makefile for sample/pguide/libpr/C++
#
# $Revision: 1.1 $
# $Date: 2002/12/09 01:19:50 $
#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I. 
EXTRALIBS=libpfui.lib libpfdu-util.lib 

PROGRAMS = \
	$(OUTDIR)/movie.exe \
	$(OUTDIR)/texman.exe \
	$(OUTDIR)/texmem.exe \
	$(NULL)

!include $(PFROOT)/pfwin32.rules

.cxx{$(OUTDIR)}.exe:
	@if exist "$(OUTDIR)/%|fF.obj" erase "$(OUTDIR)\%|fF.obj"
	$(CC) $(CXXFLAGS) -c $<
	@if not exist "$(OUTDIR)/" mkdir "$(OUTDIR)"
	@move %|fF.obj $(OUTDIR)\%|fF.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) "$(OUTDIR)\%|fF.obj"

.c{$(OUTDIR)}.exe:
	@if exist "$(OUTDIR)/%|fF.obj" erase "$(OUTDIR)\%|fF.obj"
	$(CC) $(CXXFLAGS) -c $<
	@if not exist "$(OUTDIR)/" mkdir "$(OUTDIR)"
	@move %|fF.obj $(OUTDIR)\%|fF.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) "$(OUTDIR)\%|fF.obj"
