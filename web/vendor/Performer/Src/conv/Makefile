#-------------------------------------------------------------------------#
#-- user Makefile for src/conv on win32                                 --#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2002/12/04 03:56:16 $					--#
#-------------------------------------------------------------------------#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I.
EXTRALIBS = libpfui.lib libpfdu-util.lib 

PROGRAMS=\
	$(OUTDIR)/pfconv.exe \
	$(OUTDIR)/pficonv.exe \
	$(OUTDIR)/pfpalettize.exe \
	$(OUTDIR)/wglinfo.exe \
	$(OUTDIR)/makeIBRimages.exe

!include $(PFROOT)/pfwin32.rules

.c{$(OUTDIR)}.obj:
	$(CC) $(CFLAGS) -c $<
	@if not exist "$(OUTDIR)/" mkdir "$(OUTDIR)"
	@if exist "$(OUTDIR)\$(@F)" erase "$(OUTDIR)\$(@F)"
	@move $(@F) $(OUTDIR)\$(@F)

$(OUTDIR)/pfconv.exe: $(OUTDIR)/pfconv.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) $**

$(OUTDIR)/pficonv.exe: $(OUTDIR)/pficonv.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) $**

$(OUTDIR)/pfpalettize.exe: $(OUTDIR)/pfpalettize.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) $**

$(OUTDIR)/wglinfo.exe: $(OUTDIR)/wglinfo.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) $**

$(OUTDIR)/makeIBRimages.exe: $(OUTDIR)/makeIBRimages.obj
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) $**
