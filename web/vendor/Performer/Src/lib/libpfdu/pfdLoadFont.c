/*
 * pfdLoadFont.c
 *
 * $Revision: 1.17 $
 * $Date: 2002/05/15 00:31:09 $
 *
 * Load a database file into an IRIS Performer in-memory data 
 * structure using the filename's extension to intuit the file 
 * format. The file is sought in the directories named in the
 * active performer file search path (see pfFilePath for more
 * details). 
 *
 *
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
 */
				 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <dlfcn.h>
#endif
		 
#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

static pfGeoSet* newObjExtrudedGSet(objfnt *meshFont, objfnt *outlineFont, int i);
static pfGeoSet* newObjOutlinedGSet(objfnt *outlineFont, int i);
static pfGeoSet* newObjTexturedGSet(texfnt *texFont, int i);
static pfGeoSet* newObjFilledGSet(objfnt *meshFont, int i);
static void makeObjFontGSets(pfFont *_fnt, int _type, objfnt *mesh, objfnt *outline);

static pfMaterial *
defaultMaterial (void *sharedArena)
{
    static pfMaterial   *material       = NULL;
    if (material == NULL)
    {
        /* setup shared arena pointer once */
        material = pfNewMtl(pfGetSharedArena());
	pfMtlColor(material,PFMTL_AMBIENT,  0.1, 0.1, 0.1);
	pfMtlColor(material,PFMTL_DIFFUSE,  0.8, 0.8, 0.8);
	pfMtlColor(material,PFMTL_SPECULAR, 0.1, 0.1, 0.1);
	pfMtlShininess(material,30.0);
	pfMtlColorMode(material,PFMTL_FRONT, PFMTL_CMODE_AD);
	pfRef(material); /* so no one will ever delete it */
    }
    /* return pointer to default material */
    return material;
}

PFDUDLLEXPORT pfFont*
pfdLoadFont(const char *type, const char *fontFileName, int style)
{
    /* Temporarily use if then else hack */
    /* This will eventually use dso's like pfdLoadFile */

    if (strcmp(type,"type1")==0)
	return pfdLoadFont_type1(fontFileName, style);
    else
	pfNotify(PFNFY_WARN,PFNFY_PRINT,	
		 "pfdLoadFont: Unknown Font Type - %s", type);
    return NULL;
}	
	    						    	
PFDUDLLEXPORT pfFont*
pfdLoadFont_type1(const char *name, int type)
{
    char	path[PF_MAXSTRING], file[PF_MAXSTRING];
    objfnt	*meshFont, *outlineFont;
    texfnt	*texFont;
    void	*arena = pfGetSharedArena();
    pfFont	*font = pfNewFont(arena);
    
    pfFontMode(font, PFFONT_RETURN_CHAR, '\n');
    switch(type) {
    case PFDFONT_FILLED:
	strcpy(file, name);
	strcat(file, ".mf");

	if (pfFindFile(file, path, R_OK))
	    meshFont = readobjfnt(path, arena);
	else
	{
	    pfNotify(PFNFY_WARN,PFNFY_PRINT,
		     "pfdLoadFont_type1: Could not find font file %s\n",file);
	    return NULL;
	}
	makeObjFontGSets(font, PFDFONT_FILLED, meshFont, NULL);
	return font;
    case PFDFONT_OUTLINED:
	strcpy(file, name);
	strcat(file, ".of");
  
	if (pfFindFile(file,path,R_OK))
	    outlineFont = readobjfnt(path, arena);
	else
	{
	    pfNotify(PFNFY_WARN,PFNFY_PRINT,
		     "pfdLoadFont_type1: Could not find font file %s\n",file);
	    return NULL;
	}
	makeObjFontGSets(font, PFDFONT_OUTLINED, NULL, outlineFont);
	return font;
    case PFDFONT_EXTRUDED:
	strcpy(file, name);
	strcat(file, ".mf");
      
	if (pfFindFile(file, path, R_OK))
	    meshFont = readobjfnt(path, arena);
	else
	{
	    pfNotify(PFNFY_WARN,PFNFY_PRINT,
		     "pfdLoadFont_type1: Could not find font file %s\n",file);
	    return NULL;
	}

	strcpy(file, name);
	strcat(file, ".of");
	if (pfFindFile(file, path, R_OK))
	    outlineFont = readobjfnt(path, arena);
	else
	{
	    pfNotify(PFNFY_WARN,PFNFY_PRINT,
		     "pfdLoadFont_type1: Could not find font file %s\n",file);
	    return NULL;
	}
	makeObjFontGSets(font, PFDFONT_EXTRUDED, meshFont, outlineFont);
	return font;
    case PFDFONT_TEXTURED:
	strcpy(file, name);
	strcat(file, ".bw");
	if (pfFindFile(file, path, R_OK))
	    texFont = readtexfnt(path, arena);
	else
	{
	    pfNotify(PFNFY_WARN,PFNFY_PRINT,
		     "pfdLoadFont_type1: Could not find font file %s\n",file);
	    return NULL;
	}
	makeObjFontGSets(font, PFDFONT_TEXTURED, (objfnt*)texFont, NULL);
	return font;
    default:
	pfNotify(PFNFY_WARN,PFNFY_PRINT,
		 "pfdLoadFont_type1: Unknown OBJ font type 0x%x",type);
    }
    return NULL;
}

static void
makeObjFontGSets(pfFont *_fnt, int _type, objfnt *mesh, objfnt *outline)
{
    int		i;
    float	x, y, z, unitScale;
    pfVec3	*coords;
    ushort	*icoords;
    pfVec3	space, tabSpace;
    int		nChars,start;
    pfBox	bbox,fontBBox;
    texfnt	*tfnt = (texfnt*)mesh;
    pfGeoState  *gs = pfNewGState(pfGetSharedArena());

    pfGStateAttr(gs, PFSTATE_FRONTMTL, defaultMaterial(pfGetSharedArena()));
/*    pfGStateAttr(gs, PFSTATE_BACKMTL, defaultMaterial(pfGetSharedArena()));*/
    pfGStateMode(gs, PFSTATE_ENLIGHTING, PF_ON);
    
    if (_type == PFDFONT_TEXTURED)
    {
	nChars = tfnt->charmax - tfnt->charmin + 1;
	start = tfnt->charmin;
        {
	    pfTexture *tex = pfNewTex(pfGetSharedArena());
	    pfTexImage(tex, (unsigned int *)tfnt->rasdata, 2, 
		   tfnt->rasxsize,  tfnt->rasysize,  1);
	    pfTexFilter(tex, PFTEX_MAGFILTER_ALPHA,  PFTEX_SHARPEN_ALPHA);
	    pfGStateMode(gs, PFSTATE_ENTEXTURE, PF_ON);
	    pfGStateAttr(gs, PFSTATE_TEXTURE, tex);
	    pfGStateMode(gs, PFSTATE_TRANSPARENCY, PFTR_ON | PFTR_NO_OCCLUDE);
	    pfGStateMode(gs, PFSTATE_ALPHAFUNC, PFAF_GREATER);
	    pfGStateVal(gs, PFSTATE_ALPHAREF, 4.0f / 255.0f);
        }
    }
    else if (mesh != NULL)
    {
	nChars = mesh->charmax - mesh->charmin+1;
	start = mesh->charmin;
    }
    else if (outline != NULL)
    {
	nChars = outline->charmax - outline->charmin+1;
	start = outline->charmin;
    }
    else
	return;

    pfFontAttr(_fnt, PFFONT_GSTATE, gs);
    pfMakeEmptyBox(&bbox);
    pfMakeEmptyBox(&fontBBox);

    for(i=0;i<nChars;i++)
    {
	pfGeoSet *gset = NULL;
	switch(_type)
	{
	case PFDFONT_FILLED:
	    gset = newObjFilledGSet(mesh, i+start);
	    break;
	case PFDFONT_OUTLINED:
	    gset = newObjOutlinedGSet(outline, i+start);
	    break;
	case PFDFONT_EXTRUDED:
	    gset = newObjExtrudedGSet(mesh,outline,i+start);
	    break;
	case PFDFONT_TEXTURED:
	    gset = newObjTexturedGSet(tfnt,i+start);
	    break;
	default:
	    fprintf(stderr,"BOGUS STYLE\n");
	    return;
	}
	if (gset)
	{
	    pfFontCharGSet(_fnt,i+start, gset);
	    pfGetGSetBBox(gset,&bbox);
	    pfBoxExtendByBox(&fontBBox, &bbox);
	}
    }
    x = fontBBox.max[0] - fontBBox.min[0];
    y = fontBBox.max[2] - fontBBox.min[2];
    z = fontBBox.max[1] - fontBBox.min[1];
    unitScale = 1.0f/(PF_MAX2(x, PF_MAX2(y,z)));
    unitScale = 1.0f/(PF_MAX2(x,y));
    pfFontVal(_fnt, PFFONT_UNIT_SCALE, unitScale);
    for(i=0;i<nChars;i++)
    {
	pfGeoSet *gset;
	int j;
	int *lens;
	int nVerts,nPrims;
        if (_type == PFDFONT_TEXTURED)
	    pfSetVec3(space, tfnt->chars[i].movex * unitScale, 0.0f,0.0f);
	else if (_type == PFDFONT_OUTLINED)
	    pfSetVec3(space, 
		      outline->chars[i].movex * unitScale,
		      outline->chars[i].movey * unitScale,0.0f);
	else 
	    pfSetVec3(space, 
		      mesh->chars[i].movex * unitScale,
		      mesh->chars[i].movey * unitScale,0.0f);

	pfFontCharSpacing(_fnt, i+start, space);
	
	if (i == ' ' - start)
	{
	    pfScaleVec3(space, 4.0f, space); 
	    pfFontCharSpacing(_fnt, '\t', space);
	}

	if ((gset = pfGetFontCharGSet(_fnt,i+start))==NULL)
	    continue;
	nPrims = pfGetGSetNumPrims(gset);
	lens = pfGetGSetPrimLengths(gset);
	pfGetGSetAttrLists(gset, PFGS_COORD3, (void**)(&coords), &icoords);
	nVerts = 0;
	for(j=0;j<nPrims;j++)
	    nVerts += lens[j];
	for(j=0;j<nVerts;j++)
	    if (_type != PFDFONT_TEXTURED)
	    {
		coords[j][0] *= unitScale;
		coords[j][2] *= unitScale;
	    }
	    else
	    {
		coords[j][0] *= unitScale;
	    }
	pfGSetBBox(gset, NULL, PFBOUND_DYNAMIC);
    }
    fontBBox.min[0] *= unitScale;
    fontBBox.min[1] *= unitScale;
    fontBBox.min[2] *= unitScale;
    fontBBox.max[0] *= unitScale;
    fontBBox.max[1] *= unitScale;
    fontBBox.max[2] *= unitScale;

    pfFontAttr(_fnt, PFFONT_BBOX, &fontBBox);
}

typedef short	vshort2[2];
float EXTRUDE_DIST = .25f;

static pfGeoSet*
newObjExtrudedGSet(objfnt *meshFont, objfnt *outlineFont, int i)
{
    pfGeoSet 	*gset;
    pfVec3	*coords, *norms;
    short	*sptr;
    int		len, nv, done, n, nstrips, *lens, nswaps, j;
    void	*arena = pfGetArena(meshFont);	
    
    if (!getchardesc(meshFont, i))
	return NULL;
    
    sptr = getcharprog(meshFont, i);
    
    if (!sptr)
	return NULL;
    
    len = 0;
    nstrips = 0;
    nswaps = 0;
    done = 0;
    
    while(!done) 
    {
	switch(*sptr++) {
	case TM_BGNTMESH:
	    n = 0;
	    break;
	case TM_SWAPTMESH:
	    if (n < 3)
	      pfNotify(PFNFY_WARN, PFNFY_PRINT,
		       "newObjExtrudedGSet: can't TMesh length %d", n);
	    else if (n != 3)
	    {
		len++;
		nswaps++;
	    }
	    len++;
	    break;
	case TM_ENDBGNTMESH:
	    nstrips++;
	    n = 0;
	    break;
	case TM_RETENDTMESH:
	case TM_RET:
	    nstrips++;
	    n = 0;
	    done = 1;
	    break;
	default:   
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "newObjExtrudedGSet: Bad obj font tmesh op");
	    return NULL;
	}
	
	if (done)
	    break;
	
	nv = *sptr++;
	sptr += 2*nv;
	
	if (n == 0 && !(nv == 3 && *sptr == TM_SWAPTMESH))
	{
	    nswaps++;
	    len++;
	}
	
	n += nv;
	len += 2*nv;
    }
    
    sptr = getcharprog(outlineFont, i);
    if (!sptr)
	return NULL;
    
    /* Calculate number of strips/verts in side of extruded char */
    done = 0;
    n = 0;
    while(!done) 
    {
	switch(*sptr++) {
	case PO_BGNLOOP:
	case PO_ENDBGNLOOP:
	    break;
	case PO_RETENDLOOP:
	case PO_RET:
	    done = 1;
	    break;
	default:   
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "newObjExtrudedGSet: Bad obj font tmesh op");
	    return NULL;
	}
	if (done)
	    break;
	nv = *sptr++;
	
	/* Assume 1 strip of 4 verts (quad) for each vertex of loop */
	n += nv;
	len += 4 * nv;
	
	sptr += 2*nv;
    }
    
    coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * len, arena);
    norms = (pfVec3*) pfMalloc(sizeof(pfVec3) * len, arena);
    lens = (int*) pfMalloc(sizeof(int) * n * nstrips*2, arena);
    
    /* Do front and back of character */
    nstrips = 0;
    len = 0;
    n = 0;
    nswaps = 0;
    
    for (j=0; j<2; j++)
    {
	float y;
	
	if (j)
	    y = -EXTRUDE_DIST/2.0f;
	else
	    y = EXTRUDE_DIST/2.0f;
	
	done = 0;
	sptr = getcharprog(meshFont, i);
	while(!done) 
	{
	    static short		v0[2], v1[2], v2[2];
	    
	    switch(*sptr++) {
	    case TM_BGNTMESH:
		len = 0;
		break;
	    case TM_SWAPTMESH:
		if (!j || len != 3)
		{
		    v2[0] = v0[0];
		    v2[1] = v0[1];
		    pfSetVec3(coords[n],(float) v2[0], y, (float) v2[1]);
		    if (j)
		      pfSetVec3(norms[n], 0.0f, -1.0f, 0.0f);
		    else
		      pfSetVec3(norms[n], 0.0f, 1.0f, 0.0f);
		    n++;
		    v0[0] = v1[0];
		    v0[1] = v1[1];
		    v1[0] = v2[0];
		    v1[1] = v2[1];
		    nswaps++;
		    len++;
		}
		break;
	    case TM_ENDBGNTMESH:
		lens[nstrips++] = len;
		len = 0;
		break;
	    case TM_RETENDTMESH:
	    case TM_RET:
		lens[nstrips++] = len;
		len = 0;
		done = 1;
		break;
	    default:   
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "newObjExtrudedGSet: Bad obj font tmesh op");
		return NULL;
	    }
	    if (done)
		break;
	    
	    nv = *sptr++;
	    while(nv--) 
	    {
		v2[0] = *sptr++;
		v2[1] = *sptr++;
		
		pfSetVec3(coords[n], (float) v2[0], y, (float) v2[1]);
		if (j)
		  pfSetVec3(norms[n], 0.0f, -1.0f, 0.0f);
		else
		  pfSetVec3(norms[n], 0.0f, 1.0f, 0.0f);
		n++;
		len++;
		
		/* Reverse ordering of first triangle */
		if (j && len == 3)
		{
		    short	t[2];
		    
		    t[0] = v1[0];
		    t[1] = v1[1];
		    v1[0] = v2[0];
		    v1[1] = v2[1];
		    v2[0] = t[0];
		    v2[1] = t[1];
		    
		    pfSetVec3(coords[n-1], (float) v2[0], y, (float) v2[1]);
		    pfSetVec3(coords[n-2], (float) v1[0], y, (float) v1[1]);
		}
		
		v0[0] = v1[0];
		v0[1] = v1[1];
		v1[0] = v2[0];
		v1[1] = v2[1];
		
		if (j && len == 3 && *sptr != TM_SWAPTMESH)
		{
		    v2[0] = v0[0];
		    v2[1] = v0[1];
		    pfSetVec3(coords[n], (float) v2[0], y, (float) v2[1]);
		    if (j)
		      pfSetVec3(norms[n], 0.0f, -1.0f, 0.0f);
		    else
		      pfSetVec3(norms[n], 0.0f, 1.0f, 0.0f);
		    n++;
		    v0[0] = v1[0];
		    v0[1] = v1[1];
		    v1[0] = v2[0];
		    v1[1] = v2[1];
		    nswaps++;
		    len++;
		}
	    }
	}
    }
    
    /* Do sides of character */
    done = 0;
    sptr = getcharprog(outlineFont, i);
    while(!done) 
    {
	pfVec3		n0, n1;
	int		cur;
	vshort2		*verts;
	
	switch(*sptr++) {
	case PO_BGNLOOP:
	case PO_ENDBGNLOOP:
	    break;
	case PO_RETENDLOOP:
	case PO_RET:
	    done = 1;
	    break;
	default:   
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "newObjExtrudedGSet: Bad obj font tmesh op");
	    return NULL;
	}
	if (done)
	    break;
	
	nv = *sptr++;
	
	verts = (vshort2*)sptr;
	sptr += 2*nv;
	
	j = nv;
	nv++;		/* Need ++ to close loop */
	len = 0;
	cur = 0;
	
	/* v0 == previous, v1 == current, v2 == next */
	while (nv--) 
	{
	    int		next, prev;
	    
	    next = (cur + 1) % j;
	    prev = (cur + j - 1) % j;

	    pfSetVec3(coords[n],  (float) verts[cur][0], 
			  -EXTRUDE_DIST/2.0f, (float) verts[cur][1]);
	    
	    pfSetVec3(coords[n+1],  (float) verts[cur][0], 
			    EXTRUDE_DIST/2.0f, (float) verts[cur][1]);
	    
	    len += 2;

	    pfSetVec3(n0,  -(verts[prev][1] - verts[cur][1]), 0.0f, 
		   (verts[prev][0] - verts[cur][0]));
	    pfNormalizeVec3(n0);

	    pfSetVec3(n1, -(verts[cur][1] - verts[next][1]), 0.0f, 
		   (verts[cur][0] - verts[next][0]));
	    pfNormalizeVec3(n1);
	    
	    /* Average normals if separation angle is < 45 degrees */
	    if (pfDotVec3(n0,n1) > .7071067811865475244f)
	    {
		PFCOMBINE_VEC3(norms[n], .5f, n0, .5f, n1);
		pfNormalizeVec3(norms[n]);
		PFCOPY_VEC3(norms[n+1], norms[n]);
		n += 2;
	    }
	    else	/* Need to break strip here */
	    {
		if (len > 2)
		{
		    PFCOPY_VEC3(norms[n],n0);
		    PFCOPY_VEC3(norms[n+1], norms[n]);
		    n += 2;
		    
		    lens[nstrips++] = len;
		    
		    if (nv)
		    {
			/* Start new strip */
			PFCOPY_VEC3(coords[n], coords[n-2]);
			PFCOPY_VEC3(coords[n+1], coords[n-1]);
			PFCOPY_VEC3(norms[n], n1);
			PFCOPY_VEC3(norms[n+1], norms[n]);
			n += 2;
			
			len = 2;
		    }
		    else
			len = 0;
		}
		else
		{
		    PFCOPY_VEC3(norms[n], n1);
		    PFCOPY_VEC3(norms[n+1], norms[n]);
		    n += 2;
		}
		
	    }
	    cur = (cur + 1) % j;
	}
	if (len)
	    lens[nstrips++] = len;
    }
    
    coords = (pfVec3*) pfRealloc(coords, sizeof(pfVec3) * n);
    norms = (pfVec3*) pfRealloc(norms, sizeof(pfVec3) * n);
    lens = (int*) pfRealloc(lens, sizeof(int) * nstrips);

    gset = pfNewGSet(arena);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, norms, NULL);
    pfGSetNumPrims(gset, nstrips);
    pfGSetPrimType(gset, PFGS_TRISTRIPS);
    pfGSetPrimLengths(gset, lens);
    
    return gset;
}

static pfGeoSet*
newObjOutlinedGSet(objfnt *outlineFont, int i)
{
    pfGeoSet	*gset;
    pfVec3	*coords, *norms;
    short		*sptr;
    int		len, nv, done, n, nstrips, *lens;
    void		*arena = pfGetArena(outlineFont);

    if (!getchardesc(outlineFont, i))
	return NULL;
    
    sptr = getcharprog(outlineFont, i);
    
    if (!sptr)
	return NULL;

    done = 0;
    len = 0;
    nstrips = 0;
    while(!done) 
    {
	switch(*sptr++) {
	case PO_BGNLOOP:
	    break;
	case PO_ENDBGNLOOP:
	    nstrips++;
	    break;
	case PO_RETENDLOOP:
	    nstrips++;
	    done = 1;
	    break;
	case PO_RET:
	    done = 1;
	    break;
	default:   
	    pfNotify(PFNFY_WARN,PFNFY_PRINT,
		     "newObjOutlinedGSet: bad LOOP op\n");
	    return NULL;
	}
	if (done)
	    break;
	nv = *sptr++;
	len += nv;
	sptr += 2*nv;
    }

    coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * (len+nstrips), arena);
    norms = (pfVec3*) pfMalloc(sizeof(pfVec3), arena);
    lens = (int*) pfMalloc(sizeof(int)*nstrips, arena);
    
    sptr = getcharprog(outlineFont, i);
    nstrips = 0;
    len = 0;
    done = 0;
    n = 0;
    
    while(!done) 
    {
	static short	v0[2];
	
	switch(*sptr++) {
	case PO_BGNLOOP:
	    break;
	case PO_ENDBGNLOOP:
	    pfSetVec3(coords[n++], (float) v0[0], 0.0f, (float) v0[1]);
	    lens[nstrips++] = len+1;
	    break;
	case PO_RETENDLOOP:
	    pfSetVec3(coords[n++], (float) v0[0], 0.0f, (float) v0[1]);
	    lens[nstrips++] = len+1;
	    done = 1;
	    break;
	case PO_RET:
	    done = 1;
	    break;
	default:   
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "newObjOutlineGSet: Obj font LOOP_draw, bad LOOP op");
	    return NULL;
	}
	if (done)
	    break;
	
	nv = len = *sptr++;
	v0[0] = sptr[0];
	v0[1] = sptr[1];
	while(nv--) 
	{
	    pfSetVec3(coords[n++], (float) sptr[0], 0.0f, (float) sptr[1]);
	    sptr += 2;
	}
    }
    
    pfSetVec3(norms[0], 0.0f, -1.0f, 0.0f); 
    
    gset = pfNewGSet(arena);
    pfGSetAttr(gset,PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(gset,PFGS_NORMAL3, PFGS_OVERALL, norms, NULL);
    pfGSetNumPrims(gset, nstrips);
    pfGSetPrimType(gset, PFGS_LINESTRIPS);
    pfGSetPrimLengths(gset, lens);
    pfGSetLineWidth(gset, 1);

    return gset;
}

static pfGeoSet*
newObjTexturedGSet(texfnt *texFont, int i)
{
    pfGeoSet 	*gset;
    pfVec3	*coords, *norms;
    pfVec2	*tcoords;
    int		*lens, j;
    void	*arena = pfGetArena(texFont);	
    texchardesc *cd;
    
    if (i < texFont->charmin || i > texFont->charmax)
	return NULL;
    
    j = i - texFont->charmin;
    cd = texFont->chars + j;
    
    if (!cd->haveimage)
	return NULL;
    
    coords = (pfVec3*) pfMalloc(sizeof(pfVec3) * 4, arena);
    tcoords = (pfVec2*) pfMalloc(sizeof(pfVec2) * 4, arena);
    norms = (pfVec3*) pfMalloc(sizeof(pfVec3), arena);
    lens = (int*) pfMalloc(sizeof(int), arena);

    pfSetVec3(norms[0], 0.0f, -1.0f, 0.0f);

    pfSetVec3(coords[0], cd->data[2], 0.0f, cd->data[3]);
    pfSetVec3(coords[1], cd->data[6], 0.0f, cd->data[7]);
    pfSetVec3(coords[2], cd->data[22], 0.0f, cd->data[23]);
    pfSetVec3(coords[3], cd->data[20], 0.0f, cd->data[21]);

    pfSetVec2(tcoords[0], cd->data[0], cd->data[1]);
    pfSetVec2(tcoords[1], cd->data[4], cd->data[5]);
    pfSetVec2(tcoords[2], cd->data[12], cd->data[13]);
    pfSetVec2(tcoords[3], cd->data[8], cd->data[9]);
    
    lens[0] = 4;

    gset = pfNewGSet(arena);
    pfGSetAttr(gset,PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(gset,PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, NULL);
    pfGSetAttr(gset,PFGS_NORMAL3, PFGS_OVERALL, norms, NULL);
    pfGSetNumPrims(gset,1);
    pfGSetPrimLengths(gset,lens);
    pfGSetPrimType(gset,PFGS_TRISTRIPS);
    
    return gset;
}
   
static pfGeoSet*
newObjFilledGSet(objfnt *meshFont, int i)
{
    pfGeoSet	*gset;
    pfVec3	*coords,*norms;
    short		*sptr;

    int		len,nv,done,n,nstrips, *lens, nswaps;
    void		*arena=pfGetArena(meshFont);

    if (!getchardesc(meshFont,i))
	return NULL;
    sptr = getcharprog(meshFont, i);

    if (!sptr)
	return NULL;

    len = 0;
    nstrips = 0;
    nswaps = 0;
    done = 0;

    while(!done)
    {
	switch(*sptr++) {
	case TM_BGNTMESH:
	    n = 0;
	    break;
	case TM_SWAPTMESH:
	    if (n < 3)
		pfNotify(PFNFY_WARN, PFNFY_PRINT,
			 "newObjMeshGSet: Cant handle tmesh with < 3 verts");
	    else if (n != 3)
	    {
		len++;
		nswaps++;
	    }
	    break;
	case TM_ENDBGNTMESH:
	    nstrips++;
	    n = 0;
	    break;
	case TM_RETENDTMESH:
	case TM_RET:
	    nstrips++;
	    n = 0;
	    done = 1;
	    break;
	default:
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "newObjMeshGSet: Bad TMESH Op from objfnt parser");
	}
	if (done)
	    break;
	nv = *sptr++;
	sptr += 2*nv;
	
	if (n==0 && !(nv == 3 && *sptr == TM_SWAPTMESH))
	{
	    nswaps++;
	    len++;
	}
	n += nv;
	len += nv;
    }
    coords = (pfVec3*)pfMalloc(sizeof(pfVec3) * len, arena);
    norms = (pfVec3*)pfMalloc(sizeof(pfVec3), arena);
    lens = (int*) pfMalloc(sizeof(int)*nstrips, arena);
    sptr = getcharprog(meshFont,i);
    nstrips = 0;
    len = 0;
    done = 0;
    n = 0;
    nswaps = 0;
    while (!done)
    {
	static short	v0[2], v1[2], v2[2];
	switch(*sptr++) {
	case TM_BGNTMESH:
	    len = 0;
	    break;
	case TM_SWAPTMESH:
	    if (len != 3)
	    {
		v2[0] = v0[0];
		v2[1] = v0[1];
		pfSetVec3(coords[n++], (float) v2[0], 0.0f, (float) v2[1]);
		v0[0] = v1[0];
		v0[1] = v1[1];
		v1[0] = v2[0];
		v1[1] = v2[1];
		nswaps++;
		len++;
	    }
	    break;
	case TM_ENDBGNTMESH:
	    lens[nstrips++] = len;
	    len = 0;
	    break;
	case TM_RETENDTMESH:
	case TM_RET:
	    lens[nstrips++] = len;
	    len = 0;
	    done = 1;
	    break;
	default:   
	    pfNotify(PFNFY_WARN, PFNFY_PRINT,
		     "newObjMeshGSet: Bad TMESH Op from objfnt parser");
	    return NULL;
	}
	if (done)
	    break;
	
	nv = *sptr++;
	while(nv--) 
	{
	    v2[0] = *sptr++;
	    v2[1] = *sptr++;
	    
	    pfSetVec3(coords[n++], (float)v2[0], 0.0f, (float) v2[1]);
	    len++;
	    
	    /* Reverse ordering of first triangle */
	    if (len == 3)
	    {
		short	t[2];
		
		t[0] = v1[0];
		t[1] = v1[1];
		v1[0] = v2[0];
		v1[1] = v2[1];
		v2[0] = t[0];
		v2[1] = t[1];
		
		pfSetVec3(coords[n-1], (float)v2[0], 0.0f, (float) v2[1]);
		pfSetVec3(coords[n-2], (float)v1[0], 0.0f, (float) v1[1]);
	    }
	    
	    v0[0] = v1[0];
	    v0[1] = v1[1];
	    v1[0] = v2[0];
	    v1[1] = v2[1];
	    
	    if (len == 3 && *sptr != TM_SWAPTMESH)
	    {
		v2[0] = v0[0];
		v2[1] = v0[1];
		pfSetVec3(coords[n++], (float)v2[0], 0.0f, (float) v2[1]);
		v0[0] = v1[0];
		v0[1] = v1[1];
		v1[0] = v2[0];
		v1[1] = v2[1];
		nswaps++;
		len++;
	    }
	}
    }
    pfSetVec3(norms[0], 0.0f, -1.0f, 0.0f);

    gset = pfNewGSet(arena);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OVERALL, norms, NULL);
    pfGSetNumPrims(gset,nstrips);
    pfGSetPrimType(gset,PFGS_TRISTRIPS);
    pfGSetPrimLengths(gset,lens);

    return gset;
}





