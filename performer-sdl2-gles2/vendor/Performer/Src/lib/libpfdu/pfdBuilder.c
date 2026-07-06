/*
 * Copyright 1995, Silicon Graphics, Inc.
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
 * pfdBuilder.c
 *
 * $Revision: 1.105 $
 * $Date: 2002/12/07 01:46:47 $
 *
 * The builder implements a state-aware immediate-mode data
 * generation capability for IRIS Performer. It is the main
 * method used to build Performer scene graphs.
 *
 */

#ifndef WIN32
#include <alloca.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifdef mips
#include <bstring.h>
#else
#include <string.h>
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>

static void *sharedArena = NULL;

/* Extra State callbacks that manage multiple cbacks */
/* Note users need not use these as they are automatically */
/* used by the builder to handle Multiple Extensors */
extern PFDUDLLEXPORT int pfdCallback(pfTraverser *trav, void *data,int which);
extern PFDUDLLEXPORT int pfdPreDrawCBack(pfTraverser *trav, void *data);
extern PFDUDLLEXPORT int pfdPostDrawCBack(pfTraverser *trav, void *data);
extern PFDUDLLEXPORT int pfdPreCullCBack(pfTraverser *trav, void *data);
extern PFDUDLLEXPORT int pfdPostCullCBack(pfTraverser *trav, void *data);
typedef void 	(*setStateFunc)(pfGeoState*,uint64_t,void*);
typedef void*	(*getStateFunc)(pfGeoState*,uint64_t);

typedef void 	(*setMultiStateFunc)(pfGeoState*,uint64_t,int,void*);
typedef void*	(*getMultiStateFunc)(pfGeoState*,uint64_t, int);

typedef void*	(*getCombinedStateFunc)(pfGeoState*, uint64_t, pfGeoState*);
typedef void*	(*getCombinedMultiStateFunc)(pfGeoState*, uint64_t, int, pfGeoState*);
static void pfdUpdate(void);

static void (*setAttr[3])(pfGeoState*,uint64_t,void*) = {
  (setStateFunc)pfGStateAttr,
  (setStateFunc)pfGStateMode,
  (setStateFunc)pfGStateVal
};

static void* (*getAttr[3])(pfGeoState*,uint64_t) = {
  (getStateFunc)pfGetGStateAttr,
  (getStateFunc)pfGetGStateMode,
  (getStateFunc)pfGetGStateVal
};

static void* (*getCombinedAttr[3])(pfGeoState*, uint64_t, pfGeoState*) = {
  (getCombinedStateFunc)pfGetGStateCombinedAttr,
  (getCombinedStateFunc)pfGetGStateCombinedMode,
  (getCombinedStateFunc)pfGetGStateCombinedVal
};

/*
 *	Multi-value geostate methods (e.g. for MultiTexture)
 */

static void (*setMultiAttr[3])(pfGeoState*,uint64_t,int,void*) = {
  (setMultiStateFunc)pfGStateMultiAttr,
  (setMultiStateFunc)pfGStateMultiMode,
  (setMultiStateFunc)pfGStateMultiVal
};


static void* (*getMultiAttr[3])(pfGeoState*,uint64_t,int) = {
  (getMultiStateFunc)pfGetGStateMultiAttr,
  (getMultiStateFunc)pfGetGStateMultiMode,
  (getMultiStateFunc)NULL
};

static void* (*getCombinedMultiAttr[3])(pfGeoState*, uint64_t, int, pfGeoState*) = {
  (getCombinedMultiStateFunc)pfGetGStateCombinedMultiAttr,
  (getCombinedMultiStateFunc)pfGetGStateCombinedMultiMode,
  (getCombinedMultiStateFunc)NULL
};

/* NOTE: PFSTATE_LIGHTS does not fit this paradigm since it is an array
 * of pfLight* rather than a single pointer or value.
*/

/* GState Attrs */
static char *ATTR_STRING[] =
{
  "PFSTATE_FRONTMTL",
  "PFSTATE_TEXTURE",
  "PFSTATE_COLORTABLE",
  "PFSTATE_BACKMTL",
/* FOG is set by pfEarthSky, before the scene geostate is loaded */  
/*  "PFSTATE_FOG", */
  "PFSTATE_LIGHTMODEL",
/*  "PFSTATE_LIGHTS",*/
  "PFSTATE_HIGHLIGHT",
  "PFSTATE_LPOINTSTATE",
  "PFSTATE_TEXGEN",
  "PFSTATE_TEXENV",
/* GState Modes */
  "PFSTATE_ENLIGHTING",
  "PFSTATE_ENTEXTURE",
/* EnFog is turned off in light point geostate */
/*  "PFSTATE_ENFOG", */
  "PFSTATE_ENCOLORTABLE",
  "PFSTATE_SHADEMODEL",
  "PFSTATE_ENWIREFRAME",
  "PFSTATE_ENHIGHLIGHTING",
  "PFSTATE_CULLFACE",
/* Decals cant be optimize because pfLayers depend DECAL mode inherit */
/*  "PFSTATE_DECAL",*/
  "PFSTATE_ENLPOINTSTATE",
  "PFSTATE_TRANSPARENCY",
  "PFSTATE_ANTIALIAS",
  "PFSTATE_ALPHAFUNC",
  "PFSTATE_ENTEXGEN",
/* GState Vals */
  "PFSTATE_ALPHAREF"
};
static uint64_t ATTR_TOKEN[] = 
{
  PFSTATE_FRONTMTL,
  PFSTATE_TEXTURE,
  PFSTATE_COLORTABLE,
  PFSTATE_BACKMTL,
/*  PFSTATE_FOG, */
  PFSTATE_LIGHTMODEL,
/*  PFSTATE_LIGHTS,*/
  PFSTATE_HIGHLIGHT,
  PFSTATE_LPOINTSTATE,
  PFSTATE_TEXGEN,
  PFSTATE_TEXENV,
/* GState Modes */
  PFSTATE_ENLIGHTING,
  PFSTATE_ENTEXTURE,
/*  PFSTATE_ENFOG, */
  PFSTATE_ENCOLORTABLE,
  PFSTATE_SHADEMODEL,
  PFSTATE_ENWIREFRAME,
  PFSTATE_ENHIGHLIGHTING,
  PFSTATE_CULLFACE,
/* Decals cant be optimize because pfLayers depend DECAL mode inherit */
/*  PFSTATE_DECAL,*/
  PFSTATE_ENLPOINTSTATE,
  PFSTATE_TRANSPARENCY,
  PFSTATE_ANTIALIAS,
  PFSTATE_ALPHAFUNC,
  PFSTATE_ENTEXGEN,
/* GState Vals */
  PFSTATE_ALPHAREF
};

static int NATTRS = sizeof(ATTR_TOKEN)/sizeof(uint64_t);

static int ft(int which)
{
  switch(ATTR_TOKEN[which])
    {
/* GState Attrs */
    case PFSTATE_FRONTMTL:
    case PFSTATE_TEXTURE:
    case PFSTATE_TEXENV:
    case PFSTATE_COLORTABLE:
    case PFSTATE_BACKMTL:
    case PFSTATE_FOG:
    case PFSTATE_LIGHTMODEL:
    case PFSTATE_LIGHTS:
    case PFSTATE_HIGHLIGHT:
    case PFSTATE_LPOINTSTATE:
    case PFSTATE_TEXGEN:
      return 0;
/* GState Modes */
    case PFSTATE_ENLIGHTING:
    case PFSTATE_ENTEXTURE:
    case PFSTATE_TRANSPARENCY:
    case PFSTATE_ALPHAFUNC:
    case PFSTATE_ENFOG:
    case PFSTATE_ANTIALIAS:
    case PFSTATE_CULLFACE:
    case PFSTATE_ENCOLORTABLE:
    case PFSTATE_DECAL:
    case PFSTATE_SHADEMODEL:
    case PFSTATE_ENWIREFRAME:
    case PFSTATE_ENHIGHLIGHTING:
    case PFSTATE_ENLPOINTSTATE:
    case PFSTATE_ENTEXGEN:
      return 1;
/* GState Vals */
    case PFSTATE_ALPHAREF:
      return 2;
    default:
      return 0;
    }
}

static int numValues(int which)
{
  int		i;

  switch(ATTR_TOKEN[which])
    {
    case PFSTATE_TEXTURE:
    case PFSTATE_TEXENV:
    case PFSTATE_TEXGEN:
    case PFSTATE_TEXMAT:
    case PFSTATE_TEXLOD:

    case PFSTATE_ENTEXTURE:
    case PFSTATE_ENTEXGEN:
    case PFSTATE_ENTEXLOD:
    case PFSTATE_ENTEXMAT:
	pfQuerySys (PFQSYS_MAX_TEXTURES, &i);
	return i;

    case PFSTATE_FRONTMTL:
    case PFSTATE_COLORTABLE:
    case PFSTATE_BACKMTL:
    case PFSTATE_FOG:
    case PFSTATE_LIGHTMODEL:
    case PFSTATE_LIGHTS:
    case PFSTATE_HIGHLIGHT:
    case PFSTATE_LPOINTSTATE:
    case PFSTATE_ENLIGHTING:
    case PFSTATE_TRANSPARENCY:
    case PFSTATE_ALPHAFUNC:
    case PFSTATE_ENFOG:
    case PFSTATE_ANTIALIAS:
    case PFSTATE_CULLFACE:
    case PFSTATE_ENCOLORTABLE:
    case PFSTATE_DECAL:
    case PFSTATE_SHADEMODEL:
    case PFSTATE_ENWIREFRAME:
    case PFSTATE_ENHIGHLIGHTING:
    case PFSTATE_ENLPOINTSTATE:
    case PFSTATE_ALPHAREF:
      return 1;
    default:
      return 1;
    }
}

/*****************************************************************/

struct _pfdBuilder
{
/* List of Builder with corresponding GeoState Name ExtraState triple */
    pfList*		builders;
    pfList*		nodeNames;
    pfList*		gstates;		
    pfList*		extras;

/*  List of States stored with names both user state and gstates */
    pfList*		namedStates;
    pfList*		stateNames;
    pfList*		namedExtras;

/* State stack for push and pop of  */
    pfList *		stackStates;
    pfList *		stackExtras;

/* Default GeoState, NodeName, and extra State */
    pfGeoState *	basicGState;
    pfList *		basicExtra;

/* Current Builder,GeoState,NodeName, and ExtraState */
    pfdGeoBuilder *	builder;
    pfGeoState *	gstate;
    void *		nodeName;
    pfList *		extra;

/* Cache geostates, extrastates, and names to avoid unnecessary compares */
    int			modeDataDirty;
    int			gstateDirty;
    int			nodeNameDirty;
    int			extraDirty;

/* Keep unique state token count */
    int			nextStateToken;

/* Keep lists of all attributes currently used - for sharing */
    pfList *		extraList;
    pfList *		extensorTypeList;
    pfList *		nameList;
    pfdShare *		share;

/* Builder Modes and Attributes as defined in pfd.h */
    pfList *		modes;
    pfList *		attrs;

/* Generic state attributes */
    pfTexture *		dummyTexture;
    pfClipTexture *	dummyClipTexture;
    pfFog *		dummyFog;
    pfMaterial *	dummyMaterial;
    pfTexEnv *		dummyTEnv;
    pfTexGen *		dummyTGen;
    pfHighlight *	dummyHLight;
    pfColortable *	dummyCtab;
    pfLight *		dummyLight;
    pfLightModel *	dummyLModel;
    pfTexture *		defaultTexture;
    pfClipTexture *	defaultClipTexture;
    pfFog *		defaultFog;
    pfMaterial *	defaultMaterial;
    pfTexEnv *		defaultTEnv;
    pfTexGen *		defaultTGen;
    pfHighlight *	defaultHLight;
    pfColortable *	defaultCtab;
    pfLight *		defaultLight;
    pfLightModel *	defaultLModel;

    void *		arena;
    pfGeoState *	defaultGState;

/* mem management for names */
    char *		default_string;
};

/* GLobal Builder Pointer */
static pfdBuilder *CBldr = NULL;

/******************************************************************/
/*			Utility Functions                         */
/******************************************************************/

/* Utility Function to Find a Extensor Def in a list from a token */
static int 
compareExtensorTypeToken(void *a, void *b)
{
    pfdExtensorType *ea = a;

    if ((a == NULL) || (b == NULL))
	return -1;
    if (ea->token == (long)b)
	return 0;
    return -1;
}

/* Utility Function to Find a Extensor in a list from a token */
static int 
compareExtensorToken(void *a, void *b)
{
    pfdExtensor *ea = a;

    if ((a == NULL) || (b == NULL))
	return -1;
    if (ea->token == (int)(long)b)
	return 0;
    return -1;
}

/* Utility Function to Find a Extensor Def in a list from a name */
static int 
compareExtensorTypeName(void *a,void *b)
{
    pfdExtensorType *ea = a;
    int (*compare)(void *,void *)=
	pfGet(CBldr->attrs,PFDBLDR_EXTENSOR_NAME_COMPARE);

    if ((a == NULL) || (b == NULL))
	return -1;

    return (*compare)(ea->name,b);
}

/* Generic Utility Function to find elt in a list using defined compare */
static int 
findElt(pfList *lst, void *elt, int (compare)(void *,void *))
{
    if (lst != NULL)
    {
	int i,n;
	n = pfGetNum(lst);
	if (compare != NULL)
	{
	    for(i=0; i<n; i++)
		if ((*compare)(pfGet(lst,i),elt) == 0)
		    return i;
	}
	else
	{
	    for(i=0; i<n; i++)
		if (pfGet(lst,i) == elt)
		    return i;
	}
    }

    return -1;
}

/* Generic compare using function or just comparing pointers */
static int 
doCompare(int (*compare)(void *, void *), void *elt1, void *elt2)
{
    if (compare == NULL)
    {
	if (elt1 == elt2)
	    return 0;
	else
	    return -1;
    }
    else 
	return (*compare)(elt1, elt2);
}

static int 
compareLists(pfList *l1, pfList *l2)
{
    if (l1 == l2)
	return  0;
    else 
    if (l1 == NULL) 
	return -1;
    else 
    if (l2 == NULL) 
	return  1;

    {
	int i, n;
	n = pfGetNum(l1);
	if (n != pfGetNum(l2))
	    return -1;
	for(i=0;  i<n; i++)
	    if (pfGet(l1, i) != pfGet(l2, i))
		return -1;
    }
	  
    return 0;
}

/* remove all but first in any set of duplicates in the list, using pfCompare */
static void
removeDuplicates(pfList *l)
{
    int n, i, j;
    if (l == NULL)
	return;
    n = pfGetNum(l);
    for (i = 0; i < n; ++i)
    {
	void *elt_i = pfGet(l,i);
	for (j = i+1; j < n; ++j)
	{
	    void *elt_j = pfGet(l,j);
	    if (pfCompare(elt_i, elt_j) == 0)
	    {
#ifdef DEBUG
		pfNotify(PFNFY_WARN, PFNFY_PRINT, "Removing a duplicate! ref counts = %d, %d", pfGetRef(elt_i), pfGetRef(elt_j));
#endif /* DEBUG */
		pfRef(elt_j);
		pfRemoveIndex(l,j); /* actually neither unrefs nor deletes */
#ifdef DEBUG
		pfNotify(PFNFY_WARN, PFNFY_MORE, "    (now %d, unrefdeleting)", pfGetRef(elt_j));
#endif /* DEBUG */
		pfUnrefDelete(elt_j);
		n--;
		break;
	    }
	}
    }
}

/* Keep a default Material Handy */
static pfMaterial *
defaultMaterial (void)
{
    static pfMaterial   *material       = NULL;
 
    if (material == NULL)
    {        
	/* setup shared arena pointer once */
	if (sharedArena == NULL)
	    sharedArena = pfGetSharedArena();

	material = pfNewMtl(sharedArena);
#ifdef USELESS_REDEFINITION_OF_DEFAULT_MATERIAL
	pfMtlColor(material, PFMTL_AMBIENT,  0.2f, 0.2f, 0.2f);
	pfMtlColor(material, PFMTL_DIFFUSE,  0.8f, 0.8f, 0.8f);
	pfMtlColor(material, PFMTL_SPECULAR, 0.0f, 0.0f, 0.0f);
	pfMtlShininess(material, 0.0f);
#endif
     
#ifdef CONFUSING_DYNAMIC_DEFAULT_COLOR_MODE
        if (pfdGetMesherMode(PFDMESH_SHOW_TSTRIPS))
            pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_AD);
        else
            pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_COLOR);
#else
	pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_AD);
#endif
    }
  
    /* return pointer to default material */
    return material;
}       

static int gstateCompatMode = 0;

/* Configure default geostate which specifies global modes, values, 
 * and attributes which can be inherited by basicGState.
*/
static void
makeDefaultGState (pfGeoState *gs)
{
    /* Turn all modes "off". Attributes are still inherited */
    pfMakeBasicGState(gs);

    /* Explicitly inherit those state attributes which are usually global
     * in nature. However, inherit CULLFACE instead of disabling it so
     * apps can globally enable backface culling for a huge performance gain.
    */
    pfGStateInherit(gs,
		    PFSTATE_ENFOG 		|
		    PFSTATE_ANTIALIAS 		|
		    PFSTATE_SHADEMODEL 		|
		    PFSTATE_ENWIREFRAME 	|
		    PFSTATE_ENHIGHLIGHTING 	|
		    PFSTATE_CULLFACE		|
		    PFSTATE_ALPHAREF);

    /* In 1.2, lighting and texturing are pfEnabled by default */
    if (gstateCompatMode)
    {
        pfGStateMode(gs, PFSTATE_ENLIGHTING, PFTR_ON);
	pfGStateMode(gs, PFSTATE_ENTEXTURE, PFTR_ON);
    }
    /* In 2.0, lighting and texturing are pfDisabled by default */
    else
    {
        pfGStateMode(gs, PFSTATE_ENLIGHTING, PFTR_OFF);
	pfGStateMode(gs, PFSTATE_ENTEXTURE, PFTR_OFF);
    }
}

/* impose the builder's modes on lower-level tools */
static void
pfdUpdateModes (pfdGeoBuilder *b)
{
    pfdMesherMode(PFDMESH_SHOW_TSTRIPS, 
		  (long)pfGet(CBldr->modes, PFDBLDR_MESH_SHOW_TSTRIPS));
    pfdMesherMode(PFDMESH_INDEXED,      
		  (long)pfGet(CBldr->modes, PFDBLDR_MESH_INDEXED));
    pfdMesherMode(PFDMESH_MAX_TRIS,     
		  (long)pfGet(CBldr->modes, PFDBLDR_MESH_MAX_TRIS));
    pfdMesherMode(PFDMESH_RETESSELLATE, 
		  (long)pfGet(CBldr->modes, PFDBLDR_MESH_RETESSELLATE));
    pfdMesherMode(PFDMESH_LOCAL_LIGHTING, 
		  (long)pfGet(CBldr->modes, PFDBLDR_MESH_LOCAL_LIGHTING));

    if (b!=NULL)
    {
    	pfdGeoBldrMode(b, PFDGBLDR_MESH_ENABLE,  
		       (long)pfGet(CBldr->modes, PFDBLDR_MESH_ENABLE));
    	pfdGeoBldrMode(b, PFDGBLDR_AUTO_NORMALS, 
		       (long)pfGet(CBldr->modes, PFDBLDR_AUTO_NORMALS));
    	pfdGeoBldrMode(b, PFDGBLDR_AUTO_COLORS,  
		       (long)pfGet(CBldr->modes, PFDBLDR_AUTO_COLORS));
    	pfdGeoBldrMode(b, PFDGBLDR_AUTO_TEXTURE, 
		       (long)pfGet(CBldr->modes, PFDBLDR_AUTO_TEXTURE));
    	pfdGeoBldrMode(b, PFDGBLDR_AUTO_ORIENT,  
		       (long)pfGet(CBldr->modes, PFDBLDR_AUTO_ORIENT));
    	pfdGeoBldrMode(b, PFDGBLDR_BUILD_LIMIT,  
		       (long)pfGet(CBldr->modes, PFDBLDR_BUILD_LIMIT));
    	pfdGeoBldrMode(b, PFDGBLDR_SHARE_INDEX_LISTS,  
		       (long)pfGet(CBldr->modes, PFDBLDR_SHARE_INDEX_LISTS));
    }
}

/************************************************************/
/*		Template(Dummy) Object Routines             */
/************************************************************/

/* return a new object of type */
PFDUDLLEXPORT pfObject*
pfdNewObject(pfType  *type)
{
    /* setup shared arena pointer once */
    if (sharedArena == NULL)
	sharedArena = pfGetSharedArena();

    if (type == pfGetTexClassType())
	return (pfObject *)pfNewTex(sharedArena);

    if (type == pfGetClipTextureClassType())
	return (pfObject *)pfNewClipTexture(sharedArena);

    if (type == pfGetTEnvClassType())
	return (pfObject *)pfNewTEnv(sharedArena);

    if (type == pfGetTGenClassType())
	return (pfObject *)pfNewTGen(sharedArena);

    if (type == pfGetLightClassType())
	return (pfObject *)pfNewLight(sharedArena);

    if (type == pfGetLModelClassType())
	return (pfObject *)pfNewLModel(sharedArena);

    if (type == pfGetMtlClassType())
	return (pfObject *)pfNewMtl(sharedArena);

    if (type == pfGetHlightClassType())
	return (pfObject *)pfNewHlight(sharedArena);

    if (type == pfGetCtabClassType())
	return (pfObject *)pfNewCtab(256,sharedArena);

    if (type == pfGetFogClassType())
	return (pfObject *)pfNewFog(sharedArena);

    if (type == pfGetListClassType())
	return (pfObject *)pfNewList(sizeof(void *),1,sharedArena);

    pfNotify(PFNFY_WARN, PFNFY_USAGE,
	     "pfdNewObject does not support object of type 0x%x", type);
    return NULL;
}

PFDUDLLEXPORT void
pfdResetAllTemplateObjects(void)
{
    if (CBldr == NULL)
	pfdInitBldr();
    if (CBldr->dummyTexture != NULL)
	pfCopy((pfTexture *)CBldr->dummyTexture,     CBldr->defaultTexture);
    if (CBldr->dummyClipTexture != NULL)
	pfCopy((pfClipTexture *)CBldr->dummyClipTexture,
						     CBldr->defaultClipTexture);
    if (CBldr->dummyTGen != NULL)
	pfCopy((pfTexGen *)CBldr->dummyTGen,	     CBldr->defaultTGen);
    if (CBldr->dummyTEnv != NULL)
	pfCopy((pfTexEnv *)CBldr->dummyTEnv,	     CBldr->defaultTEnv);
    if (CBldr->dummyMaterial != NULL)
	pfCopy((pfMaterial *)CBldr->dummyMaterial,   CBldr->defaultMaterial);
    if (CBldr->dummyHLight != NULL)
	pfCopy((pfHighlight *)CBldr->dummyHLight,    CBldr->defaultHLight);
    if (CBldr->dummyCtab != NULL)
	pfCopy((pfColortable *)CBldr->dummyCtab,     CBldr->defaultCtab);
    if (CBldr->dummyFog != NULL)
	pfCopy((pfFog *)CBldr->dummyFog,	     CBldr->defaultFog);
    if (CBldr->dummyLModel != NULL)
	pfCopy((pfLightModel *)CBldr->dummyLModel,   CBldr->defaultLModel);
    if (CBldr->dummyLight != NULL)
	pfCopy((pfLight *)CBldr->dummyLight,	     CBldr->defaultLight);
}

/* Ask for a reusable default object */
PFDUDLLEXPORT pfObject*
pfdGetTemplateObject(pfType *type)
{
    if (CBldr == NULL)
	pfdInitBldr();

    /* setup shared arena pointer once */
    if (sharedArena == NULL)
	sharedArena = pfGetSharedArena();

    if (type == pfGetTexClassType())
    {
	if (CBldr->dummyTexture == NULL)
	{
	    CBldr->dummyTexture = pfNewTex(sharedArena);
	    CBldr->defaultTexture = pfNewTex(sharedArena);
	}
	return (pfObject *)CBldr->dummyTexture;
    }

    if (type == pfGetClipTextureClassType())
    {
	if (CBldr->dummyClipTexture == NULL)
	{
	    CBldr->dummyClipTexture = pfNewClipTexture(sharedArena);
	    CBldr->defaultClipTexture = pfNewClipTexture(sharedArena);
	}
	return (pfObject *)CBldr->dummyClipTexture;
    }

    if (type == pfGetTGenClassType())
    {
	if (CBldr->dummyTGen == NULL)
	{
	    CBldr->dummyTGen = pfNewTGen(sharedArena);
	    CBldr->defaultTGen = pfNewTGen(sharedArena);
	}
	return (pfObject *)CBldr->dummyTGen;
    }

    if (type == pfGetTEnvClassType())
    {
	if (CBldr->dummyTEnv == NULL)
	{
	    CBldr->dummyTEnv = pfNewTEnv(sharedArena);
	    CBldr->defaultTEnv = pfNewTEnv(sharedArena);
	}
	return (pfObject *)CBldr->dummyTEnv;
    }
    
    if (type == pfGetLightClassType())
    {
	if (CBldr->dummyLight == NULL)
	{
	    CBldr->dummyLight = pfNewLight(sharedArena);
	    CBldr->defaultLight = pfNewLight(sharedArena);
	}
	return (pfObject *)CBldr->dummyLight;
    }

    if (type == pfGetLModelClassType())
    {
	if (CBldr->dummyLModel == NULL)
	{
	    CBldr->dummyLModel = pfNewLModel(sharedArena);
	    CBldr->defaultLModel = pfNewLModel(sharedArena);
	}
	return (pfObject *)CBldr->dummyLModel;
    }

    if (type == pfGetMtlClassType())
    {
	if (CBldr->dummyMaterial == NULL)
	{
	    CBldr->dummyMaterial = pfNewMtl(sharedArena);
	    CBldr->defaultMaterial = pfNewMtl(sharedArena);
	}
	return (pfObject *)CBldr->dummyMaterial;
    }

    if (type == pfGetHlightClassType())
    {
	if (CBldr->dummyHLight == NULL)
	{
	    CBldr->dummyHLight = pfNewHlight(sharedArena);
	    CBldr->defaultHLight = pfNewHlight(sharedArena);
	}
	return (pfObject *)CBldr->dummyHLight;
    }

    if (type == pfGetCtabClassType())
    { 
	if (CBldr->dummyCtab == NULL)
	{
	    CBldr->dummyCtab = pfNewCtab(256,sharedArena);
	    CBldr->defaultCtab = pfNewCtab(256,sharedArena);
	}
	return (pfObject *)CBldr->dummyCtab;
    }

    if (type == pfGetFogClassType())
    {
	if (CBldr->dummyFog == NULL)
	{
	    CBldr->dummyFog = pfNewFog(sharedArena);
	    CBldr->defaultFog = pfNewFog(sharedArena);
	}
	return (pfObject *)CBldr->dummyFog;
    }

    pfNotify(PFNFY_WARN, PFNFY_USAGE,
	     "pfdGetTemplateObject: does not support object of type 0x%x",
	     type);
    return NULL;
}

/* Ask for a reusable default object */
PFDUDLLEXPORT void
pfdResetObject(pfObject *obj)
{
    pfType *type = pfGetType(obj);

    /* Just to force initialization of default object of this type */
    pfdGetTemplateObject(type);

    if (type == pfGetTexClassType())
	pfCopy((pfTexture *)obj,	CBldr->defaultTexture);

    else if (type == pfGetClipTextureClassType())
	pfCopy((pfClipTexture *)obj,	CBldr->defaultClipTexture);

    else if (type == pfGetTEnvClassType())
	pfCopy((pfTexEnv *)obj,		CBldr->defaultTEnv);

    else if (type == pfGetTGenClassType())
	pfCopy((pfTexGen *)obj,		CBldr->defaultTGen);

    else if (type == pfGetLightClassType())
	pfCopy((pfLight *)obj,		CBldr->defaultLight);

    else if (type == pfGetLModelClassType())
	pfCopy((pfLightModel *)obj,	CBldr->defaultLModel);

    else if (type == pfGetMtlClassType())
	pfCopy((pfMaterial *)obj,	CBldr->defaultMaterial);

    else if (type == pfGetHlightClassType())
	pfCopy((pfHighlight *)obj,	CBldr->defaultHLight);

    else if (type == pfGetCtabClassType())
	pfCopy((pfColortable *)obj,	CBldr->defaultCtab);

    else if (type == pfGetFogClassType())
	pfCopy((pfFog *)obj,		CBldr->defaultFog);

    else 
    	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfdResetObject: does not support object of type 0x%x", type);
}

/* Set a reusable default object (by copy)*/
PFDUDLLEXPORT void
pfdMakeDefaultObject(pfObject *obj)
{
    pfType *type = pfGetType(obj);

    /* Just to force initialization of default object of this type */
    pfdGetTemplateObject(type);

    if (type == pfGetTexClassType())
	pfCopy(CBldr->defaultTexture,		(pfTexture *)obj);

    else if (type == pfGetClipTextureClassType())
	pfCopy(CBldr->defaultClipTexture,	(pfClipTexture *)obj);

    else if (type == pfGetTEnvClassType())
	pfCopy(CBldr->defaultTEnv,		(pfTexEnv *)obj);

    else if (type == pfGetTGenClassType())
	pfCopy(CBldr->defaultTGen,		(pfTexGen *)obj);

    else if (type == pfGetLightClassType())
	pfCopy(CBldr->defaultLight,		(pfLight *)obj);

    else if (type == pfGetLModelClassType())
	pfCopy(CBldr->defaultLModel,		(pfLightModel *)obj);

    else if (type == pfGetMtlClassType())
	pfCopy(CBldr->defaultMaterial,		(pfMaterial *)obj);

    else if (type == pfGetHlightClassType())
	pfCopy(CBldr->defaultHLight,		(pfHighlight *)obj);

    else if (type == pfGetCtabClassType())
	pfCopy(CBldr->defaultCtab,		(pfColortable *)obj);

    else if (type == pfGetFogClassType())
	pfCopy(CBldr->defaultFog,		(pfFog *)obj);

    else 
    	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		 "pfdResetObject: does not support object of type 0x%x", type);
}

/******************************************************************/
/*			Builder Construct/Destruct                */
/******************************************************************/
static void SetupDefaultModes(pfdBuilder *b);

/* Create and Initialize a Builder */
PFDUDLLEXPORT pfdBuilder*
pfdNewBldr(void)
{
    pfdBuilder *b = NULL;

    /* setup shared arena pointer once */
    if (sharedArena == NULL)
	sharedArena = pfGetSharedArena();

    b = (pfdBuilder *)pfCalloc(1, sizeof(pfdBuilder), sharedArena);
    b->arena =  sharedArena;

    b->default_string = strdup("default");
    /* Init Bldr/Name/GState/Extra quadtouple lists */
    b->builders = pfNewList(sizeof(pfdGeoBuilder *),1, b->arena);
    b->nodeNames = pfNewList(sizeof(char *),1, b->arena);
    b->gstates = pfNewList(sizeof(pfGeoState *),1, b->arena);
    b->extras = pfNewList(sizeof(pfList *),1, b->arena);

    /* Init Named States */
    b->namedStates = pfNewList(sizeof(pfGeoState *),1, b->arena);
    b->stateNames = pfNewList(sizeof(void *),1, b->arena);
    b->namedExtras = pfNewList(sizeof(pfList *),1, b->arena);

    /* Init State Stacks */
    b->stackStates = pfNewList(sizeof(pfGeoState *),1, b->arena);
    b->stackExtras = pfNewList(sizeof(pfList *),1, b->arena);

    /* Init Share Lists/structures */
    b->extensorTypeList = pfNewList(sizeof(pfdExtensorType *),1, b->arena);
    b->nameList = pfNewList(sizeof(void *),1, b->arena);
    b->extraList = pfNewList(sizeof(pfdExtensor *),1, b->arena);
    b->share = pfdGetGlobalShare();

    /* Init Build Modes and Attributes */
    b->modes = pfNewList(sizeof(long),1, b->arena);
    b->attrs = pfNewList(sizeof(void *),1, b->arena);
    SetupDefaultModes(b);

    /* Init New State Token Starting Token */
    b->nextStateToken = PFDBLDR_START_NEW_STATES;

    /* Everything in basicGState is inherited from defaultGState */
    b->basicGState = pfNewGState(b->arena);
    b->basicExtra = pfNewList(sizeof(pfdExtensor *),1, b->arena);
    b->defaultGState = pfNewGState(b->arena);
    makeDefaultGState(b->defaultGState);

    /* Init Immediate mode State,ExtraState,NodeName */
    b->gstate = pfNewGState(b->arena);
    pfCopy(b->gstate,b->basicGState);
    b->extra = pfNewList(sizeof(pfdExtensor *),1, b->arena);
    pfCopy(b->extra,b->basicExtra);
    b->nodeName = NULL;

    b->dummyMaterial = NULL;
    b->dummyTexture = NULL;
    b->dummyClipTexture = NULL;
    b->dummyFog = NULL;
    b->dummyTEnv = NULL;
    b->dummyTGen = NULL;
    b->dummyHLight = NULL;
    b->dummyCtab = NULL;
    b->dummyLight = NULL;
    b->dummyLModel = NULL;
    b->defaultMaterial = NULL;
    b->defaultTexture = NULL;
    b->defaultClipTexture = NULL;
    b->defaultFog = NULL;
    b->defaultTEnv = NULL;
    b->defaultTGen = NULL;
    b->defaultHLight = NULL;
    b->defaultCtab = NULL;
    b->defaultLight = NULL;
    b->defaultLModel = NULL;
    
    b->gstateDirty = TRUE;
    return b;
}

static void 
SetupDefaultModes(pfdBuilder *b)
{
    pfSet(b->modes, PFDBLDR_PASS_REFERENCES, (void *)0);
    pfSet(b->modes, PFDBLDR_SHARE_MASK,(void *)0xffffffff);
    pfSet(b->modes, PFDBLDR_ATTACH_NODE_NAMES,(void *)1);

    pfSet(b->modes, PFDBLDR_BREAKUP, (void *)0);
    pfSet(b->modes, PFDBLDR_BREAKUP_SIZE, (void *)1000);
    pfSet(b->modes, PFDBLDR_BREAKUP_STRIP_LENGTH, (void *)10);
    pfSet(b->modes, PFDBLDR_BREAKUP_BRANCH, (void *)4);

    pfSet(b->modes, PFDBLDR_AUTO_COLORS,  (void *)PFDBLDR_COLORS_PRESERVE);
    pfSet(b->modes, PFDBLDR_AUTO_NORMALS, (void *)PFDBLDR_NORMALS_MISSING);
    pfSet(b->modes, PFDBLDR_AUTO_TEXTURE, (void *)PFDBLDR_TEXTURE_PRESERVE);
    pfSet(b->modes, PFDBLDR_AUTO_ORIENT,  (void *)PFDBLDR_ORIENT_VERTICES);
    pfSet(b->modes, PFDBLDR_AUTO_CMODE, (void *)1);

    pfSet(b->modes, PFDBLDR_MESH_ENABLE,(void *)1);
    pfSet(b->modes, PFDBLDR_MESH_INDEXED, (void *)0);
    pfSet(b->modes, PFDBLDR_MESH_SHOW_TSTRIPS,(void *)0);
    pfSet(b->modes, PFDBLDR_MESH_LOCAL_LIGHTING,(void *)0);
    pfSet(b->modes, PFDBLDR_MESH_MAX_TRIS,
	  (void *)pfdGetMesherMode(PFDMESH_MAX_TRIS));
    pfSet(b->modes, PFDBLDR_MESH_RETESSELLATE,
	  (void *)pfdGetMesherMode(PFDMESH_RETESSELLATE));

    pfSet(b->modes, PFDBLDR_AUTO_ENABLES, (void *)0);
    pfSet(b->modes, PFDBLDR_PF12_STATE_COMPATIBLE, (void *)0);

    pfSet(b->attrs, PFDBLDR_NODE_NAME_COMPARE, NULL);
    pfSet(b->attrs, PFDBLDR_STATE_NAME_COMPARE, NULL);
    pfSet(b->attrs, PFDBLDR_EXTENSOR_NAME_COMPARE, NULL);
    pfSet(b->modes, PFDBLDR_DESTROY_DATA_UPON_BUILD, (void *)1);
    pfSet(b->modes, PFDBLDR_AUTO_DISABLE_TCOORDS_BY_STATE, (void *) 	1);
    pfSet(b->modes, PFDBLDR_AUTO_DISABLE_NCOORDS_BY_STATE, (void *) 	0);
    pfSet(b->modes, PFDBLDR_AUTO_LIGHTING_STATE_BY_NCOORDS, (void *) 	0);
    pfSet(b->modes, PFDBLDR_AUTO_LIGHTING_STATE_BY_MATERIALS, (void *)	1);
    pfSet(b->modes, PFDBLDR_AUTO_TEXTURE_STATE_BY_TEXTURES, (void *) 	1);
    pfSet(b->modes, PFDBLDR_AUTO_TEXTURE_STATE_BY_TCOORDS, (void *) 	0);
    pfSet(b->modes, PFDBLDR_OPTIMIZE_COUNTS_NULL_ATTRS, (void *)	0);

    pfSet(b->modes, PFDBLDR_GEN_OPENGL_CLAMPED_TEXTURE_COORDS, (void *)	0);
    pfSet(b->modes, PFDBLDR_BUILD_LIMIT, (void *)9999);
    pfSet(b->modes, PFDBLDR_SHARE_INDEX_LISTS, (void *)1);
}

PFDUDLLEXPORT const pfGeoState *
pfdGetDefaultGState(void)
{
    if (CBldr == NULL)
	pfdInitBldr();

    return CBldr->defaultGState;
}

PFDUDLLEXPORT void
pfdDefaultGState(pfGeoState *defGState)
{
    if (CBldr == NULL)
	pfdInitBldr();

    if (defGState != NULL)
    	pfCopy(CBldr->defaultGState, defGState);
}

#define		LIST_PF_OBJECT			1
#define		LIST_POINTER			2
#define		LIST_PF_OBJECT_ARRAY		3
#define		LIST_POINTER_ARRAY		4

static int 
exclusiveAddArray(void **_list, int *_listType, void **_val, int n)
{
    int i, j;
    int	newNum, oldNum;
    int equal;

    if (_list == NULL)
        return -1;

    newNum = (int) _val[0];

    for(i=0; i<n; i++)
    {
	if (_listType[i] == LIST_PF_OBJECT_ARRAY)
	{
	    void 	**item;

	    item = _list[i];
	    oldNum = (int) item[0];

	    if (oldNum == newNum)
	    {
		equal = 1;
		for (j = 1 ; j < newNum ; j ++)
		{
		    pfObject	*new_o, *old_o;

		    new_o = _val[j];
		    old_o = item[j];

		    if (! (new_o == old_o
		     || (pfGetType(old_o) == pfGetType(new_o) && 
			    pfCompare(old_o, new_o) == 0)))
			equal = 0;
		}

		if (equal)
		    return i;
	    }
	}
    }

    _listType[i] = LIST_PF_OBJECT_ARRAY;
    _list[i] = (void *) _val;

    return n;
}

static int 
exclusiveAddPointerArray(void **_list, int *_listType, void **_val, int n)
{
    int i, j;
    int	newNum, oldNum;
    int equal;

    if (_list == NULL)
        return -1;

    newNum = (int) _val[0];

    for(i=0; i<n; i++)
    {
	if (_listType[i] == LIST_POINTER_ARRAY)
	{
	    void 	**item;

	    item = _list[i];
	    oldNum = (int) item[0];

	    if (oldNum == newNum)
	    {
		equal = 1;
		for (j = 1 ; j < newNum ; j ++)
		{
		    void 	*new_o, *old_o;

		    new_o = _val[j];
		    old_o = item[j];

		    if (new_o != old_o)
			equal = 0;
		}

		if (equal)
		    return i;
	    }
	}
    }

    _list[i] = (void *) _val;
    _listType[i] = LIST_POINTER_ARRAY;

    return n;
}

#if 0
static int 
exclusiveAddPointer(void **_list, int *_listType, void *_val, int n)
{
    int i;
    if (_list == NULL)
        return -1;

    for(i=0; i<n; i++)
	if (_listType[i] == LIST_POINTER)
	{
	    if (_list[i] == _val)
		return i;
	}

    _list[i] = _val;
    _listType[i] = LIST_POINTER;

    return n;
}
#endif

static int 
exclusiveAddIntegerPointer(void **_list, int *_listType, int *_val, int n)
{
    int i;
    if (_list == NULL)
        return -1;

    for(i=0; i<n; i++)
	if (_listType[i] == LIST_POINTER)
	{
	    if (*((int *) _list[i]) == *_val)
		return i;
	}

    _list[i] = (void *) _val;
    _listType[i] = LIST_POINTER;

    return n;
}

static int 
exclusiveAddFloatPointer(void **_list, int *_listType, float *_val, int n)
{
    int i;
    if (_list == NULL)
        return -1;

    for(i=0; i<n; i++)
	if (_listType[i] == LIST_POINTER)
	{
	    if (*((float *) _list[i]) == *_val)
		return i;
	}

    _list[i] = (void *) _val;
    _listType[i] = LIST_POINTER;

    return n;
}


static int 
exclusiveAdd(pfObject **_list, int *_listType, pfObject *_obj, int n)
{
    int i;
    if (_list == NULL)
        return -1;

    for(i=0; i<n; i++)
    {
	if (_listType[i] == LIST_PF_OBJECT)
	{
	    pfObject	*o;

	    o = _list[i];
	    if (o == _obj
	     || (pfGetType(o) == pfGetType(_obj) && pfCompare(o, _obj) == 0))
		return i;
	    /* pfCompare will compare the types, but will emit a warning like
	       "texture is not a cliptexture" which is obnoxious */
	}
    }
    _list[i] = _obj;
    _listType[i] = LIST_PF_OBJECT;
    
    return n;
}

PFDUDLLEXPORT void
pfdMakeSceneGState(pfGeoState 		*sceneGState, 
		   pfList 		*gstateList, 
		   const pfGeoState 	*previousGlobalState)
{
    int 	i, j, k, countNull;
    int 	n;
    pfObject 	**attrList;
    int	 	*attrTypeList;
    int	 	*countList, inheritCount;
    int		attrCount;
    uint64_t	globalInherit;

    if (gstateList == NULL)
	return;

    n = pfGetNum(gstateList);
    attrList = (pfObject**) alloca(sizeof(pfObject*) * n);
    attrTypeList = (int*) alloca(sizeof(int) * n);
    countList = (int*) alloca(sizeof(int) * n);

    countNull = (int)pfGet(CBldr->modes, 
			   PFDBLDR_OPTIMIZE_COUNTS_NULL_ATTRS) > 0;

    if (previousGlobalState)
	globalInherit = pfGetGStateInherit(previousGlobalState);
    else
	globalInherit = ~0;	/* everything inherited */

    for(j=0; j<NATTRS; j++)
    {
	long 		max, attrIndex;
	int		f = ft(j);	
	uint64_t	atoken = ATTR_TOKEN[j]; 
	int		num_values = numValues(j);
        int		val;
	
	attrCount = 0;

	inheritCount = 0;
	bzero(countList, n * sizeof(int));

	/* 
	 *	Count number of unique PFSTATE_ elements 
	 *	For each unique element count the number of geostates
	 *	containint it.
	 */
	for(i=0; i<n; i++)
	{	
	    int 		index;
	    void 		*attr;
	    float		*f_attr;
	    int			*i_attr;
	    uint64_t		inherit;
	    pfGeoState 		*cur = pfGet(gstateList, i);
	    void 		**newArray;
	    
	    inherit = pfGetGStateInherit(cur);

	    if (inherit & atoken)
	    {
		/* PFSTATE_ globally inherited but not known */
		if (globalInherit & atoken)
		{
		    inheritCount++;
		    continue;
		}
		else	/* PFSTATE_ globally inherited and known */
		{
		    if (num_values == 1)
		    {
			switch (f)
			{	
			    case 0:
				/* pointers */
				attr = (*getAttr[f]) ((pfGeoState*) 
						previousGlobalState, atoken); 
				break;

			    case 1:
				/* ints */
				i_attr = (int *) alloca (sizeof (int));
				*i_attr = pfGetGStateMode(
					(pfGeoState*) previousGlobalState, 
					atoken);
				break;

			    case 2:
				/* floats */
				f_attr = (float *) alloca (sizeof (float));
				*f_attr = pfGetGStateVal
				    ((pfGeoState*) previousGlobalState, atoken);
				break;
			}
		    }
		    else
		    {
			newArray = (void **) 
			    alloca ((num_values + 1) * sizeof (void *));
			newArray[0] = (void *) num_values;

			for (val = 1 ; val <= num_values ; val ++)
			    newArray[val] = (*getMultiAttr[f]) 
				((pfGeoState*) previousGlobalState, 
				atoken, val - 1); 

			attr = newArray;

			if (f == 2)
			    pfNotify (PFNFY_WARN, PFNFY_PRINT, 
				"pfdMakeSceneGState: Sharing multi-value "
				"pfGeoState values (floats) not supported.");
		    }
		}
	    }
	    else	/* PFSTATE_ locally set */
	    {
		if (num_values == 1)
		{
		    switch (f)
		    {
			case 0:
			    /* pointers */
			    attr = (*getAttr[f]) (cur, atoken); 
			    break;

			case 1:
			    /* ints */
			    i_attr = (int *) alloca (sizeof (int));
			    *i_attr = pfGetGStateMode(cur, atoken);
			    break;

			case 2:
			    /* floats */
			    f_attr = (float *) alloca (sizeof (float));
			    *f_attr = pfGetGStateVal(cur, atoken);
			    break;
		    }
		}
		else
		{
		    newArray = (void **) 
			    alloca ((num_values + 1) * sizeof (void *));
		    newArray[0] = (void *) num_values;

		    for (val = 1 ; val <= num_values ; val ++)
			newArray[val] = (*getMultiAttr[f])
					    (cur, atoken, val - 1);

		    attr = newArray;

		    if (f == 2)
			pfNotify (PFNFY_WARN, PFNFY_PRINT, 
			    "pfdMakeSceneGState: Sharing multi-value "
			    "pfGeoState values (floats) not supported.");
		}
	    }

	    if (num_values <= 1)
	    {
		switch (f)
		{
		    case 0:
			index = exclusiveAdd(attrList, attrTypeList, 
					    attr, attrCount);
			break;
		    case 1:
			index = exclusiveAddIntegerPointer(
					    attrList, attrTypeList, 
					    i_attr, attrCount);
			break;
		    case 2:
			index = exclusiveAddFloatPointer(
					    attrList, attrTypeList, 
					    f_attr, attrCount);
			break;
		}
	    }
	    else
	    {
		if (f == 0) /* pfGeoSet attributes - pointers to pfObjects */
		    index = exclusiveAddArray
			    ((void **)attrList, attrTypeList, 
					    (void **) attr, attrCount);
		else
		    index = exclusiveAddPointerArray
			    ((void**)attrList, attrTypeList, 
					    (void **)attr, attrCount);
	    }

	    countList[index]++;
	    attrCount = PF_MAX2(attrCount, index+1);
	}

	/* 
	 *	For each geostate element type (e.g. fog) find the element 
	 *	most used across all geostates (e.g. find the most used pfFog
	 *	pointer).
	 */ 
	max = 0;
	attrIndex = -1;
	for(i=0; i<attrCount; i++)
	{
	    if (max < countList[i])
	    {
		max = countList[i];
		attrIndex = i;
	    }
	}

	/* inherit state element by default */
	pfGStateInherit(sceneGState, atoken);

	/* It's OK to inherit PFSTATE_ATTRS since they require separate 
	 * enabling but MODES and VALUES can not be made global if any
	 * geostate inherits them.
	*/
	if ((f == 0 && inheritCount > max) ||
	    (f != 0 && inheritCount > 0))
	{
	    pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
		     "%6d inheritance%s   of %s", 
		     inheritCount,
		     (inheritCount < 2) ? " " : "s", 
		     ATTR_STRING[j]); 

	}
	else if (attrIndex >= 0)
	{
	    /* Explicitly set state element for scene to inherit */
	    if (num_values <= 1)
	    {
		float *f_attr = attrList[attrIndex];
		int   *i_attr = attrList[attrIndex];

		switch (f)
		{
		    case 0:
			(*setAttr[f])(sceneGState, atoken, attrList[attrIndex]);
			break;
		    case 1:
			pfGStateMode(sceneGState, atoken, *i_attr);
			break;
		    case 2:
			pfGStateVal(sceneGState, atoken, *f_attr);
			break;
		}
	    }
	    else
	    {
		void  **attr = (void **) (attrList[attrIndex]);
		int  num = (int) (attr[0]);

		for (k = 1 ; k <= num ; k ++)
		{
		    void	*attr_value;

		    attr_value = attr[k];

		    /*
		     *	Avoid setting zero attributes because they may 
		     *	trigger a warning message if current hardware 
		     *	doesn't support this many units (e.g. texture-units).
		     */
		    if (k)
			(*setMultiAttr[f])(sceneGState, atoken, 
					   k - 1, attr_value);
		}
	    }
		

	    if (atoken == PFSTATE_TEXTURE && attrList[attrIndex] != NULL)
	    {
		pfTexture 	*texture;
		char 		*name;
		int  		num;

		if (num_values == 1)
		{
		    texture = (pfTexture *) attrList[attrIndex];

		    if (texture)
			name = (char *)pfGetTexName(texture);
		    else
			name = NULL;

		    pfNotify(PFNFY_DEBUG, PFNFY_MORE,
			 "%6d reference%s     to %s \"%s\"",
			 countList[attrIndex],
			 (countList[attrIndex] < 2) ? " " : "s",
			 ATTR_STRING[j],
			 (name == NULL) ? "<UNNAMED>" : name);
		}
		else
		{
		    num = (int) (((void **) (attrList[attrIndex]))[0]);
		    for (k = 0 ; k < num ; k ++)
		    {
			texture = (pfTexture *) 
				    (((void **) (attrList[attrIndex]))[k + 1]);

			if (texture)
			    name = (char *)pfGetTexName(texture);
			else
			    name = NULL;

			pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
			     "%6d reference%s     to %s \"%s\"", 
			     countList[attrIndex], 
			     (countList[attrIndex] < 2) ? " " : "s", 
			     ATTR_STRING[j], 
			     (name == NULL) ? "<UNNAMED>" : name);
		    }
		}
	    }
	    else
	    {
		if (f == 0)
		    pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
			     "%6d reference%s     to %s 0x%p", 
			     countList[attrIndex],
			     (countList[attrIndex] < 2) ? " " : "s", 
			     ATTR_STRING[j], 
			     (void*)attrList[attrIndex]);
		else if (f == 1)
		    pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
			     "%6d specification%s of %s %d", 
			     countList[attrIndex],
			     (countList[attrIndex] < 2) ? " " : "s", 
			     ATTR_STRING[j], 
			     (int)attrList[attrIndex]);
		else if (f == 2)
		    pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
			     "%6d specification%s of %s %f", 
			     countList[attrIndex],
			     (countList[attrIndex] < 2) ? " " : "s", 
			     ATTR_STRING[j], 
			     *(float*) &attrList[attrIndex]);
	    }
	}
    }
}

PFDUDLLEXPORT void
pfdOptimizeGStateList(pfList 		*gstateList, 
		      const pfGeoState 	*prevGlobalGState, 
		      const pfGeoState 	*newGlobalGState)
{
    int 	i, j, n;
    uint64_t	prevInherit, newInherit;

    if (gstateList == NULL)
	return;

    if (prevGlobalGState)
	prevInherit = pfGetGStateInherit(prevGlobalGState);
    else
	prevInherit = ~0;

    if (newGlobalGState)
	newInherit = pfGetGStateInherit(newGlobalGState);
    else
	newInherit = ~0;

    n = pfGetNum(gstateList);
    for(j=0;  j<NATTRS; j++)
    {
	int		f = ft(j);	
	uint64_t	atoken = ATTR_TOKEN[j];
	int		num_values = numValues(j);
	int		val;

    	for(i=0;  i<n; i++)
	{
	    pfGeoState 	*curGS = pfGet(gstateList,i);

	    /*
	     *	Yair: For multi-value elements, must compare all elements
	     *	      before setting gstate inheritance.
	     */
	    if (num_values > 1) 
	    {
		/* Flatten out previous Inheritance */
		if ((pfGetGStateInherit(curGS) & atoken) && 
		    !(prevInherit & atoken))
		{
		    for (val = 0 ; val < num_values ; val ++)
			(*setMultiAttr[f]) (curGS, atoken, val, 
				  (*getCombinedMultiAttr[f])(curGS, 
					atoken, val, 
					(pfGeoState*) prevGlobalGState));
		}

		/* Create new Inheritance */
		if (!(newInherit & atoken))
		{
		    int		same_values;

		    same_values = 1;
		    for (val = 0 ; val < num_values ; val ++)
		    {
			if ((*getMultiAttr[f]) (curGS, atoken, val) !=
			    (*getMultiAttr[f]) ((pfGeoState*) newGlobalGState, 
						atoken, val))
			    same_values = 0;
		    }

		    if (same_values)
			pfGStateInherit(curGS, atoken);
		}
	    }
	    else
	    {

		/* Flatten out previous Inheritance */
		if ((pfGetGStateInherit(curGS) & atoken) && 
		    !(prevInherit & atoken))
		{
		    (*setAttr[f]) (curGS, atoken,
			      (*getCombinedAttr[f])(curGS, 
				    atoken, (pfGeoState*) prevGlobalGState));
		}

		/* Create new Inheritance */
		if (!(newInherit & atoken) && 
		    (*getAttr[f]) (curGS, atoken) == 
		    (*getAttr[f]) ((pfGeoState*) newGlobalGState, atoken))
		{
		    pfGStateInherit(curGS, atoken);
		}
	    }
      	}
    }
}

PFDUDLLEXPORT pfGeoState *
pfdOptimizeAllBldrGStates(void)
{
    pfdMakeSceneGState(CBldr->defaultGState, 
		       CBldr->gstates, CBldr->defaultGState);
    pfdOptimizeGStateList(CBldr->gstates, 
			  CBldr->defaultGState, CBldr->defaultGState);

    return CBldr->defaultGState;
}


/* Delete a builder */
PFDUDLLEXPORT void 
pfdDelBldr(pfdBuilder *b)
{
    free(b->default_string);
    if (b->builder != NULL)
	pfdDelGeoBldr(b->builder);
    pfDelete(b->builders);
    pfDelete(b->nodeNames);
    pfDelete(b->gstates);
    pfDelete(b->extras);

    pfDelete(b->stateNames);
    pfDelete(b->namedStates);
    pfDelete(b->namedExtras);

    pfDelete(b->stackStates);
    pfDelete(b->stackExtras);

    pfDelete(b->extraList);
    pfDelete(b->extensorTypeList);
    pfDelete(b->nameList);

    pfDelete(b->modes);
    pfDelete(b->attrs);
    /* pfdDelShare(b->share,FALSE);	share now global */

    if (CBldr->dummyMaterial)
    	pfFree(CBldr->dummyMaterial);
    if (CBldr->dummyTexture)
    	pfFree(CBldr->dummyTexture);
    if (CBldr->dummyClipTexture)
    	pfFree(CBldr->dummyClipTexture);
    if (CBldr->dummyFog)
    	pfFree(CBldr->dummyFog);
    if (CBldr->dummyTEnv)
    	pfFree(CBldr->dummyTEnv);
    if (CBldr->dummyTGen)
    	pfFree(CBldr->dummyTGen);
    if (CBldr->dummyHLight)
    	pfFree(CBldr->dummyHLight);
    if (CBldr->dummyCtab)
    	pfFree(CBldr->dummyCtab);
    if (CBldr->dummyLight)
    	pfFree(CBldr->dummyLight);
    if (CBldr->dummyLModel)
	pfFree(CBldr->dummyLModel);

    if (CBldr->defaultMaterial)
    	pfFree(CBldr->defaultMaterial);
    if (CBldr->defaultTexture)
    	pfFree(CBldr->defaultTexture);
    if (CBldr->defaultClipTexture)
    	pfFree(CBldr->defaultClipTexture);
    if (CBldr->defaultFog)
    	pfFree(CBldr->defaultFog);
    if (CBldr->defaultTEnv)
    	pfFree(CBldr->defaultTEnv);
    if (CBldr->defaultTGen)
    	pfFree(CBldr->defaultTGen);
    if (CBldr->defaultHLight)
    	pfFree(CBldr->defaultHLight);
    if (CBldr->defaultCtab)
    	pfFree(CBldr->defaultCtab);
    if (CBldr->defaultLight)
    	pfFree(CBldr->defaultLight);
    if (CBldr->defaultLModel)
    	pfFree(CBldr->defaultLModel);
    
    pfFree(b);
}

/****************************************************************/
/*		Initialize/Set/Get/Reset Global Builder         */
/****************************************************************/

/* Return the current Global Builder */
PFDUDLLEXPORT pfdBuilder*
pfdGetCurBldr(void)
{
    return CBldr;
}

/* Set the current Global Builder */
PFDUDLLEXPORT void 
pfdSelectBldr(pfdBuilder *bldr)
{
    if (bldr != NULL)
	CBldr = bldr;
}

/* Init Global Builder - Call this at start of load of each file */
PFDUDLLEXPORT void 
pfdInitBldr(void)
{
    if (CBldr == NULL)
	CBldr = pfdNewBldr();
    else
    	pfdResetBldrGeometry();
    pfdSelectBldrName(CBldr->default_string);
}

/* Stop using Builder - also deletes the global builder */
PFDUDLLEXPORT void 
pfdExitBldr(void)
{
/*    pfdDeleteBldr(CBldr);*/
    CBldr = NULL;
}
PFDUDLLEXPORT void
pfdFreeBuilderBucket(int i)
{
    int n;

    if (CBldr->builders != NULL)
    {
	n = pfGetNum(CBldr->builders);
	if ((i>=0) && (i<n))
	{
#ifdef	DEBUG
	    pfNotify(PFNFY_DEBUG,PFNFY_PRINT,
		"Reseting Builder Bucket[%d]", i);
#endif
	    pfdDelGeoBldr(pfGet(CBldr->builders, i));
	    pfRemoveIndex(CBldr->builders, i);
	    pfRemoveIndex(CBldr->extras, i);
	    pfRemoveIndex(CBldr->gstates, i);
	    pfRemoveIndex(CBldr->nodeNames, i);
	}
    }
}

/* Reset Global Builder Geometry - generally called from pfdInit */
PFDUDLLEXPORT void 
pfdResetBldrGeometry(void)
{
    void *arena;
    int i,n;
    if (CBldr == NULL)
	pfdInitBldr();
    arena  = CBldr->arena;

    if (CBldr->builders != NULL)
    {
	n = pfGetNum(CBldr->builders);
	for(i=n-1; i >= 0; i--)
	    pfdFreeBuilderBucket(i);
	pfDelete(CBldr->builders);
    }
    pfDelete(CBldr->nodeNames);
    pfDelete(CBldr->extras);
    pfDelete(CBldr->gstates);

    for (i = pfGetNum(CBldr->extraList)-1; i >= 0; i--)
        pfDelete(pfGet(CBldr->extraList, i));
    pfDelete(CBldr->extraList);

    CBldr->builders = pfNewList(sizeof(pfdGeoBuilder *),1,arena);
    CBldr->nodeNames = pfNewList(sizeof(void *),1,arena);
    CBldr->extras = pfNewList(sizeof(pfList *),1,arena);
    CBldr->gstates = pfNewList(sizeof(pfGeoState *),1,arena);
    CBldr->extraList = pfNewList(sizeof(pfList *),1,CBldr->arena);

    pfdUpdate();
}

/* Reset Global Builder Sharing - Advanced use only - sharing is a good idea */
PFDUDLLEXPORT void 
pfdResetBldrShare(void)
{
    if (CBldr == NULL)
	pfdInitBldr();

    pfdDelShare(CBldr->share, FALSE);
    CBldr->share = pfdGetGlobalShare();
}

/******************************************************************/
/*			Manipulate Builder State                  */
/******************************************************************/

/* Set the Default state - so that reset will return to this state */
PFDUDLLEXPORT void 
pfdCaptureDefaultBldrState(void)
{
    if (CBldr == NULL)
	pfdInitBldr();

    pfCopy(CBldr->basicGState, CBldr->gstate);
    pfdCopyExtraStates(CBldr->basicExtra, CBldr->extra);
}

/* Make Default state current */
PFDUDLLEXPORT void 
pfdResetBldrState(void)
{
    if (CBldr == NULL)
	pfdInitBldr();

    pfCopy(CBldr->gstate, CBldr->basicGState);
    pfdCopyExtraStates(CBldr->extra, CBldr->basicExtra);
    CBldr->modeDataDirty = TRUE;
    CBldr->gstateDirty = TRUE;
    CBldr->extraDirty = TRUE;
}

/* Push the Current state onto a stack */
PFDUDLLEXPORT void 
pfdPushBldrState(void)
{
    pfGeoState *n;
    pfList *lst;
    int count = 32;

    if (CBldr == NULL)
	pfdInitBldr();

    n = pfNewGState(CBldr->arena);
    if (CBldr->extra != NULL)
	count = pfGetNum(CBldr->extra);
    lst = pfNewList(sizeof(pfList *),count,CBldr->arena);
    pfCopy(n,CBldr->gstate);
    pfdCopyExtraStates(lst,CBldr->extra);
    pfInsert(CBldr->stackStates, 0, n);
    pfInsert(CBldr->stackExtras, 0, lst);
}

/* Pop the State stack and make the popped state current */
PFDUDLLEXPORT void 
pfdPopBldrState(void)
{
    int n;
    pfGeoState *gs;
    pfList *es;

    if (CBldr == NULL)
	pfdInitBldr();

    if (CBldr->stackStates != NULL && 
       (n = pfGetNum(CBldr->stackStates)) > 0)
    {
	gs = pfGet(CBldr->stackStates,0);
	es = pfGet(CBldr->stackExtras,0);
	pfCopy(CBldr->gstate, gs);
	pfdCopyExtraStates(CBldr->extra, es);
	pfRemove(CBldr->stackStates, gs);
	pfRemove(CBldr->stackExtras, es);
	pfDelete(gs);
	pfDelete(es);
	CBldr->modeDataDirty = TRUE;
	CBldr->gstateDirty = TRUE;
	CBldr->extraDirty = TRUE;
    }
    else
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "Builder: Attempt to pop empty state stack");
}

/* Store the current state under a name - useful for making mtl libraries */
PFDUDLLEXPORT void 
pfdSaveBldrState(void *name)
{
    pfGeoState *gs;
    pfList *es;
    int n;
    int (*compare)(void *,void *);
    void *arena;
    int count = 32;

    if (CBldr == NULL)
	pfdInitBldr();
    arena = CBldr->arena;

    compare = pfGet(CBldr->attrs,PFDBLDR_STATE_NAME_COMPARE);

    n = findElt(CBldr->stateNames, name, compare);
    if (n > -1)
    {
	gs = pfGet(CBldr->namedStates,n);
	es = pfGet(CBldr->namedExtras,n);
    }
    else
    {
    	gs = pfNewGState(arena);
    	if (CBldr->extra != NULL)
	    count = pfGetNum(CBldr->extra);
    	es = pfNewList(sizeof(pfdExtensor *),count,arena);
    }
    pfCopy(gs,CBldr->gstate);
    pfdCopyExtraStates(es,CBldr->extra);
    if (n < 0)
    {
    	pfAdd(CBldr->stateNames,name);
    	pfAdd(CBldr->namedStates,gs);
    	pfAdd(CBldr->namedExtras,es);
    }
}

/* Load the named state if it exists - use mtl "steel" */
PFDUDLLEXPORT void 
pfdLoadBldrState(void *name)
{
    int n;
    int (*compare)(void *,void *);
    if (CBldr == NULL)
	pfdInitBldr();
    compare = pfGet(CBldr->attrs,PFDBLDR_STATE_NAME_COMPARE);

    n = findElt(CBldr->stateNames, name, compare);
    if (n > -1)
    {
	pfCopy(CBldr->gstate, pfGet(CBldr->namedStates,n));
	pfdCopyExtraStates(CBldr->extra, pfGet(CBldr->namedExtras,n));
    }
    else
	pfdResetBldrState();
    CBldr->modeDataDirty = TRUE;
    CBldr->gstateDirty = TRUE;
    CBldr->extraDirty = TRUE;
}

/* Copy gstate to be the current gstate */
PFDUDLLEXPORT void 
pfdBldrGState(const pfGeoState *gstate)
{
    if (CBldr == NULL)
	pfdInitBldr();
    pfCopy(CBldr->gstate,(pfGeoState *)gstate);
    CBldr->gstateDirty = TRUE;
}

/* Return a pointer to current gstate */
PFDUDLLEXPORT const pfGeoState*
pfdGetBldrGState(void)
{
    if (CBldr == NULL)
	pfdInitBldr();
    return CBldr->gstate;
}

/*****************************************************************/
/*		Set State modes,attributes,inherit               */
/*		Notice same api for setting state for            */
/*		both external user-defined state and             */
/*		for internal Performer gstates                   */
/*****************************************************************/

/* Set State Inherit Mask */
/*  XXX - should there be inherit for user-defined state ? */
PFDUDLLEXPORT void 
pfdBldrStateInherit(uint64_t mask)
{
    if (CBldr == NULL)
	pfdInitBldr();
    CBldr->gstateDirty = TRUE;
    pfGStateInherit(CBldr->gstate, mask);
}
PFDUDLLEXPORT void
pfdBldrStateVal(uint64_t which, float val)
{
    if (CBldr == NULL)
	pfdInitBldr();
    if ((which & PFDBLDR_START_NEW_STATES) == 0)
    {
	CBldr->gstateDirty = TRUE;
	pfGStateVal(CBldr->gstate, which, val);
    }
}

/* Set State Mode to val */
PFDUDLLEXPORT void 
pfdBldrStateMode(uint64_t which, int val)
{
    if (CBldr == NULL)
	pfdInitBldr();

    if ((which & PFDBLDR_START_NEW_STATES) == 0)
    {
	CBldr->gstateDirty = TRUE;
	pfGStateMode(CBldr->gstate, which, val);
    }
    else
    {
	pfdExtensor *eng = pfdGetExtensor(which);
	CBldr->extraDirty = TRUE;
	eng->mode = val;
    }	
}

/* Set State Attribute to a
   Note that when setting Performer state, a is always passed by value (copied) 
   This is also the case of External Extensor state as long as both functions 
   copy and compare are defined in the Extensor Definition 
   Note however that extra states are copied by value lazily - ie when final 
   state definitions are needed - normal gstate attribues do not act like this
   to provide a generic interface to gstates that need not be changed 
   when the definition of a Performer gstate changes */
PFDUDLLEXPORT void 
pfdBldrStateAttr(uint64_t which, const void *a)
{
    void *attr = (void *)a;
    if (CBldr == NULL)
	pfdInitBldr();
    
    if (which < PFDBLDR_START_NEW_STATES)
    {
	if (which & (int)(long)pfGet(CBldr->modes,PFDBLDR_SHARE_MASK))
	{
	    attr = (void *)pfdNewSharedObject(CBldr->share, (pfObject *)a);
	    if (which == PFSTATE_TEXTURE)
	    {
		unsigned int *img;
		int comp,sx,sy,sz;
		if ((attr != NULL)&&(pfGetType(attr) == pfGetTexClassType()))
		{
		    pfGetTexImage((pfTexture *)attr, &img, &comp, &sx, &sy, &sz);
		    if ((long)pfGet(CBldr->modes,PFDBLDR_AUTO_TEXTURE_STATE_BY_TEXTURES))
			if ((img == NULL) || (comp < 1) || (sx < 1) || (sy < 1))
			{
			    pfNotify(PFNFY_INFO,PFNFY_PRINT,
				     "Auto Disabling Texture because of bogus texture");
			    pfdBldrStateMode(PFSTATE_ENTEXTURE,PF_OFF);
			    attr = NULL;
			}
			else
			    pfdBldrStateMode(PFSTATE_ENTEXTURE,PF_ON);
		}
	    }
	    if (which == PFSTATE_FRONTMTL)
	    {
		if ((long)pfGet(CBldr->modes,PFDBLDR_AUTO_LIGHTING_STATE_BY_MATERIALS)>0)
		{
		    if ((attr == NULL) && 
			(pfdGetBldrStateMode(PFSTATE_ENLIGHTING) == PF_ON))
		    {
			pfNotify(PFNFY_DEBUG,PFNFY_PRINT,
				 "pfdBuilder: Auto Disabling lighting (NULL) material");
			pfdBldrStateMode(PFSTATE_ENLIGHTING,PF_OFF);
		    }
		    else if ((attr != NULL)  && 
			     (pfdGetBldrStateMode(PFSTATE_ENLIGHTING) == PF_OFF))
		    {
			pfNotify(PFNFY_DEBUG,PFNFY_PRINT,
				 "Auto Enabling lighting because of non-null material use");
			pfdBldrStateMode(PFSTATE_ENLIGHTING,PF_ON);
		    }
		}
	    }
	}
	else
	{
	    attr = pfdNewObject(pfGetType(a)); 
	    pfCopy(attr,(void *)a);
	}
	if ((long)pfGet(CBldr->modes, PFDBLDR_PASS_REFERENCES) == 0)
	    pfGStateAttr(CBldr->gstate, which, attr);
	else
	    pfGStateAttr(CBldr->gstate, which, (void*)a);

	CBldr->gstateDirty = TRUE;
    }
    else
    {
	pfdExtensor *eng = pfdGetExtensor(which);
	eng->data = (void *)a;

	CBldr->extraDirty = TRUE;
    }
}

PFDUDLLEXPORT void 
pfdCopyExtensor(pfdExtensor *dst, pfdExtensor *src)
{
    dst->token = src->token;
    dst->data = src->data;
    dst->mode = src->mode;
    dst->extensorType = src->extensorType;
}

PFDUDLLEXPORT void 
pfdCopyExtraStates(pfList *dst, pfList *src)
{
    int i,n;

    if (dst == NULL)
	return;

    pfResetList(dst);

    if (src == NULL)
	return;

    n = pfGetNum(src);
    for(i=0; i<n; i++)
    {
	pfdExtensor *eng = pfGet(src,i);

	if (eng->mode != 0)
	{
	    void *attr = NULL;
	    int result;
	    pfdExtensor *newEng = pfdNewExtensor(eng->token);
	    pfdExtensorType *eDef = eng->extensorType;

	    pfdCopyExtensor(newEng,eng);

	    if (eDef->share)
		attr = pfdUniqifyData(eDef->shareList,
				       eng->data, eDef->dataSize,
				       NULL, eDef->compare, eDef->copy, &result);
	    else if (eng->data)
	    {
		attr = pfMalloc(eDef->dataSize,CBldr->arena);
		if (eDef->initialize)
		    (*eDef->initialize)(attr);
		if (eDef->copy)
		    (*eDef->copy)(attr,eng->data);
		else
		    bcopy(eng->data,attr,eDef->dataSize);
   
	    }
	    newEng->data = attr;

	    pfAdd(dst,newEng);
	}
    }
}

/* Return appropriate state Inheritance */
/* XXX - no inheritence for external extensor state yet */
PFDUDLLEXPORT uint64_t
pfdGetBldrStateInherit(void)
{
    if (CBldr == NULL)
	pfdInitBldr();
    return pfGetGStateInherit(CBldr->gstate);
}

/* Return appropriate state val */
PFDUDLLEXPORT float
pfdGetBldrStateVal(uint64_t which)
{
    if (CBldr == NULL)
	pfdInitBldr();
    if ((which & PFDBLDR_START_NEW_STATES) == 0)
    {
 	return pfGetGStateVal(CBldr->gstate, which);
    }
}

/* Return appropriate state mode */
PFDUDLLEXPORT int 
pfdGetBldrStateMode(uint64_t mode)
{
    if (CBldr == NULL)
	pfdInitBldr();
    if (mode < PFDBLDR_START_NEW_STATES)
	return pfGetGStateMode(CBldr->gstate, mode);
    else
    {
	pfdExtensor *eng = pfdGetExtensor(mode);
	return eng->mode;
    }	
}

/* Return internal or external state attr */
PFDUDLLEXPORT const void*
pfdGetBldrStateAttr(uint64_t which)
{
    if (CBldr == NULL)
	pfdInitBldr();
    if (which < PFDBLDR_START_NEW_STATES)
	return pfGetGStateAttr(CBldr->gstate, which);
    else
    {
	pfdExtensor *eng = pfdGetExtensor(which);
	return eng->data;
    }
}




/**************************************************************/
/*		Arbitrary Uniqify and Match Routines          */
/*		Also Sharing for Lists                        */
/**************************************************************/

/* Match Arbitary data against list of data via compare routine */
/* Note make sure to provide compare to avoid mere pointer compare */
PFDUDLLEXPORT void*
pfdMatchData(pfList *dataList,
	     void *data,
	     int (*compare)(void *,void *))
{
    int i,n;

    if ((dataList == NULL) || (data == NULL))
	return NULL;
    n = pfGetNum(dataList);
    for(i=0; i<n; i++)
	if (compare)
	{
	    if ((*compare)(data,pfGet(dataList,i)) == 0)
		return pfGet(dataList,i);
	}
        else
	    if (pfGet(dataList,i) == data)
		return data;
    return NULL;

}

/* Uniqify Arbitrary Data */
/* Make sure to provide all fields including compare and copy */
PFDUDLLEXPORT void*
pfdUniqifyData(pfList *dataList,
		      const void *data,
		      long dataSize,
		      void *(*newData)(long),
		      int (*compare)(void *,void *),
		      long (*copy)(void *dst, void *src),
		      int *result)
{
    void *udata;

    *result = -1;
    if (data == NULL)
	return NULL;
    udata = pfdMatchData(dataList,(void *)data,compare);

    *result = 0;
    if (udata != NULL)
	return udata;

    *result = 1;
    if (newData == NULL)
	udata = pfMalloc(dataSize,CBldr->arena);
    else
	udata = (*newData)(dataSize);
    if (copy != NULL)
	(*copy)(udata,(void *)data);
    else
	bcopy((void *)data,udata,dataSize);
    pfAdd(dataList,udata);
    return udata;
}

/**********************************************************************/
/*		Compare Routines for Lists of Extensors, Extensors    */
/**********************************************************************/

PFDUDLLEXPORT int 
pfdCompareExtensor(void *a, void *b)
{
    pfdExtensor *ea = a;
    pfdExtensor *eb = b;

    if (ea == eb)
	return 0;
    if ((ea == NULL) || (eb == NULL))
	return -1;
    if (ea->token != eb->token)
	return -1;
    if (ea->mode != eb->mode)
	return -1;
    if ((*ea->extensorType->compare)(ea->data,eb->data) != 0)
	return -1;
    return 0;
}

PFDUDLLEXPORT int 
pfdCompareExtraStates(void *lista, void *listb)
{
    int i,n;
    pfList *a = lista;
    pfList *b = listb;

    if (a == b)
	return 0;
    else
    if (a == NULL)
	return -1;
    else
    if (b == NULL)
	return  1;

    n = pfGetNum(a);
    for(i=0; i<n; i++)
    {
	pfdExtensor *ea = pfGet(a,i);
	int elt_n = findElt(b,(void *)ea->token,compareExtensorToken);
	
	if (elt_n > 0)
	    if (pfdCompareExtensor(ea,pfGet(b,elt_n)) != 0)
		return -1;
	if ((ea->mode != 0)  || 
	    (ea->data != NULL))
	    return -1;
    }
    n = pfGetNum(b);
    for(i=0; i<n; i++)
    {
	pfdExtensor *eb = pfGet(b,i);
	int elt_n = findElt(a,(void *)eb->token,compareExtensorToken);
	
	if (elt_n > 0)
	    if (pfdCompareExtensor(eb,pfGet(a,elt_n)) != 0)
		return -1;
	if ((eb->mode != 0)  || 
	    (eb->data != NULL))
	    return -1;
    }
    return 0;
}

/*********************************************************************/
/*		Set/Get Name of Geode to hold added prims            */
/*********************************************************************/
PFDUDLLEXPORT void 
pfdSelectBldrName(void *name)
{
    int i;
    int (*compare)(void *,void *);
    if (CBldr == NULL)
	pfdInitBldr();
    compare = pfGet(CBldr->attrs,PFDBLDR_NODE_NAME_COMPARE);

    CBldr->nodeName = name;
    if (findElt(CBldr->nameList, name, compare) == -1)
	pfAdd(CBldr->nameList, name);
    CBldr->nodeNameDirty = TRUE;
}

PFDUDLLEXPORT void*
pfdGetCurBldrName(void)
{
    if (CBldr == NULL)
	pfdInitBldr();
    return CBldr->nodeName;
}

/***********************************************************************/
/*			Build Functions                                */
/*		These generate and retrieve Performer structures       */
/*		holding all the data that was input into the builder   */
/*		Build Mode Functions - Specify how Building of         */
/*		Performer data structures should occur                 */
/*		See pfd.h for thorough listing of Modes and Attributes */
/***********************************************************************/

/* Set Build Mode */
PFDUDLLEXPORT void 
pfdBldrMode(int mode, int val)
{
    if (CBldr == NULL)
	pfdInitBldr();

    pfSet(CBldr->modes, mode, (void *)val);

    if (mode == PFDBLDR_PF12_STATE_COMPATIBLE)
    {
	gstateCompatMode = mode;
	makeDefaultGState(CBldr->defaultGState);
    }
    pfdUpdateModes(NULL);
    CBldr->modeDataDirty = TRUE;
}

/* Get Build Mode */
PFDUDLLEXPORT int 
pfdGetBldrMode(int mode)
{
    if (CBldr == NULL)
	pfdInitBldr();
    return (int)(long)pfGet(CBldr->modes, mode);
}

/* Set Build Attribute */
PFDUDLLEXPORT void 
pfdBldrAttr(int which, void *attr)
{
    if (CBldr == NULL)
	pfdInitBldr();
    pfSet(CBldr->attrs, which, attr);
    CBldr->modeDataDirty = TRUE;
}

/* Get Build Attrubute */
PFDUDLLEXPORT void *
pfdGetBldrAttr(int which)
{
    if (CBldr == NULL)
	pfdInitBldr();
    return pfGet(CBldr->attrs, which);
}

static pfMaterial*
getWhiteMtl(void)
{
    static pfMaterial *mtl = NULL;

    if (!mtl)
    {
	mtl = pfNewMtl(pfGetSharedArena());
	pfMtlColorMode(mtl, PFMTL_BOTH, PFMTL_CMODE_OFF);
    }
    return mtl;
}

PFDUDLLEXPORT pfNode*
pfdBuildNthNode(int j)
{
    int 		k, nGSets, jj, q;
    const pfList 	*gsets;
    pfGeode 		*gd;
    pfNode 		*nd;
    pfdGeoBuilder 	*b;
    pfGeoState 		*curGState;

    /* building geometry for builder j */
    if ((b = pfGet(CBldr->builders,  j)) == NULL)
	return NULL;

    /* make sure that modes are up-to-date */
    pfdUpdateModes(b);

    /* generate geosets from builder contents */
    if ((gsets = pfdBuildGSets(b)) == NULL)
	return NULL;

    /* bail out if there are no geosets */
    if (pfGetNum(gsets) < 1)
	return NULL;

    pfQuerySys(PFQSYS_GL, &q);

    /*
     *  note: one or more geosets in gset list at this point
     */

    /* make new geode for geometry */
    gd = pfNewGeode();

    /* set geode's name */
    if ((long)pfGet(CBldr->modes, PFDBLDR_ATTACH_NODE_NAMES))
	pfNodeName(gd, (char *)pfGet(CBldr->nameList, j));

    /* get complete geostate for this builder */
    curGState = pfGet(CBldr->gstates, j);

    /* reduce geostate to differences from default state */
#ifdef __linux__  /* XXX (taf) HACK: Avoiding call to pfGStateGetVal() */
    for (jj = 0; jj < NATTRS-1; jj++)
#else
    for (jj = 0; jj < NATTRS; jj++)
#endif
    {
	int	f = ft(jj);
	uint64_t atoken = ATTR_TOKEN[jj];

	if ((*getAttr[f])(curGState, atoken) == 
	    (*getCombinedAttr[f])(CBldr->defaultGState, atoken, NULL))
	  pfGStateInherit(curGState, atoken);
    }

    /* bind reduced geostate to each geoset */
    nGSets = pfGetNum(gsets);
    for(k = 0; k < nGSets; k++)
    {
	pfGeoSet *gsp = pfGet(gsets, k);

	/* !!!!! WARNING !!!!! 
            All the AUTO stuff which follows 
	    can do the wrong thing if nGSets > 1, e.g., if one gset
	    has texcoords and another does not then the auto texture 
	    logic may do the wrong thing.
	   !!!!! WARNING !!!!! 
       */
	

	/*--------------- TEXCOORD CLAMPING ----------------------*/

	if ((long)pfGet(CBldr->modes, 
			PFDBLDR_GEN_OPENGL_CLAMPED_TEXTURE_COORDS))
	{
	    int 	rs, rt, comp, sx, sy, sz, nprims, cnt, ii, jj, *lengths;
	    float 	inv, multS, multT, addS, addT;
	    uint 	*img;
	    ushort 	*itc;
	    void 	*tc;
	    pfVec2	*curTC, *ptc;
	    pfTexture 	*tex = pfGetGStateAttr(curGState, PFSTATE_TEXTURE);

	    if (tex == NULL)
		break;
	    rs = pfGetTexRepeat(tex, PFTEX_WRAP_S);
	    rt = pfGetTexRepeat(tex, PFTEX_WRAP_T);
	    if ((q != PFGL_OPENGL) || ((rs != PFTEX_CLAMP) && (rt != PFTEX_CLAMP)))
		break;
	    pfNotify(PFNFY_INFO, PFNFY_PRINT, 
		     "Generating Texture Clamped coords for gset");
	    pfGetTexImage(tex, &img, &comp, &sx, &sy, &sz);
	    inv = 1.0f/(float)sx;
	    multS = (sx - 1.0f)*inv;
	    addS = 0.5f * inv;
	    inv = 1.0f/(float)sy;
	    multT = (sy - 1.0f)*inv;
	    addT = 0.5f * inv;
	    pfGetGSetAttrLists(gsp, PFGS_TEXCOORD2, &tc, &itc);
	    lengths = pfGetGSetPrimLengths(gsp);
   	    nprims = pfGetGSetNumPrims(gsp);
	    cnt = 0;
	    ptc = (pfVec2 *)tc;
	    if (tc == NULL)
		break;
	    for(jj=0; jj<nprims;jj++)
		for(ii=0; ii<lengths[jj];ii++)
		{
		    if (itc != NULL)
		    	curTC = &ptc[itc[cnt++]];
		    else 
			curTC = &ptc[cnt++];
		    if (rs == PFTEX_CLAMP)
		    	(*curTC)[PF_S] = (*curTC)[PF_S]*multS + addS;
		    if (rt == PFTEX_CLAMP)
		    	(*curTC)[PF_T] = (*curTC)[PF_T]*multT + addT;
		}
	}

	/*--------------- AUTO LIGHTING ----------------------*/

        if ((pfGetGStateCombinedMode(curGState, 
	     PFSTATE_ENLIGHTING, CBldr->defaultGState) == PF_OFF)  && 
	    (pfGetGSetAttrBind(gsp, PFGS_NORMAL3) != PFGS_OFF))
	{
	    if ((long)pfGet(CBldr->modes, 
			    PFDBLDR_AUTO_DISABLE_NCOORDS_BY_STATE))
	    {
		void 	*nlist;
		ushort 	*ilist;

		pfGetGSetAttrLists(gsp, PFGS_NORMAL3, &nlist, &ilist);
		pfGSetAttr(gsp, PFGS_NORMAL3, PFGS_OFF, NULL, NULL);
		if (nlist != NULL)
		    pfDelete(nlist);
		if (ilist != NULL)
		    pfDelete(ilist);
		pfNotify(PFNFY_INFO, PFNFY_PRINT, 
			 "Auto Disabling normals because of lighting is off");
	    }
	    else if ((long)pfGet(CBldr->modes, 
				 PFDBLDR_AUTO_LIGHTING_STATE_BY_NCOORDS))
	    {
		pfGStateMode(curGState, PFSTATE_ENLIGHTING, PF_ON);
	    }
	}
        else if ((pfGetGStateCombinedMode(curGState, 
		  PFSTATE_ENLIGHTING, CBldr->defaultGState) == PF_ON)  && 
		 (pfGetGSetAttrBind(gsp, PFGS_NORMAL3) == PFGS_OFF))
	{
	    if ((long)pfGet(CBldr->modes, 
			    PFDBLDR_AUTO_LIGHTING_STATE_BY_NCOORDS))
		pfGStateMode(curGState, PFSTATE_ENLIGHTING, PF_OFF);
	}

	/*--------------- AUTO TEXTURE ----------------------*/

        if ((pfGetGStateCombinedMode(curGState, 	
	     PFSTATE_ENTEXTURE, CBldr->defaultGState) == PF_OFF) && 
	    (pfGetGSetAttrBind(gsp, PFGS_TEXCOORD2) != PFGS_OFF))
	{
	    if ((long)pfGet(CBldr->modes, 
			    PFDBLDR_AUTO_DISABLE_TCOORDS_BY_STATE))
	    {
		void 	*tlist;
		ushort 	*ilist;

		pfGetGSetAttrLists(gsp, PFGS_TEXCOORD2, &tlist, &ilist);
		pfGSetAttr(gsp, PFGS_TEXCOORD2, PFGS_OFF, NULL, NULL);
		if (tlist != NULL)
		    pfDelete(tlist);
		if (ilist != NULL)
		    pfDelete(ilist);
		pfNotify(PFNFY_INFO, PFNFY_PRINT, 
			 "Auto Disabling Texture coords because texture is off");
	    }
	    else if ((long)pfGet(CBldr->modes, 
				 PFDBLDR_AUTO_TEXTURE_STATE_BY_TCOORDS))
	    {
		pfGStateMode(curGState, PFSTATE_ENTEXTURE, PF_ON);
	    }
	}
        else if ((pfGetGStateCombinedMode(curGState, 
		  PFSTATE_ENTEXTURE, CBldr->defaultGState) == PF_ON) && 
	    (pfGetGSetAttrBind(gsp, PFGS_TEXCOORD2) == PFGS_OFF))
	{
	    if ((long)pfGet(CBldr->modes, 
			    PFDBLDR_AUTO_TEXTURE_STATE_BY_TCOORDS))
		pfGStateMode(curGState, PFSTATE_ENTEXTURE, PF_OFF);
	}

	/*--------------- AUTO CMODE ----------------------*/

	if ((long)pfGet(CBldr->modes, PFDBLDR_AUTO_CMODE) > 0)
	{
	    pfVec3		*coords;
	    pfVec4		*colors;
	    ushort		*index;
	    int			ntris;
	    pfMaterial 		*mtl = NULL;
	    int			cmode=PFMTL_CMODE_OFF;

	    if (curGState != NULL)
		mtl = pfGetGStateAttr(curGState, PFSTATE_FRONTMTL);

	    switch (pfGetGSetAttrBind(gsp, PFGS_COLOR4)) {
	    case PFGS_OFF:

		pfQueryGSet(gsp, PFQGSET_NUM_VERTS, &ntris);

		/* Assign a white CMODE_OFF material to geoset without
		  color. However, make sure geoset is
		 big enough to warrant the extra material switch. */
		if (curGState && (!mtl ||
		    pfGetMtlColorMode(mtl, PFMTL_FRONT) != PFMTL_CMODE_OFF) &&
		    ntris > 20)
		{
		    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
			     "pfdBuilder: GeoSet with no color. Using "
			     "white material.");
		    pfGStateAttr(curGState, PFSTATE_FRONTMTL, getWhiteMtl());
		}
		/* Add a white color and assume gset uses default material
		    with CMODE_AD enabled */
		else
		{
		    pfGetGSetAttrLists(gsp, PFGS_COORD3, 
				       (void**)&coords, &index);

		    colors = pfMalloc(sizeof(pfVec4), pfGetArena(gsp));
		    pfSetVec4(colors[0], .8f, .8f, .8f, 1.0f); 
		    if (index)
		    {
			index = pfMalloc(sizeof(ushort), pfGetArena(gsp));
			*index = 0;
			pfGSetAttr(gsp, PFGS_COLOR4, PFGS_OVERALL,
				   colors, index);
		    }
		    else
			pfGSetAttr(gsp, PFGS_COLOR4, PFGS_OVERALL,
				   colors, NULL);
		}
		break;
	    case PFGS_OVERALL:

		pfQueryGSet(gsp, PFQGSET_NUM_VERTS, &ntris);

		/* Create a CMODE_OFF material whose diffuse color is 
		 the overall geoset color. However, make sure geoset is
		 big enough to warrant the extra material switch. */
		if (curGState && (!mtl || 
		   ((cmode = pfGetMtlColorMode(mtl, PFMTL_FRONT)) != PFMTL_CMODE_OFF)) &&
		   ntris > 20)
		{
		    pfMaterial	*newmtl;
		    float r, g, b, a;
		    pfGetGSetAttrLists(gsp, PFGS_COLOR4, 
				       (void**)&colors, &index);

		    newmtl = pfNewMtl(pfGetArena(curGState));
		    pfCopy(newmtl, mtl);
		    pfMtlColorMode(newmtl, PFMTL_BOTH, PFMTL_CMODE_OFF);
		    if (index)
		    {
			r = colors[index[0]][0];
			g = colors[index[0]][1];
			b = colors[index[0]][2];
		    }
		    else
		    {
			r = colors[0][0];
			g = colors[0][1];
			b = colors[0][2];
		    }
		    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
				 "pfdBuilder: Flattening OVERALL color "
				 "(%f %f %f) into "
				 "new CMODE_OFF pfMaterial", 
				 r, g, b);
		    switch(cmode)
		    {
		    case PFMTL_CMODE_DIFFUSE:
			pfMtlColor(newmtl, PFMTL_DIFFUSE, r, g, b);
			break;
		    case PFMTL_CMODE_AMBIENT:
			pfMtlColor(newmtl, PFMTL_AMBIENT, r, g, b);
			break;
		    case PFMTL_CMODE_AMBIENT_AND_DIFFUSE:
			pfMtlColor(newmtl, PFMTL_AMBIENT, r, g, b);
			pfMtlColor(newmtl, PFMTL_DIFFUSE, r, g, b);
			break;
		    case PFMTL_CMODE_EMISSION:
			pfMtlColor(newmtl, PFMTL_EMISSION, r, g, b);
			break;
		    case PFMTL_CMODE_SPECULAR:
			pfMtlColor(newmtl, PFMTL_SPECULAR, r, g, b);
			break;
		    case PFMTL_CMODE_OFF:
		    default:
			break;
		    }
		    
		    pfGStateAttr(curGState, PFSTATE_FRONTMTL, newmtl);

		}
		/* else if mtl is NULL then assume we pick up a default 
		 * material which enables CMODE_AD */
		break;
	    case PFGS_PER_VERTEX:
	    case PFGS_PER_PRIM:
		/* Make sure our material enables CMODE so vertex/prim
		 colors have effect */
		if (curGState && mtl &&
		    pfGetMtlColorMode(mtl, PFMTL_FRONT) == PFMTL_CMODE_OFF)
		{
		    pfMaterial	*newmtl;

		    newmtl = pfNewMtl(pfGetArena(curGState));
		    pfCopy(newmtl, mtl);
		    pfMtlColorMode(newmtl, PFMTL_BOTH, PFMTL_CMODE_AD);
		    pfGStateAttr(curGState, PFSTATE_FRONTMTL, newmtl);

		    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
			     "pfdBuilder: Creating new CMODE_AD material");
		}
		/* else if mtl is NULL then assume we pick up a default 
		 * material which enables CMODE_AD */
		break;
	    }
   	}
	if (curGState != NULL)
  	{
	    pfMaterial *material = pfGetGStateAttr(curGState, 
						   PFSTATE_FRONTMTL);
	    if ((material != NULL) && 
		((long)pfGet(CBldr->modes, PFDBLDR_MESH_SHOW_TSTRIPS) > 0)  && 
		(pfGetMtlColorMode(material, PFMTL_FRONT) != PFMTL_CMODE_AD))
	    {
		pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_AD);
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
		"pfdBuilder: ColorMode of mtl -> CMODE_AD to view tstrips\n");
	    }
	}	
	/*
	 * XXX HACK HACK HACK
	 *     curGState is a shared object, and we have just modified
	 *     it, which is illegal.
	 *     To avoid a memory leak, check the share list again
	 *     to see whether there is a different (older)
	 *     geostate with the modified values; if so,
	 *     use it instead.
	 *     At the end of pfdBuild,
	 *     all but the first (oldest) geostate
	 *     will be deleted from the list.
	 */
	{
	pfGeoState *prevCurGState = curGState;
	curGState = (pfGeoState *)pfdNewSharedObject(CBldr->share, 
						     (pfObject *)curGState);
	if (curGState != prevCurGState)
	    pfUnref(prevCurGState); /* makes no sense, just make it work */
	}

	pfGSetGState(gsp, curGState);
	pfAddGSet(gd, gsp);
    }
    nd = (pfNode *)gd;

    /* breakup geode as specified */
    if ((long)pfGet(CBldr->modes, PFDBLDR_BREAKUP))
    {
	pfNode *brokeup = pfdBreakup(gd, 
	   (long)pfGet(CBldr->modes, PFDBLDR_BREAKUP_SIZE), 
	   (long)pfGet(CBldr->modes, PFDBLDR_BREAKUP_STRIP_LENGTH), 
	   (long)pfGet(CBldr->modes, PFDBLDR_BREAKUP_BRANCH));

	if (brokeup != NULL)
	    nd = brokeup;
	else
	    nd = (pfNode *)gd;

	pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "BREAKING UP GSETS");
    }
		
    /* attach extra state information */
    if ((pfGet(CBldr->extras, j) !=  NULL)  && 
	(pfGetNum(pfGet(CBldr->extras, j)) > 0))
    {
	pfList *es = pfGet(CBldr->extras, j);

	pfNodeTravData(nd, PFTRAV_CULL, es);
	pfNodeTravFuncs(nd, PFTRAV_CULL, 
	    (pfNodeTravFuncType)pfdPreCullCBack, 
	    (pfNodeTravFuncType)pfdPostCullCBack);

	pfNodeTravData(nd, PFTRAV_DRAW, es);
	pfNodeTravFuncs(nd, PFTRAV_DRAW, 
	    (pfNodeTravFuncType)pfdPreDrawCBack, 
	    (pfNodeTravFuncType)pfdPostDrawCBack);
    }

    /* return resulting node to caller */
    return nd;
}

/* Get All Geometry Back */
/* Default is a geode, but if breakup is turned on, or multiple */
/* node names have been used then this routine returns a group */
PFDUDLLEXPORT pfNode*
pfdBuild(void)
{
    int i,n;
    pfNode *nd;
    pfGroup *grp;
    int (*compare)(void *,void *) = 
	pfGet(CBldr->attrs,PFDBLDR_NODE_NAME_COMPARE);

    /* create group to parent nodes being built */
    grp = pfNewGroup();

    /* for each named collection of geometry... */
    n = pfGetNum(CBldr->nameList);
    for(i=0; i<n; i++)
    {
	int j, nBuilders;
	void *curName = pfGet(CBldr->nameList, i);
	
	/* ...for each builder in the list... */
	nBuilders = pfGetNum(CBldr->builders);
	for(j=0; j<nBuilders; j++)
	{
	    /* if this builder's name matches the current target */
	    if (doCompare(compare, curName, pfGet(CBldr->nodeNames, j)) == 0)
	    {
		/* add this node to the overall parent group */
		if ((nd = pfdBuildNthNode(j)) != NULL)
		    pfAddChild(grp, nd);
	    }
	}
    }

    /*
     * XXX HACK HACK HACK
     *     pfdBuildNthNode() illegally changes geostates,
     *     resulting in possible duplicates on the share list.
     *     However, if it changes the values to something
     *     occurring eairlier on the share list, it uses that
     *     instead.   So there can be multiple geostates
     *     on the share list that look the same, but only
     *     the oldest will be used.  Clean up here by
     *     deleting all but the oldest in any set of duplicates.
     *     (This is also below in pfdBuildNode()).
     */
    removeDuplicates(pfdGetSharedList(CBldr->share,
				      pfGetGStateClassType()));

    /* check for no geometry in builders */
    if (pfGetNumChildren(grp) < 1)
    {
	pfDelete(grp);
	return NULL;
    }
    else 
    /* don't need group if only one child */
    if (pfGetNumChildren(grp) == 1)
    {
	nd = pfGetChild(grp,0);
	pfRemoveChild(grp,nd);
	pfDelete(grp);
	return nd;
    }

    /* clean up builders if so desired */
    if ((long)pfGet(CBldr->modes,PFDBLDR_DESTROY_DATA_UPON_BUILD))
	pfdResetBldrGeometry();

    return (pfNode *)grp;
}

/* Return all the Performer structures loaded while NodeName was set to name */
PFDUDLLEXPORT pfNode*
pfdBuildNode(void *name)
{
    int j,n;
    int nBuilders;
    pfNode *nd;
    pfGroup *grp;
    int (*compare)(void *,void *) = 
	pfGet(CBldr->attrs,PFDBLDR_NODE_NAME_COMPARE);

    /* create group to parent nodes being built */
    grp = pfNewGroup();

    /* for each builder in the list... */
    nBuilders = pfGetNum(CBldr->builders);
    for(j=nBuilders-1; j>=0; j--) /* backwards since we're deleting */
    {
	/* if this builder's name matches the given name */
	if (doCompare(compare, name, pfGet(CBldr->nodeNames, j)) == 0)
	{
	    /* add this node to the overall parent group */
	    if ((nd = pfdBuildNthNode(j)) != NULL)
		pfAddChild(grp,nd);

	    /* clean up builder if so desired */
    	    if ((long)pfGet(CBldr->modes,PFDBLDR_DESTROY_DATA_UPON_BUILD))
		pfdFreeBuilderBucket(j);
	}
    }

    /*
     * XXX see note in pfdBuild() above
     */
    if (pfGetNum(CBldr->builders) == 0)
	removeDuplicates(pfdGetSharedList(CBldr->share,
					  pfGetGStateClassType()));

    /* check for no geometry in builders */
    if (pfGetNumChildren(grp) < 1)
    {
	pfDelete(grp);
	return NULL;
    }
    else 
    /* don't need group if only one child */
    if (pfGetNumChildren(grp) == 1)
    {
	nd = pfGetChild(grp,0);
	pfRemoveChild(grp,nd);
	pfDelete(grp);
	return nd;
    }
	    
    return (pfNode *)grp;
}


/**********************************************************************/
/*			Database retrieval functions                  */
/*			ie output a Performer database                */
/*			One poly at a time...                         */
/**********************************************************************/
	
/* XXX Load an entire scene graph back into the builder */
/* Not yet! */
PFDUDLLEXPORT void 
pfdLoadSceneGraph(pfNode *nd)
{
}

PFDUDLLEXPORT pfdPrim*
pfdGetNextBldrPrim(void)
{
}

/**********************************************************************/
/*			Update Builder                                */
/*	       Force the builder to reconsider state                  */
/*		and name appropriately                                */
/**********************************************************************/

/* Force an update of state and decide on the current builder */
static void 
pfdUpdate(void)
{
    pfGeoState *gs;
    pfList *es;
    int i, n;
    int (*compare)(void *,void *) = 
	pfGet(CBldr->attrs,PFDBLDR_NODE_NAME_COMPARE);

    n = pfGetNum(CBldr->builders);
    
    if (CBldr->gstateDirty || (n == 0))
	gs = (pfGeoState *)pfdNewSharedObject(CBldr->share, 
					    (pfObject *)CBldr->gstate);
    else
	gs = pfGet(CBldr->gstates, n-1);

    if (CBldr->extraDirty)
    {
	es = pfNewList(sizeof(pfdExtensor *),1,CBldr->arena);
	pfdCopyExtraStates(es, CBldr->extra);
	pfAdd(CBldr->extraList, es);
    }
    else
	es = pfGet(CBldr->extras, n-1);

    for(i=0; i<n; i++)
	if ((pfCompare(gs, pfGet(CBldr->gstates, i)) == 0)  && 
	    (doCompare(compare,CBldr->nodeName,pfGet(CBldr->nodeNames,i)) == 0) && 
	    (compareLists(es, pfGet(CBldr->extras, i)) == 0))
	{
	    CBldr->builder = pfGet(CBldr->builders, i);
	    break;
	}
    if (i >= n)
    {
	CBldr->builder = pfdNewGeoBldr();
	pfAdd(CBldr->builders, CBldr->builder);
	pfAdd(CBldr->gstates, gs);
	pfAdd(CBldr->extras, es);
	pfAdd(CBldr->nodeNames, CBldr->nodeName);
    }

    /* update geobuilder modes */
    pfdUpdateModes(CBldr->builder);

    CBldr->modeDataDirty = FALSE;
    CBldr->gstateDirty = FALSE;
    CBldr->nodeNameDirty = FALSE;
    CBldr->extraDirty = FALSE;
}

/**********************************************************************/
/*			Add Prim Routine                              */
/*		converts a primitive (ie line, triangle,polygon)      */
/*		to Performer data structure and remembers the current */
/*		graphics state                                        */
/**********************************************************************/

PFDUDLLEXPORT void 
pfdAddBldrGeom(pfdGeom *p, int n)
{
    if (CBldr->gstateDirty   || 
	CBldr->nodeNameDirty || 
	CBldr->extraDirty    || 
	CBldr->modeDataDirty )
	pfdUpdate();

    /* reset internal pfdGeom flags */
    p->flags = 0;

    pfdAddGeom(CBldr->builder, p, n);
}

PFDUDLLEXPORT void 
pfdAddIndexedBldrGeom(pfdGeom *p, int n)
{
    if (CBldr->gstateDirty   || 
	CBldr->nodeNameDirty || 
	CBldr->extraDirty    || 
	CBldr->modeDataDirty )
	pfdUpdate();

    /* reset internal pfdGeom flags */
    p->flags = PFD_INDEXED;

    pfdAddGeom(CBldr->builder, p, n);
}

PFDUDLLEXPORT int 
pfdPreDrawCBack(pfTraverser *trav, void *data)
{
    return pfdCallback(trav,data,PFDEXT_DRAW_PREFUNC);
}

PFDUDLLEXPORT int 
pfdPostDrawCBack(pfTraverser *trav, void *data)
{
    return pfdCallback(trav,data,PFDEXT_DRAW_POSTFUNC);
}

PFDUDLLEXPORT int 
pfdPreCullCBack(pfTraverser *trav, void *data)
{
    return pfdCallback(trav,data,PFDEXT_CULL_PREFUNC);
}

PFDUDLLEXPORT int 
pfdPostCullCBack(pfTraverser *trav, void *data)
{
    return pfdCallback(trav,data,PFDEXT_CULL_POSTFUNC);
}

PFDUDLLEXPORT int 
pfdCallback(pfTraverser *trav, void *data,int which)
{
    pfList *extraList = data;
    if (extraList != NULL)
    {
	int i,n;
	n = pfGetNum(extraList);
	for(i=0; i<n; i++)
	{
	    pfdExtensor *eng = pfGet(extraList,i);
	    pfNodeTravFuncType cback = pfGet(eng->extensorType->callbacks,which);
	    if (eng->mode == 0)
		pfNotify(PFNFY_WARN,PFNFY_PRINT,"Callback for disabled engine");
	    if ((eng->mode > 0)  && 
		(cback != NULL))
		(*cback)(trav,eng);
	}
    }
    return PFTRAV_CONT;
}

/**********************************************************************/
/*			ExtraState routines                           */
/*		These are used to add functionality to Performer      */
/*		and Performer nodes                                   */
/**********************************************************************/
PFDUDLLEXPORT int
pfdGetUniqueStateToken(void)
{
    return CBldr->nextStateToken++;
}

PFDUDLLEXPORT int
pfdAddState(void *name,
	     long dataSize,
	     void (*initialize)(void *data),
	     void (*deletor)(void *data),
	     int (*compare)(void *data1, void *data2),
	     long (*copy)(void *dst, void *src),
	     int token)
{
    pfdExtensorType *eng;

    if (token < PFDBLDR_START_NEW_STATES)
	eng = pfdNewExtensorType(pfdGetUniqueStateToken());
    else
	eng = pfdNewExtensorType(token);
    eng->dataSize = dataSize;
    eng->initialize = initialize;
    eng->deletor = deletor;
    eng->compare = compare;
    eng->copy = copy;
    eng->name = name;

    return eng->token;
}

PFDUDLLEXPORT pfdExtensorType*
pfdNewExtensorType(int token)
{
    int i;
    pfdExtensorType *eng = pfMalloc(sizeof(pfdExtensorType),CBldr->arena);

    eng->dataSize = 0;
    eng->initialize = NULL;
    eng->deletor = NULL;
    eng->compare = NULL;
    eng->copy = NULL;
    eng->share = 0;
    if (token < PFDBLDR_START_NEW_STATES)
	eng->token = token;
    else
	eng->token = pfdGetUniqueStateToken();

    /* setup shared arena pointer once */
    if (sharedArena == NULL)
	sharedArena = pfGetSharedArena();

    eng->callbacks = 
	pfNewList(sizeof(pfNodeTravFuncType),1, sharedArena);

    pfSet(eng->callbacks, PFDEXT_APP_FUNC,NULL);
    pfSet(eng->callbacks, PFDEXT_CULL_PREFUNC,NULL);
    pfSet(eng->callbacks, PFDEXT_CULL_POSTFUNC,NULL);
    pfSet(eng->callbacks, PFDEXT_DRAW_PREFUNC,NULL);
    pfSet(eng->callbacks, PFDEXT_DRAW_POSTFUNC ,NULL);
    pfSet(eng->callbacks, PFDEXT_GSTATE_PREFUNC,NULL);
    pfSet(eng->callbacks, PFDEXT_GSTATE_POSTFUNC,NULL);
    pfSet(eng->callbacks, PFDEXT_GSET_PREFUNC,NULL);
    pfSet(eng->callbacks, PFDEXT_GSET_POSTFUNC, NULL);
    pfAdd(CBldr->extensorTypeList,eng);
    return eng;
}

/**********************************************************************/
/*		Defining External state via callbacks                 */
/*		The beginning of "extensors"                          */
/*		5:16am, October 20, 1994                              */
/**********************************************************************/

PFDUDLLEXPORT pfNodeTravFuncType
pfdGetStateCallback(int stateToken, int which)
{
    pfdExtensorType *edef = pfdGetExtensorType(stateToken);
    if (edef != NULL)
	return pfGet(edef->callbacks,which);
    return NULL;
}

PFDUDLLEXPORT void
pfdStateCallback(int cback, int which, pfNodeTravFuncType callback)
{
    pfdExtensorType *eng;
    eng = pfdGetExtensorType(cback);
    if (eng == NULL)
	return;
    pfSet(eng->callbacks,which,(void *)callback);
}

PFDUDLLEXPORT int 
pfdGetStateToken(void *name)
{
    pfdExtensorType *eng;
	int which = findElt(CBldr->extensorTypeList,
		 name,
		 compareExtensorTypeName);
    eng = (pfdExtensorType *)pfGet(CBldr->extensorTypeList,which);
    if (eng)
	return eng->token;
    else
	return -1;
}

/* Create a new extensor of type which */
PFDUDLLEXPORT pfdExtensor*
pfdNewExtensor(int which)
{
    pfdExtensor *n = pfMalloc(sizeof(pfdExtensor),CBldr->arena);
    n->token = which;
    n->extensorType = pfdGetExtensorType(which);
    n->data = NULL;
    n->mode = 0;
    return n;
}

PFDUDLLEXPORT pfdExtensor*
pfdGetExtensor(int token)
{
    pfdExtensor *extensor;
    int which = findElt(CBldr->extra,(void *)token,compareExtensorToken);
    if (which < 0)
    {
	if (CBldr->extra == NULL)
	    CBldr->extra =pfNewList(sizeof(pfdExtensor *),1,CBldr->arena);
	extensor = pfdNewExtensor(token);
	pfAdd(CBldr->extra,extensor);
    }
    else
	extensor = pfGet(CBldr->extra,which);
    return extensor;
}

PFDUDLLEXPORT pfdExtensorType*
pfdGetExtensorType(int token)
{
    pfdExtensorType *e;
    int which = findElt(CBldr->extensorTypeList,
			 (void *)token,compareExtensorTypeToken);
    if (which < 0)
	return NULL;
    return pfGet(CBldr->extensorTypeList, which);
}

PFDUDLLEXPORT void
pfdCleanBldrShare(void)
{
    if (CBldr == NULL)
	pfdInitBldr();
    pfdCleanShare(CBldr->share);
}

PFDUDLLEXPORT pfdShare *
pfdGetBldrShare(void)
{
    if (CBldr == NULL)
	pfdInitBldr();
    return(CBldr->share);
}

PFDUDLLEXPORT void
pfdSetBldrShare(pfdShare *share)
{
    if (CBldr == NULL)
	pfdInitBldr();
    CBldr->share = share;
}

PFDUDLLEXPORT void
pfdBldrDeleteNode(pfNode *node)
{
    int i,j,n;
    pfList *gsList;

    if (node == NULL)
	return;
    if (CBldr == NULL)
	pfdInitBldr();
    gsList = pfdGetNodeGStateList(node);

    pfDelete(node);

    if (gsList)    
	n = pfGetNum(gsList);
    else
	n = 0;

    for(j=0; j<n; j++)
    	for(i=0; i<NATTRS; i++)
	    if (ft(i) == 0)
	    	pfdRemoveSharedObject(
				      CBldr->share,
				      (*getAttr[0]) (
						     pfGet(gsList,j),
						     ATTR_TOKEN[i]
						     )
				      );
}
