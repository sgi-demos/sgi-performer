#-------------------------------------------------------------------------#
#-- Makefile for Performer/src/lib/libpfutil				--#
#-------------------------------------------------------------------------#
#-- RCS version information						--#
#--   $Revision: 1.1 $							--#
#--   $Date: 2000/11/21 21:39:36 $					--#
#-------------------------------------------------------------------------#

ifndef PFROOT
PFROOT = $(ROOT)
endif

include $(PFROOT)/usr/share/Performer/src/pfmakedefs.linux


#-- base name of library
LIBRARY	= libpfutil

#-- see if we have POSIX for the process control stuff
#if $(RELEASE) >= 6.4
PFHAVEPOSIX=1
#else

# To get POSIX scheduling on 6.2, install patches (needs patch 1717 and patchset 43)
# or replacement patches and set or define PFHAVEPOSIX or switch the #if below. 
#if 1
#PFHAVEPOSIX?=0
#else # try to be "smart" and see if we have POSIX installed on 6.2
# see if sched.h for POSIX can be found (needs patch 1717 and patchset 43)
# WARN: this is searching a list of standard dirs defined by 
#	/usr/include/make/commondefs and pfmakedefs.linux.  
#	If this Makefile is edited, this search may no longer work.
#	If this search fails, this lib will be compiled without POSIX
#	process control and will use the old 6.2 style control API.
INCS = $(CINCS) $(CXXINCS)
TMP0=$(INCS:-nostdinc=)
TMP1=`echo $(TMP0) | sed 's/ -I/\/sched.h -ZZZ/g' `/sched.h -ZZZ ../sched.h
TESTEXPR=$(TMP1:S/-ZZZ/-o -r /g:S/-I/-r /g)
PFHAVEPOSIX != (test $(TESTEXPR))  && echo 1 || echo 0
#endif

#endif

#if $(PFHAVEPOSIX) == 1
LCDEFS += -DHAVE_POSIX
LCXXDEFS += -DHAVE_POSIX
#endif

CFILES = \
        asdnode.c               \
        barrier.c               \
        boxlod.c                \
        clipcenter.c            \
	cliptexture.c		\
        closest.c               \
        collide.c               \
        cursor.c                \
        eventq.c                \
        flybox.c                \
        gui.c                   \
        hash.c                  \
        input.c                 \
        lockcpu.c               \
        lpoint.c                \
        mco.c                   \
        mipmin.c                \
	mpstats.c		\
        path.c                  \
        pup.c                   \
        random.c                \
        reallyinvalidate.c      \
        rvous.c                 \
        shmem.c                 \
        smoke.c                 \
        snapwin.c               \
        style.c                 \
        tex.c                   \
        timer.c                 \
        trav.c                  \
        xfont.c                 \
        xwin.c

CXXFILES = \
	clipparams.C 		\
	gridify.C		\
        icacheviewer.C          \
        pfuAutoList.C     	\
        pfuClipCenterNode.C     \
	pfuPalettizer.C		\
        pfuProcessManager.C     \
	pfuTextureManager.C 	\
	pfuTextureMemory.C  	\
        spherefront.C           \
        spherefront2.C 		\
	texderivs.C


HFILES = \
        pfuAutoList.h     	\
        collide.h               \
        cursor.h                \
        eventq.h                \
        gui.h                   \
        input.h                 \
	mpstats.h		\
        pfuClipCenterNode.h     \
	pfuPalettizer.h         \
        pfuProcessManager.h     \
	pfuTextureManager.h     \
        pup.h                   \
        rvous.h                 \
        smoke.h                 \
        xwin.h
        


include $(PFROOT)/usr/share/Performer/src/pfmakerules.linux
