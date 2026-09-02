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
 * pfdShare.c
 *
 * $Revision: 1.48 $
 * $Date: 2002/11/21 03:21:20 $
 *
 * Utilities to generate lists of shared state attributes, and to then
 * use those lists to optimize an IRIS Performer scene graph for maximal 
 * state sharing.
 *
 */

#include <stdlib.h>
#include <string.h>

#ifdef _POSIX_SOURCE
#ifndef __linux__
extern char *strdup(const char *);
#endif
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

/*
 * state sharing object definition
 */
struct _pfdShare
{
    pfList      *ctab;
    pfList      *fog;
    pfList      *gstate;
    pfList      *hlight;
    pfList      *lmodel;
    pfList      *light;
    pfList      *mtl;
    pfList      *tenv;
    pfList      *tgen;
    pfList      *tex;
    pfList      *lpstate;
};


/*
 * A global pfdShare structure that the builder/loaders/etc. can share.
 */
static pfdShare *global_share = NULL;


/***
 ***	pfShare data atructure management
 ***/

/*
 * construct a new pfdShare structure in shared memory
 */
PFDUDLLEXPORT pfdShare *
pfdNewShare(void)
{
    pfdShare	*share;
    void	*arena	= pfGetSharedArena();

    /* allocate new "shared state element" object */
    share = (pfdShare *)pfMalloc(sizeof(pfdShare), arena);

    /* initialize shared-object lists */
    share->ctab   = pfNewList(sizeof(pfObject *), 1, arena);
    share->fog    = pfNewList(sizeof(pfObject *), 1, arena);
    share->gstate = pfNewList(sizeof(pfObject *), 1, arena);
    share->hlight = pfNewList(sizeof(pfObject *), 1, arena);
    share->lmodel = pfNewList(sizeof(pfObject *), 1, arena);
    share->light  = pfNewList(sizeof(pfObject *), 1, arena);
    share->mtl    = pfNewList(sizeof(pfObject *), 1, arena);
    share->tenv   = pfNewList(sizeof(pfObject *), 1, arena);
    share->tgen   = pfNewList(sizeof(pfObject *), 1, arena);
    share->tex    = pfNewList(sizeof(pfObject *), 1, arena);
    share->lpstate= pfNewList(sizeof(pfObject *), 1, arena);

    /* return address of newly allocated object to caller */
    return share;
}


/*
 * Get the global pfdShare structure.
 */
PFDUDLLEXPORT pfdShare *
pfdGetGlobalShare(void)
{
    if (global_share == NULL)
	global_share = pfdNewShare();

    return(global_share);
}


/*
 * Set the global pfdShare structure.
 */
PFDUDLLEXPORT void
pfdSetGlobalShare(pfdShare *share)
{
    global_share = share;
}


/*
 * clean the data a pfdShare structure references
 */
static int
cleanShareList(pfList *list)
{
    int	  	 i;
    int	  	 count;
    int		 cleaned = 0;
    pfObject 	*object;

    /* consider NULL pointer as not needing cleaning */
    if (list == NULL)
	return cleaned;

    /* delete objects that are referenced only by this list */
    count = pfGetNum(list);
    for (i = count - 1; i >= 0; i--)
	if ((object = (pfObject *)pfGet(list, i)) != NULL)
	    if (pfGetRef(object) < 2)
	    {
		pfRemove(list, object);
		pfUnrefDelete(object);
		++cleaned;
	    }

    /* report number of objects deleted */
    return cleaned;
}

/*
 * clean a the data that a pfdShare structure and references
 */
PFDUDLLEXPORT int
pfdCleanShare(pfdShare *share)
{
    int          cleaned = 0;

    /* consider NULL pointer as not needing cleaning */
    if (share == NULL)
	return cleaned;

    /* 
     * MUST clean geostates FIRST so they release attribute references.
     */
    cleaned += cleanShareList(share->gstate);

    /* clean objects on the shared-object lists */
    cleaned += cleanShareList(share->ctab);
    cleaned += cleanShareList(share->fog);
    cleaned += cleanShareList(share->hlight);
    cleaned += cleanShareList(share->lmodel);
    cleaned += cleanShareList(share->light);
    cleaned += cleanShareList(share->mtl);
    cleaned += cleanShareList(share->tenv);
    cleaned += cleanShareList(share->tgen);
    cleaned += cleanShareList(share->tex);
    cleaned += cleanShareList(share->lpstate);

    /* report number of objects deleted */
    return cleaned;
}

/*
 * delete a pfList and optionally the data it references
 */
static void
delShareList(pfList *list, int deepDelete)
{
    /* delete objects referenced by the list */
    if (deepDelete)
    {
	int	  i;
	int	  count = pfGetNum(list);
	pfObject *object;

	/* delete objects referenced by the list */
	for (i = 0; i < count; i++)
	    if ((object = (pfObject *)pfGet(list, i)) != NULL)
		pfDelete(object);
    }

    /* delete the list itself */
    pfDelete(list);
}

/*
 * delete a pfdShare structure and optionally the data it references
 */
PFDUDLLEXPORT void
pfdDelShare(pfdShare *share, int deepDelete)
{
    /* consider NULL pointer as not needing deletion */
    if (share == NULL)
	return;

    /* 
     * MUST delete geostates FIRST so they release attribute references.
     */
    delShareList(share->gstate, deepDelete);

    /* delete the shared-object lists */
    delShareList(share->ctab,   deepDelete);
    delShareList(share->fog,    deepDelete);
    delShareList(share->hlight, deepDelete);
    delShareList(share->lmodel, deepDelete);
    delShareList(share->light,  deepDelete);
    delShareList(share->mtl,    deepDelete);
    delShareList(share->tenv,   deepDelete);
    delShareList(share->tgen,   deepDelete);
    delShareList(share->tex,    deepDelete);
    delShareList(share->lpstate,deepDelete);

    if (global_share == share)
	global_share = NULL;

    /* free actual pfdShare structure */
    pfFree(share);

}

/*
 * print the cardinality of each pfdShare structure component type
 */
PFDUDLLEXPORT void
pfdPrintShare(pfdShare *share)
{
    int		count;
    int		total = 0;

    /* consider NULL pointer as not needing printing */
    if (share == NULL)
	return;

    /* print information about all non-zero-length lists */
    if ((count = pfGetNum(share->ctab)) > 0)
    {
	total += count;
	pfNotify(PFNFY_INFO, PFNFY_MORE, "    Color Tables:    %5d", count);
    }
    if ((count = pfGetNum(share->fog)) > 0)
    {
	total += count;
	pfNotify(PFNFY_INFO, PFNFY_MORE, "    Fogs:            %5d", count);
    }
    if ((count = pfGetNum(share->gstate)) > 0)
    {
	total += count;
	pfNotify(PFNFY_INFO, PFNFY_MORE, "    GeoStates:       %5d", count);
    }
    if ((count = pfGetNum(share->hlight)) > 0)
    {
	total += count;
	pfNotify(PFNFY_INFO, PFNFY_MORE, "    Highlights:      %5d", count);
    }
    if ((count = pfGetNum(share->lmodel)) > 0)
    {
	total += count;
	pfNotify(PFNFY_INFO, PFNFY_MORE, "    Light Models:    %5d", count);
    }
    if ((count = pfGetNum(share->light)) > 0)
    {
	total += count;
	pfNotify(PFNFY_INFO, PFNFY_MORE, "    Lights:          %5d", count);
    }
    if ((count = pfGetNum(share->mtl)) > 0)
    {
	total += count;
	pfNotify(PFNFY_INFO, PFNFY_MORE, "    Materials:       %5d", count);
    }
    if ((count = pfGetNum(share->tenv)) > 0)
    {
	total += count;
	pfNotify(PFNFY_INFO, PFNFY_MORE, "    Texture Envs:    %5d", count);
    }
    if ((count = pfGetNum(share->tgen)) > 0)
    {
	total += count;
	pfNotify(PFNFY_INFO, PFNFY_MORE, "    Texture Gens:    %5d", count);
    }
    if ((count = pfGetNum(share->tex)) > 0)
    {
	total += count;
	pfNotify(PFNFY_INFO, PFNFY_MORE, "    Textures:        %5d", count);
    }
    if ((count = pfGetNum(share->lpstate)) > 0)
    {
	total += count;
	pfNotify(PFNFY_INFO, PFNFY_MORE, "    LPStates:        %5d", count);
    }

    /* when pfdShare structure is empty, say so */
    if (total == 0)
	pfNotify(PFNFY_INFO, PFNFY_MORE, "    None");
}

/*
 * count the total number of pfdShare objects
 */
PFDUDLLEXPORT int
pfdCountShare(pfdShare *share)
{
    int		total = 0;

    /* accumulate total number of shared objects */
    if (share != NULL)
    {
	total += pfGetNum(share->ctab);
	total += pfGetNum(share->fog);
	total += pfGetNum(share->gstate);
	total += pfGetNum(share->hlight);
	total += pfGetNum(share->lmodel);
	total += pfGetNum(share->light);
	total += pfGetNum(share->mtl);
	total += pfGetNum(share->tenv);
	total += pfGetNum(share->tgen);
	total += pfGetNum(share->tex);
	total += pfGetNum(share->lpstate);
    }

    /* return total to caller */
    return total;
}

/***
 ***	Match geostate objects
 ***/

/*
 * select proper shared-object list based on object type
 */
PFDUDLLEXPORT pfList*
pfdGetSharedList(pfdShare *share, pfType *type)
{
    if (pfIsDerivedFrom(type, pfGetCtabClassType()))
	return share->ctab;	
    else if (pfIsDerivedFrom(type, pfGetFogClassType()))
	return share->fog;	
    else if (pfIsDerivedFrom(type, pfGetGStateClassType()))
	return share->gstate;	
    else if (pfIsDerivedFrom(type, pfGetHlightClassType()))
	return share->hlight;	
    else if (pfIsDerivedFrom(type, pfGetLightClassType()))
	return share->lmodel;	
    else if (pfIsDerivedFrom(type, pfGetLModelClassType()))
	return share->light;	
    else if (pfIsDerivedFrom(type, pfGetMtlClassType()))
	return share->mtl;	
    else if (pfIsDerivedFrom(type, pfGetTEnvClassType()))
	return share->tenv;	
    else if (pfIsDerivedFrom(type, pfGetTGenClassType()))
	return share->tgen;	
    else if (pfIsDerivedFrom(type, pfGetTexClassType()))
	return share->tex;	
    else if (pfIsDerivedFrom(type, pfGetLPStateClassType()))
	return share->lpstate;	
    else if (pfIsDerivedFrom(type, pfGetClipTextureClassType()))
	return NULL;
    else
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		 "pfuGetSharedList() unknown object type");
	return NULL;
}

PFDUDLLEXPORT pfObject*
pfdNewSharedObject(pfdShare *share, pfObject *obj)
{
    pfObject 	*match;
    void 	*arena = NULL;
    pfType	*type = pfGetType(obj);
    
    if (obj == NULL)
	return NULL;

    arena = pfGetArena(obj);

    match = pfdFindSharedObject(share, obj);

    if (match == NULL)
    {
	if (type == pfGetTexClassType())
	{
	    unsigned int 	*image;
	    int 		comp,ns,nt,nr;

	    pfTexture *tex = pfNewTex(arena);
	    pfCopy(tex,obj);

	    pfGetTexImage(tex,&image,&comp,&ns,&nt,&nr);
	    if (image == NULL)
	    {
		if ((pfGetTexName(tex) == NULL) && 
		    (pfGetTexFormat(tex,PFTEX_FAST_DEFINE)!=PF_ON))
		    pfNotify(PFNFY_WARN, PFNFY_PRINT,
			     "pfdNewSharedObject() Null texture image and"
			     " name - could not load texture");
		else if (pfGetTexName(tex) && 
			 (pfLoadTexFile(tex,strdup(pfGetTexName(tex))) == 0))
		    pfNotify(PFNFY_WARN, PFNFY_PRINT,
			     "pfdNewSharedObject() Could not load texture: %s",	
			     pfGetTexName(tex));
		else
		    pfNotify(PFNFY_INFO, PFNFY_MORE,
			     "pfdNewSharedObject() Loaded Texture: %s",
			     pfGetTexName(tex));
	    }
#if VERBOSE
	    else
	    {
		pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
		"pfdNewSharedObject() Sharing previously loaded texture: %s",
		 pfGetTexName(tex));
	    }
#endif
	    match = (pfObject *)tex;
	}

	else if (type == pfGetClipTextureClassType())
 	{
	    match = (pfObject *)pfNewClipTexture(arena);	
	    pfCopy(match, obj);
	}

	else if (type == pfGetGStateClassType())
	{
	    match = (pfObject *)pfNewGState(arena);
	    pfCopy(match, obj);
	}

	else if (type == pfGetFogClassType())
	{
	    match = (pfObject *)pfNewFog(arena);
	    pfCopy(match, obj);
	}

	else if (type == pfGetLightClassType())
	{
	    match = (pfObject *)pfNewLight(arena);
	    pfCopy(match, obj);
	}

	else if (type == pfGetLModelClassType())
	{
	    match = (pfObject *)pfNewLModel(arena);
	    pfCopy(match, obj);
	}

	else if (type == pfGetTEnvClassType())
	{
	    match = (pfObject *)pfNewTEnv(arena);
	    pfCopy(match, obj);
	}

	else if (type == pfGetTGenClassType())
	{
	    match = (pfObject *)pfNewTGen(arena);
	    pfCopy(match, obj);
	}

	else if (type == pfGetCtabClassType())
	{
	    match = (pfObject *)pfNewCtab(pfGetCtabSize((pfColortable*)obj),
					  arena);
	    pfCopy(match, obj);
	}

	else if (type == pfGetMtlClassType())
	{
	    match = (pfObject *)pfNewMtl(arena);
	    pfCopy(match, obj);
	}

	else if (type == pfGetHlightClassType())
	{
	    match = (pfObject *)pfNewHlight(arena);
	    pfCopy(match, obj);
	}

	else if (type == pfGetLPStateClassType())
	{
	    match = (pfObject *)pfNewLPState(arena);
	    pfCopy(match, obj);
	}
	pfdAddSharedObject(share, match);
    }
    return match;
}


/*
 * look through pfdShare structure for a matching object
 */
PFDUDLLEXPORT pfObject *
pfdFindSharedObject(pfdShare *share, pfObject *object)
{
    int		 index;
    int		 count;
    pfObject	*item;
    pfList	*list;

    /* check share argument */
    if (share == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		 "pfdFindSharedObject() NULL pfdShare argument");
	return NULL;
    }

    /* nothing matches a NULL object argument */
    if (object == NULL)
	return NULL;

    /* select right list within pfdShare */
    list = pfdGetSharedList(share, pfGetType(object));
    if (list == NULL)
	return NULL;

    if (pfIsOfType(object, pfGetClipTextureClassType()))
	return NULL;

    /* look through list for an equivalent item */
    count = pfGetNum(list);
    if (pfIsOfType(object, pfGetTexClassType()))
    {
	const char *oName = pfGetTexName((pfTexture *)object);
	const char *iName;

	for (index = 0; index < count; index++)
	{
	    item = (pfObject *)pfGet(list, index);
	    iName = pfGetTexName((pfTexture *)item);

	    /* NOTICE: pfTextures compared by name only so be careful */
	    if (oName != NULL && iName != NULL && strcmp(oName, iName) == 0)
		return item;
	}
    }
    else
    {
	for (index = 0; index < count; index++)
	{
	    item = (pfObject *)pfGet(list, index);
	    if (pfCompare(object, item) == 0)
		return item;
	}
    }

    /* no equivalent item found */
    return NULL;
}

PFDUDLLEXPORT int
pfdAddSharedObject(pfdShare *share, pfObject *object)
{
    pfList	*list;
    int		find;

    /* check share argument */
    if (share == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		 "pfdAddSharedObject() NULL pfdShare argument");
	return -1;
    }

    /* ignore NULL object arguments */
    if (object == NULL)
	return -1;

    /* select right list within pfdShare */
    list = pfdGetSharedList(share, pfGetType(object));
    if (list == NULL)
	return -1;

    /* add item to list if it is not already present there */
    find = pfSearch(list, object);
    if (find == -1)
    {
	pfAdd(list, object);
	pfRef(object);
	find = pfSearch(list, object);
    }

    return find;
}

PFDUDLLEXPORT int
pfdRemoveSharedObject(pfdShare *share, pfObject *object)
{
    pfList	*list;
    int		find;

    /* check share argument */
    if (share == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
		 "pfdAddSharedObject() NULL pfdShare argument");
	return -1;
    }

    /* ignore NULL object arguments */
    if (object == NULL)
	return -1;

    /* select right list within pfdShare */
    list = pfdGetSharedList(share, pfGetType(object));
    if (list == NULL)
	return -1;

    /* add item to list if it is not already present there */
    find = pfSearch(list, object);
    if ((find != -1) && (pfGetRef(object) < 2))
    {
    	pfRemove(list, object);
	pfUnrefDelete(object);
    }

    return find;
}

/***
 ***	Use pfdFindSharedObject and pfdAddSharedObject to optimize state sharing
 ***/

/*
 * make sure geosets and their attributes are shared
 */
static void
pfdShareGStateMultiAttr(pfdShare *unique, pfdShare *discard,
		   pfGeoState *gstate, uint64_t attr, int index)
{
    pfObject	*object;
    pfObject	*match;

    /* get pointer to GeoState attribute indicated by "attr" */
    if ((pfGetGStateInherit(gstate) & attr) || 
	(object = (pfObject*)pfGetGStateMultiAttr(gstate, attr, index)) == NULL)
	return;

    /* share each light in light-array between pfGeoStates */
    if (attr == PFSTATE_LIGHTS)
    {
	int		changed = 0, i;
	pfLight		**light = (pfLight**)object;

	/* share each of the lights */
	for (i = 0; i < PF_MAX_LIGHTS; i++)
	{
	    if (light[i] == NULL)
		continue;
	    else
	    if ((match = pfdFindSharedObject(unique, 
					     (pfObject*)light[i])) == NULL)
		pfdAddSharedObject(unique, (pfObject*)light[i]);
	    else
	    if (match != (pfObject*)light[i])
	    {
		if (discard)
		    pfdAddSharedObject(discard, (pfObject*)light[i]);
		else
		    pfDelete((pfObject*)light[i]);
		    
		light[i] = (pfLight*)match;
		changed++;
	    }
	}

	/* update GeoState light definitions if any have been changed */
	if (changed > 0)
	    pfGStateAttr(gstate, PFSTATE_LIGHTS, light);

	return;
    }

    /* look for a match to object in shared attribute lists */
    match = pfdFindSharedObject(unique, object);

    /* add unmatched objects to the unique-list */
    if (match == NULL)
	pfdAddSharedObject(unique, object);
    else 
    /* delete redundant objects and update GeoState */
    if (match != object)
    {
	/* change attr first so object's reference count gets decremented */
	pfGStateMultiAttr(gstate, attr, index, match);

	if (discard)
	    pfdAddSharedObject(discard, object);
	else
	    pfDelete(object);
    }
}

/*
 * make sure geosets and their attributes are shared
 */
static void
pfdShareGStateAttr(pfdShare *unique, pfdShare *discard,
		   pfGeoState *gstate, uint64_t attr)
{
    pfObject	*object;
    pfObject	*match;

    /* get pointer to GeoState attribute indicated by "attr" */
    if ((pfGetGStateInherit(gstate) & attr) || 
	(object = (pfObject*)pfGetGStateAttr(gstate, attr)) == NULL)
	return;

    /* share each light in light-array between pfGeoStates */
    if (attr == PFSTATE_LIGHTS)
    {
	int		changed = 0, i;
	pfLight		**light = (pfLight**)object;

	/* share each of the lights */
	for (i = 0; i < PF_MAX_LIGHTS; i++)
	{
	    if (light[i] == NULL)
		continue;
	    else
	    if ((match = pfdFindSharedObject(unique, 
					     (pfObject*)light[i])) == NULL)
		pfdAddSharedObject(unique, (pfObject*)light[i]);
	    else
	    if (match != (pfObject*)light[i])
	    {
		if (discard)
		    pfdAddSharedObject(discard, (pfObject*)light[i]);
		else
		    pfDelete((pfObject*)light[i]);
		    
		light[i] = (pfLight*)match;
		changed++;
	    }
	}

	/* update GeoState light definitions if any have been changed */
	if (changed > 0)
	    pfGStateAttr(gstate, PFSTATE_LIGHTS, light);

	return;
    }

    /* look for a match to object in shared attribute lists */
    match = pfdFindSharedObject(unique, object);

    /* add unmatched objects to the unique-list */
    if (match == NULL)
	pfdAddSharedObject(unique, object);
    else 
    /* delete redundant objects and update GeoState */
    if (match != object)
    {
	/* change attr first so object's reference count gets decremented */
	pfGStateAttr(gstate, attr, match);

	if (discard)
	    pfdAddSharedObject(discard, object);
	else
	    pfDelete(object);
    }
}

static void
pfdShareGSet(pfdShare *unique, pfdShare *discard, pfGeoSet *gset)
{
    void	*match;
    pfGeoState	*gstate;
    int		i;

    /* make sure there's something to share */
    if (gset == NULL || unique == NULL)
	return;
    if ((gstate = pfGetGSetGState(gset)) == NULL)
	return;

    /* share simple state objects between pfGeoStates */
    pfdShareGStateAttr(unique, discard, gstate, PFSTATE_BACKMTL);
    pfdShareGStateAttr(unique, discard, gstate, PFSTATE_COLORTABLE);
    pfdShareGStateAttr(unique, discard, gstate, PFSTATE_FOG);
    pfdShareGStateAttr(unique, discard, gstate, PFSTATE_FRONTMTL);
    pfdShareGStateAttr(unique, discard, gstate, PFSTATE_HIGHLIGHT);
    pfdShareGStateAttr(unique, discard, gstate, PFSTATE_LIGHTMODEL);

    for (i = 0 ; i < PF_MAX_TEXTURES ; i ++)
	pfdShareGStateMultiAttr(unique, discard, gstate, PFSTATE_TEXENV, i);

    for (i = 0 ; i < PF_MAX_TEXTURES ; i ++)
	pfdShareGStateMultiAttr(unique, discard, gstate, PFSTATE_TEXTURE, i);

    pfdShareGStateAttr(unique, discard, gstate, PFSTATE_LIGHTS);
    pfdShareGStateAttr(unique, discard, gstate, PFSTATE_LPOINTSTATE);

    /* share pfGeoStates between pfGeoSets */
    if ((match = pfdFindSharedObject(unique, (pfObject*)gstate)) == NULL)
	pfdAddSharedObject(unique, (pfObject*)gstate);
    else
    if (match != gstate)
    {
	/* change geostate first so gstate's reference count gets decremented */
	pfGSetGState(gset, match);

	if (discard)
	    pfdAddSharedObject(discard, (pfObject*)gstate);
	else
	    pfDelete((pfObject*)gstate);
    }
}

typedef struct
{
    pfdShare	*unique;
    pfdShare	*discard;
    pfList	*gstateList;

} pfdShareCBData;


static int
shareTrav(pfuTraverser *trav)
{
    /* handle pfGeodes, pfBillboards, and subclassed types */
    if (pfIsOfType(trav->node, pfGetGeodeClassType()))
    {
	/* ensure state sharing within each geoset of this geode */
	pfGeode *	geode = (pfGeode *)trav->node;
	int		count = pfGetNumGSets(geode);
	pfdShareCBData *cbd = (pfdShareCBData*)trav->data;
	int		i;

	/* make each geoset in this pfGeode share state objects */
	for (i = 0; i < count; i++)
	{
	    pfGeoSet *gset = pfGetGSet(geode, i);
	    pfdShareGSet(cbd->unique, cbd->discard, gset);
	}
    }
    else
    /* handle pfLightPoints */
    if (pfIsOfType(trav->node, pfGetLPointClassType()))
    {
	pfLightPoint	*lpoint = (pfLightPoint *)trav->node;
	pfdShareCBData	*cbd = (pfdShareCBData*)trav->data;

	/* make the geoset in this pfLighPoint share state objects */
	pfdShareGSet(cbd->unique, cbd->discard, pfGetLPointGSet(lpoint));
    }
    else
    /* handle pfText here (strings & fonts) */
    if (pfIsOfType(trav->node, pfGetTextClassType()))
    {
	/* this space available */
    }

    /* continue traversal */
    return PFTRAV_CONT;
}

PFDUDLLEXPORT void
pfdMakeShared(pfNode *node)
{
    int			count;
    int			i;
    int			nu;
    int			nd;
    float		total;
    pfObject *		object;
    pfuTraverser	trav;
    pfdShareCBData 	cbd;
    double		startTime;
    double		elapsedTime;

    /* check argument */
    if (node == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdMakeShared: NULL pfNode");
	return;
    }

    pfNotify(PFNFY_INFO, PFNFY_PRINT, "pfdMakeShared optimizing traversal");

    /* define state-sharing traversal */
    cbd.unique   = pfdNewShare();
    cbd.discard  = pfdNewShare();
    cbd.gstateList = pfNewList(sizeof(pfGeoState*),1,NULL);
    pfuInitTraverser(&trav);
    trav.preFunc = shareTrav;
    trav.data    = &cbd;

    /* perform state-sharing traversal */
    startTime = pfGetTime();
    pfuTraverse(node, &trav);
    elapsedTime = pfGetTime() - startTime;

    /* report results of state-sharing traversal */
    pfNotify(PFNFY_INFO, PFNFY_MORE, "  Unique state objects:");
    pfdPrintShare(cbd.unique);
    pfNotify(PFNFY_INFO, PFNFY_MORE, "  Redundant state objects:");
    pfdPrintShare(cbd.discard);
    pfNotify(PFNFY_INFO, PFNFY_MORE, "  Performance statistics:");

    nu = pfdCountShare(cbd.unique);
    nd = pfdCountShare(cbd.discard);
    if ((total = nu + nd) != 0.0f)
    {
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Input objects:   %5d (%6.2f%%)", nu + nd, 100.0f);
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Output objects:  %5d (%6.2f%%)", nu, 100.0f*nu/total);
	pfNotify(PFNFY_INFO, PFNFY_MORE, 
	    "    Deleted objects: %5d (%6.2f%%)", nd, 100.0f*nd/total);
    }
    pfNotify(PFNFY_INFO, PFNFY_MORE, 
	"    Elapsed time:        %.3f sec", elapsedTime);
    pfNotify(PFNFY_INFO, PFNFY_MORE, NULL);

    /* delete shared object structures (and objects on the discard list) */
    pfdDelShare(cbd.unique, FALSE);
    pfdDelShare(cbd.discard, TRUE);
}

/***
 ***	construct optimizing scene geostate
 ***/

typedef struct
{
    pfList	*geoStates;
} pfdRecordCBData;

static int
recordTrav(pfuTraverser *trav)
{
    /* handle pfGeodes, pfBillboards, and subclassed types */
    if (pfIsOfType(trav->node, pfGetGeodeClassType()))
    {
	pfGeode *	geode = (pfGeode *)trav->node;
	pfdRecordCBData *cbd = (pfdRecordCBData*)trav->data;
	int		count = pfGetNumGSets(geode);
	int		i;

	/* remember the geostate of each unique referenced geoset */
	for (i = 0; i < count; i++)
	{
	    pfGeoSet *gset = pfGetGSet(geode, i);
	    pfGeoState *gstate = pfGetGSetGState(gset);
	    if (gstate != NULL && pfSearch(cbd->geoStates, gstate) == -1)
		pfAdd(cbd->geoStates, gstate);
	}
    }
    else
    /* handle pfLightPoints */
    if (pfIsOfType(trav->node, pfGetLPointClassType()))
    {
	pfLightPoint	*lpoint = (pfLightPoint *)trav->node;
	pfGeoSet	*gset = pfGetLPointGSet(lpoint);
	pfGeoState	*gstate = pfGetGSetGState(gset);
	pfdRecordCBData *cbd = (pfdRecordCBData*)trav->data;

	/* make the geoset in this pfLighPoint share state objects */
	if (gstate != NULL && pfSearch(cbd->geoStates, gstate) == -1)
	    pfAdd(cbd->geoStates, gstate);
    }
    else
    /* handle pfText here (strings & fonts) */
    if (pfIsOfType(trav->node, pfGetTextClassType()))
    {
	/* this space available */
    }

    /* continue traversal */
    return PFTRAV_CONT;
}

PFDUDLLEXPORT pfList *
pfdGetNodeGStateList(pfNode *node)
{
    pfdRecordCBData 	cbd;
    pfuTraverser	trav;

    /* check argument */
    if (node == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdGetNodeGStateList: "
	    "NULL argument");
	return NULL;
    }

    /* define recording traversal */
    cbd.geoStates  = pfNewList(sizeof(pfObject *), 1, pfGetSharedArena());
    pfuInitTraverser(&trav);
    trav.preFunc = recordTrav;
    trav.data    = &cbd;

    pfuTraverse(node, &trav);

    return cbd.geoStates;
}

PFDUDLLEXPORT void
pfdMakeSharedScene(pfScene *scene)
{
    int			count;
    int			i;
    int			nu;
    int			nd;
    int			numGeoStates;
    float		total;
    pfObject *		object;
    double		startTime;
    double		elapsedTime;
    pfList*		gsList;

    /* check argument */
    if (scene == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdMakeSharedScene: "
	    "NULL argument");
	return;
    }
    if (!pfIsOfType(scene, pfGetSceneClassType()))
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, "pfdMakeSharedScene: "
	    "argument must be a pfScene");
	return;
    }

    pfNotify(PFNFY_INFO, PFNFY_PRINT, 
	"pfdMakeSharedScene scene pfGeoState traversal");


    /* perform recording traversal */
    startTime = pfGetTime();

    gsList = pfdGetNodeGStateList((pfNode*)scene);

    if (gsList == NULL)
	return;

    /* construct scene geostate */
    numGeoStates = pfGetNum(gsList);
    if (numGeoStates > 0)
    {
	pfGeoState *oldContext = NULL;
	pfGeoState *newContext = NULL;

	pfNotify(PFNFY_DEBUG, PFNFY_MORE, 
	    "  Most popular pfGeoState attributes and values:");

	/* get existing global-context used by scene's geostates */
	oldContext = (pfGeoState*)pfGetSceneGState(scene);

	/* use builder's default gstate as backup */
	if (oldContext == NULL)
	    oldContext = (pfGeoState*)pfdGetDefaultGState();

	newContext = pfNewGState(pfGetSharedArena());

	/* construct an optimizing scene geostate */
	pfdMakeSceneGState(newContext, gsList, oldContext);

	/* update scene's geostates based on new global-context */
	pfdOptimizeGStateList(gsList, oldContext, newContext);

	/* install new global-context geostate as scene geostate */
	pfSceneGState(scene, newContext);

	/* save optimized global context as new builder default */
	pfdDefaultGState(newContext);
    }
    else
    {
	/* no geostate - use default builder state as scene geostate */
	pfGeoState *gs;
	gs = pfNewGState(pfGetSharedArena());
	pfCopy(gs, (pfGeoState*)pfdGetDefaultGState());
	pfSceneGState(scene, gs);
    }

    elapsedTime = pfGetTime() - startTime;

    /* report results of scene geostate creation process */
    pfNotify(PFNFY_INFO, PFNFY_MORE, 
	"  Performance statistics:");
    pfNotify(PFNFY_INFO, PFNFY_MORE,
	"    pfGeoStates:     %5d", numGeoStates);
    pfNotify(PFNFY_INFO, PFNFY_MORE, 
	"    Elapsed time:    %9.3f sec", elapsedTime);
    pfNotify(PFNFY_INFO, PFNFY_MORE, NULL);

    /* delete geostate list */
    pfDelete(gsList);
}
