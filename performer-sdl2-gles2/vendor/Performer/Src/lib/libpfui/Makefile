#-------------------------------------------------------------------------#
#-- user Makefile for libpfui on win32                             --#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.2 $							--#
#--   $Date: 2002/11/22 23:16:38 $					--#
#-------------------------------------------------------------------------#

!include $(PFROOT)/pfwin32.defs

LIBNAME=pfui
LIBS=lib$(LIBNAME).lib

EXTRACFLAGS=-D__BUILD_PFUI__ -D_DLL $(LOCALCFLAGS)
EXTRACXXFLAGS=$(EXTRACFLAGS)
EXTRALIBS=libpfdu-util.lib

!include $(PFROOT)/pfwin32.rules

OBJS=\
	$(OUTDIR)/pfiInit.obj \
	$(OUTDIR)/pfiCollide.obj \
	$(OUTDIR)/pfiInputXform.obj \
	$(OUTDIR)/pfiPick.obj \
	$(OUTDIR)/pfiXformer.obj \
	$(OUTDIR)/pfiDrive.obj \
	$(OUTDIR)/pfiFly.obj \
	$(OUTDIR)/pfiSpheric.obj \
	$(OUTDIR)/pfiTrackball.obj \
	$(OUTDIR)/cCollide.obj \
	$(OUTDIR)/cPick.obj \
	$(OUTDIR)/cInputXform.obj \
	$(OUTDIR)/cXformer.obj \
	$(NULL)

lib$(LIBNAME).lib: $(OBJS) 
	$(LINK) $(DLL_LINK_FLAGS) /out:$(OUTDIR)/$(@:.lib=).dll $(EXTRALIBS) $**
