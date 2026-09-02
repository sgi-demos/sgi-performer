#-------------------------------------------------------------------------#
#-- user Makefile for libpfdu-util on win32                             --#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.2 $							--#
#--   $Date: 2002/11/19 03:22:50 $					--#
#-------------------------------------------------------------------------#

!include $(PFROOT)/pfwin32.defs

LIBNAME=pfdu-util
LIBS=lib$(LIBNAME).lib

EXTRACFLAGS=-D__BUILD_PFDU__ -D__BUILD_PFUTIL__ -D_DLL $(LOCALCFLAGS)
EXTRACXXFLAGS=$(EXTRACFLAGS)

!include $(PFROOT)/pfwin32.rules

OBJS=\
	$(OUTDIR)/objfnt.obj         \
	$(OUTDIR)/pfdASDAlign.obj    \
	$(OUTDIR)/pfdASDClipring.obj \
	$(OUTDIR)/pfdASDGen.obj      \
	$(OUTDIR)/pfdBillboard.obj   \
	$(OUTDIR)/pfdBreakup.obj     \
	$(OUTDIR)/pfdBuildASD.obj    \
	$(OUTDIR)/pfdProcASD.obj     \
	$(OUTDIR)/pfdBuilder.obj     \
	$(OUTDIR)/pfdCallbacks.obj   \
	$(OUTDIR)/pfdCleanTree.obj   \
	$(OUTDIR)/pfdExtractTris.obj \
	$(OUTDIR)/pfdGSet.obj        \
	$(OUTDIR)/pfdGeoBuilder.obj  \
	$(OUTDIR)/pfdLayer.obj       \
	$(OUTDIR)/pfdLoadImage.obj   \
	$(OUTDIR)/pfdLoadFile.obj    \
	$(OUTDIR)/pfdLoadFont.obj    \
	$(OUTDIR)/pfdOpenFile.obj    \
	$(OUTDIR)/pfdShare.obj       \
	$(OUTDIR)/pfdSpatial.obj     \
	$(OUTDIR)/pfdTMesher.obj     \
	$(OUTDIR)/pfdImage.obj       \
	$(OUTDIR)/type1rd.obj        \
	$(OUTDIR)/asdnode.obj               \
	$(OUTDIR)/barrier.obj               \
	$(OUTDIR)/boxlod.obj                \
	$(OUTDIR)/cliptexture.obj           \
	$(OUTDIR)/clipcenter.obj		\
	$(OUTDIR)/closest.obj               \
	$(OUTDIR)/collide.obj               \
	$(OUTDIR)/cursor.obj                \
	$(OUTDIR)/eventq.obj                \
	$(OUTDIR)/getopt.obj                \
	$(OUTDIR)/gui.obj                   \
	$(OUTDIR)/hash.obj                  \
	$(OUTDIR)/lockcpu.obj               \
	$(OUTDIR)/lpoint.obj                \
	$(OUTDIR)/mipmin.obj                \
	$(OUTDIR)/mpstats.obj               \
	$(OUTDIR)/path.obj                  \
	$(OUTDIR)/random.obj                \
	$(OUTDIR)/reallyinvalidate.obj      \
	$(OUTDIR)/rvous.obj                 \
	$(OUTDIR)/shmem.obj                 \
	$(OUTDIR)/smoke.obj                 \
	$(OUTDIR)/snapwin.obj               \
	$(OUTDIR)/style.obj                 \
	$(OUTDIR)/tex.obj                   \
	$(OUTDIR)/timer.obj                 \
	$(OUTDIR)/trav.obj                  \
	$(OUTDIR)/xfont.obj                 \
	$(OUTDIR)/input.obj                 \
	$(OUTDIR)/pup.obj                   \
	$(OUTDIR)/mco.obj                   \
	$(OUTDIR)/xwin.obj \
	$(OUTDIR)/pfuAutoList.obj \
	$(OUTDIR)/clipparams.obj \
	$(OUTDIR)/gridify.obj \
	$(OUTDIR)/icacheviewer.obj \
	$(OUTDIR)/pfuProcessManager.obj \
	$(OUTDIR)/pfuClipCenterNode.obj \
	$(OUTDIR)/pfuPalettizer.obj \
	$(OUTDIR)/pfuTextureManager.obj \
	$(OUTDIR)/pfuTextureMemory.obj \
	$(OUTDIR)/spherefront.obj \
	$(OUTDIR)/spherefront2.obj \
	$(OUTDIR)/texderivs.obj \
	$(OUTDIR)/pfuPixelFormat.obj \
	$(NULL)

{..\libpfutil}.c{$(OUTDIR)}.obj:
	$(CC) $(CFLAGS) -c $<
	@move $(@F) "$(OUTDIR)\$(@F)"

{..\libpfutil}.cxx{$(OUTDIR)}.obj:
	$(CC) $(CXXFLAGS) -c $<
	@move $(@F) $(OUTDIR)\$(@F)

lib$(LIBNAME).lib: $(OBJS) 
	$(LINK) $(DLL_LINK_FLAGS) /out:$(OUTDIR)/$(@:.lib=).dll $(EXTRALIBS) $**
