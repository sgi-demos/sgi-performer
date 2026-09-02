/*
 * Copyright 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
 *
 * file: prstats.h
 * ---------------
 * Include file for Performer rendering statistics.
 *
 * $Revision: 1.44 $
 * $Date: 2002/10/28 23:22:57 $
 *
 */

#ifndef __PRSTATS_H__
#define __PRSTATS_H__


/* Default enables C++ API for C++ compiles */
#ifndef PF_CPLUSPLUS_API
#ifdef __cplusplus
#define PF_CPLUSPLUS_API 1 /* enable C++ API, types */
#else
#define PF_CPLUSPLUS_API 0 /* do not enable C++ API, types */
#endif
#endif

/* Default only enables C API if not using C++ API */
#ifndef PF_C_API
#define PF_C_API !PF_CPLUSPLUS_API 
#endif

#if PF_CPLUSPLUS_API

#ifndef PFINTERNAL
#define PFINTERNAL 	protected
#endif

#else


#ifndef __PR_H__
typedef struct _pfStats     pfStats;
typedef struct _pfType      pfType;
typedef struct _pfGeoSet    pfGeoSet;
#endif

#endif /* PF_CPLUSPLUS_API */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ---------------------- C API --------------------------------
 */

#if PF_C_API


#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
#ifndef __PFSTATS_STRUCTS__
#define __PFSTATS_STRUCTS__

#define USE_BTLNCK_STATS 0 /* */


/* ----------------------- pfStats Tokens ----------------------- */


#define PFSTATS_MAX_CPUS		64
#define PFSTATS_OFF			0
#define PFSTATS_ON			1
#define PFSTATS_DEFAULT			2
#define PFSTATS_SET			3
#define PFSTATS_ALL			(~((unsigned int)0))

/* pfStatsClassMode() - class IDs */
#define PFSTATS_CLASSES				0
#define PFSTATS_GFX				1
#define PFSTATSHW_GFXPIPE_FILL			2
#define PFSTATSHW_GFXPIPE_TIMES			3
#define PFSTATSHW_CPU				4
#define PFSTATS_TEXLOAD				5
#define PFSTATS_CALLIG				6



/* pfStatsHWAttr */
#define PFSTATSHW_FILL_DCBITS			1

/* pfStatsClassMode() - mode bitmasks */

#define PFSTATS_GFX_GEOM			0x0001
#define PFSTATS_GFX_TSTRIP_LENGTHS		0x0002
#define PFSTATS_GFX_ATTR_COUNTS			0x0004
#define PFSTATS_GFX_STATE			0x0010
#define PFSTATS_GFX_XFORM                       0x0020
#define PFSTATS_GFX_CULL_SIDEKICK               0x0040

#define PFSTATSHW_GFXPIPE_TIMES_ON		0x0001
#define PFSTATSHW_GFXPIPE_TIMES_TOTAL		0x0001
#define PFSTATSHW_GFXPIPE_TIMES_STAGE		0x0002
#define PFSTATSHW_GFXPIPE_TIMES_AUTO_COLLECT	0x0004
#define PFSTATSHW_GFXPIPE_TIMES_MASK		(PFSTATSHW_GFXPIPE_TIMES_TOTAL | PFSTATSHW_GFXPIPE_TIMES_STAGE | \
							PFSTATSHW_GFXPIPE_TIMES_AUTO_COLLECT)
							
#define PFSTATSHW_GFXPIPE_FILL_DCPAINT		0x0001
#define PFSTATSHW_GFXPIPE_FILL_DCCOUNT		0x0002
#define PFSTATSHW_GFXPIPE_FILL_DEPTHCMP		0x0004
#define PFSTATSHW_GFXPIPE_FILL_TRANSPARENT	0x0008

#define PFSTATSHW_CPU_SYS			0x1000 /* system stats */
#define PFSTATSHW_CPU_IND			0x2000 /* stats for individual processors */

#define PFSTATS_TEXLOAD_BYSIZE			0x0001

/* pfStatsEnable() - class enable bitmasks */

#define PFSTATS_ENGFX				0x0001
/* visualize depth complexity but don't calculate */
#define PFSTATSHW_ENGFXPIPE_FILL		0x0002
#define PFSTATSHW_ENGFXPIPE_TIMES		0x0004
#define PFSTATSHW_ENGFXPIPE			(PFSTATSHW_ENGFXPIPE_FILL | PFSTATSHW_ENGFXPIPE_TIMES)
#define PFSTATSHW_ENCPU				0x0010
#define PFSTATS_ENTEXLOAD			0x0020
#define PFSTATS_ENCALLIG			0x0040


#define PFSTATS_EN_BITS				16
#define PFSTATS_EN_MASK				0xffff


/* constants for individual classes */
#define PFSTATSHW_NUM_TIMESTAMPS		8   
#define PFSTATSHW_MAX_TIMESTAMPS		32

/*
 * Below are query structures and matching tokens 
 * for getting their value via pfQueryStats
 */

/* 
 * pfStatsValIsect is used by libpf for cull hit tests. 
 * Can also be used in a user's isect traversal 
 */   
typedef struct pfStatsValIsect {
    float total, test, hit, miss, accept, reject, cont;
} pfStatsValIsect;


/* 
 * Query structures for the PFSTATS_GFX class
 */
typedef struct pfStatsValAttrs	/* PFSTATSVAL_GFX_GEOM_ATTRS */
{
    float bytes;		/* PFSTATSVAL_GFX_GEOM_ATTR_BYTES */
    float verts;		/* PFSTATSVAL_GFX_GEOM_ATTR_VERTS */
    float colors;		/* PFSTATSVAL_GFX_GEOM_ATTR_COLORS */
    float normals;		/* PFSTATSVAL_GFX_GEOM_ATTR_NORMALS */
    float texcoords;		/* PFSTATSVAL_GFX_GEOM_ATTR_TEXCOORDS */
} pfStatsValAttrs;

/* 
 * For tStripLengths array 0 loc is total number of tstrips
 * (PFSTATS_TSTRIP_LENGTHS_MAX) loc is num strips 
 * with length >= PFSTATS_TSTRIP_LENGTHS_MAX
 */
#define PFSTATS_TSTRIP_LENGTHS_MAX  14

#define PFSTATSVAL_NUM_ATTR_TYPES   4
#define PFSTATSVAL_NUM_BIND_TYPES   4	
#define PFSTATSVAL_NUM_PFPRIMS	    18					 

typedef struct pfStatsValGeom	/* PFSTATSVAL_GFX_GEOM */
{
    float	    gsets;	/*  PFSTATSVAL_GFX_GEOM_GSETS */
    float	    gsetsFlat;  /*  PFSTATSVAL_GFX_GEOM_GSETS_FLAT */
    float	    gsetsHlight;/*  PFSTATSVAL_GFX_GEOM_GSETS_HLIGHT */
    float	    gsetsTransp;/*  PFSTATSVAL_GFX_GEOM_GSETS_TRANSP */

    /* gsets of each bind type for each attr */	    
    float	    gsetAttrBindCounts[PFSTATSVAL_NUM_ATTR_TYPES]
				      [PFSTATSVAL_NUM_BIND_TYPES];	
				/* PFSTATSVAL_GFX_GEOM_GSETS_PVCOLORS
				 * PFSTATSVAL_GFX_GEOM_GSETS_PVNORMALS
				 * PFSTATSVAL_GFX_GEOM_GSETS_TEXCOORDS
				 */		    
    /* num of each type of Performer primitive */
    float	    pfprims[PFSTATSVAL_NUM_PFPRIMS]; 
				/* PFSTATSVAL_GFX_GEOM_PFPRIMS */

    /* num GL prims (tris, points, lines) for each pf prim types */
    float	    pfglprims[PFSTATSVAL_NUM_PFPRIMS]; 
				/* PFSTATSVAL_GFX_GEOM_PFGLPRIMS */

    /* prim counts condensed into main gl prims */
    float	    tris;	/* PFSTATSVAL_GFX_GEOM_TRIS */
    float	    lines;	/* PFSTATSVAL_GFX_GEOM_LINES */
    float	    points;	/* PFSTATSVAL_GFX_GEOM_POINTS */
    float	    totalPrims; /* PFSTATSVAL_GFX_GEOM_GLPRIMS_TOTAL */

    /* total numbers off attrs */
    float	    primsPerGSet;   /* PFSTATSVAL_GFX_GEOM_PRIMS_PER_GSET */
    float	    primsPerGState; /* PFSTATSVAL_GFX_GEOM_PRIMS_PER_GSTATE */
    float	    gsetsPerGState; /* PFSTATSVAL_GFX_GEOM_GSETS_PER_GSTATE */
    pfStatsValAttrs attrs;	    /* PFSTATSVAL_GFX_GEOM_ATTRS */
    float	    clears;	    /* PFSTATSVAL_GFX_GEOM_CLEARS */
    float	    dispLists;	    /* PFSTATSVAL_GFX_GEOM_DISPLISTS */
    float	    tStripTris;	    /* PFSTATSVAL_GFX_GEOM_TSTRIPSTRIS */
    float	    trisPerTStrip;  /* PFSTATSVAL_GFX_GEOM_TRISPERSTRIP */
    float	    tStripLengths[PFSTATS_TSTRIP_LENGTHS_MAX+1]; 
				    /* PFSTATSVAL_GFX_GEOM_TSTRIPLENGTHS */

    /* Total counts removed by CULL_SIDEKICK processes */
    float	    removed_tris;
    float	    removed_lines;
    float	    removed_points;
    float	    removed_verts;
} pfStatsValGeom;

/* 
 * pfStatsValModes is used for PFSTATSVAL_GFX_MODECHANGES 
 * and PFSTATSVAL_GFX_MODECALLS 
 */
typedef struct pfStatsValModes	/* PFSTATSVAL_GFX_MODE* */
{ 
    float total;		/* PFSTATSVAL_GFX_MODE*_TOTAL */
    float enlighting;		/* PFSTATSVAL_GFX_MODE*_ENLIGHTING */
    float entexture;		/* PFSTATSVAL_GFX_MODE*_ENTEXTURE */
    float enfog;		/* PFSTATSVAL_GFX_MODE*_ENFOG */
    float enctab;		/* PFSTATSVAL_GFX_MODE*_ENCTAB */
    float enwireframe;		/* PFSTATSVAL_GFX_MODE*_ENWIREFRAME */
    float enhlight;		/* PFSTATSVAL_GFX_MODE*_ENHLIGHT */
    float shade;		/* PFSTATSVAL_GFX_MODE*_SHADE */
    float cullface;		/* PFSTATSVAL_GFX_MODE*_CULLFACE */
    float afunction;		/* PFSTATSVAL_GFX_MODE*_AFUNCTION */
    float antialias;		/* PFSTATSVAL_GFX_MODE*_ANTIALIAS */
    float transparent;		/* PFSTATSVAL_GFX_MODE*_TRANSPARENT */
    float decals;		/* PFSTATSVAL_GFX_MODE*_DECALS */
    float decalsStencil;	/* PFSTATSVAL_GFX_MODE*_DECALS_STENCIL */
    float decalsDisplaced;	/* PFSTATSVAL_GFX_MODE*_DECALS_DISPLACE */
    float decalPlanes;		/* PFSTATSVAL_GFX_MODE*_DECALPLANES */
    float textures;		/* PFSTATSVAL_GFX_MODE*_TEXTURES */
    float texturesSharpened;	/* PFSTATSVAL_GFX_MODE*_TEXTURES_SHARPENED*/
    float texturesDetailed;	/* PFSTATSVAL_GFX_MODE*_TEXTURES_DETAILED */
    float detailTextures;       /* PFSTATSVAL_GFX_MODE*_DETAIL_TEXTURES */
    float texturesClipmapped;	/* PFSTATSVAL_GFX_MODE*_TEXTURES_CLIPMAPPED */
    float texEnvs;		/* PFSTATSVAL_GFX_MODE*_TEXENVS */
    float texEnvsBlended;	/* PFSTATSVAL_GFX_MODE*_TEXENVS_BLENDED */
    float texLoads;		/* PFSTATSVAL_GFX_MODE*_TEXLOADS */
    float texBytes;/*RE only */	/* PFSTATSVAL_GFX_MODE*_TEXBYTES */
    float materials;		/* PFSTATSVAL_GFX_MODE*_MTLS */
    float lights;		/* PFSTATSVAL_GFX_MODE*_LIGHTS */
    float lightsInf;		/* PFSTATSVAL_GFX_MODE*_LIGHTS_INF */
    float lightsLocal;		/* PFSTATSVAL_GFX_MODE*_LIGHTS_LOCAL */
    float lmodels;		/* PFSTATSVAL_GFX_MODE*_LMODELS */
    float lmodelsTwoSided;	/* PFSTATSVAL_GFX_MODE*_LMODELS_TWOSIDED */
    float lmodelsLocalViewer;	/* PFSTATSVAL_GFX_MODE*_LMODELS_LOCALVIEWER */
    float fog;			/* PFSTATSVAL_GFX_MODE*_FOG */
    float fogRamps;		/* PFSTATSVAL_GFX_MODE*_FOG_RAMPS */
    float ctabs;		/* PFSTATSVAL_GFX_MODE*_CTABS */
    float hlights;		/* PFSTATSVAL_GFX_MODE*_HLIGHTS */
    float enlpState;		/* PFSTATSVAL_GFX_MODE*_ENLPOINTSTATE */
    float lpStates;		/* PFSTATSVAL_GFX_MODE*_LPOINTSTATES */
    float entexGen;		/* PFSTATSVAL_GFX_MODE*_ENTEXGEN */
    float texGens;		/* PFSTATSVAL_GFX_MODE*_TEXGENS */
    float entexLODs;		/* PFSTATSVAL_GFX_MODE*_ENTEXLOD */
    float texLODs;		/* PFSTATSVAL_GFX_MODE*_TEXLODS */
    float entexMats;		/* PFSTATSVAL_GFX_MODE*_ENTEXMAT */
    float texMats;		/* PFSTATSVAL_GFX_MODE*_TEXMATS */
} pfStatsValModes;

typedef struct pfStatsValState /* PFSTATSVAL_GFX_STATE */
{
    float gstates;			/* PFSTATSVAL_GFX_STATE_GSTATES */
    float pushState;			/* PFSTATSVAL_GFX_STATE_PUSH */
    float popState;			/* PFSTATSVAL_GFX_STATE_POP */
    float flushState;			/* PFSTATSVAL_GFX_STATE_FLUSH */
    float basicState;			/* PFSTATSVAL_GFX_STATE_BASIC */
    float gstatesIndexed;		/* PFSTATSVAL_GFX_STATE_GSTATES_INDEXED */
} pfStatsValState;

typedef struct pfStatsValXforms	/* PFSTATSVAL_GFX_XFORM */
{
    float total;			/* PFSTATSVAL_GFX_XFORM_TOTAL */
    float translates;			/* PFSTATSVAL_GFX_XFORM_TRANSLATES */ 
    float scales;			/* PFSTATSVAL_GFX_XFORM_SCALES */
    float rotates;			/* PFSTATSVAL_GFX_XFORM_ROTATES */
    float pushMatrix;			/* PFSTATSVAL_GFX_XFORM_PUSHMATRIX */
    float popMatrix;			/* PFSTATSVAL_GFX_XFORM_POPMATRIX */
    float loadMatrix;			/* PFSTATSVAL_GFX_XFORM_LOADMATRIX */
    float multMatrix;			/* PFSTATSVAL_GFX_XFORM_MULTMATRIX */
} pfStatsValXforms;

typedef struct pfStatsValCalligraphic	/* PFSTATSVAL_CALLIG */
{
    float pointsProcessed;		/* PFSTATSVAL_CALLIG_POINTS_PROCESSED */
    float pointsSent; 			/* PFSTATSVAL_CALLIG_POINTS_SENT */
    float pointsDrawn;			/* PFSTATSVAL_CALLIG_POINTS_LPBDRAWN */
    float drawTime; 			/* PFSTATSVAL_CALLIG_LPB_DRAWTIME */
    float overrun;			/* PFSTATSVAL_CALLIG_LPB_OVERRUN */
} pfStatsValCalligraphic;

#define PFSTATS_MAX_TEXLOAD_SIZE_2	32
#define PFSTATS_MAX_TEXLOAD_INDEX	65
#define PFSTATS_TEXLOAD_TBLSIZE		66
/* This array is intended to be indexed with the following macro: 
 *  PFSTATS_TEXLOAD_INDEX(i, s) where i will receive the index for size s.
 */
#define PFSTATS_TEXLOAD_INDEX(_i, _x) {	\
    int _floor=-1, __x = _x;		\
    if (_x <= 0) _i = -1;		\
    else { while (__x > 0) { __x>>=1; _floor++; } \
	_i = ( (_x == (1<<_floor)) ? 2*_floor +1: 2*_floor + 2 ); } }

typedef float pfTexLoadSizeArray2D[PFSTATS_TEXLOAD_TBLSIZE][PFSTATS_TEXLOAD_TBLSIZE];

typedef struct pfStatsValTexLoads	/* PFSTATSVAL_TEXLOAD */
{
    float totalCount;			/* PFSTATSVAL_TEXLOAD_TOTAL_COUNT */
    float totalBytes;			/* PFSTATSVAL_TEXLOAD_TOTAL_BYTES */
    float totalTime;			/* PFSTATSVAL_TEXLOAD_TOTAL_MSECS */
    float totalStrided;			/* PFSTATSVAL_TEXLOAD_TOTAL_STRIDED */
    float minLoadSize[3];		/* PFSTATSVAL_TEXLOAD_MIN_SIZE_{TOTAL,X,Y} */
    float maxLoadSize[3];		/* PFSTATSVAL_TEXLOAD_MAX_SIZE_{TOTAL,X,Y}*/
    float rate;	/* bytes/sec */		/* PFSTATSVAL_TEXLOAD_RATE */
    pfTexLoadSizeArray2D loadCounts;	/* PFSTATSVAL_TEXLOAD_COUNTS */
    pfTexLoadSizeArray2D loadBytes;	/* PFSTATSVAL_TEXLOAD_BYTES */
    pfTexLoadSizeArray2D loadTimes;	/* PFSTATSVAL_TEXLOAD_TIMES */
} pfStatsValTexLoads;

typedef struct pfStatsValGfx 
{
    pfStatsValGeom	geom;		/* PFSTATSVAL_GFX_GEOM */
    pfStatsValModes	modeChanges;	/* PFSTATSVAL_GFX_MODECHANGES */
    pfStatsValModes	modeCalls;	/* PFSTATSVAL_GFX_MODECALLS */
    pfStatsValState	state;		/* PFSTATSVAL_GFX_STATE */
    pfStatsValXforms	xform;		/* PFSTATSVAL_GFX_XFORM */
} pfStatsValGfx;

/* 
 * Query structures for the PFSTATSHW_GFXPIPE class
 */
 
typedef struct pfStatsValFill		/* PFSTATSVAL_GFXPIPE_FILL */ 
{
    float pixels;			/* PFSTATSVAL_GFXPIPE_FILL_PIXELS */
    float depthComplexity;		/* PFSTATSVAL_GFXPIPE_FILL_DC */
} pfStatsValFill;

typedef struct pfStatsValGfxPipeline
{
    float host, xform,  fill, total;
} pfStatsValGfxPipeline;

typedef struct pfStatsValGfxPipeTimes
{
    float which;
    pfStatsValGfxPipeline time;
    pfStatsValGfxPipeline percentage;
    
} pfStatsValGfxPipeTimes;

/* 
 * Query structures for the PFSTATSHW_CPU class
 */

typedef struct pfStatsValCPUUsage
{
    float user;			/* PFSTATSVAL_CPU_*_USER */
    float kernel;		/* PFSTATSVAL_CPU_*_KERNEL */
    float intr;			/* PFSTATSVAL_CPU_*_INTR */

    /* busy = user + kernel + inter */
    float busy;			/* PFSTATSVAL_CPU_*_BUSY */

    float idle;			/* PFSTATSVAL_CPU_*_IDLE */
    /* wait = waitcpu + waitGfx + waitSwap */
    float wait;			/* PFSTATSVAL_CPU_*_WAIT */
    /* waitcpu = time waiting on another process */
    float waitCPU;		/* PFSTATSVAL_CPU_*_WAITCPU */
    float waitGfx;		/* PFSTATSVAL_CPU_*_WAITGFX */
    float waitSwap;		/* PFSTATSVAL_CPU_*_WAITSWAP */
} pfStatsValCPUUsage;

/* individual CPU stats */
typedef struct pfStatsValCPUInd /* PFSTATSVAL_CPU_IND */
{
    pfStatsValCPUUsage	usage;	/* PFSTATSVAL_CPU_IND_USAGE */    	    
} pfStatsValCPUInd;

/* type of system spent in various stats */
typedef struct pfStatsValCPUSys /* PFSTATSVAL_CPU_SYS */
{
    pfStatsValCPUUsage	usage;	    /* PFSTATSVAL_CPU_SYS_USAGE */    
    float		pCxtSwitch; /* PFSTATSVAL_CPU_SYS_PCXTSWITCH */
    float		sysCalls;   /* PFSTATSVAL_CPU_SYS_SYSCALLS */
    
} pfStatsValCPUSys;

typedef struct pfStatsValCPUGfx /* PFSTATSVAL_CPU_GFX */
{
    float		fifoNoWait;  /* PFSTATSVAL_CPU_GFX_FIFONOWAIT */
    float		fifoWait;    /* PFSTATSVAL_CPU_GFX_FIFOWAIT */
    float		cxtSwitch;   /* PFSTATSVAL_CPU_GFX_CXTSWITCH */
    float		swapBuffers; /* PFSTATSVAL_CPU_GFX_SWAPBUFFERS */
    float		IOCtls;	     /* PFSTATSVAL_CPU_GFX_IOCTLS */
} pfStatsValCPUGfx;


typedef struct pfStatsValCPU	/* PFSTATSVAL_CPU */
{
    /* elapsed time for cpu stats in seconds */
    float		secs;		/* PFSTATSVAL_CPU_SECS */
    pfStatsValCPUSys	sys;		/* PFSTATSVAL_CPU_SYS */
    pfStatsValCPUInd	ind[PFSTATS_MAX_CPUS]; 
    				        /* PFSTATSVAL_CPU_IND - gets back all CPUs */
    pfStatsValCPUGfx	gfx;		/* PFSTATSVAL_CPU_GFX */
} pfStatsValCPU;

typedef struct pfStatsValues {	/* to get back all stats: PFSTATSVAL_ALL */
    /* the PFSTATS_GFX class:	    PFSTATSVAL_GFX */
    pfStatsValGeom	geom;		/* PFSTATSVAL_GFX_GEOM */
    pfStatsValModes	modeChanges;	/* PFSTATSVAL_GFX_MODECHANGES */
    pfStatsValModes	modeCalls;	/* PFSTATSVAL_GFX_MODECALLS */
    pfStatsValState	state;		/* PFSTATSVAL_GFX_STATE */
    pfStatsValXforms	xform;		/* PFSTATSVAL_GFX_XFORM */
    
    /* the PFSTATSHW_GFXPIPE_FILL class: PFSTATSVAL_GFXPIPE_FILL */
    pfStatsValFill	fill;		/* PFSTATSVAL_GFXPIPE_FILL */

    pfStatsValCalligraphic callig;	/* PFSTATSVAL_CALLIG */

    /* the PFSTATSHW_CPU class:	   PFSTATSVAL_CPU */
    pfStatsValCPU	cpu;		/* PFSTATSVAL_CPU */  

#ifdef USE_BTLNCK_STATS    
    /* the PFSTATSHW_GFXPIPE_BTLNCK class */
    pfStatsValGfxPipeTimes	btlnck;
#endif /* USE_BTLNCK_STATS */
} pfStatsValues;

/* pfQueryStats select tokens */
#define PFSTATSVAL_NULL				    0
#define PFSTATSVAL_ALL				    1
#define PFSTATSVAL_GFXPIPE			    1000
#define PFSTATSVAL_GFXPIPE_FILL			    1010
#define PFSTATSVAL_GFXPIPE_FILL_PIXELS		    1020
#define PFSTATSVAL_GFXPIPE_FILL_DC		    1030
#define PFSTATSVAL_GFX				    2000
#define PFSTATSVAL_GFX_GEOM			    2010
#define PFSTATSVAL_GFX_GEOM_GSETS		    2020
#define PFSTATSVAL_GFX_GEOM_GSETS_FLAT		    2030
#define PFSTATSVAL_GFX_GEOM_GSETS_HLIGHT	    2040
#define PFSTATSVAL_GFX_GEOM_GSETS_TRANSP	    2050
#define PFSTATSVAL_GFX_GEOM_GSETS_PVCOLORS	    2060
#define PFSTATSVAL_GFX_GEOM_GSETS_PVNORMALS	    2070
#define PFSTATSVAL_GFX_GEOM_GSETS_TEXCOORDS	    2080
#define PFSTATSVAL_GFX_GEOM_PFPRIMS		    2200
#define PFSTATSVAL_GFX_GEOM_PFPRIMS_POINTS	    2210
#define PFSTATSVAL_GFX_GEOM_PFPRIMS_LINES	    2220
#define PFSTATSVAL_GFX_GEOM_PFPRIMS_LINESTRIPS	    2230
#define PFSTATSVAL_GFX_GEOM_PFPRIMS_TRIS	    2240
#define PFSTATSVAL_GFX_GEOM_PFPRIMS_QUADS	    2250
#define PFSTATSVAL_GFX_GEOM_PFPRIMS_TRISTRIPS	    2260
#define PFSTATSVAL_GFX_GEOM_PFPRIMS_FLAT_LINESTRIPS 2270
#define PFSTATSVAL_GFX_GEOM_PFPRIMS_FLAT_TRISTRIPS  2280
#define PFSTATSVAL_GFX_GEOM_PFPRIMS_POLYS  	    2290
#define PFSTATSVAL_GFX_GEOM_PFGLPRIMS		    2300
#define PFSTATSVAL_GFX_GEOM_PFGLPRIMS_POINTS	    2310
#define PFSTATSVAL_GFX_GEOM_PFGLPRIMS_LINES	    2320
#define PFSTATSVAL_GFX_GEOM_PFGLPRIMS_LINESTRIPS    2330
#define PFSTATSVAL_GFX_GEOM_PFGLPRIMS_TRIS	    2340
#define PFSTATSVAL_GFX_GEOM_PFGLPRIMS_QUADS	    2350
#define PFSTATSVAL_GFX_GEOM_PFGLPRIMS_TRISTRIPS	    2360
#define PFSTATSVAL_GFX_GEOM_PFGLPRIMS_FLAT_LINESTRIPS 2370
#define PFSTATSVAL_GFX_GEOM_PFGLPRIMS_FLAT_TRISTRIPS 2380
#define PFSTATSVAL_GFX_GEOM_PFGLPRIMS_POLYS	    2390
#define PFSTATSVAL_GFX_GEOM_GLPRIMS_TOTAL	    2400
#define PFSTATSVAL_GFX_GEOM_TRIS		    2420
#define PFSTATSVAL_GFX_GEOM_LINES		    2430
#define PFSTATSVAL_GFX_GEOM_POINTS		    2440
#define PFSTATSVAL_GFX_GEOM_PRIMS_PER_GSET	    2450
#define PFSTATSVAL_GFX_GEOM_PRIMS_PER_GSTATE	    2460
#define PFSTATSVAL_GFX_GEOM_GSETS_PER_GSTATE	    2470
#define PFSTATSVAL_GFX_GEOM_ATTRS		    2480
#define PFSTATSVAL_GFX_GEOM_ATTR_BYTES		    2481
#define PFSTATSVAL_GFX_GEOM_ATTR_VERTS		    2482
#define PFSTATSVAL_GFX_GEOM_ATTR_COLORS		    2483
#define PFSTATSVAL_GFX_GEOM_ATTR_NORMALS	    2484
#define PFSTATSVAL_GFX_GEOM_ATTR_TEXCOORDS	    2485
#define PFSTATSVAL_GFX_GEOM_CLEARS		    2486
#define PFSTATSVAL_GFX_GEOM_DISPLISTS		    2487
#define PFSTATSVAL_GFX_GEOM_TSTRIPSTRIS		    2488
#define PFSTATSVAL_GFX_GEOM_TRISPERSTRIP	    2489
#define PFSTATSVAL_GFX_GEOM_TSTRIPLENGTHS	    2490
#define PFSTATSVAL_GFX_STATE			    2600 
#define PFSTATSVAL_GFX_STATE_GSTATES		    2610 
#define PFSTATSVAL_GFX_STATE_PUSH		    2620 
#define PFSTATSVAL_GFX_STATE_POP		    2630 
#define PFSTATSVAL_GFX_STATE_FLUSH		    2640
#define PFSTATSVAL_GFX_STATE_BASIC		    2650 
#define PFSTATSVAL_GFX_STATE_GSTATES_INDEXED	    2651 
#define PFSTATSVAL_GFX_MODECHANGES		    2800 
#define PFSTATSVAL_GFX_MODECHANGES_TOTAL	    2810
#define PFSTATSVAL_GFX_MODECHANGES_SHADE	    2820 
#define PFSTATSVAL_GFX_MODECHANGES_CULLFACE	    2830
#define PFSTATSVAL_GFX_MODECHANGES_AFUNCTION	    2840
#define PFSTATSVAL_GFX_MODECHANGES_ANTIALIAS	    2850
#define PFSTATSVAL_GFX_MODECHANGES_TRANSPARENT	    2860
#define PFSTATSVAL_GFX_MODECHANGES_DECALS	    2870
#define PFSTATSVAL_GFX_MODECHANGES_DECALS_STENCIL   2873
#define PFSTATSVAL_GFX_MODECHANGES_DECALS_DISPLACE  2875
#define PFSTATSVAL_GFX_MODECHANGES_DECALPLANES      2877
#define PFSTATSVAL_GFX_MODECHANGES_ENLIGHTING	    2880
#define PFSTATSVAL_GFX_MODECHANGES_ENTEXTURE	    2890
#define PFSTATSVAL_GFX_MODECHANGES_ENFOG	    2900
#define PFSTATSVAL_GFX_MODECHANGES_ENCTAB	    2910
#define PFSTATSVAL_GFX_MODECHANGES_ENWIREFRAME	    2920
#define PFSTATSVAL_GFX_MODECHANGES_ENHLIGHT	    2930
#define PFSTATSVAL_GFX_MODECHANGES_TEXTURES	    2940
#define PFSTATSVAL_GFX_MODECHANGES_TEXTURES_DETAILED 2941
#define PFSTATSVAL_GFX_MODECHANGES_TEXTURES_SHARPENED 2942
#define PFSTATSVAL_GFX_MODECHANGES_DETAIL_TEXTURES  2950
#define PFSTATSVAL_GFX_MODECHANGES_TEXTURES_CLIPMAPPED 2955
#define PFSTATSVAL_GFX_MODECHANGES_TEXENVS	    2960
#define PFSTATSVAL_GFX_MODECHANGES_TEXENVS_BLENDED  2961
#define PFSTATSVAL_GFX_MODECHANGES_TEXLOADS	    2970
#define PFSTATSVAL_GFX_MODECHANGES_TEXBYTES	    2980
#define PFSTATSVAL_GFX_MODECHANGES_MTLS		    2990 
#define PFSTATSVAL_GFX_MODECHANGES_LIGHTS	    3000
#define PFSTATSVAL_GFX_MODECHANGES_LIGHTS_INF	    3001
#define PFSTATSVAL_GFX_MODECHANGES_LIGHTS_LOCAL	    3002
#define PFSTATSVAL_GFX_MODECHANGES_LMODELS	    3010
#define PFSTATSVAL_GFX_MODECHANGES_LMODELS_TWOSIDED 3011
#define PFSTATSVAL_GFX_MODECHANGES_LMODELS_LOCALVIEWER 3012
#define PFSTATSVAL_GFX_MODECHANGES_FOG		    3020
#define PFSTATSVAL_GFX_MODECHANGES_CTABS	    3030
#define PFSTATSVAL_GFX_MODECHANGES_HLIGHTS	    3040
#define PFSTATSVAL_GFX_MODECHANGES_ENLPOINTSTATE    3050
#define PFSTATSVAL_GFX_MODECHANGES_LPOINTSTATES	    3060
#define PFSTATSVAL_GFX_MODECHANGES_ENTEXGEN	    3070
#define PFSTATSVAL_GFX_MODECHANGES_TEXGENS	    3080
#define PFSTATSVAL_GFX_MODECHANGES_ENTEXLOD         3082
#define PFSTATSVAL_GFX_MODECHANGES_TEXLODS          3084
#define PFSTATSVAL_GFX_MODECHANGES_ENTEXMAT         3086
#define PFSTATSVAL_GFX_MODECHANGES_TEXMATS          3088      
#define PFSTATSVAL_GFX_MODECALLS		    3200
#define PFSTATSVAL_GFX_MODECALLS_TOTAL		    3210 
#define PFSTATSVAL_GFX_MODECALLS_SHADE		    3220 
#define PFSTATSVAL_GFX_MODECALLS_CULLFACE	    3230
#define PFSTATSVAL_GFX_MODECALLS_AFUNCTION	    3240
#define PFSTATSVAL_GFX_MODECALLS_ANTIALIAS	    3250
#define PFSTATSVAL_GFX_MODECALLS_TRANSPARENT	    3260
#define PFSTATSVAL_GFX_MODECALLS_DECALS		    3270
#define PFSTATSVAL_GFX_MODECALLS_DECALS_STENCIL     3273
#define PFSTATSVAL_GFX_MODECALLS_DECALS_DISPLACE    3275
#define PFSTATSVAL_GFX_MODECALLS_DECALPLANES        3277
#define PFSTATSVAL_GFX_MODECALLS_ENLIGHTING	    3280
#define PFSTATSVAL_GFX_MODECALLS_ENTEXTURE	    3290
#define PFSTATSVAL_GFX_MODECALLS_ENFOG		    3300
#define PFSTATSVAL_GFX_MODECALLS_ENCTAB		    3310
#define PFSTATSVAL_GFX_MODECALLS_ENWIREFRAME	    3320
#define PFSTATSVAL_GFX_MODECALLS_ENHLIGHT	    3330
#define PFSTATSVAL_GFX_MODECALLS_TEXTURES	    3400
#define PFSTATSVAL_GFX_MODECALLS_TEXTURES_DETAILED  3401
#define PFSTATSVAL_GFX_MODECALLS_TEXTURES_SHARPENED 3402
#define PFSTATSVAL_GFX_MODECALLS_DETAIL_TEXTURES    3410
#define PFSTATSVAL_GFX_MODECALLS_TEXTURES_CLIPMAPPED 3415
#define PFSTATSVAL_GFX_MODECALLS_TEXENVS	    3420
#define PFSTATSVAL_GFX_MODECALLS_TEXENVS_BLENDED    3421
#define PFSTATSVAL_GFX_MODECALLS_TEXLOADS	    3430
#define PFSTATSVAL_GFX_MODECALLS_TEXBYTES           3435
#define PFSTATSVAL_GFX_MODECALLS_MTLS		    3440
#define PFSTATSVAL_GFX_MODECALLS_LIGHTS		    3450
#define PFSTATSVAL_GFX_MODECALLS_LIGHTS_INF	    3451
#define PFSTATSVAL_GFX_MODECALLS_LIGHTS_LOCAL	    3452
#define PFSTATSVAL_GFX_MODECALLS_LMODELS	    3460
#define PFSTATSVAL_GFX_MODECALLS_LMODELS_TWOSIDED   3461
#define PFSTATSVAL_GFX_MODECALLS_LMODELS_LOCALVIEWER 3462
#define PFSTATSVAL_GFX_MODECALLS_FOG		    3470
#define PFSTATSVAL_GFX_MODECALLS_CTABS		    3480
#define PFSTATSVAL_GFX_MODECALLS_HLIGHTS	    3490
#define PFSTATSVAL_GFX_MODECALLS_ENLPOINTSTATE      3500
#define PFSTATSVAL_GFX_MODECALLS_LPOINTSTATES	    3510
#define PFSTATSVAL_GFX_MODECALLS_ENTEXGEN	    3520
#define PFSTATSVAL_GFX_MODECALLS_TEXGENS	    3540
#define PFSTATSVAL_GFX_MODECALLS_ENTEXLOD           3542
#define PFSTATSVAL_GFX_MODECALLS_TEXLODS            3544
#define PFSTATSVAL_GFX_MODECALLS_ENTEXMAT           3546
#define PFSTATSVAL_GFX_MODECALLS_TEXMATS            3548      
#define PFSTATSVAL_GFX_XFORM			    3600 
#define PFSTATSVAL_GFX_XFORM_TOTAL		    3610 
#define PFSTATSVAL_GFX_XFORM_TRANSLATES		    3620 
#define PFSTATSVAL_GFX_XFORM_SCALES		    3630
#define PFSTATSVAL_GFX_XFORM_ROTATES		    3640 
#define PFSTATSVAL_GFX_XFORM_PUSHMATRIX		    3650 
#define PFSTATSVAL_GFX_XFORM_POPMATRIX		    3660
#define PFSTATSVAL_GFX_XFORM_LOADMATRIX		    3670
#define PFSTATSVAL_GFX_XFORM_MULTMATRIX		    3680
/* calligraphic stats */
#define PFSTATSVAL_CALLIG 			    3800
#define PFSTATSVAL_CALLIG_POINTS_PROCESSED	    3810
#define PFSTATSVAL_CALLIG_POINTS_SENT		    3820
#define PFSTATSVAL_CALLIG_POINTS_LPBDRAWN	    3830
#define PFSTATSVAL_CALLIG_LPB_DRAWTIME	    	    3840
#define PFSTATSVAL_CALLIG_LPB_OVERRUN	    	    3850
/* texture loads */
#define PFSTATSVAL_TEXLOAD                          3900
#define PFSTATSVAL_TEXLOAD_TOTAL_COUNT              3910
#define PFSTATSVAL_TEXLOAD_TOTAL_BYTES              3920
#define PFSTATSVAL_TEXLOAD_TOTAL_TIME               3930
#define PFSTATSVAL_TEXLOAD_TOTAL_STRIDED            3940
#define PFSTATSVAL_TEXLOAD_MIN_SIZE_TOTAL           3950
#define PFSTATSVAL_TEXLOAD_MIN_SIZE_X               3951
#define PFSTATSVAL_TEXLOAD_MIN_SIZE_Y               3952
#define PFSTATSVAL_TEXLOAD_COUNTS                   3960
#define PFSTATSVAL_TEXLOAD_BYTES                    3970
#define PFSTATSVAL_TEXLOAD_TIMES                    3980

#define PFSTATSVAL_CPU				    6000
#define PFSTATSVAL_CPU_SECS			    6010
#define PFSTATSVAL_CPU_SYS			    6100
#define PFSTATSVAL_CPU_SYS_PCXTSWITCH		    6110
#define PFSTATSVAL_CPU_SYS_SYSCALLS		    6120
    /* the are all system CPU usage stats */
#define PFSTATSVAL_CPU_SYS_USAGE		    6200
#define PFSTATSVAL_CPU_SYS_BUSY			    6210
#define PFSTATSVAL_CPU_SYS_USER			    6211
#define PFSTATSVAL_CPU_SYS_KERNEL		    6212
#define PFSTATSVAL_CPU_SYS_INTR			    6213
#define PFSTATSVAL_CPU_SYS_IDLE			    6220
#define PFSTATSVAL_CPU_SYS_WAIT			    6230
#define PFSTATSVAL_CPU_SYS_WAITCPU		    6231
#define PFSTATSVAL_CPU_SYS_WAITGFX		    6232
#define PFSTATSVAL_CPU_SYS_WAITSWAP		    6233
    /* GFX system stats */
#define PFSTATSVAL_CPU_GFX			    6300
#define PFSTATSVAL_CPU_GFX_FIFONOWAIT		    6310
#define PFSTATSVAL_CPU_GFX_FIFOWAIT		    6320
#define PFSTATSVAL_CPU_GFX_CXTSWITCH		    6330
#define PFSTATSVAL_CPU_GFX_SWAPBUFFERS		    6340
#define PFSTATSVAL_CPU_GFX_IOCTLS		    6350
#define PFSTATSVAL_CPU_IND			    6700
    /* the are all system CPU usage stats */
#define PFSTATSVAL_CPU_IND_USAGE		    6800
#define PFSTATSVAL_CPU_IND_BUSY			    6810
#define PFSTATSVAL_CPU_IND_USER			    6811
#define PFSTATSVAL_CPU_IND_KERNEL		    6812
#define PFSTATSVAL_CPU_IND_INTR			    6813
#define PFSTATSVAL_CPU_IND_IDLE			    6820
#define PFSTATSVAL_CPU_IND_WAIT			    6830
#define PFSTATSVAL_CPU_IND_WAITCPU		    6831
#define PFSTATSVAL_CPU_IND_WAITGFX		    6832
#define PFSTATSVAL_CPU_IND_WAITSWAP		    6833
/* the sysinfo queries get back the sysinfo structures defined
 * in /usr/include/sys/sysinfo.h gotten at the open and close of CPU stats.
 * The destination buffer for these queries must be of PFSTATSVAL_CPU_SYSINFO_SIZE
 * which is machine dependent.
 */
#define PFSTATSVAL_CPU_SYSINFO_SIZE		    7101 /* size of sysinfo buffer */
#define PFSTATSVAL_CPU_SYS_SYSINFO_START	    7102
#define PFSTATSVAL_CPU_SYS_SYSINFO_END		    7103
/*
 * The IND sysinfo queries get back sysinfo structures for
 * all of CPUs on the machine.  The structures are each of size
 * PFSTATSVAL_CPU_SYSINFO_SIZE and are concatonated together in the
 * destination bufffer.
 */
#define PFSTATSVAL_CPU_IND_SYSINFO_START	    7104
#define PFSTATSVAL_CPU_IND_SYSINFO_END		    7105

/* btlnck query tokens */
#define PFSTATSVAL_GFXPIPE_TIMES			8000
#define PFSTATSVAL_GFXPIPE_TIMES_STAGE			8001
#define PFSTATSVAL_GFXPIPE_TIMES_PERCENTAGE    		8010
#define PFSTATSVAL_GFXPIPE_TIMES_PERCENTAGE_HOST    	8011
#define PFSTATSVAL_GFXPIPE_TIMES_PERCENTAGE_XFORM   	8012
#define PFSTATSVAL_GFXPIPE_TIMES_PERCENTAGE_FILL    	8013
#define PFSTATSVAL_GFXPIPE_TIMES_SECS    		8020
#define PFSTATSVAL_GFXPIPE_TIMES_SECS_HOST    		8021
#define PFSTATSVAL_GFXPIPE_TIMES_SECS_XFORM   		8022
#define PFSTATSVAL_GFXPIPE_TIMES_SECS_FILL    		8023


#define PFSTATSVAL_NUM_TOKENS			    1000000

extern void pfGfxPipeTimestamp(uint flag);
/* this must be the first timestamp in a set */
#define PFSTATSHW_TIMESTAMP_BIT		0x10000000
#define PFSTATSHW_TIMESTAMP_MASK	0xffff0000
#define PFSTATSHW_TIMESTAMP_CLOSE	0xffff0000
#define PFSTATSHW_TIMESTAMP_FINISH	0xfffe0000 /* end of frame = does raster flush */
#define PFSTATSHW_TIMESTAMP_FRAME_END	0xfffd0000 /* end of frame */
#define PFSTATSHW_TIMESTAMP_END		0xfffc0000 /* end of drawing */
#define PFSTATSHW_TIMESTAMP_GEOM_END	0x10100000 /* start of drawing geometry */
#define PFSTATSHW_TIMESTAMP_GEOM_START	0x10020000 /* start of drawing geometry */
#define PFSTATSHW_TIMESTAMP_START	0x10010000 /* start of frame */
#define PFSTATSHW_TIMESTAMP_OPEN	0x10000000 /* stats open */


/* ------------------ pfStats Related Functions--------------------- */

extern pfStats* pfGetCurStats(void);

#endif /* !__PFSTATS_STRUCTS__ */
#endif /* !PF_CPLUSPLUS_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  begin backdoor export to libpf */

/*
 * Stats Class and Mode Masks
 */
 
#define PFSTATS_NUM_CLASSES			8

#define PFSTATS_MEM_MASK		0x4000
#define PFSTATS_GFX_MASK		0x00ff
#define PFSTATSHW_GFXPIPE_FILL_DCMASK	0x0003
#define PFSTATSHW_GFXPIPE_FILL_MASK	0x000f
#define PFSTATSHW_GFXPIPE_MASK		(PFSTATSHW_GFXPIPE_FILL_MASK)
#define PFSTATSHW_CPU_MASK		0x3000
#define PFSTATS_TEXLOAD_MASK		0x0001
#define PFSTATS_CALLIG_MASK		0x0001
#define PFSTATSHW_MASK			(PFSTATSHW_GFXPIPE_MASK | PFSTATSHW_CPU_MASK)

#define PFSTATSHW_EN_MASK		(PFSTATSHW_ENGFXPIPE | PFSTATSHW_ENCPU)



/*
 * Redefine of Query Structure types that are used internally
 */

typedef struct pfStatsValMemory
{
    float sharedArena;
} pfStatsValMemory;


typedef struct pfStatsValGfxPipe
{
    float backface, frontface;
    float clipTotal, clipFront, clipBack, clipSides;
} pfStatsValGfxPipe;

typedef struct pfStatsValGfxPipe pfGfxPipeStats;
typedef struct pfStatsValGfxPipeTimes pfGfxPipeTimesStats;
typedef struct pfStatsValGeom pfGeomStats;
typedef struct pfStatsValCPU pfCPUStats;
typedef struct pfStatsValMemory pfMemoryStats;
typedef struct pfStatsValModes pfModeStats;
typedef struct pfStatsValState pfStateStats;
typedef struct pfStatsValXforms pfXformStats;
typedef struct pfStatsValFill pfFillStats;
typedef struct pfStatsValIsect pfIsectCounts;

/* XXX All counts MUST be of type float because of the way the
 * routines in stats.c operate on the structures.
 */

typedef struct _pfCPUSysInfo {
    double		startTime, endTime;
    int		numCPUs;
    ptrdiff_t		sysInfoSize;
    int		inited;
    int		started;
    struct sysinfo	*sysStartBuf, *sysEndBuf;
    struct sysinfo	*cpuStartBuf[PFSTATS_MAX_CPUS];
    struct sysinfo	*cpuEndBuf[PFSTATS_MAX_CPUS];
} pfCPUSysInfo;

#ifdef USE_BTLNCK_STATS

typedef struct _pfGEClipCounts {
    int total, trivialReject, cullFace, clip;
} pfGEClipCounts;

typedef struct _pfGECountsDat {
    pfGEClipCounts tris, quads, tmesh, qstrip;
} pfGECountsDat;


typedef struct pfPipeRecordBEF {
    uint		    id;
    uint		    size;
    uint		    host;
    uint		    draw;
    uint		    load;
    uint		    stamp;
    uint		    clock;
    uint		    pad;
    uint		    marker;
    uint		    tag;
}pfPipeRecordBEF;

#define BEFSIZE (sizeof(pfPipeRecordBEF))
#define BEFCOUNT (BEFSIZE/(sizeof(uint)))

typedef struct pfGfxTStampData {
    int		    name;
    int		    glName;
    pfGfxPipeTimesStats btlnck;
    pfPipeRecordBEF   befStamp;
}pfGfxTStampData;


typedef struct _pfGfxTStampBuf
{
    int total;
    uint lastStamp;
    uint needsFinish;
    pfGfxTStampData stamps[PFSTATSHW_MAX_TIMESTAMPS];
} pfGfxTStampBuf;

#endif

typedef struct pfGfxTimesData {
    float startTime, drawTime, geomTime, fillTime;
#ifdef USE_BTLNCK_STATS
    pfGfxPipeTimesStats btlnck;
#endif /* USE_BTLNCK_STATS */
} pfGfxTimesData;

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfStats CAPI ------------------------------*/

extern DLLEXPORT pfStats*             pfNewStats(void *arena);
extern DLLEXPORT pfType*              pfGetStatsClassType(void);
extern DLLEXPORT unsigned int         pfStatsClassMode(pfStats* _stats, int _class, unsigned int _mask, int _val);
extern DLLEXPORT unsigned int         pfGetStatsClassMode(pfStats* _stats, int _class);
extern DLLEXPORT void                 pfStatsAttr(pfStats* _stats, int _attr, float _val);
extern DLLEXPORT float                pfGetStatsAttr(pfStats* _stats, int _attr);
extern DLLEXPORT unsigned int         pfStatsClass(pfStats* _stats, unsigned int _enmask, int _val);
extern DLLEXPORT unsigned int         pfGetStatsClass(pfStats* _stats, unsigned int _enmask);
extern DLLEXPORT int                  pfGetStatsGfxPipeTimestampStatus(pfStats* _stats, unsigned int stamp);
extern DLLEXPORT int                  pfGetStatsNumGfxPipeTimestamp(pfStats* _stats);
extern DLLEXPORT unsigned int         pfGetOpenStats(pfStats* _stats, unsigned int _enmask);
extern DLLEXPORT unsigned int         pfOpenStats(pfStats* _stats, unsigned int _enmask);
extern DLLEXPORT unsigned int         pfCloseStats(unsigned int _enmask);
extern DLLEXPORT unsigned int         pfCollectGfxPipeStats(pfStats* _stats, unsigned int _enmask);
extern DLLEXPORT void                 pfResetStats(pfStats* _stats);
extern DLLEXPORT void                 pfClearStats(pfStats* _stats, unsigned int _which);
extern DLLEXPORT void                 pfAccumulateStats(pfStats* _stats, pfStats* _src, unsigned int _which);
extern DLLEXPORT void                 pfAverageStats(pfStats* _stats, pfStats* _src, unsigned int _which, int _num);
extern DLLEXPORT void                 pfCopyStats(pfStats* _stats, const pfStats *_src, uint _which);
extern DLLEXPORT void                 pfStatsCountGSet(pfStats* _stats, pfGeoSet * _gset);
extern DLLEXPORT int                  pfQueryStats(pfStats* _stats, unsigned int _which, void *_dst, int size);
extern DLLEXPORT int                  pfMQueryStats(pfStats* _stats, unsigned int * _which, void *_dst, int size);
extern DLLEXPORT void                 pfStatsHwAttr(int _attr, float _val);
extern DLLEXPORT float                pfGetStatsHwAttr(int _attr);
extern DLLEXPORT void                 pfEnableStatsHw(unsigned int _which);
extern DLLEXPORT void                 pfDisableStatsHw(unsigned int _which);
extern DLLEXPORT unsigned int         pfGetStatsHwEnable(unsigned int _which);

#endif /* !PF_C_API */
#endif /* PF_C_API */


#ifdef __cplusplus
}
#endif

#endif /* !__PRSTATS_H__ */ 
