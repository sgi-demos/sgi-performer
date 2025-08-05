#!smake
#
# Makefile for C++ rotorwash
#
# $Revision: 1.1 $
# $Date: 2000/11/21 21:39:45 $
#

PFDEPTH = ../../../..

# override default SRCDIR in pfcommondefs
SRCDIR = $(PFDEPTH)/sample/apps/C++/rotorwash

# for regular make
VPATH = $(SRCDIR)

TARGETCMDS = framework

CXXFLAGS = -O -I$(PFDEPTH)/include -I$(PFROOT)/usr/include

# look in SRCDIR first for C files. If file is not found, look
# in the common code directory
.PATH.C : $(SRCDIR)

INSTC++FILES = framework.C sea.C

C++FILES = framework.C sea.C

HFILES = sea.h

DEFCMDS=$(TARGETCMDS)
DEFOBJS=$(OBJECTS)

OBJECTS = $(C++FILES:.C=.o)

default: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGETCMDS) -L$(PFROOT)/usr/lib -lpf -lpfdu -lpfui -lpfutil -lGL -lGLU -L/usr/X11R6/lib -lX11 -ldl -lm 
