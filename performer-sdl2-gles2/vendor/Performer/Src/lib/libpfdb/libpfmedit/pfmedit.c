/*--------------------------Message start-------------------------------

This software is copyright (C) 1994-1995 by Medit Productions


This software is provided under the following terms, if you do not
agree to all of these terms, delete this software now. Any use of
this software indicates the acceptance of these terms by the person
making said usage ("The User").

1) Medit Productions provides this software "as is" and without
warranty of any kind. All consequences of the use of this software
fall completely on The User.

2) This software may be copied without restriction, provided that an
unaltered and clearly visible copy of this message is placed in any
copies of, or derivatives of, this software.


---------------------------Message end--------------------------------*/




/************************************************************************
The performer loader for Medit files...
************************************************************************/

#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <limits.h>
#include "Meditstuff/useful.h"
#include "libmedit.h"


/************************************************************************
Medit to Performer - dso version.

You can customise the Performer loader in various ways by setting the
mode as described below.
************************************************************************/

static int flags = 0;	    /* The default setting... */

PFDB_DLLEXPORT void pfdConverterMode_medit(int mode, int val)
{
    /* Don't flatten the scene graph after load */
    /* flags += PFM_noflatten; */

    /* Load database without textures (gives better colours than just disabling texture) */
    /* flags += PFM_notexture; */

    /* Optimise for LOD fading (only use this if you really are fading...) */
    /* flags += PFM_lodfade; */

    /* This can really speed things up, but changes colours slightly */
    /* flags += PFM_fastlighting; */

    /* If you have a Reality Engine, but you are not multisampling (eg. 1 RM machines) */
    /* flags += PFM_nomultisample; */

    /* If you want medit subobjects to be converted to separate, named pfGroups */
    /* flags += PFM_convertliteral; */

    if (val)
	flags |= mode;
    else
	flags &= ~mode;
}

PFDB_DLLEXPORT int pfdGetConverterMode_medit(int mode)
{
    return (flags & mode);
}

PFDB_DLLEXPORT pfNode *pfdLoadFile_medit(char *file)
{
    return LoadMedit(file, flags);
}

/************************************************************************
Global vars
************************************************************************/
flag FastLighting;
static void *MeditArena;
static pfGroup *MeditRoot;
static int DoTextures, ShadingType;
static flag MultiSampling, LoadingTextures, FadeLods, LiteralConvert;
static flag ItsARealityEngine, AfunctionAvailable, GoodAFunction;
#define NOTIFY fprintf(stderr,
/************************************************************************
Include files.
************************************************************************/
#include "Meditstuff/storage.c"		/* Storage of polygon lists etc		*/
#include "Meditstuff/geostates.c"	/* pfGeostate generator			*/
#include "Meditstuff/tmesh.c"		/* Make polygons into tmeshes		*/
#include "Meditstuff/billboard.c"	/* Deal with billboard objects		*/
#include "Meditstuff/addpolys.c"	/* Convert lists of polys to geosets	*/
#include "Meditstuff/lods.c"		/* Deal with a medit LOD		*/
#include "Meditstuff/cleantree.c"	/* Clean up the tree after flatten	*/
#include "Meditstuff/convert.c"		/* Do the conversion			*/

/************************************************************************
Medit to Performer - opt version with user supplied "flags"
************************************************************************/
pfNode *LoadMedit(char *file, int flags)
{
    char gv[20];

    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    NOTIFY"\n\n<<<<< pfLoadMedit version 1.5.05 >>>>>\n");

/* ----- First, load the .medit file using libmedit.a	----- */

    NOTIFY"Loading Medit file: %s\n",file);
    if (ReadMedit(file)) {
	if (!MeditFileHadNormals) {
	    fprintf(stderr,"Error while loading file:\n");
	    fprintf(stderr,"That file contains no surface normals, ");
	    fprintf(stderr,"please read the file into Medit,\n");
	    fprintf(stderr,"then write it out again...\007\n");
	    return NULL;
	}


/* ----- Second, work out system capabilities		----- */

	ItsARealityEngine = TRUE;
	AfunctionAvailable = GoodAFunction = TRUE;


/* ----- Third, deal with user parameters		----- */

	LoadingTextures	= ((flags & PFM_notexture) IS 0);
	FadeLods	= ((flags & PFM_lodfade) ISNT 0);
	FastLighting	= ((flags & PFM_fastlighting) ISNT 0);
	MultiSampling	= ItsARealityEngine AND ((flags & PFM_nomultisample) IS 0);
	LiteralConvert	= ((flags & PFM_convertliteral) ISNT 0);

/* ----- Fourth, convert the file			----- */

	NOTIFY"Converting file...\n");
	MeditArena = pfGetSharedArena();
	MeditRoot = pfNewGroup();
	InitialiseObjectList();
	ConvertMeditFile();
    
	if ((flags & PFM_noflatten) IS 0) {
	    NOTIFY"Flattening scene...\n");
	    pfFlatten(MeditRoot,0);
	    RemoveSCSs((pfNode*)MeditRoot);
	}


/* ----- Fifth, free the memory used by the loader	----- */

	FreeObjectList();
	Medit_FreeFile(CurrentMeditFile);


/* ----- Last, return a pointer to the database		----- */

	return (pfNode*)MeditRoot;

    }
    else {


/* ----- Error message if we failed to load the .medit file ----- */
	NOTIFY"Error %d during file read: %s\007\n", MeditErrorType, MeditErrorList[MeditErrorType]);
	return NULL;
    }
}

