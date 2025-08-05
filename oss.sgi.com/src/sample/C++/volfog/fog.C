//
// Copyright 1999, 2000, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// This source code ("Source Code") was originally derived from a
// code base owned by Silicon Graphics, Inc. ("SGI")
// 
// LICENSE: SGI grants the user ("Licensee") permission to reproduce,
// distribute, and create derivative works from this Source Code,
// provided that: (1) the user reproduces this entire notice within
// both source and binary format redistributions and any accompanying
// materials such as documentation in printed or electronic format;
// (2) the Source Code is not to be used, or ported or modified for
// use, except in conjunction with OpenGL Performer; and (3) the
// names of Silicon Graphics, Inc.  and SGI may not be used in any
// advertising or publicity relating to the Source Code without the
// prior written permission of SGI.  No further license or permission
// may be inferred or deemed or construed to exist with regard to the
// Source Code or the code base of which it forms a part. All rights
// not expressly granted are reserved.
// 
// This Source Code is provided to Licensee AS IS, without any
// warranty of any kind, either express, implied, or statutory,
// including, but not limited to, any warranty that the Source Code
// will conform to specifications, any implied warranties of
// merchantability, fitness for a particular purpose, and freedom
// from infringement, and any warranty that the documentation will
// conform to the program, or any warranty that the Source Code will
// be error free.
// 
// IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
// LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
// ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
// SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
// OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
// PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
// OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
// USE, THE SOURCE CODE.
// 
// Contact information:  Silicon Graphics, Inc., 
// 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
// or:  http://www.sgi.com
//
// fog.C
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfPipe.h>
#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfMaterial.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pf/pfBillboard.h>

#include <Performer/pr/pfLight.h>


#include "fog.h"


#define XRANGE (2.5*M_PI)
#define ZRANGE (3*M_PI)

/*************************************/
static int terrainType;


/************************************************************************/
static float invrand = 1.0/(RAND_MAX);

static float rand2(float range)
{
    return (float)rand() * invrand * range;
}
    
/************************************************************************/
#define TERR_SCALE 100

float terrainFunction(float x, float z, int type) 
{

    switch(type) {
    case 0:
	return (TERR_SCALE*fsin((x)+(z))*fcos((x)-(z)));

    case 1:
	return (TERR_SCALE*fsin((x)+(z))*fcos((x)-(z)) + 
		TERR_SCALE*0.4*fsin((x*3)+(z*2))*fcos((x*2)-(z*3)));
    }

    return 0;
}


int fogRes = 0;

static float *terrainFog;


/***********************************************************/
pfNode * MakeGroundFog(float height, float randomness, float variance, 
		       pfBox *bbox)
{
    int i,j;
    float stepx, stepz, x, z, ter;
    int PatchStep = 4;
    float xstep,zstep;
    float xl, zl;
    int u,v;
    float xmin,xmax,zmin,zmax;

    xmin = bbox->min[0];
    xmax = bbox->max[0];
    zmin = bbox->min[1];
    zmax = bbox->max[1];

    if(fogRes == 0)
	fogRes = 40;

    fogRes =  1 + PatchStep * ((fogRes + PatchStep-1)/PatchStep);

    terrainFog = new float[fogRes*fogRes];

    if(!terrainFog) {
	printf("Note enough memory for fog\n");
	exit(0);
    }
    
    if(height == 0)
      height = 25;

    x = -XRANGE*0.5;
    stepx = XRANGE/(float)(fogRes-1);

    for(i=0; i<fogRes; i++) {
	z = -ZRANGE*0.5;
	stepz = ZRANGE/(float)(fogRes-1);

	for(j=0; j<fogRes; j++) {
	    ter = terrainFunction(x,z,terrainType) +
		(6+rand2(5))*fsin(7*(x+z))*fcos(7*(x-z));

	    terrainFog[i + fogRes*j] = 
		height * (1 + rand2(randomness)) * 
		(1 - variance * ter / (float)TERR_SCALE) + ter;
	    
	    z+=stepz;
	}
	x+=stepx;
    }


    xstep = (xmax-xmin+2)/(fogRes-1);
    zstep = (zmax-zmin+2)/(fogRes-1);
    
    

    pfGeoState *gstate = new pfGeoState;
    gstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
    

    // material - only for debugging
#if 0
    pfMaterial *mat = new pfMaterial;
    mat->setColor(PFMTL_DIFFUSE, 0.3, 0.6, 1.0);
    mat->setColor(PFMTL_AMBIENT, 0.0, 0.0, 0.0);
    gstate->setAttr(PFSTATE_FRONTMTL, (void *)mat);
    gstate->setAttr(PFSTATE_BACKMTL, (void *)mat);
#endif

    pfGeode *fog = new pfGeode;

#if 1   
    z = zmin-1;

    int *lengths = new int[PatchStep];

    for(i=0;i<PatchStep;i++)
	lengths[i] = PatchStep*2+2;

    for(i=0;i<fogRes-1;i+=PatchStep) {
	x = xmin-1;
	for(j=0;j<fogRes-1;j+=PatchStep) {
	    pfGeoSet *gset = new pfGeoSet;
	    int index;
		
	    zl = z;
	    
	    gset->setPrimType(PFGS_TRISTRIPS);
	    gset->setPrimLengths(lengths);
	    gset->setNumPrims(PatchStep);
		
	    pfVec3 *coords = new pfVec3[PatchStep * (PatchStep*2 + 2)];
		
	    index = 0;

	    for(u=0;u<PatchStep;u++) {
		
		xl = x;
				
		for(v=0;v<=PatchStep;v++) {
		  coords[index++].set(xl, zl+zstep, 
				      terrainFog[(i+u+1)*fogRes+j+v]);
		  coords[index++].set(xl, zl, terrainFog[(i+u)*fogRes+j+v]);
		  xl += xstep;
		}
		zl += zstep;
	    }
	    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
	
	    gset->setGState(gstate);
   
	    fog->addGSet(gset);
	    
	    x += PatchStep*xstep;
	}
	z += PatchStep*zstep;
    }
#endif
    //return fog;

    // close the fog boundary at height -200
    int *lengths2 = new int[2];
    int index = 0;
    pfGeoSet *gset = new pfGeoSet;
    
    lengths2[0] = 4;
    lengths2[1] = 2*(1+(fogRes-1)*4);

    gset->setPrimType(PFGS_TRISTRIPS);
    gset->setPrimLengths(lengths2);
    gset->setNumPrims(2);
    
    float yb=-200;

    pfVec3 *coords = new pfVec3[lengths2[0] + lengths2[1]];

    /* bottom poly */
    coords[index++].set(xmin-1, zmin-1, yb);
    coords[index++].set(xmin-1, zmax+1, yb);
    coords[index++].set(xmax+1, zmin-1,  yb);
    coords[index++].set(xmax+1, zmax+1,  yb);
	
    /* sides */
    z = zmin-1;
    for(i=0; i < fogRes; i++) {
	coords[index++].set(xmin-1, z, yb);
	coords[index++].set(xmin-1, z, terrainFog[i*fogRes]);
	z+=zstep;
    }
    x = xmin-1;
    for(j=1; j < fogRes; j++) {
	x+=xstep;
	coords[index++].set(x, zmax+1, yb);
	coords[index++].set(x, zmax+1, terrainFog[(fogRes-1)*fogRes + j]);
    }
    z = zmax+1;
    for(i=fogRes-2; i >= 0; i--) {
	z-=zstep;
	coords[index++].set(xmax+1, z, yb);
	coords[index++].set(xmax+1, z, terrainFog[i*fogRes + fogRes-1]);
    }
    x = xmax+1;
    for(j=fogRes-2; j >= 0; j--) {
	x-=xstep;
	coords[index++].set(x, zmin-1, yb);
	coords[index++].set(x, zmin-1, terrainFog[j]);
    }

    if(index != lengths2[0] +  + lengths2[1]) 
	printf("Warning: incorrect number of vertices in fog geoset\n");

#if 0
    pfGeoState *gstate2 = new pfGeoState;
    gstate2->setMode(PFSTATE_ENLIGHTING, PF_OFF);

    pfMaterial *mat2 = new pfMaterial;
    mat2->setColor(PFMTL_DIFFUSE, 1.0, 0.6, 0.3);
    mat2->setColor(PFMTL_AMBIENT, 0.0, 0.0, 0.0);
    gstate2->setAttr(PFSTATE_FRONTMTL, (void *)mat2);
    gstate2->setAttr(PFSTATE_BACKMTL, (void *)mat2);
#endif

    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setGState(gstate);
	
    fog->addGSet(gset);
    // XXX add a callback that would not draw the closed part if not required


    return fog;
}

/***************************************************************************/
/* 
** To be able to determine whether we are inside a fogged area or outside
** the boundary has to be closed !!! 
**
** Often, we don't need the full closed boundary when the geometry is drawn.
** The function parmeter closed then specifies, whether we have to draw all
** boundaries or not.
*/
pfNode *MakeOneLayerFog(pfBox *bbox, float top, float bottom)
{
    pfGeode *fog = new pfGeode;
    pfGeoSet *gset = new pfGeoSet;
    float xmin,xmax,zmin,zmax;
    int i,j;

#ifdef __linux__
    /* linux is sensitive to the size of fog boundaries */
#define N 10
#else
#define N 1
#endif

    xmin = bbox->min[0];
    xmax = bbox->max[0];
    zmin = bbox->min[1];
    zmax = bbox->max[1];

    int *lengths = new int[2*N+1];
    
    for(i=0;i<2*N;i++)
      lengths[i] = 2*(N+1);
    lengths[i] = 2*(4*N+1);
    
    gset->setPrimType(PFGS_TRISTRIPS);
    gset->setPrimLengths(lengths);
    gset->setNumPrims(2*N+1);
    
    pfVec3 *coords = new pfVec3[2*N*2*(N+1) + 2*(4*N+1)];

    int index = 0;

#define X(x) (xmin-1 + (float)(x)/(float)(N)*(xmax-xmin+2))
#define Z(x) (zmin-1 + (float)(x)/(float)(N)*(zmax-zmin+2))

    // top poly
    for(i=0;i<N;i++)
      for(j=0;j<=N;j++) {
	coords[index++].set(X(i), Z(j), top);
	coords[index++].set(X(i+1), Z(j), top);
      }

    // bottom poly 
    for(i=0;i<N;i++)
      for(j=0;j<=N;j++) {
	coords[index++].set(X(i+1), Z(j), bottom);
	coords[index++].set(X(i), Z(j), bottom);
      }
    
    // sides 
    for(i=0;i<=N;i++) {
      coords[index++].set(X(0), Z(i), bottom);
      coords[index++].set(X(0), Z(i), top);
    }
    for(i=1;i<=N;i++) {
      coords[index++].set(X(i), Z(N), bottom);
      coords[index++].set(X(i), Z(N), top);
    }
    for(i=N-1;i>=0;i--) {
      coords[index++].set(X(N), Z(i), bottom);
      coords[index++].set(X(N), Z(i), top);
    }
    for(i=N-1;i>=0;i--) {
      coords[index++].set(X(i), Z(0), bottom);
      coords[index++].set(X(i), Z(0), top);
    }
    
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);

    pfGeoState *gstate = new pfGeoState;
    gset->setGState(gstate);
	
    fog->addGSet(gset);
    // XXX add a callback that would not draw the closed part if not required


    return fog;
}

