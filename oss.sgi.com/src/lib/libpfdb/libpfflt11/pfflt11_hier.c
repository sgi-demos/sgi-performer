/*
 * Copyright 1992, 1995, Silicon Graphics, Inc.
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
 * pfflt_hier.c $Revision: 1.1 $ 
 * 
 * FLIGHT version 11 .flt file converter. This file contains the 
 * .flt hierarchy traverser. 
 * 
 */
#include <stdio.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h> 
#include <varargs.h>
#include <math.h>
#include <bstring.h>

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdb/pfflt11.h>

/* Database counters */
static int     Clean11Count;
static int	DCSCount, SCSCount, LODCount, LayerCount, GroupCount;
static int	GeodeCount, BBoardCount, LPCount;
static int	FGroupCount, ObjectCount; 

static void	*Arena;		/* Performer shared memory arena */
static int	InstDepth;	

static void	hierInit(void);
static void	hierExit(void);
static char*	loadFltFile(const char*, int *);
static char*	convTree(pfGroup*, char*, char*, int);
static void	lookAhead(char *ptr, mgComment **, mgMatrix **,mgReplicate**);
static void	convHeader(mgHeader*);
static void	convMaterialTable(mgMaterial *mt);
static void	convColorTable(mgColorTable *ct);
static pfLOD    *convLOD(mgLod *);
static pfSequence* convSeq(mgGroup* group);
static char*	convObject(pfGroup*, char*, char*, int);
static void	combineLODs(pfGroup *group);
static pfNode*	cleanTree(pfNode *);
static void	countTree(pfNode *);
static MgFile*	newMgFile(const char *file);

MgFile		*CurMgFile11;	/* Currently active FLIGHT file */
MgFile	        *RootMgFile11;	/* Initial FLIGHT file */
int		FlatFlag11;	/* Flat shading flag for FLIGHT object */
int	    	ComputeNormals11 = 1;
int	    	Clean11 = 1;
int	    	Flatten11 = 1;
int	    	CombineLODs11 = 1;

extern char *OpcodeNames11[] = {
			"unknown",		/*  0 */
			"header",		/*  1 */
			"group",		/*  2 */
			"lod",			/*  3 */
			"object",		/*  4 */
			"poly",			/*  5 */
			"unknown",		/*  6 */
			"vert_abs",		/*  7 */
			"vert_shad",		/*  8 */
			"vert_norm",		/*  9 */
			"push",			/* 10 */
			"pop",			/* 11 */
			"unknown",		/* 12 */
			"unknown",		/* 13 */
			"unknown",		/* 14 */
			"unknown",		/* 15 */
			"unknown",		/* 16 */
			"unknown",		/* 17 */
			"unknown",		/* 18 */
			"push_sf",		/* 19 */
			"pop_sf",		/* 20 */
			"unknown",		/* 21 */
			"unknown",		/* 22 */
			"unknown",		/* 23 */
			"unknown",		/* 24 */
			"unknown",		/* 25 */
			"unknown",		/* 26 */
			"unknown",		/* 27 */
			"unknown",		/* 28 */
			"unknown",		/* 29 */
			"unknown",		/* 30 */
			"comment",		/* 31 */
			"color_table",		/* 32 */
			"unknown",		/* 33 */
			"unknown",		/* 34 */
			"unknown",		/* 35 */
			"unknown",		/* 36 */
			"unknown",		/* 37 */
			"unknown",		/* 38 */
			"unknown",		/* 39 */
			"unknown_matrix",	/* 40 */
			"unknown_matrix",	/* 41 */
			"unknown_matrix",	/* 42 */
			"unknown_matrix",	/* 43 */
			"unknown_matrix",	/* 44 Translate */	
			"unknown_matrix",	/* 45 Scale */
			"unknown_matrix",	/* 46 Rotate */
			"unknown_matrix",	/* 47 */
			"unknown_matrix",	/* 48 */
			"matrix",		/* 49 */
			"vector",		/* 50 */
			"bbox",			/* 51 */
			"unknown",		/* 52 */
			"unknown",		/* 53 */
			"unknown",		/* 54 */
			"unknown",		/* 55 */
			"unknown",		/* 56 */
			"unknown",		/* 57 */
			"unknown",		/* 58 */
			"unknown",		/* 59 */
			"replicate",		/* 60 */
			"instance_ref",		/* 61 */
			"instance_def",		/* 62 */
			"external_ref",		/* 63 */
			"texture_ref",		/* 64 */
			"eyepoints",		/* 65 */
			"material_table",	/* 66 */
			"unknown",		/* 67 */
			"unknown",		/* 68 */
			"unknown",		/* 69 */
			"unknown",		/* 70 */
			"unknown",		/* 71 */
			"unknown",		/* 72 */
			"unknown",		/* 73 */
			"unknown",		/* 74 */
			"unknown"};		/* 75 */

    /*---------------------------------------------------*/

void
pfdConverterMode_flt11(int mode, int val)
{
    switch(mode) {
	case PFFLT11_COMPUTE_NORMALS:
	    ComputeNormals11 = val;
	    break;
	case PFFLT11_FLATTEN:
	    Flatten11 = val;
	    break;
	case PFFLT11_CLEAN:
	    Clean11 = val;
	    break;
	case PFFLT11_COMBINE_LODS:
	    CombineLODs11 = val;
	    break;
    }
}

int
pfdGetConverterMode_flt11(int mode)
{
    switch(mode) {
	case PFFLT11_COMPUTE_NORMALS:
	    return ComputeNormals11;
	case PFFLT11_FLATTEN:
	    return Flatten11;
	case PFFLT11_CLEAN:
	    return Clean11;
	case PFFLT11_COMBINE_LODS:
	    return CombineLODs11;
	default:
	    return -1;
    }
}

/*
 * Load and FLIGHT file and convert to in-memory Performer database.
 */
pfNode *
pfdLoadFile_flt11 (const char *file)
{
    int            size;	
    char           *mgbuf;
    
    RootMgFile11 = CurMgFile11 = newMgFile(file);\

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "FLIGHT loader\n");
    pfNotify(PFNFY_NOTICE, PFNFY_MORE, "$Revision: 1.1 $, $Date: 2000/11/21 21:39:33 $\n");

    hierInit();
    GeomInit11();

    /*
     * Load FLIGHT binary file into memory.
    */
    mgbuf = loadFltFile(file, &size);

    if(mgbuf == NULL)
	CurMgFile11->root = NULL;
    else
    {
    	CurMgFile11->root = pfNewGroup();

	/* 
	 * Convert FLIGHT tree to Performer tree 
	*/
    	if(convTree(CurMgFile11->root, mgbuf, mgbuf + size, 0) == NULL)
    	{
	    if(pfDelete(CurMgFile11->root) == NULL)
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Could not delete root");
	    CurMgFile11->root = NULL;
        }
	pfFree(mgbuf);
    }

    if(CurMgFile11->root == NULL)
    {
        pfNotify(PFNFY_WARN,PFNFY_RESOURCE, 
		    "LoadFlt() File \"%s\" not loaded.",  file);
	GeomExit11();
	hierExit();
	return NULL;
    }

    /*
     * Remove static modelling matrices from Performer tree for improved
     * performance.
    */
    if (Flatten11)
    	pfFlatten(CurMgFile11->root, 0);

    /* 
     * Remove extraneous nodes from Performer tree for improved performance
     * and memory savings.
    */
    if (Clean11)
	cleanTree((pfNode*)CurMgFile11->root);

    if (pfGetNotifyLevel() >= PFNFY_INFO)
 	countTree((pfNode*)CurMgFile11->root);

    GeomExit11();
    hierExit();

    return (pfNode *)CurMgFile11->root;
}

/*
 * Initialize static globals
*/
static void
hierInit(void)
{

    /* Get Performer shared memory arena */
    Arena = pfGetSharedArena();

    /*
     * Initialize counters.
    */
    Clean11Count = 0;
    GroupCount = DCSCount = SCSCount = LODCount = LayerCount = 0;
    GeodeCount = BBoardCount = LPCount = 0;
    FGroupCount = ObjectCount = 0;

    InstDepth = -2;
}


/*
 * Print out stats and clean up.
*/
static void
hierExit(void)
{
    MgFile         *mgf, *next;
    pfVec3         *rootCTab;

    if (RootMgFile11->root)
    {
	/*
	 * Print out hierarchy statistics. 
	*/
	pfNotify(PFNFY_INFO, PFNFY_PRINT, "\nFLIGHT Hierarchy Statistics:\n");
	pfNotify(PFNFY_INFO, PFNFY_MORE, "\tGroups = %ld, Objects = %ld\n",
	    FGroupCount, ObjectCount);
	pfNotify(PFNFY_INFO, PFNFY_MORE, "\nPerformer Hierarchy:\n");
	pfNotify(PFNFY_INFO, PFNFY_MORE,
	    "\tGroups = %ld, DCSs = %ld, SCSs = %ld, LODs = %ld, Layers = %ld\n",
	    GroupCount, DCSCount, SCSCount, LODCount, LayerCount);
	pfNotify(PFNFY_INFO, PFNFY_MORE, "\tClean11ed  = %ld\n", Clean11Count);
	pfNotify(PFNFY_INFO, PFNFY_MORE,
	    "\tGeodes = %ld, Billboards = %ld, LightPoints = %ld\n",
	    GeodeCount, BBoardCount, LPCount);

    }

    /* Free MgFile structure list */
    mgf = RootMgFile11;
    rootCTab = RootMgFile11->colorTable;
    pfFree(rootCTab);
    while (mgf)
    {
	GeoState	*gs, *gsnext;

	next = mgf->next;
	if (mgf->colorTable != RootMgFile11->colorTable)
	    pfFree(mgf->colorTable);

	/* Free GeoState list */
	gs = mgf->gsList;
	while (gs)
	{
	    gsnext = gs->next;
	    pfdDelGeoBldr(gs->builder);
	    if (gs->bbuilder)
		pfdDelGeoBldr(gs->bbuilder);
	    pfFree(gs);
	    gs = gsnext;
	}
	
	pfFree(mgf);
	mgf = next;
    }
}


/*
 * Create and initialize a new MgFile structure
*/
static MgFile*
newMgFile(const char *file)
{
    MgFile  *mgf;
    int    i;

    mgf = (MgFile *)pfMalloc(sizeof(MgFile), NULL);
    strcpy(mgf->file, file);
    mgf->root = NULL;
    mgf->mgTexCount = mgf->mgInstCount = mgf->pfTexCount = 0;
    
    for (i = 0; i < MG_MAX_TEXTURES; i++)
	mgf->pfTexIndex[i] = -1;

    for (i = 0; i < 64; i++)
	mgf->pfMtlList[i] = NULL;

    mgf->colorTable = NULL;
    mgf->mgMtlList = NULL;
    mgf->gsList = NULL;
    mgf->next = NULL;

    return mgf;
}


/*
 * Load a .flt file into memory
*/
static char*
loadFltFile(const char *file, int *size)
{
    int             ifd;
    struct stat     stbuf;
    char           *mgbuf;
    char	    path[PF_MAXSTRING];

    if (file == NULL)
	return NULL;

    if (!pfFindFile(file, path, R_OK))
	return NULL;
	
    /*
     * Read in the FLIGHT file. 
     */
    if ((ifd = open(path, O_RDONLY)) == -1)
    {
 	pfNotify(PFNFY_WARN,PFNFY_RESOURCE, 
		"LoadFlt: Could not access \"%s\"", path);
	return (NULL);
    }

    fstat((int)ifd, &stbuf);		    /* find the size of the file */
    if (stbuf.st_size == 0)
    {
 	pfNotify(PFNFY_WARN,PFNFY_RESOURCE, "LoadFlt: \"%s\" is empty", path);
	close((int)ifd);
	return (NULL);
    }

    mgbuf = (char *) pfMalloc((unsigned int)stbuf.st_size, NULL);
    read((int)ifd, mgbuf, (unsigned int)stbuf.st_size);	/* read the file into memory */
    close((int)ifd);
    
    if (((mgBead *) mgbuf)->opcode != MG_HEADER)
    {
 	pfNotify(PFNFY_WARN,PFNFY_RESOURCE, 
		"LoadFlt: Bogus FLT header in \"%s\"", path);
	return (NULL);
    }
    *size = stbuf.st_size;
    return mgbuf;
}


/*
 * Recursively traverse .flt tree and convert to Performer tree.
*/
static char*
convTree(pfGroup *parent, char *tree, char *end, int depth)
{
    register char   *ptr;
    int             done;
    static int	    def = -1;
    mgBead	    *bead;
    pfGroup	    *newParent = parent;

    done = FALSE;

    ptr = tree;
    while(ptr < end && !done)
    {
	bead = (mgBead *) ptr;
	switch (bead->opcode)
	{
	case MG_HEADER:
	    /* Warn if version is > 11 */
	    if (((mgHeader *) bead)->frev > 11)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT, "=== WARNING ==================\n");
		pfNotify(PFNFY_WARN, PFNFY_MORE, "Loading FLIGHT version %d file: \"%s\".\n", 
		    ((mgHeader *) bead)->frev, CurMgFile11->file);
		pfNotify(PFNFY_WARN, PFNFY_MORE, "This loader does not support versions past 11.\n");
		pfNotify(PFNFY_WARN, PFNFY_MORE, "Attempting to load but skipping all opcodes not\n");
		pfNotify(PFNFY_WARN, PFNFY_MORE, "defined in the FLIGHT version 11 specification.\n");
		pfNotify(PFNFY_WARN, PFNFY_MORE, "==============================\n");

#ifdef	DON_T_EVEN_TRY_TO_READ_THE_FILE
		return NULL;
#endif
	    }

	    convHeader((mgHeader *) bead);
	    ptr += bead->length;
	    break;
	case MG_COLOR_TABLE:
	    convColorTable((mgColorTable *) bead);
	    ptr += bead->length;
	    break;
	case MG_MATERIAL_TABLE:
	    convMaterialTable(((mgMaterialTable *) bead)->mat);
	    ptr += bead->length;
	    break;
	case MG_TEXTURE_REF:
	    if (CurMgFile11->mgTexCount  >= MG_MAX_TEXTURES)
	    {
		pfNotify(PFNFY_WARN, PFNFY_PRINT, 
				"Max number of textures exceeded");
		break;
	    }
	    CurMgFile11->mgTexList[CurMgFile11->mgTexCount++] = 
						(mgTextureRef *) bead;
	    ptr += bead->length;
	    break;
	case MG_PUSH:
	    depth++;
	    ptr = convTree(newParent, ptr + bead->length, end, depth);
	    depth--;	/* Decrement depth because previous convTree got POP */
	    if (depth == 0)
		done = TRUE;
	    break;
	case MG_POP:
	    if (CombineLODs11)
		combineLODs(parent);
	    ptr += bead->length;
	    return ptr;
	case MG_GROUP:
	    {
	    mgComment	*comment;
	    mgMatrix	*matrix;
	    mgReplicate	*replicate;
	    pfGroup	*group;

	    FGroupCount++;

	    lookAhead(ptr + bead->length, &comment, &matrix, &replicate);

	    if(matrix != NULL)
	    {
		/* Special flag of 44 indicates that matrix is a pfDCS */
	    	if (((mgGroup *) bead)->fxId1 == 44)
	    	{
		    pfDCS 		*dcs;

		    dcs = pfNewDCS();

		    /* Convert translation vector */
	    	    MG2PF_COORD3(matrix->matrix[3], matrix->matrix[3]);

		    pfDCSMat(dcs, matrix->matrix);

		    group = (pfGroup*)dcs;
	    	}
	    	else		/* It's an SCS */
	    	{
		    pfSCS *scs;

		    /* Convert translation vector */
	    	    MG2PF_COORD3(matrix->matrix[3], matrix->matrix[3]);

		    scs =  pfNewSCS(matrix->matrix);

		    group = (pfGroup*)scs;
	  	}
	    }
	    /* .flt animation bead */
	    else
	    {
		mgGroup *mggroup = (mgGroup *) bead;

		if (mggroup->flags & 0x60000000)
		    group = (pfGroup*)convSeq(mggroup);
	    	else
	    	    group = pfNewGroup();
	    }

	    pfNodeName((pfNode *) group, ((mgGroup *) bead)->id);

	    if(InstDepth == depth - 1)
	    {
		MgFile	*cmf = CurMgFile11;

		cmf->mgInstList[cmf->mgInstCount].inst = (pfNode*)group;
		cmf->mgInstList[cmf->mgInstCount].id = def;
		cmf->mgInstCount++;
	    }
	    else
		pfAddChild(parent, group);

	    newParent = group;
	    }
	    ptr += bead->length;
	    break;
	case MG_LOD:
	    {
		pfLOD 	*lod;
		pfGroup *group;

		lod = convLOD((mgLod *) bead);

		group = pfNewGroup();
		pfAddChild(lod, group);

		if(InstDepth == depth - 1)
		{
		    MgFile	*cmf = CurMgFile11;

		    cmf->mgInstList[cmf->mgInstCount].inst = (pfNode*)lod;
		    cmf->mgInstList[cmf->mgInstCount].id = def;
		    cmf->mgInstCount++;
		}
		else
		    pfAddChild(parent,  lod);

		newParent = group;
	    }
	    ptr += bead->length;
	    break;
	case MG_OBJECT:
	    {
		pfGroup		*grp = pfNewGroup();
	        mgComment	*comment;
	        mgMatrix	*matrix = NULL;
	        mgReplicate	*replicate;

		ObjectCount++;

	    	lookAhead ( ptr + bead->length, &comment, &matrix, 
							&replicate );
		if(matrix != NULL)
		{
		    pfSCS *scs;

		    /* Convert translation vector */
	    	    MG2PF_COORD3(matrix->matrix[3], matrix->matrix[3]);

		    scs =  pfNewSCS(matrix->matrix);

		    grp = (pfGroup*)scs;
	    	}
	    	else
	    	    grp = pfNewGroup();

		ptr = convObject(grp, ptr, end, 0);

		if(InstDepth == depth - 1)
		{
		    MgFile	*cmf = CurMgFile11;

		    cmf->mgInstList[cmf->mgInstCount].inst = (pfNode*)grp;
		    cmf->mgInstList[cmf->mgInstCount].id = def;
		    cmf->mgInstCount++;
		}
		else
		    pfAddChild(parent, grp);
	    }
	    break;
	case 40: case 41: case 42: case 43: case 44: case 45: case 46: 
	case 47: case 48: 	/* Ignored matrix types */
	case MG_COMMENT:
	case MG_MATRIX:
	case MG_REPLICATE:
	    ptr += bead->length;
	    break;
	case MG_INSTANCE_DEF:
	    /*
	     * The following is to correct for a bug in FLIGHT files, 
	     * where all definition numbers are 0. 
	     */
	    if (((mgInstanceDef *) bead)->defNumber <= def)
	    {
		def++;
		((mgInstanceDef *) bead)->defNumber = (short)def;
	    }
	    else
		def = ((mgInstanceDef *) bead)->defNumber;

	    InstDepth = depth;
	    ptr += bead->length;
	    break;
	case MG_INSTANCE_REF:
	    {
		int	    i, ref;
		MgFile	    *cmf = CurMgFile11;

		/* Try to find instance definition */
		ref = ((mgInstanceRef *) bead)->defNumber;
		for(i=0; i<cmf->mgInstCount; i++)
		{
		    if(cmf->mgInstList[i].id == ref)
			break;
		}

		if(i == cmf->mgInstCount)
		{
		    pfNotify(PFNFY_WARN,PFNFY_PRINT, 
		      "LoadFlt() Could not find Instance ref %ld", ref);
		}
		else
		{
		    pfAddChild(newParent, pfClone(cmf->mgInstList[i].inst, 0));
		}
	    }
	    ptr += bead->length;
	    break;
	case MG_EXTERNAL_REF:
	    {
		char	*exptr, *file;
		int	exsize;
		MgFile	*mgf = RootMgFile11;

		/* Search to see if file has already been referenced */
		file = 	((mgExternalRef*)bead)->file;
		while(mgf)
		{
		    if(strcmp(file, mgf->file) == 0)
			break;
		    mgf = mgf->next;
		}

		/* Not found, must load file */
		if(mgf == NULL)
		{
		    MgFile  *prevFile, *next;
		    
		    /* Make new MgFile structure */
		    mgf = newMgFile(file);
		    mgf->root = pfNewGroup();
		    pfNodeName(mgf->root, file);

		    /* Insert MgFile into list */
		    next = CurMgFile11->next;
		    CurMgFile11->next = mgf;
		    prevFile = CurMgFile11;
		    CurMgFile11 = mgf;
		    CurMgFile11->next = next;

		    /* Now recursively convert the external reference */
		    if((exptr = loadFltFile(file, &exsize)) != NULL)
		    {
		    	convTree(CurMgFile11->root, exptr, exptr + exsize, 0);
		    	pfAddChild(parent, CurMgFile11->root);
		    	pfFree(exptr);
		    }
		    
		    /* Restore previous file */
		    CurMgFile11 = prevFile;
		}
		else
		{
		    pfAddChild(parent, pfClone(mgf->root, 0));
		}

		ptr += bead->length;
	    }
	    break;
	/* Don't print error message for these beads. */
	case MG_BBOX:
	case MG_EYEPOINTS:
	    ptr += bead->length;
	    break;
	default:
	    if (bead->opcode < MG_NUM_OPCODES)
		pfNotify(PFNFY_WARN,PFNFY_PRINT, 
		      "LoadFlt() opcode %ld \"%s\" not implemented.",
			bead->opcode, OpcodeNames11[bead->opcode]);
	    else
		pfNotify(PFNFY_WARN,PFNFY_PRINT, 
		      "LoadFlt() opcode %ld not implemented.", bead->opcode);
			
	    ptr += bead->length;
	    break;
	}
    }

    return ptr;
}


/*
 * Look ahead in .flt hieararchy for beads which change the type of the
 * current bead.
*/
static void
lookAhead(char *ptr, mgComment **c, mgMatrix **m, mgReplicate **r)
{
    *c = NULL;
    *m = NULL;
    *r = NULL;

    while(1) 
    {
	switch(((mgBead*)ptr)->opcode)
	{
	    case MG_COMMENT:
		*c = (mgComment*)ptr;
		break;
	    case MG_MATRIX:
		*m = (mgMatrix*)ptr;
		break;
	    case MG_REPLICATE:
		*r = (mgReplicate*)ptr;
		break;
	    default:
		return;
	}
	ptr += ((mgBead*)ptr)->length;
    }
}


/*
 * Convert .flt file header 
*/
static void
convHeader(mgHeader *header)
{
    /*
     * generate vertex conversion multiplier 
     */
    switch (header->unitType)
    {
    case MGU_METERS:
	CurMgFile11->vertMult = 1.0;
	break;
    case MGU_KILOMETERS:
	CurMgFile11->vertMult = 1000.0;
	break;
    case MGU_FEET:
	CurMgFile11->vertMult = 0.3048;
	break;
    case MGU_INCHES:
	CurMgFile11->vertMult = 0.3048 / 12.0;
	break;
    case MGU_NAUT_MILES:
	CurMgFile11->vertMult = 1852.0;
	break;
    default:
	CurMgFile11->vertMult = 1.0;
	pfNotify(PFNFY_WARN,PFNFY_RESOURCE, 
		  "LoadFlt() Unknown unit type %ld found in \"%s\"",
		    header->unitType, CurMgFile11->file);
	break;
    }

    if (header->unitMulDiv < 0)
	CurMgFile11->vertMult /= -header->unitMulDiv;
    else
	CurMgFile11->vertMult *= header->unitMulDiv;
}


static void
convMaterialTable(mgMaterial *mt)
{
    register int    i;

    CurMgFile11->mgMtlList = mt;

    /* Clamp colors to 12 bit color grid */	       
    for(i=0; i<64; i++)
    {
	mgMaterial *mgm = &mt[i];

	mgm->ambient[0] = ((int)(mgm->ambient[0] * 65535.0f)) / 65535.0f;
	mgm->ambient[1] = ((int)(mgm->ambient[1] * 65535.0f)) / 65535.0f;
	mgm->ambient[2] = ((int)(mgm->ambient[2] * 65535.0f)) / 65535.0f;

	mgm->diffuse[0] = ((int)(mgm->diffuse[0] * 65535.0f)) / 65535.0f;
	mgm->diffuse[1] = ((int)(mgm->diffuse[1] * 65535.0f)) / 65535.0f;
	mgm->diffuse[2] = ((int)(mgm->diffuse[2] * 65535.0f)) / 65535.0f;

	mgm->specular[0] = ((int)(mgm->specular[0] * 65535.0f)) / 65535.0f;
	mgm->specular[1] = ((int)(mgm->specular[1] * 65535.0f)) / 65535.0f;
	mgm->specular[2] = ((int)(mgm->specular[2] * 65535.0f)) / 65535.0f;

	mgm->emission[0] = ((int)(mgm->emission[0] * 65535.0f)) / 65535.0f;
	mgm->emission[1] = ((int)(mgm->emission[1] * 65535.0f)) / 65535.0f;
	mgm->emission[2] = ((int)(mgm->emission[2] * 65535.0f)) / 65535.0f;
    }
}

	    
/*
 * Convert color table into floating point rgb values. Attempt to share
 * colortables between .flt files since converting them is expensive.
*/
static void
convColorTable(mgColorTable * ct)
{
    int	     diff = 0;
    register int    i, j, k, intensity;
    register int    r, g, b, *c, *rc;

    CurMgFile11->mgCTab = ct;

    /* Attempt to share colortable with root file's */
    if (CurMgFile11 != RootMgFile11)
    {
	/*
	 * First see if color table is the same as the root file's 
	 */
	c = (int *) ct->color;
	rc = (int *) RootMgFile11->mgCTab->color;

	for (i = 0; i < (MG_CT_SHADED * 3) >> 1; i++)
	    if (c[i] != rc[i])
	    {
		diff = 1;
		break;
	    }

	c = (int *) ct->fixedColor;
	rc = (int *) RootMgFile11->mgCTab->fixedColor;

	for (i = 0; i < (MG_CT_FIXED * 3) >> 1; i++)
	    if (c[i] != rc[i])
	    {
		diff = 1;
		break;
	    }

	if (!diff)
	{
	    CurMgFile11->mgCTab = RootMgFile11->mgCTab;
	    CurMgFile11->colorTable = RootMgFile11->colorTable;
	    return;
	}
    }

    CurMgFile11->colorTable = (pfVec3*)pfMalloc(sizeof(pfVec3) * 
				(4096 + MG_CT_FIXED), pfGetSharedArena());

    /*
     * Make shaded portion of color table 
     */
    for (i = 0, k = 0; i < MG_CT_SHADED; i++)
    {
	for (j = 0; j < MG_CT_NO_SHADE; j++, k++)
	{
	    intensity = j + 1;
	    r = (ct->color[i][0] * intensity + 1) / MG_CT_NO_SHADE;
	    g = (ct->color[i][1] * intensity + 1) / MG_CT_NO_SHADE;
	    b = (ct->color[i][2] * intensity + 1) / MG_CT_NO_SHADE;

	    CurMgFile11->colorTable[k][0] = r / 255.0f; 
	    CurMgFile11->colorTable[k][1] = g / 255.0f;
	    CurMgFile11->colorTable[k][2] = b / 255.0f;
	}
    }

    /*
     * Make fixed portion of color table 
     */
    for (i = 0, k = 4096; i < MG_CT_FIXED; i++, k++)
    {
	PFSCALE_VEC3(CurMgFile11->colorTable[k], 1/255., ct->fixedColor[i]);
    }
}


/*
 * Convert .flt LOD bead into pfLOD node.
*/
static pfLOD*
convLOD(mgLod* lod)
{
    int            v[3];
    float           vtmp[3], ctmp[3];
    pfLOD          *pflod;

    /* Convert ranges to floats */
    v[0] = lod->inDist;
    v[1] = v[2] = lod->outDist;
    MG2PF_COORD3(vtmp, v);

    if (vtmp[0] == vtmp[1])
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "LoadFlt11()/convLOD() %s"
		"Zero distance LOD interval.", lod->id);
	vtmp[1] += .01f;
    }	

    v[0] = lod->center[0];
    v[1] = lod->center[1];
    v[2] = lod->center[2];
    MG2PF_COORD3(ctmp, v);

    pflod = pfNewLOD();
    pfLODRange(pflod, 0, vtmp[1]);
    pfLODRange(pflod, 1, vtmp[0]);
    pfLODCenter(pflod, ctmp);

    pfNodeName( pflod, lod->id);
    return pflod;
}

/*
 * Convert .flt group bead into pfSequence node.
*/
static pfSequence*
convSeq(mgGroup* group)
{
    pfSequence 	*seq = pfNewSeq();
    int	smode = (group->flags & 0x40000000) ? PFSEQ_CYCLE : PFSEQ_SWING;

    pfSeqTime(seq, PFSEQ_ALL, 0.1f);

    pfSeqInterval(seq, smode, 0, PFSEQ_ALL);

    if(smode == PFSEQ_CYCLE)
	pfSeqDuration(seq, 1.0f, PFSEQ_ALL);
    else 
	pfSeqDuration(seq, 1.0f, PFSEQ_ALL);

    pfSeqMode(seq, PFSEQ_START);

    return seq;
}

#define MAXSUBFACES	256	/* Max subfaces in one object */

static pfNode*	makeLayer(char **nptr, char *ptr, char *end, int depth);

/*
 * Convert .flt object bead. This usually results in a pfGeode but could
 * generate pfLightPoint or pfBillboard nodes.
*/
static char*
convObject(pfGroup *parent, char *ptr, char* end, int depth)
 {
    int            sfdepth=0, done = 0;
    pfLayer        *layer = NULL;
    mgBead	    *bead, *tmpBead;
    pfNode	    *geom;
    char	    *nptr;

    while(ptr < end && !done)
    {
	bead = (mgBead*)ptr;
	switch (bead->opcode)
	{
	case MG_OBJECT:
    	    /* Set flat-shading flag for object */
    	    FlatFlag11 = (((mgObject*)ptr)->flags & 0x08000000);
	    ptr += bead->length;
	    break;
	case MG_POLYGON:

            /* 
	     * Look ahead for PUSH_SF to see if this poly is a base poly 
	    */
            tmpBead = (mgBead*)(ptr + bead->length);
	    while(tmpBead->opcode != MG_POP) {
            	tmpBead = (mgBead*)((char*)tmpBead + tmpBead->length);
	    }

            tmpBead = (mgBead*)((char*)tmpBead + tmpBead->length);

            if(tmpBead->opcode == MG_PUSH_SF)	/* It's a base poly */
	    {
		/* Flush non base geometry */
		if((geom = MakeGeometry11()) != NULL)
		    pfAddChild(parent,  geom);

            	ptr = AddPoly11((mgPolygon*)bead);

		layer = pfNewLayer();
	    	pfNodeName((pfNode *) layer, ((mgPolygon *) bead)->id);

		pfAddChild(parent, layer);

		/* Flush base geometry */
		if((geom = MakeGeometry11()) != NULL)
		    pfLayerBase(layer, geom);

		sfdepth = 0;

		/* Flush layer geometry */
		while (geom = makeLayer(&nptr, ptr, end, sfdepth++))
		    pfAddChild(layer, geom);

		sfdepth = 0;
		ptr = nptr;
	    }
	    else			
            	ptr = AddPoly11((mgPolygon*)bead);

            break;
	case MG_PUSH:
	    depth++;
	    ptr += bead->length;
	    break;
	case MG_POP:
	    depth--;
	    ptr += bead->length;

	    if(depth <= 0)
		done = 1;

	    break;
	case MG_PUSH_SF:
	case MG_POP_SF:
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		    "LoadFlt11() convObject Shouldn't be here %d", 
				bead->opcode);
	    break;
	case 40: case 41: case 42: case 43: case 44: case 45: case 46: 
	case 47: case 48: case 76: case 77: case 78: case 79: case 80: 
	case 81: case 82: /* Ignored transformation record types */
	case MG_MATRIX:
	case MG_COMMENT:
	    ptr += bead->length;
	    break;
	default:
	    pfNotify(PFNFY_WARN,PFNFY_PRINT, 
		    "LoadFlt11() Unsupported node %ld \"%s\" found in obj.",
		    bead->opcode,
		    OpcodeNames11[bead->opcode]);
	    ptr += bead->length;
	    break;
	}
    }

    if((geom = MakeGeometry11()) != NULL)
    	pfAddChild(parent, geom);

    return ptr;
}

static pfNode*
makeLayer(char **nptr, char *ptr, char *end, int depth)
{
    int	sfdepth = 0, done = 0;
    mgBead	*bead;

    while(ptr < end && !done)
    {
	bead = (mgBead*)ptr;
	switch (bead->opcode)
	{
	case MG_POLYGON:
	    if (sfdepth-1 == depth)
            	ptr = AddPoly11((mgPolygon*)bead);
	    else	/* Skip over polygon */
	    {
		mgBead	*tmpBead;

            	tmpBead = (mgBead*)(ptr + bead->length);
	    	while(tmpBead->opcode != MG_POP) 
            	    tmpBead = (mgBead*)((char*)tmpBead + tmpBead->length);

            	ptr = ((char*)tmpBead + tmpBead->length);
	    }
            break;
	case MG_PUSH_SF:
	    sfdepth++;
	    ptr += bead->length;
	    break;
	case MG_POP_SF:
	    sfdepth--;
	    if (sfdepth <= 0)
		done = 1;
	    ptr += bead->length;
	    *nptr = ptr;
	    break;
	case 40: case 41: case 42: case 43: case 44: case 45: case 46: 
	case 47: case 48: case 76: case 77: case 78: case 79: case 80: 
	case 81: case 82: /* Ignored transformation record types */
	case MG_MATRIX:
	case MG_COMMENT:
	    ptr += bead->length;
	    break;
	default:
	    pfNotify(PFNFY_WARN,PFNFY_PRINT, 
		    "LoadFlt11() Unsupported node %ld \"%s\" found in obj.",
		    bead->opcode,
		    OpcodeNames11[bead->opcode]);
	    ptr += bead->length;
	    break;
	}
    }
    return MakeGeometry11();
}

#define MAXLODS	    64

typedef struct combinestruc {
    int	count;
    pfVec3	center;
    pfLOD	*lods[MAXLODS];
} combine, *combinept;

/* 
 * FLIGHT LOD beads only have a single switch in/out range pair. 
 * Performer LOD nodes have multiple switch ranges and multiple children.
 * This routine gathers all pfLOD's directly under a group and combines 
 * them into a single pfLOD with multiple children. This will significantly
 * improve cull traversal performance since there are fewer LODs to traverse.
 * 
 * NOTE: This may combine LODs which should not be combined and does not
 * check if switch in and switch out ranges match.
*/
static void
combineLODs(pfGroup *group)
{
    int	i, j, k = 0, n = pfGetNumChildren(group);
    combine	clod[MAXLODS];
    pfLOD	*newLOD = NULL;
    int	numLODs = 0;
    pfVec3	center;
    float 	tol;
    pfBox	bbox;

    /* initialize the combine struc */
    for (i=0;i<MAXLODS;i++)
    {
	clod[i].count = 0;
	pfSetVec3(clod[i].center, 0.0, 0.0, 0.0);
	for(j=0;j<MAXLODS;j++)
	    clod[i].lods[j] = NULL;
    }

    for(i=0;i<n;i++)
    {
	pfNode	*child;

	child = pfGetChild(group, i);
	if(pfIsOfType(child, pfGetLODClassType()))
	{
	    pfGetLODCenter((pfLOD*)child, center);
	    pfuTravCalcBBox((pfLOD*)child, &bbox);

	    /* the center tolerance for combination of lods is 
	       .2 of the bbox diagonal */
	    tol = sqrt( (double)(bbox.max[PF_X]-bbox.min[PF_X])*
			(double)(bbox.max[PF_X]-bbox.min[PF_X])+
			(double)(bbox.max[PF_Y]-bbox.min[PF_Y])*
			(double)(bbox.max[PF_Y]-bbox.min[PF_Y])+
			(double)(bbox.max[PF_Z]-bbox.min[PF_Z])*
			(double)(bbox.max[PF_Z]-bbox.min[PF_Z]))/5.;

	    for (j=0;j<numLODs;j++)
	    {
		if (PFALMOST_EQUAL_VEC3(center, clod[j].center, tol))
		{
		    clod[j].lods[clod[j].count++] = (pfLOD*)child;
		    break;
		}
	    }
	    if(j==numLODs)
	    {
		clod[numLODs].count = 1;
		pfCopyVec3(clod[numLODs].center, center);
		clod[numLODs++].lods[0] = (pfLOD*)child;
	    }
	}
    }

    if(numLODs == 0)
	return;

    /* Now combine LODs. Sort LOD ranges in increasing order. */
    for(i=0; i<numLODs; i++)
    {
	int 	lodi, minind;
	pfNode  *node;

	newLOD = pfNewLOD();
	pfNodeName ( newLOD, "combLOD");
	lodi = 0;

	for(j=0; j<clod[i].count; j++)
	{
	    float out, min=PF_HUGEVAL;

	    for(k=0; k<clod[i].count; k++)
	    {
		/* find the smallest left */
		if(!clod[i].lods[k])
		    continue;
    	    	out = pfGetLODRange(clod[i].lods[k], 0);
		if(out<=min)
		{
		    minind = k;
		    min = out;
		}
	    }
	    pfLODRange(newLOD, lodi, min);
	    pfLODRange(newLOD, lodi+1, 
		pfGetLODRange(clod[i].lods[minind], 1));

	    if(j==0)
		pfLODCenter(newLOD, clod[i].center);

	    /* Reparent child of old LOD to new LOD */
	    pfRemoveChild(group, clod[i].lods[minind]);
	    node = pfGetChild(clod[i].lods[minind], 0);
	    pfAddChild(newLOD, node);
	    pfRemoveChild(clod[i].lods[minind], node);
	    pfDelete(clod[i].lods[minind]);

	    clod[i].lods[minind] = NULL;
	    lodi++;
	}
	pfAddChild(group, newLOD);
    }	
}

/*
 * Traverse .flt hierarchy and print out beads.
*/
void
pfdScan_flt11(const char *file)
{
    char           *mgbuf, *mgTmp, *mgendbuf;
    int             size;

    mgbuf = loadFltFile(file, &size);
    mgendbuf = mgbuf + size;

    for (mgTmp = mgbuf;
	 mgTmp < mgendbuf;
	 mgTmp += ((mgBead *) mgTmp)->length)
    {
	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "  %2d %s: ",
	       ((mgBead *) mgTmp)->opcode,
	       OpcodeNames11[((mgBead *) mgTmp)->opcode]);

	switch (((mgBead*)mgTmp)->opcode)
	{
	case MG_HEADER:
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "    \"%s\"", ((mgHeader*)mgTmp)->id);
	    break;
	case MG_GROUP:
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "    \"%s\"", ((mgGroup*)mgTmp)->id);
	    break;
	case MG_LOD:
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "    \"%s\"", ((mgLod*)mgTmp)->id);
	    break;
	case MG_OBJECT:
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "    \"%s\"", ((mgObject*)mgTmp)->id);
	    break;
	}

    }

    pfFree(mgbuf);
}


/*
 * Remove unnecessary nodes from hierarchy.
 * 1. Remove/delete group nodes which have no children.
 * 2. Reparent single children of a group node to group's parents. Delete
 *    group node.
*/
static pfNode*
cleanTree(pfNode *node)
{
    int	i;

    if(pfIsOfType(node, pfGetGroupClassType()))
    {
	int nc;

        /* 
	 * Recurse on children. Act on return value.
        */
    	for(i = 0; i < (nc = pfGetNumChildren(node)); i++)
	{
            pfNode     *child = pfGetChild(node, i), *cnode;

            cnode = cleanTree(child);
        
    	    if(cnode == NULL)	/* Nuke empty group */
	    {
	        pfRemoveChild(node, child);
		if(pfDelete(child) == NULL)
	    	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
				"cleanTree() Could not delete empty group");
		i -= 1;
		Clean11Count++;
	    }
    	    else if(cnode != child)	/* Reparent child */
	    {
		pfRemoveChild(child, cnode);
	        pfReplaceChild(node, child, cnode);
		if(pfDelete(child) == NULL)
	    	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, 
				"cleanTree() Could not delete parent of single child");
		Clean11Count++;
	    }
	}		    
	if(nc == 0)
	    return NULL;
	/* Reparent only child; Can do for SCS because of pfFlatten11 */
    	else if(nc == 1)
	{
	    int	ok = 0;

	    if (pfGetType(node) == pfGetGroupClassType())
	   	ok = 1;
	    else if (pfGetType(node) == pfGetSCSClassType())
	    {
		static pfMatrix identMat = {1.0f, 0.0f, 0.0f, 0.0f, 
					    0.0f, 1.0f, 0.0f, 0.0f, 
					    0.0f, 0.0f, 1.0f, 0.0f, 
					    0.0f, 0.0f, 0.0f, 1.0f}; 
		pfMatrix	scsMat;


		pfGetSCSMat((pfSCS*)node, scsMat);
		if(PFEQUAL_MAT(scsMat, identMat))
		    ok = 1;
	    }
	
	    if(ok)	   
		return pfGetChild(node, 0);
	    else
		return node;
	}
	else 
	    return node;
    }
    else
	return node;
}


/*
 * Traverse tree and compile statistics.
 */
static void
countTree(pfNode *node)
{
    int        i, j, n;  

    if (pfIsOfType(node, pfGetGeodeClassType())) 
    {
	if (pfIsOfType(node, pfGetBboardClassType())) 
	    BBoardCount++;
	else
	    GeodeCount++;
	n = pfGetNumGSets(node);
	for(j=0; j<n; j++)
	    CountGSet11(pfGetGSet(node, j));
    }
    else if (pfIsOfType(node, pfGetLPointClassType()))
	LPCount++;
    else if (pfIsOfType(node, pfGetGroupClassType()))
    {
	int nc = pfGetNumChildren(node);
	
	if (pfIsOfType(node, pfGetLODClassType()))
	    LODCount++;
	else if (pfIsOfType(node, pfGetLayerClassType()))
            LayerCount++;
	else if (pfIsOfType(node, pfGetSCSClassType()))
	{
	    if (pfIsOfType(node, pfGetDCSClassType()))
		DCSCount++;
	    else
		SCSCount++;
	}
	else
	    GroupCount++;
	
	/*
	 * Recurse on children.
	 */
	for(i = 0; i < nc; i++)
	    countTree(pfGetChild(node, i));
    }
}

void
OverrideMtls11(char *dstFile, char *srcFile)
{
    int		    fd;
    int            dstSize, srcSize;	
    char           *dst, *src, *dp, *sp;

    dp = dst = loadFltFile(dstFile, &dstSize);
    sp = src = loadFltFile(srcFile, &srcSize);

    if(dst == 0 || src == 0)
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, "OverrideMtls() Bad files %s %s", dstFile, srcFile);

    while(dp < dst + dstSize)
    {
	if(((mgBead*)dp)->opcode == MG_MATERIAL_TABLE)
	    break;
	else
	    dp += ((mgBead*)dp)->length;
    }
    if(((mgBead*)dp)->opcode != MG_MATERIAL_TABLE)
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, "OverrideMtls() Could not find %s material table", dstFile);

    while(sp < src + srcSize)
    {
	if(((mgBead*)sp)->opcode == MG_MATERIAL_TABLE)
	    break;
	else
	    sp += ((mgBead*)sp)->length;
    }
    if(((mgBead*)sp)->opcode != MG_MATERIAL_TABLE)
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, "OverrideMtls() Could not find %s material table", srcFile);

    bcopy(sp, dp, ((mgBead*)sp)->length);

    fd = open(dstFile, O_WRONLY);

    if(fd < 0)
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, "OverrideMtls() Could not open %s for writing.", dstFile);
	
    write(fd, dst, (unsigned int)dstSize);

    close(fd);
}

void
OverrideCtab11(char *dstFile, char *srcFile)
{
    int		    fd;
    int            dstSize, srcSize;	
    char           *dst, *src, *dp, *sp;

    dp = dst = loadFltFile(dstFile, &dstSize);
    sp = src = loadFltFile(srcFile, &srcSize);

    if(dst == 0 || src == 0)
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, "OverrideCtab() hosed");

    while(dp < dst + dstSize)
    {
	if(((mgBead*)dp)->opcode == MG_COLOR_TABLE)
	    break;
	else
	    dp += ((mgBead*)dp)->length;
    }
    if(((mgBead*)dp)->opcode != MG_COLOR_TABLE)
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, "OverrideCtab() hosed");

    while(sp < src + srcSize)
    {
	if(((mgBead*)sp)->opcode == MG_COLOR_TABLE)
	    break;
	else
	    sp += ((mgBead*)sp)->length;
    }
    if(((mgBead*)sp)->opcode != MG_COLOR_TABLE)
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, "OverrideCtab() hosed");

    bcopy(sp, dp, ((mgBead*)sp)->length);

    fd = open(dstFile, O_WRONLY);

    if(fd < 0)
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, "OverrideCtab() hosed");
	
    write(fd, dst, (unsigned int)dstSize);

    close(fd);
}

void
SimplifyMtls11(char *srcFile)
{
    int		    fd;
    int            srcSize, mid;	
    char           *src, *sp;
    mgMaterial	    *mList = NULL;

    sp = src = loadFltFile(srcFile, &srcSize);

    if(src == 0)
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, "SimplifyMtls() hosed");

    while(sp < src + srcSize)
    {
	switch(((mgBead*)sp)->opcode) {
	    case MG_MATERIAL_TABLE:
                mList = ((mgMaterialTable *)sp)->mat;
                sp += ((mgBead*)sp)->length;
	        break;
	    case MG_POLYGON:
		mid = ((mgPolygon*)sp)->material;

		if(mid < 0 || mList == NULL)
		    ((mgPolygon*)sp)->material = 3;
		else
		{
		    if(mList[mid].emission[0] != 0.0f ||
		       mList[mid].emission[1] != 0.0f ||
		       mList[mid].emission[2] != 0.0f)
			((mgPolygon*)sp)->material = 7;
		    else if(mList[mid].specular[0] != 0.0f ||
		       mList[mid].specular[1] != 0.0f ||
		       mList[mid].specular[2] != 0.0f)
			((mgPolygon*)sp)->material = 11;
		    else if(mList[mid].alpha != 1.0f)
			((mgPolygon*)sp)->material = 15;
		    else
			((mgPolygon*)sp)->material = 3;
	        }
                sp += ((mgBead*)sp)->length;
	        break;
            default:
                sp += ((mgBead*)sp)->length;
	        break;
        }

    }

    fd = open(srcFile, O_WRONLY);

    if(fd < 0)
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, "SimplifyMtls() hosed");
	
    write(fd, src, (unsigned int)srcSize);

    close(fd);
}

void
SetMtl11(char *srcFile, int mtl)
{
    int		    fd;
    int            srcSize;	
    char           *src, *sp;
    mgMaterial	    *mList = NULL;

    sp = src = loadFltFile(srcFile, &srcSize);

    if(src == 0)
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, "SetMtl() hosed");

    while(sp < src + srcSize)
    {
	switch(((mgBead*)sp)->opcode) {
	    case MG_POLYGON:
		((mgPolygon*)sp)->material = (short)mtl;
                sp += ((mgBead*)sp)->length;
	        break;
            default:
                sp += ((mgBead*)sp)->length;
	        break;
        }

    }

    fd = open(srcFile, O_WRONLY);

    if(fd < 0)
	pfNotify(PFNFY_FATAL, PFNFY_USAGE, "SetMtl() hosed");
	
    write(fd, src, (unsigned int)srcSize);

    close(fd);
}

