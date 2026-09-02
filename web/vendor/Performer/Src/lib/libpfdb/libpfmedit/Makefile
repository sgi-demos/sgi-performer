#-------------------------------------------------------------------------#
#-- Makefile for libpmedit -- the MEDIT file loader			--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.2 $							--#
#--   $Date: 2002/12/01 21:11:03 $					--#
#-------------------------------------------------------------------------#

!include $(PFROOT)/pfwin32.defs

LIBNAME = pfmedit

LOCALCFLAGS=

LIBS=lib$(LIBNAME).lib

OBJS=\
	$(OUTDIR)/$(LIBNAME).obj  \
	$(OUTDIR)/meditfile.obj       \
	$(OUTDIR)/pfSaveMedit.obj       \
	$(NULL)

EXTRACFLAGS=-D__BUILD_LIBPFDB__ -D_DLL $(LOCALCFLAGS)
EXTRALIBS=libpfdu-util.lib /LIBPATH:"$(PFROOT)"\lib\libpfdb
EXTRACXXFLAGS=$(EXTRACFLAGS)

!include $(PFROOT)/pfwin32.rules

{$(OUTDIR)}.obj:
	$(CC) $(CFLAGS) -c $<
	@move $(@F) "$(OUTDIR)\$(@F)"

.cxx{$(OUTDIR)}.obj:
	$(CC) $(CXXFLAGS) -c $<
	@move $(@F) $(OUTDIR)\$(@F)

lib$(LIBNAME).lib: $(OBJS) 
	$(LINK) $(DLL_LINK_FLAGS) /out:$(OUTDIR)/$(@:.lib=).dll $(EXTRALIBS) $**
