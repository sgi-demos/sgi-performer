/*
 * Copyright 1994, 1995, Silicon Graphics, Inc.
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
 * pfptu.c: $Revision: 1.1 $ $Date: 2000/11/21 21:39:34 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bstring.h>
#include <math.h>
#include <ctype.h>

#ifdef	_POSIX_SOURCE
extern char *strdup (const char *s1);
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>
#include <Performer/pfdb/pfptu.h>
#include <Performer/image.h>
#include "ilgettile.h"

static void MakeTri(pfdGeoBuilder *b, 
		    pfVec3 co1, pfVec3 co2, pfVec3 co3, 
		    float tex1s, float tex1t, 
		    float tex2s, float tex2t, 
		    float tex3s, float tex3t);
static void MakeQuad(pfdGeoBuilder *b, 
		     pfVec3 co1, pfVec3 co2, pfVec3 co3, pfVec3 co4, 
		     float tex1s, float tex1t, float tex2s, float tex2t, 
		     float tex3s, float tex3t, float tex4s, float tex4t);

static TNode	    *NewTerrainNode(char *fileName);
static void	    MakeGStateList(TNode *T);
static void	    AttachLists(TNode *T);
static void	    MakeTextureList(TNode *T);
static void	    MakeGeodeList(TNode *T);

#define	USE_TEXGEN

static pfdGeom *geometry = NULL;
static pfGeoState *state = NULL;

pfNode *
pfdLoadFile_ptu (char *fileName)
{
    TNode	*T;
    
    /* restore builder to initial state */
    pfdResetBldrGeometry();
    pfdResetBldrState();

    /* space for four vertices */
    geometry = pfdNewGeom(4);

    T = NewTerrainNode(fileName);
    if (T == NULL)
	return (pfNode *)T;
    
    ildeffile(T->imageFile, T->imageFile, T->imageSizeX, T->imageSizeY);
    ildeffile(T->postFile, T->postFile, T->postSizeX, T->postSizeY);

    MakeTextureList(T);
    
    MakeGeodeList(T);
   
    ilfreetiles();
    
    MakeGStateList(T);
    
    AttachLists(T);
    
    pfNodeName(T->group, T->name);
    
    pfUserData(T->group, (void *)T);
    pfNotify(PFNFY_INFO, PFNFY_PRINT, 
	    "<terrain node>\t%s\n\t\tpost <%d %d>: %s\n\t\ttexture <%d %d>: %s\n\t\tnlods: %d\
	    \n\t\ttiles: %d\t%d\n\t\tscale: %f\t%f\t%f\n<end %s>\n", 
	    T->name, 
	    T->postSizeX, T->postSizeY, T->postFile, 
	    T->imageSizeX, T->imageSizeY, T->imageFile, 
	    T->nLODs, T->tilesX, T->tilesY, 
	    T->scaleX, T->scaleY, T->scaleZ, 
	    T->name);

    /* release storage used by geometry */
    pfdDelGeom(geometry);

    return (pfNode *)T->group;
}


static int defaultDetProps[5] = { 16, 16, 4, 4, 0 };

static void
MakeTri(pfdGeoBuilder *b, 
	pfVec3 co1, pfVec3 co2, pfVec3 co3, 
	float tex1s, float tex1t, 
	float tex2s, float tex2t, 	
	float tex3s, float tex3t)
{
    pfdGeom p, *pb;
    pfVec3 tmp1, tmp2;
    int	   t; 
    
    geometry->cbind = PFGS_PER_PRIM;
    geometry->nbind = PFGS_PER_PRIM;
#ifdef USE_TEXGEN
    geometry->tbind[0] = PFGS_OFF;
#else
    geometry->tbind[0] = PFGS_PER_VERTEX;
#endif

    for (t = 1 ; t < PF_MAX_TEXTURES ; t ++)
	geometry->tbind[t] = PFGS_PER_VERTEX;

    geometry->flags = 0;
    geometry->numVerts = 3;
    
   /* specify vertices */
    pfCopyVec3( geometry->coords[0], co1);
    pfCopyVec3( geometry->coords[1], co2);
    pfCopyVec3( geometry->coords[2], co3);

#ifdef USE_TEXGEN
#else
    /* specify texture coordinates */
    pfSetVec2(geometry->texCoords[0],	tex1s, tex1t);
    pfSetVec2(geometry->texCoords[1],   tex2s, tex2t); 
    pfSetVec2(geometry->texCoords[2],   tex3s, tex3t);
#endif

    /* specify color */
    pfSetVec4(geometry->colors[0], 1.0, 1.0, 1.0, 1.0);
				
    /* compute face normal */
    pfSubVec3(tmp1, geometry->coords[1], geometry->coords[0]);
    pfSubVec3(tmp2, geometry->coords[2], geometry->coords[1]);
    pfCrossVec3(geometry->norms[0], tmp1, tmp2);
    pfNormalizeVec3(geometry->norms[0]);
			    
    /* add face to builder */
    pfdAddPoly(b, geometry);
}

static void 
MakeQuad(pfdGeoBuilder *b, 
	 pfVec3 co1, pfVec3 co2, pfVec3 co3, pfVec3 co4, 
	 float tex1s, float tex1t, float tex2s, float tex2t, 
	 float tex3s, float tex3t, float tex4s, float tex4t)
{
    MakeTri(b, co1, co2, co4, tex1s, tex1t, tex2s, tex2t, tex4s, tex4t);
    MakeTri(b, co4, co2, co3, tex4s, tex4t, tex2s, tex2t, tex3s, tex3t);
}


static TNode*
NewTerrainNode(char *fileName)
{
    TNode	*T;
    FILE	*fp;
    char	buf[200];
    int		i;
    char	filePath[PF_MAXSTRING];
    

    if (pfFindFile(fileName, filePath, R_OK))
	fp = fopen(filePath, "r");
    else
	fp = NULL;

    if (fp == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, 
		 "pfdLoadFile_ptu: can't open file %s", fileName);
	return NULL;
    }
    
    T = (TNode *)pfMalloc(sizeof(TNode), pfGetSharedArena());
    
    fscanf(fp, "%s", buf);
    T->name = strdup(buf);

    fscanf(fp, "%d", &T->nLODs);
    fscanf(fp, "%d %d", &T->tilesX, &T->tilesY);
    fscanf(fp, "%f %f %f", &T->scaleX, &T->scaleY, &T->scaleZ);

    fscanf(fp, "%s", buf);
    T->postFile = strdup(buf);
    fscanf(fp, "%d %d", &T->postSizeX, &T->postSizeY);   
    
    fscanf(fp, "%s", buf);
    T->imageFile = strdup(buf);
    fscanf(fp, "%d %d", &T->imageSizeX, &T->imageSizeY);   
        
    fscanf(fp, "%s", buf);
    T->detailTextureFile = strdup(buf);
    T->detailTexture = pfNewTex(pfGetSharedArena());
    pfLoadTexFile(T->detailTexture, T->detailTextureFile);
    
    if (fscanf(fp, "%d %d %d %d %d", 
		    &T->detProps[0], &T->detProps[1], &T->detProps[2], 
		    &T->detProps[3], &T->detProps[4]) != 5)
	for(i=0;i<5;i++)
	    T->detProps[i] = defaultDetProps[i];

    T->postTileSizeX = T->postSizeX/T->tilesX;
    T->postTileSizeY = T->postSizeY/T->tilesY;
    T->imageTileSizeX = T->imageSizeX/T->tilesX;
    T->imageTileSizeY = T->imageSizeY/T->tilesY;
    
    return T;
}

static void
MakeGStateList(TNode *T)
{
    int i;
    pfMaterial *material = NULL;
    pfTexture *texture = NULL;
    pfTexGen *texgen = NULL;

    if (state == NULL)
    {
	state = pfNewGState(pfGetSharedArena());

	material = pfNewMtl(pfGetSharedArena());
        pfMtlColor(material, PFMTL_AMBIENT,  0.1, 0.1, 0.1);
        pfMtlColor(material, PFMTL_SPECULAR, 0.8, 0.8, 0.8);
        pfMtlShininess(material, 10.0);
        if (pfdGetMesherMode(PFDMESH_SHOW_TSTRIPS))
            pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_AD);
        else
            pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_COLOR);
	pfGStateAttr(state, PFSTATE_FRONTMTL, material);
	pfGStateMode(state, PFSTATE_ENLIGHTING, PF_ON);

	texture = pfNewTex(pfGetSharedArena());  
        pfLoadTexFile(texture, T->imageFile);
	pfTexName(texture, T->imageFile); 
#ifdef USE_TEXGEN
#else
	pfTexRepeat(texture, PFTEX_WRAP_T, PFTEX_CLAMP);
	pfTexRepeat(texture, PFTEX_WRAP_S, PFTEX_CLAMP);
#endif
	pfGStateAttr(state, PFSTATE_TEXTURE, texture);
	pfGStateMode(state, PFSTATE_ENTEXTURE, PF_ON);

#ifdef USE_TEXGEN
        {
	float xScale = (1.0f-(T->tilesX-1)/(float)T->postSizeX)
				     / (T->scaleX*T->postSizeX);
	float yScale = (1.0f-(T->tilesY-1)/(float)T->postSizeY)
				     / (T->scaleY*T->postSizeY);
	texgen = pfNewTGen(pfGetSharedArena());
	pfTGenMode(texgen, PF_S, PFTG_OBJECT_LINEAR);
	pfTGenPlane(texgen, PF_S, xScale, 0.0, 0.0, 0.0);
	pfTGenMode(texgen, PF_T, PFTG_OBJECT_LINEAR);
	pfTGenPlane(texgen, PF_T, 0.0, yScale, 0.0, 0.0);
	pfGStateAttr(state, PFSTATE_TEXGEN, texgen);
	pfGStateMode(state, PFSTATE_ENTEXGEN, PF_ON);
        }
#endif
    }
}

static void
AttachLists(TNode *T)
{
    int		i, j;
    
    T->group = pfNewGroup();
   
    for(j=0;j<T->tilesX*T->tilesY;j++)
    {
	pfLOD *tileLOD = pfNewLOD();
	
	pfAddChild(T->group, tileLOD);
	for(i=0;i<T->nLODs;i++)
	{
	    int k;
	    pfGeode *geode = (pfGeode *)pfGet(T->geodeList, i*T->tilesX*T->tilesY + j);
	    
	    pfAddChild(tileLOD, geode);
	    for(k=0;k<pfGetNumGSets(geode);k++)
	    {
		pfGeoSet *gset = pfGetGSet(geode, k);
		pfGSetGState(gset, state);
	    }
	}
	{
 	    pfBox box;
	    pfVec3 center;
	    float size;
	    int k;
	    
	    pfLODRange(tileLOD, 0, 0);
	    pfuTravCalcBBox(pfGetChild(tileLOD, 1), &box);
	    size = 2*pfDistancePt3(box.min, box.max);
	    pfCombineVec3(center, 0.5f, box.min, 0.5f, box.max);
	    pfLODCenter(tileLOD, center);

	    for(k=1;k<T->nLODs+2;k++)
	    {
		pfLODRange(tileLOD, k, size*k);
	    }
	}
    }    
}

static void
MakeTextureList(TNode *T)
{
    int	thisLOD, i, j;
    
    T->texList = pfNewList(sizeof(pfTexture *), 
			T->nLODs*T->tilesX*T->tilesY, 
			pfGetSharedArena());
        
    for(thisLOD = 0; thisLOD < T->nLODs; thisLOD++)
    {
	int	    realTileSizeX = (int)(T->imageTileSizeX/pow(2, thisLOD));
	int	    realTileSizeY = (int)(T->imageTileSizeY/pow(2, thisLOD));
	
	if (realTileSizeX == 0) realTileSizeX = 1;
	if (realTileSizeY == 0) realTileSizeY = 1;

	for(j=0; j < T->tilesY; j++)
	{
	    for(i=0; i < T->tilesX; i++)
	    {
		pfTexture	*tex;
	        unsigned long	*img;
	        unsigned int	*newimg;
		char		*cimg, *newcimg;
		int		k;
		
		img = (unsigned long *)ilgettile(T->imageFile, 
				    (float)(i*(T->imageTileSizeX-1)), 
				    (float)(j*(T->imageTileSizeY-1)), 
				    T->imageTileSizeX, T->imageTileSizeY, 
				    realTileSizeX, realTileSizeY);
				    
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
		    "ITILE(%d,%d): <%.3f %.3f> to <%.3f %.3f> at <%d %d>", 
			thisLOD, 
			i+j*T->tilesX,
			(float)(i*(T->imageTileSizeX-1)),
			(float)(j*(T->imageTileSizeY-1)),
			(float)(i*(T->imageTileSizeX-1)) + (T->imageTileSizeX-1), 
			(float)(j*(T->imageTileSizeY-1)) + (T->imageTileSizeX-1), 
			realTileSizeX, realTileSizeY);
		
		newimg = (unsigned int *)pfMalloc(realTileSizeX*realTileSizeY*sizeof(unsigned int), 
				  pfGetSharedArena());
		newcimg = (char *)newimg;
		cimg = (char *)img;
		for(k=0;k<realTileSizeX*realTileSizeY;k++)
		{
		    newcimg[k*3+0] = cimg[k*4+1];
		    newcimg[k*3+1] = cimg[k*4+2];
		    newcimg[k*3+2] = cimg[k*4+3];
		}
		if (pfGetNotifyLevel() == 5)
		{
		    int k, l;
		    
		    for(l=0;l<realTileSizeY;l++)
		    {
			fprintf(stderr, "ROW %3d: ", l);
			for(k=0;k<realTileSizeX;k++)
			    fprintf(stderr, "%3d ", 
				cimg[l*realTileSizeX*4 + k*4+0] +
				cimg[l*realTileSizeX*4 + k*4+1] +
				cimg[l*realTileSizeX*4 + k*4+2]);
			fprintf(stderr, "\n");
		    }
		}
		
 		tex = pfNewTex(pfGetSharedArena());  
		pfTexName(tex, T->imageFile); 
		pfTexImage(tex, newimg, 3, realTileSizeX, realTileSizeY, 1);
		pfTexRepeat(tex, PFTEX_WRAP_T, PFTEX_CLAMP);
		pfTexRepeat(tex, PFTEX_WRAP_S, PFTEX_CLAMP);
		pfAdd(T->texList, tex);
	    }
	 }
    }
}

static void
MakeGeodeList(TNode *T)
{
    int	realTileSizeX,	realTileSizeY;
    int	thisLOD, i, j, offset;
    
    T->geodeList = pfNewList(sizeof(long), 
			    T->nLODs*T->tilesX*T->tilesY, 
			    pfGetSharedArena());
    
    for(thisLOD = 0; thisLOD < T->nLODs; thisLOD++)
    {
   	pfVec3		*myData, *vertical, *horizontal;
	int		rImageTileSizeX, rImageTileSizeY;
	
	rImageTileSizeX = (int)(T->imageTileSizeX/pow(2, thisLOD));
	rImageTileSizeY = (int)(T->imageTileSizeY/pow(2, thisLOD));
	if (rImageTileSizeX < 1) rImageTileSizeX = 1;
	if (rImageTileSizeY < 1) rImageTileSizeY = 1;
	
	realTileSizeX = (int)(T->postTileSizeX/pow(2, thisLOD))-1;
	realTileSizeY = (int)(T->postTileSizeY/pow(2, thisLOD))-1;
	if (realTileSizeX < 1) realTileSizeX = 3;
	if (realTileSizeY < 1) realTileSizeY = 3;
	
	if (thisLOD == 0)
	{
	    horizontal = (pfVec3 *)pfMalloc(T->tilesX * (T->tilesY+1) * realTileSizeX * sizeof(pfVec3), pfGetSharedArena());
	    vertical = (pfVec3 *)pfMalloc((T->tilesX+1) * T->tilesY * realTileSizeY * sizeof(pfVec3), pfGetSharedArena());
	}
	
	myData = (pfVec3 *) pfMalloc(realTileSizeX*realTileSizeY*sizeof(pfVec3), pfGetSharedArena());	
	
	offset = (thisLOD != 0);

	for(j=0; j < T->tilesY; j++)
	{
	    for(i=0; i < T->tilesX; i++)
	    {
		pfdGeoBuilder	*b;
	        unsigned short		*img;
		int		k, m, n;

		float noffX = .5f/(float)rImageTileSizeX;
		float noffY = .5f/(float)rImageTileSizeY;
		float offX = (1.0f-2*noffX)/(float)(realTileSizeX-1);
		float offY = (1.0f-2*noffY)/(float)(realTileSizeY-1);		
		float hroffX = (1.0f - 2*noffX)/(float)(T->postTileSizeX-1);
		float hroffY = (1.0f - 2*noffY)/(float)(T->postTileSizeY-1);
		
		b = pfdNewGeoBldr();
		pfNotify(PFNFY_DEBUG, PFNFY_PRINT, 
			"PTILE(%d,%d): <%.3f %.3f> to <%.3f %.3f> at <%d %d>", 
			thisLOD, i+j*T->tilesX,
			(float)i*(float)(T->postTileSizeX-1)+.5f,
			(float)j*(float)(T->postTileSizeY-1)+.5f,
			(float)i*(float)(T->postTileSizeX-1)+.5f+T->postTileSizeX-1, 
			(float)j*(float)(T->postTileSizeY-1)+.5f+T->postTileSizeY-1, 
			realTileSizeX, realTileSizeY);
			
		img = (unsigned short *)ilgettile(T->postFile, 
				    (float)i*(float)(T->postTileSizeX-2)+.5f, 
				    (float)j*(float)(T->postTileSizeY-2)+.5f, 
				    T->postTileSizeX-1, T->postTileSizeY-1, 
				    realTileSizeX, realTileSizeY);

		for(k = 0; k < realTileSizeY; k++)
		    for(m = 0; m < realTileSizeX; m++)
			pfSetVec3(myData[k*realTileSizeX + m], 
			    (float)m * T->scaleX * (T->imageTileSizeX-1)/(realTileSizeY-1) +
			    (float)i * T->scaleX * (T->imageTileSizeX - 1), 
			    (float)k * T->scaleY * (T->imageTileSizeY-1)/(realTileSizeY-1) +
			    (float)j * T->scaleY * (T->imageTileSizeY - 1),  
			    (float)img[(k*realTileSizeX + m)] * T->scaleZ);

		if (pfGetNotifyLevel() == 5)
		{
		    fprintf(stderr, "\n");
		    for(k = 0; k < realTileSizeY; k++)
		    {
			for(m = 0; m < realTileSizeX; m++)
			    fprintf(stderr, "%3d ", 
				    img[(k*realTileSizeX + m)]);
			fprintf(stderr, "\n");
		    }
		    fprintf(stderr, "\n");
		}
		
		if (thisLOD == 0)
		{
 		    for(k = 0; k < realTileSizeX; k++)
			pfCopyVec3( horizontal[(T->tilesX*j+i) * realTileSizeX + k], 
				    myData[k]);
		    for(k = 0; k < realTileSizeY; k++)
			pfCopyVec3( vertical[((T->tilesX+1)*j+i) * realTileSizeY + k], 
				    myData[k*realTileSizeX]);
		    if (j == (T->tilesY-1))
			for(k=0; k < realTileSizeX; k++)
			    pfCopyVec3( horizontal[(T->tilesX*(j+1)+i) * realTileSizeX + k], 
					myData[realTileSizeX*(realTileSizeY-1) + k]);
		    if (i == (T->tilesX-1))
			for(k=0; k < realTileSizeY; k++)
			    pfCopyVec3( vertical[((T->tilesX+1)*j+i+1) * realTileSizeY + k], 
					myData[realTileSizeX * (k+1) - 1]);
					
		}
		else
		{
			int cur = 1;
			for(k = 1; k < realTileSizeY-1; k++)
			{   
			    while( (((float *)vertical[((T->tilesX+1)*j+i)*(T->postTileSizeY-1)+cur])[1] <= 
				    ((float *)myData[k * realTileSizeX + 1])[1])
				    && (cur < (T->postTileSizeY-1)))
			    {
				MakeTri(b, 
					myData[k * realTileSizeX + 1], 
					vertical[((T->tilesX+1)*j+i)*(T->postTileSizeY-1)+cur], 
					vertical[((T->tilesX+1)*j+i)*(T->postTileSizeY-1)+cur-1], 
					offX + noffX, offY*k + noffY, 
					noffX, hroffY*cur + noffY, 
					noffX, hroffY*(cur-1) + noffY);					
				cur++;
			    }
			    if ((k+1)<realTileSizeY-1)
			    {
				MakeTri(b, 
					myData[k * realTileSizeX + 1], 
					myData[(k+1) * realTileSizeX + 1], 
					vertical[((T->tilesX+1)*j+i)*(T->postTileSizeY-1)+(cur-1)], 
					offX + noffX, offY*k + noffY, 
					offX + noffX, offY*(k+1) + noffY, 
					noffX, hroffY*(cur-1) + noffY);
			    }
			    else
			    {
				while(cur < (T->postTileSizeY-1))
				{
				    MakeTri(b, 
					myData[k * realTileSizeX + 1], 
					vertical[((T->tilesX+1)*j+i)*(T->postTileSizeY-1)+cur], 
					vertical[((T->tilesX+1)*j+i)*(T->postTileSizeY-1)+cur-1], 
					offX + noffX, offY*k + noffY, 
					noffX, hroffY*cur + noffY, 
					noffX, hroffY*(cur-1) + noffY);
				    cur++;  
				}
			    }
			}
			cur = 1;
			for(k = 1; k < realTileSizeX-1; k++)
			{   
			    while( (	((float *)horizontal[(T->tilesX*j+i)*(T->postTileSizeX-1)+cur])[0] <= 
					((float *)myData[k + realTileSizeX])[0])
					&& (cur < (T->postTileSizeX-1)))
			    {
				MakeTri(b, 
					horizontal[(T->tilesX*j+i)*(T->postTileSizeX-1)+cur-1], 
					horizontal[(T->tilesX*j+i)*(T->postTileSizeX-1)+cur], 
					myData[k + realTileSizeX], 
					(cur-1)*hroffX + noffX, noffY, 
					cur*hroffX + noffX, noffY, 
					k*offX + noffX, noffY + offY); 
				cur++;
			    }
			    if ((k+1)<realTileSizeX-1)
			    {
				MakeTri(b, 
					horizontal[(T->tilesX*j+i)*(T->postTileSizeX-1)+(cur-1)], 
					myData[(k + realTileSizeX+1)], 
					myData[k + realTileSizeX], 
					(cur-1)*hroffX + noffX, noffY, 
					(k+1)*offX + noffX, noffY + offY, 
					k*offX + noffX, noffY + offY);
			    }
			    else
			    {
				while(cur < (T->postTileSizeX-1))
				{
				    MakeTri(b, 
					horizontal[(T->tilesX*j+i)*(T->postTileSizeX-1)+cur-1], 
					horizontal[(T->tilesX*j+i)*(T->postTileSizeX-1)+cur], 
					myData[k + realTileSizeX], 
					(cur-1)*hroffX + noffX, noffY, 
					cur*hroffX + noffX, noffY, 
					k*offX + noffX, noffY + offY); 
				    cur++;  
				}
			    }
			}
			cur = 1;
			for(k = 1; k < realTileSizeX-1; k++)
			{   
			    int n = k + realTileSizeX*(realTileSizeY - 2);
			    while( (	((float *)horizontal[(T->tilesX*(j+1)+i)*(T->postTileSizeX-1)+cur])[0] <= 
					((float *)myData[n])[0]) 
					&& (cur < (T->postTileSizeX-1)))
			    {
				MakeTri(b, 
					myData[n], 
					horizontal[(T->tilesX*(j+1)+i)*(T->postTileSizeX-1)+cur], 
					horizontal[(T->tilesX*(j+1)+i)*(T->postTileSizeX-1)+cur-1], 
					k*offX + noffX, 1.0 - (noffY + offY), 
					cur*hroffX + noffX, 1.0 - noffY, 
					(cur-1)*hroffX + noffX, 1.0 - noffY);
				cur++;
			    }
			    if ((k+1)<realTileSizeX-1)
			    {
				MakeTri(b, 
					myData[n], 
					myData[n+1], 
					horizontal[(T->tilesX*(j+1)+i)*(T->postTileSizeX-1)+(cur-1)], 
					k*offX + noffX, 1.0 - (noffY + offY), 
					(k+1)*offX + noffX, 1.0 - (noffY + offY), 
					(cur-1)*hroffX + noffX, 1.0 - noffY); 
			    }
			    else
			    {
				while(cur < (T->postTileSizeX-1))
				{
				    MakeTri(b, 
					myData[n], 
					horizontal[(T->tilesX*(j+1)+i)*(T->postTileSizeX-1)+cur], 
					horizontal[(T->tilesX*(j+1)+i)*(T->postTileSizeX-1)+cur-1], 
					k*offX + noffX, 1.0 - (noffY + offY), 
					cur*hroffX + noffX, 1.0 - noffY, 
					(cur-1)*hroffX + noffX, 1.0 - noffY); 
				    cur++;  
				}
			    }
			}
			cur = 1;
			for(k = 1; k < realTileSizeY-1; k++)
			{   
			    int n = (k+1) * realTileSizeX - 2;
			    while( (	((float *)vertical[((T->tilesX+1)*j+i+1)*(T->postTileSizeY-1)+cur])[1] <= 
					((float *)myData[k * realTileSizeX + 1])[1])
					&& (cur < (T->postTileSizeY-1)))
			    {
				MakeTri(b, 
					vertical[((T->tilesX+1)*j+i+1)*(T->postTileSizeY-1)+cur-1], 
					vertical[((T->tilesX+1)*j+i+1)*(T->postTileSizeY-1)+cur], 
					myData[n], 
					1.0 - noffX, hroffY*(cur-1) + noffY, 
					1.0 - noffX, hroffY*cur + noffY, 
					1.0 - (offX + noffX), offY*k + noffY);
				cur++;
			    }
			    if ((k+1)<realTileSizeY-1)
			    {
				MakeTri(b, 
					vertical[((T->tilesX+1)*j+i+1)*(T->postTileSizeY-1)+(cur-1)], 
					myData[n+realTileSizeX], 
					myData[n], 
					1.0 - noffX, hroffY*(cur-1) + noffY, 
					1.0 - (offX + noffX), offY*(k+1) + noffY, 
					1.0 - (offX + noffX), offY*k + noffY);
			    }
			    else
			    {
				while(cur < (T->postTileSizeY-1))
				{
				    MakeTri(b, 
					vertical[((T->tilesX+1)*j+i+1)*(T->postTileSizeY-1)+cur-1], 
					vertical[((T->tilesX+1)*j+i+1)*(T->postTileSizeY-1)+cur], 
					myData[n], 
					1.0 - noffX, hroffY*(cur-1) + noffY, 
					1.0 - noffX, hroffY*cur + noffY, 
					1.0 - (offX + noffX), offY*k + noffY); 
				    cur++;  
				}
			    }
			}

		}
		
		for(k = offset; k < realTileSizeY - offset - 1; k++)
		{
		    for(m = offset; m < realTileSizeX - offset - 1; m++)
		    {
			MakeQuad(   b, 
				    myData[(k * realTileSizeX + m)], 
				    myData[(k * realTileSizeX + m + 1)], 
				    myData[((k + 1) * realTileSizeX + m + 1)], 
				    myData[((k + 1) * realTileSizeX + m)], 
				    offX*(m+0) + noffX, offY*(k+0) + noffY, 
				    offX*(m+1) + noffX,	offY*(k+0) + noffY, 
				    offX*(m+1) + noffX, offY*(k+1) + noffY, 
				    offX*(m+0) + noffX, offY*(k+1) + noffY);
		    }
		}
		{
		    int ii;
		    
		    pfGeode *newGeode = pfNewGeode();
		    const pfList *tmpList = pfdBuildGSets(b);

		    n = pfGetNum(tmpList);
		    for(ii=0;ii<n;ii++)
		    {
			pfAddGSet(newGeode, (pfGeoSet *)pfGet(tmpList, ii));
		    }
		    pfAdd(T->geodeList, newGeode);
		    pfdDelGeoBldr(b);
		}
	    }
	}
	pfFree(myData);
	if (thisLOD == (T->nLODs - 1))
	{
	    pfFree(vertical);
	    pfFree(horizontal);
	}
    }
}

#ifdef USE_DETAIL
	if (i < 3*T->tilesX*T->tilesY)
	{
	    static float	detClamp = 0.75f;
	    static pfVec2	detSpline[4];
	    pfSetVec2(detSpline[0], 0.0f, .5*0.0f*T->scaleY);
	    pfSetVec2(detSpline[1], 2.0f, .5*0.3f*T->scaleY);
	    pfSetVec2(detSpline[2], 4.0f, .5*0.6f*T->scaleY);
	    pfSetVec2(detSpline[3], 6.0f, .5*2.0f*T->scaleY);
/*	    pfTexDetail(pfGet(T->texList, i), PFTEX_DEFAULT, T->detailTexture);*/
	    pfTexDetail(pfGet(T->texList,i), 6-(log((float)T->detProps[3])/M_LN2), T->detailTexture);
	    pfTexSpline(pfGet(T->texList, i), PFTEX_DETAIL_SPLINE, detSpline, detClamp);
    	    pfTexFilter(pfGet(T->texList,i), PFTEX_MAGFILTER_COLOR, PFTEX_ADD_DETAIL);
       /* 
     * XXX !!! need to use 
     *	level = 6 - (log((float) T->detProps[3]) / M_LN2);
     *	pfDetailTex(baseTex, level, T->detailTexture) 
     */
/*    	    pfDetailTexTile(	T->detailTexture, T->detProps[0], T->detProps[1], 
				T->detProps[2], T->detProps[3], T->detProps[4]);*/
	    

	}
#endif
