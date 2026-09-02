/*
 * Copyright 2000, Silicon Graphics, Inc.
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

#include <Performer/pfdu.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef mips
#include <bstring.h>
#else
#include <string.h>
#endif
#include <math.h>

#ifndef WIN32
#include <sys/resource.h>
#endif

static int defaultNumRing(pfASD *asd, pfASDLODRange *lods, int numlods);
static float edgeLength(pfASDFace *faces, pfASDVert *verts, int faceid);

PFDUDLLEXPORT void
pfdASDClipring(pfASD *asd, int numrings)
{
    int lodi;
    pfASDLODRange *lods;
    int numlods;
    float length, maxlength;

    if(asd == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, 
	    "the pfASD node is NULL");
	return;
    }
	
    pfGetASDAttr(asd, PFASD_LODS, NULL, &numlods, (void**)&lods);
    if(lods == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, 
            "the pfASD LODRange is not set yet");
	return;
    }

    if(numrings < 0)
    	numrings = defaultNumRing(asd, lods, numlods);
    pfNotify(PFNFY_DEBUG, PFNFY_PRINT, "pfdASDClipring:numrings is %d\n", numrings);

    for(lodi = numlods; lodi < numlods+numrings; lodi++)
    {
        lods[lodi].switchin = lods[lodi-1].switchin/2;
        lods[lodi].morph = 0.6*lods[lodi].switchin;
    }

    numlods += numrings;

    pfASDAttr(asd, PFASD_LODS, NULL, numlods, lods);
    pfASDEnableClipRings(asd);
}

#ifdef WIN32
#define M_LOG2E 1.4426950408889634074
#endif
#define log2(x) (log(x)*M_LOG2E)

static int
defaultNumRing(pfASD *asd, pfASDLODRange *lods, int numlods)
{
    int facei, numfaces, numverts, numfaces0, numsample;
    float length;
    pfASDFace *faces, *face;
    pfASDVert *verts;

    pfGetASDAttr(asd, PFASD_FACES, NULL, &numfaces, (void**)&faces);
    pfGetASDAttr(asd, PFASD_COORDS, NULL, &numverts, (void**)&verts);
    numfaces0 = pfGetASDNumBaseFaces(asd);
    if(faces == NULL)
    {
        pfNotify(PFNFY_WARN, PFNFY_PRINT,
            "the pfASD faces are not set yet");
	return 0;
    }

    if(lods[0].switchin == 0)
    {
        pfNotify(PFNFY_WARN, PFNFY_PRINT,
            "the pfASD LODRange value is 0");
	return 0;
    }

    numsample = 0;
    length = 0;
    for(facei = 0; (numsample<6)&&(facei<numfaces0); facei++)
    {
  	length += edgeLength(faces, verts, facei);
	numsample++;
    }
    length /= numsample;
    if (lods[numlods-1].switchin <= 0)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		 "pfdASDClipring: last (smallest) LOD %d has switchin = %f, no way to figure out how many additional clip rings are necessary",
		 numlods-1, lods[numlods-1].switchin);
	return 0;
    }
    return (int)log2(PF_MAX2(lods[numlods-1].switchin*2 / length, 1.));
}

static float 
edgeLength(pfASDFace *faces, pfASDVert *verts, int faceid)
{
    int i;
    float max, length; 
    pfASDFace *face;

    face = &faces[faceid];
    if((face->child[0] == PFASD_NIL_ID) && (face->child[1] == PFASD_NIL_ID) &&
	(face->child[2] == PFASD_NIL_ID) && (face->child[3] == PFASD_NIL_ID))
    {
	max = 0;
   	for(i = 0; i < 3; i++)
	{
	    length = pfDistancePt3(verts[face->vert[i]].v0, verts[face->vert[(i+1)%3]].v0);
	    if(max < length)
		max = length;
	}
	printf("max length is %f\n", max);
	return max;
    }
	
    i = 0;
    while(i < 4)
	if(face->child[i] == PFASD_NIL_ID)
	    i++;
	else
	    break;

    return(edgeLength(faces, verts, face->child[i]));
}

