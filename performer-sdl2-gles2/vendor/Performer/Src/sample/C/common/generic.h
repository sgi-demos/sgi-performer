/*
 * Copyright 1993, 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * This source code ("Source Code") was originally derived from a
 * code base owned by Silicon Graphics, Inc. ("SGI")
 * 
 * LICENSE: SGI grants the user ("Licensee") permission to reproduce,
 * distribute, and create derivative works from this Source Code,
 * provided that: (1) the user reproduces this entire notice within
 * both source and binary format redistributions and any accompanying
 * materials such as documentation in printed or electronic format;
 * (2) the Source Code is not to be used, or ported or modified for
 * use, except in conjunction with OpenGL Performer; and (3) the
 * names of Silicon Graphics, Inc.  and SGI may not be used in any
 * advertising or publicity relating to the Source Code without the
 * prior written permission of SGI.  No further license or permission
 * may be inferred or deemed or construed to exist with regard to the
 * Source Code or the code base of which it forms a part. All rights
 * not expressly granted are reserved.
 * 
 * This Source Code is provided to Licensee AS IS, without any
 * warranty of any kind, either express, implied, or statutory,
 * including, but not limited to, any warranty that the Source Code
 * will conform to specifications, any implied warranties of
 * merchantability, fitness for a particular purpose, and freedom
 * from infringement, and any warranty that the documentation will
 * conform to the program, or any warranty that the Source Code will
 * be error free.
 * 
 * IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
 * LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
 * ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
 * SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
 * OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
 * PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
 * OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
 * USE, THE SOURCE CODE.
 * 
 * Contact information:  Silicon Graphics, Inc., 
 * 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
 * or:  http://www.sgi.com
 */

/*
 *	  generic.h
 */

#ifndef __GENERIC_H__
#define __GENERIC_H__

#include "main.h"

#define MAX_PIPES	32
#define MAX_CHANS	64
#define HYPERPIPE	2
#define COMPOSITOR	3
#define MAX_HYPERPIPES	16

/* Command Line optional settings used at initialization */
extern int	NotifyLevel;	    
extern int	ProcSplit;
extern int	PrioritizeProcs;
extern int	GangDraw;
extern int	InitHPR;
extern int	InitXYZ;
extern char* 	WriteFileDbg;
extern int	WinSizeX, WinSizeY;
extern int	NumFiles;
extern int	NumPipes;			/* total pfPipes */
extern int	NumChans;

extern int 	MultiPipeInput;
extern int	Multipipe;
extern int	ChanOrder[MAX_CHANS];
extern int	ChanPipes[MAX_CHANS];
extern int	InitFOV;
extern int	Multisamples;
extern int	ZBits;
extern int      ChooseShaderVisual;

/* These are for hyperpipe configuration */
extern int	NumHyperpipes;			/* number of HyperpipeNets */
extern int	NumHyperpipeSingles;		/* number of HyperpipeSingle */
extern int	TotHyperpipePipes;		/* total pfPipes for hyperpipes */
extern int	NumHyperpipePipes[MAX_HYPERPIPES]; /* pfPipes per hyperpipe */
extern int	HyperpipeNetIds[MAX_HYPERPIPES];/* network ids for hyperpipes */
extern char*	HyperpipeSingles[MAX_PIPES];	/* names for single pipes */

/* These are visible for the Application process only */
extern char	**DatabaseFiles;

/* Function Prototypes */
extern int 	SimDone(void);

extern void InitConfig(void);
extern void InitSharedMem(int, char **);
extern void InitScene(void);
extern void InitPipe(void);
extern void InitGUI(void);
extern void InitChannel(void);
extern void ConfigCull(int, unsigned int);
extern void ConfigLPoint(int, unsigned int);
extern void OpenPipeline(int, unsigned int);
extern void OpenXWin(pfPipeWindow *);
extern void OpenWin(pfPipeWindow *);
extern void InitGfx(pfPipeWindow *);
extern void ConfigPWin(pfPipeWindow *);

extern void PreFrame(void);
extern void PostFrame(void);

extern void PreDraw(pfChannel *, void *);
extern void PostDraw(pfChannel *, void *);

extern void PreCull(pfChannel *, void *);
extern void PostCull(pfChannel *, void *);

extern void PreLpoint(pfChannel *, void *);
extern void PostLpoint(pfChannel *, void *);

extern void PreApp(pfChannel *, void *);
extern void PostApp(pfChannel *, void *);
#endif
