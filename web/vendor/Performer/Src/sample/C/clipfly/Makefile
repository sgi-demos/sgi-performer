#
# nmake makefile for clipfly
#
# XXX Needs a little tweaking so that it does not rebuild
# objects in ../common directory every time ...
#
# Also there are a whole bunch of libs which are specified
# on the link line. In reality very few of these are req'd.
# (Basically what's specified in the PFLIBS variable below.)
#

!IF "$(CFG)" == ""
CFG=Release
!MESSAGE No configuration specified. Defaulting to Release.
!ENDIF 

!IF "$(CFG)" != "Release" && "$(CFG)" != "Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE CFG="Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(PFROOT)" == ""
!ERROR PFROOT environment variable not set. Can't build without this ...
!ENDIF

PFINCLUDES = -I"$(PFROOT)"/Include -I"$(PFROOT)"/Include/Performer -I. -I../common

!IF  "$(CFG)" == "Release"
PFLIB_FLAGS = /LIBPATH:"$(PFROOT)"/Lib msvcrt.lib
!ELSE
PFLIB_FLAGS = /LIBPATH:"$(PFROOT)"/Lib/Debug msvcrt.lib
!ENDIF

PFLIBS = $(PFLIB_FLAGS) /INCREMENTAL:NO libpfuiD.lib libpfui.lib libpfdu-util.lib libpf.lib glu32.lib opengl32.lib gdi32.lib user32.lib

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Release"

OUTDIR=.\OPT.I386.WIN32
INTDIR=.\OPT.I386.WIN32
# Begin Custom Macros
OutDir=.\OPT.I386.WIN32
# End Custom Macros

ALL : "$(OUTDIR)\clipfly.exe"


CLEAN :
	-@erase "$(INTDIR)\cmdline.obj"
	-@erase "$(INTDIR)\configfile.obj"
	-@erase "$(INTDIR)\gui.obj"
	-@erase "$(INTDIR)\keybd.obj"
	-@erase "$(INTDIR)\perfly.obj"
	-@erase "$(INTDIR)\generic.obj"
	-@erase "$(INTDIR)\env.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\clipfly.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\clipfly.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c $(PFINCLUDES) /D "_DLL"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\clipfly.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\clipfly.pdb" /machine:I386 /out:"$(OUTDIR)\clipfly.exe" $(PFLIBS)
LINK32_OBJS= \
	"$(INTDIR)\perfly.obj" \
	"$(INTDIR)\cmdline.obj" \
	"$(INTDIR)\configfile.obj" \
	"$(INTDIR)\gui.obj" \
	"$(INTDIR)\keybd.obj" \
	"$(INTDIR)\generic.obj" \
	"$(INTDIR)\env.obj" \
	"$(INTDIR)\main.obj"

"$(OUTDIR)\clipfly.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Debug"

OUTDIR=.\DBG.I386.WIN32
INTDIR=.\DBG.I386.WIN32
# Begin Custom Macros
OutDir=.\DBG.I386.WIN32
# End Custom Macros

ALL : "$(OUTDIR)\clipfly.exe"


CLEAN :
	-@erase "$(INTDIR)\cmdline.obj"
	-@erase "$(INTDIR)\configfile.obj"
	-@erase "$(INTDIR)\gui.obj"
	-@erase "$(INTDIR)\keybd.obj"
	-@erase "$(INTDIR)\perfly.obj"
	-@erase "$(INTDIR)\generic.obj"
	-@erase "$(INTDIR)\env.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\clipfly.exe"
	-@erase "$(OUTDIR)\clipfly.ilk"
	-@erase "$(OUTDIR)\clipfly.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"


CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\clipfly.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ  /c $(PFINCLUDES) /D "_DLL"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\clipfly.bsc" 
BSC32_SBRS= \
	

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\clipfly.pdb" /debug /machine:I386 /out:"$(OUTDIR)\clipfly.exe" /pdbtype:sept $(PFLIBS)
LINK32_OBJS= \
	"$(INTDIR)\perfly.obj" \
	"$(INTDIR)\cmdline.obj" \
	"$(INTDIR)\configfile.obj" \
	"$(INTDIR)\gui.obj" \
	"$(INTDIR)\keybd.obj" \
	"$(INTDIR)\generic.obj" \
	"$(INTDIR)\env.obj" \
	"$(INTDIR)\main.obj"

"$(OUTDIR)\clipfly.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("clipfly.dep")
!INCLUDE "clipfly.dep"
!ELSE 
!MESSAGE Warning: cannot find "clipfly.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Release" || "$(CFG)" == "Debug"

"$(INTDIR)\perfly.obj" : perfly.c "$(INTDIR)"

"$(INTDIR)\cmdline.obj" : cmdline.c "$(INTDIR)"

"$(INTDIR)\configfile.obj" : configfile.c "$(INTDIR)"

"$(INTDIR)\gui.obj" : gui.c "$(INTDIR)"

"$(INTDIR)\keybd.obj" : keybd.c "$(INTDIR)"

"$(INTDIR)\generic.obj" : ../common/generic.c "$(INTDIR)"
	   $(CPP) $(CPP_PROJ) ../common/generic.c

"$(INTDIR)\env.obj" : ../common/env.c "$(INTDIR)"
	$(CPP) $(CPP_PROJ) ../common/env.c

"$(INTDIR)\main.obj" : ../common/main.c "$(INTDIR)"
	   $(CPP) $(CPP_PROJ) ../common/main.c


!ENDIF 

