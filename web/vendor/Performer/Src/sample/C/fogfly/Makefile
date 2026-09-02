#-------------------------------------------------------------------------#
#-- user Makefile for sample/C/fogfly on win32                          --#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2002/11/22 23:16:38 $					--#
#-------------------------------------------------------------------------#

!include $(PFROOT)/pfwin32.defs

EXTRACFLAGS=-I../common -I.
EXTRALIBS = libpfui.lib libpfdu-util.lib 

PROGRAMS=\
	$(OUTDIR)/fogfly.exe

OBJS=\
	$(OUTDIR)/perfly.obj      \
	$(OUTDIR)/fog.obj      \
	$(OUTDIR)/cmdline.obj     \
	$(OUTDIR)/configfile.obj  \
	$(OUTDIR)/gui.obj         \
	$(OUTDIR)/keybd.obj       \
	$(OUTDIR)/generic.obj     \
	$(OUTDIR)/env.obj         \
	$(OUTDIR)/main.obj

!include $(PFROOT)/pfwin32.rules

{..\common}.c{$(OUTDIR)}.obj:
	$(CC) $(CFLAGS) -c $<
	@if not exist "$(OUTDIR)/" mkdir "$(OUTDIR)"
	@if exist "$(OUTDIR)\$(@F)" erase "$(OUTDIR)\$(@F)"
	@move $(@F) "$(OUTDIR)\$(@F)"

.c{$(OUTDIR)}.obj:
	$(CC) $(CFLAGS) -c $<
	@if not exist "$(OUTDIR)/" mkdir "$(OUTDIR)"
	@if exist "$(OUTDIR)\$(@F)" erase "$(OUTDIR)\$(@F)"
	@move $(@F) $(OUTDIR)\$(@F)

$(PROGRAMS): $(OBJS) 
	$(LINK) $(PROGRAM_LINK_FLAGS) /out:$@ $(EXTRALIBS) $**

