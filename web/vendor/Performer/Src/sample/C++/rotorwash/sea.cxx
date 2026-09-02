/*
 * Copyright 1999, 2000, Silicon Graphics, Inc.
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

#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <math.h>

#include <Performer/pr/pfGeoState.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfRotorWash.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pr/pfFlux.h>
#include <Performer/pr/pfList.h>
#include <Performer/pr/pfMaterial.h>
#include <Performer/pfutil.h>

#define ROTATE
#define NUM_FRAMES 16
#define MESH_SIZE 150.0f
#define RES       60
#define VPOS ((j*2*RES) + i*2)

pfGeoSet *fluxedGset;

static pfVec3 *norms;
static int *lengths;

pfTexture *TiTex[NUM_FRAMES];

void UpdateSea(pfVec3 *eye)
{
    int i;
    pfVec3 V;
    float fres;
    pfGeoSet *gset;
    pfGeoState *gstate;
    pfVec4 *Dcolors;
    pfVec3 *coords;
    unsigned short *dummyList;
    static float phase = 0.0f;
    static int frame_count = 0;

    frame_count ++;
    if (frame_count >= NUM_FRAMES*2) frame_count = 0;

    gset = (pfGeoSet *)((pfFlux *)fluxedGset)->getWritableData();
    gstate = gset->getGState();

    gset->getAttrLists(PFGS_COORD3, (void **)&coords, &dummyList);
    gset->getAttrLists(PFGS_COLOR4, (void **)&Dcolors, &dummyList);

    phase += .05;
    if(phase > M_PI*2.0f) phase -= M_PI*2.0f;
    if(TiTex[frame_count/2])
      gstate->setAttr(PFSTATE_TEXTURE, TiTex[frame_count/2]);
  
    if(eye)
    {

      // We know the ocean is horizontal so
      // it's simpler just to reflect z not the
      // v around N

      for(i = 0; i < 2*RES*RES; i++)
      {
        coords[i][2] =  (sinf(phase + 0.25f*coords[i][1]) + 0.65f * cosf(phase + 0.15f * coords[i][1]+coords[i][0]));

        norms[i].set(-sinf(phase + 0.15f * coords[i][1]+coords[i][0]), sinf(phase + 0.15f * coords[i][1]+coords[i][0]) - cosf(phase + 0.25f * coords[i][1])  , 2.0f);
        norms[i].normalize();

        V = coords[i] - *eye;
        V.normalize();
        fres = V.dot(norms[i]);

        Dcolors[i].set(0.2f, 0.35f+.05f * fres, .35f+.15f * fres, 1.0f );
      }
    }

    ((pfFlux *)fluxedGset)->writeComplete();
}

static int makeFluxedGset(pfFluxMemory *fmem)
{
    pfGeoState *gstate;
    pfGeoSet   *gset;
    pfVec4 *Dcolors;
    pfVec3 *coords;
    pfVec2 *Btex;
    int i,j;
    float step;

    if (fmem == NULL)
	return pfFluxedGSetInit(fmem);

    pfFluxedGSetInit(fmem);

    gset = (pfGeoSet*)fmem->getData();

    pfTexEnv *tev = new pfTexEnv;
    tev->setMode(PFTE_MODULATE);

    gstate = new pfGeoState;
    gstate->setMode(PFSTATE_TRANSPARENCY, PFTR_OFF);
    gstate->setMode(PFSTATE_ENTEXTURE, PF_ON);
    gstate->setMode(PFSTATE_ENLIGHTING,0);
    gstate->setMode(PFSTATE_CULLFACE,PFCF_OFF);
    gstate->setAttr(PFSTATE_TEXENV, tev);
    gstate->setAttr(PFSTATE_TEXTURE, TiTex[0]);

    Dcolors = (pfVec4*) new(2*RES*RES*sizeof(pfVec4)) pfMemory;
    coords = (pfVec3*) new(2*RES*RES*sizeof(pfVec3) ) pfMemory;
    Btex = (pfVec2*) new(2*RES*RES*sizeof(pfVec2)) pfMemory;

    step = 2.0 / (float)(RES-1);

    // ceiling information
    for(j = 0; j < RES; j++)
    {
      *(lengths+j) = 2*RES;
      for(i = 0; i < RES; i++)
      {
        coords[VPOS].set(-MESH_SIZE+i*MESH_SIZE*step, -MESH_SIZE+j*MESH_SIZE*step, 0.0f );
        coords[VPOS+1].set(-MESH_SIZE+i*MESH_SIZE*step, -MESH_SIZE+(j+1)*MESH_SIZE*step, 0.0f );

        norms[VPOS].set(0.0f, 0.0f, 1.0f);
        norms[VPOS+1].set(0.0f, 0.0f, 1.0f);

        Btex[VPOS].set(.15f * coords[VPOS][1], .15f * coords[VPOS][0]);
        Btex[VPOS+1].set(.15f * coords[VPOS+1][1], .15f * coords[VPOS+1][0]);
      }
    }

    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, 0);
    gset->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX, Dcolors, 0);
    gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, Btex, 0);
    gset->setPrimLengths(lengths);

    gset->setPrimType(PFGS_TRISTRIPS);
    gset->setNumPrims(RES);
    gset->setGState(gstate);

    return 0;
}


pfNode *MakeSea(int animate)
{
    int i;

    // Set up geoset

    lengths = (int *) new(RES*sizeof(int)) pfMemory;
    norms = (pfVec3*) new(2*RES*RES*sizeof(pfVec3)) pfMemory;

    for(i=0; i<NUM_FRAMES; i++)
    {
      char fname[80];
      if(!i || animate)
      {
        TiTex[i] = new pfTexture;
        TiTex[i]->setFilter(PFTEX_MINFILTER, PFTEX_MIPMAP_TRILINEAR);
        TiTex[i]->setFilter(PFTEX_MAGFILTER, PFTEX_BILINEAR);
        TiTex[i]->setRepeat(PFTEX_WRAP_S, PFTEX_REPEAT);
        TiTex[i]->setRepeat(PFTEX_WRAP_T, PFTEX_REPEAT);
        if(i < 10)
          sprintf(fname, "Animations_small/Ranim0%1d.bw", i);
        else
          sprintf(fname, "Animations_small/Ranim%2d.bw", i);
        TiTex[i]->loadFile(fname);
        TiTex[i]->setFilter(PFTEX_MAGFILTER, PFTEX_BILINEAR);
      }
      else TiTex[i] = NULL;
    }

    pfGeode *geode0 = new pfGeode;


    fluxedGset = (pfGeoSet *)new pfFlux(makeFluxedGset,
					PFFLUX_DEFAULT_NUM_BUFFERS);
    geode0->addGSet(fluxedGset);

    pfGroup *group = new pfGroup;
    group->addChild(geode0);

    return (pfNode  *) group;
}



/************************************************************************/
/************************************************************************/

extern pfRotorWash *wash_mesh;

typedef struct terrain_vertex {
    float normal[3];
    float height;
} terrain_vertex;

static terrain_vertex *terrain = NULL;
#define XRANGE (2.5*M_PI)
#define ZRANGE (3*M_PI)

#define TERR_SCALE 5

/************************************************************************/

float terrainFunction(float x, float z, int type) 
{
#define RAD 50
#define PEAK 15

    switch(type) {
    case 0:
	return (TERR_SCALE*fsin((x)+(z))*fcos((x)-(z)));

    case 1:
	return -RAD*150+PEAK + 150*fsqrt(RAD*RAD - x*x-z*z) + 
	    (TERR_SCALE*fsin((x)+(z))*fcos((x)-(z)) + 
	     TERR_SCALE*0.4*fsin((x*3)+(z*2))*fcos((x*2)-(z*3)));
    }

    return 0;
}

/************************************************************************/
pfNode *MakeLand(int res, int ftype) 
{
    float x,z,stepx,stepz,delta,scalex,scalez;
    float derx, derz, nx, ny, nz, invlen;
    float xmax, xmin, zmax, zmin;
    int i,j;
    float xstep,zstep;
    float xl, zl;
    int u,v;
    int PatchStep = 4;
    pfGroup *root = new pfGroup;
    pfGeode *terr = new pfGeode;
    int terrRes;

    xmax =  MESH_SIZE*0.4*4/3;
    xmin = -MESH_SIZE*0.4*4/3;
    zmax =  MESH_SIZE*0.4*4/3;
    zmin = -MESH_SIZE*0.4*4/3;

    x = -XRANGE*0.5;

    terrRes =  1 + PatchStep * ((res + PatchStep-1)/PatchStep);

    if(!terrain) {
	terrain = new terrain_vertex[terrRes*terrRes*sizeof(terrain_vertex)];
	if(!terrain) {
	    printf("Not enough memory for terrain mesh\n");
	    exit(0);
	}
    }


    stepx = XRANGE/(float)(terrRes-1);
    /* 1 unit in x or z is how much in eye-coords */
    scalex = (xmax-xmin)/(-2*x);

    delta =  stepx/10.0;

    for(i=0; i<terrRes; i++) {
	z = -ZRANGE*0.5;
	stepz = ZRANGE/(float)(terrRes-1);
	scalez = (zmax-zmin)/(-2*z);

	for(j=0; j<terrRes; j++) {
	    terrain[i+terrRes*j].height = terrainFunction(x,z,ftype);

	    /*printf("%d,%d: %g\n", i,j,terrain[i+terrRes*j]);*/
	    
	    derx = terrainFunction(x+delta,z,ftype) - terrainFunction(x-delta,z,ftype);
	    derz = terrainFunction(x,z+delta,ftype) - terrainFunction(x,z-delta,ftype);
	    
	    /* normal is a cross products of vectors (0,derz,2*delta*scalez)
	       and  (2*delta*scalex,derx,0) */
	    /* note the result is predivided by 2*delta */
	    nx = -derx*scalez;
	    ny = 2*delta*scalex*scalez;
	    nz = -derz*scalex;
	    
	    /* normalize */
	    invlen = 1.0/fsqrt(nx*nx + ny*ny + nz*nz);
	    terrain[i+terrRes*j].normal[0] = nx *invlen;
	    terrain[i+terrRes*j].normal[1] = ny *invlen;
	    terrain[i+terrRes*j].normal[2] = nz *invlen;	    
	    
	    z+=stepz;
	}
	x+=stepx;
    }


    xstep = (xmax-xmin)/(terrRes-1);
    zstep = (zmax-zmin)/(terrRes-1);

#define TEX_SCALE 0.03
    /* terrain */

    /* create a set of square areas and compute a 4 point bbox for each
     */
    
    // material
    pfMaterial *mat = new pfMaterial;
    mat->setColor(PFMTL_AMBIENT, 0.0, 0.0, 0.0);
    mat->setColor(PFMTL_DIFFUSE, 0.9, 1.0, 0.15);
    mat->setColor(PFMTL_SPECULAR, 0.0, 0.0, 0.0);
    mat->setColor(PFMTL_EMISSION, 0.0, 0.0, 0.0);
    mat->setColorMode(PFMTL_BOTH, PFMTL_CMODE_AMBIENT_AND_DIFFUSE);
    
    // texture
    pfTexture *tex = new pfTexture;
    tex->setFilter(PFTEX_MINFILTER, PFTEX_BILINEAR);
    tex->setFilter(PFTEX_MAGFILTER, PFTEX_BILINEAR);
    tex->setRepeat(PFTEX_WRAP_S, PFTEX_REPEAT);
    tex->setRepeat(PFTEX_WRAP_T, PFTEX_REPEAT);
    tex->loadFile("ground.rgb");

    pfTexEnv *tev = new pfTexEnv;
    tev->setMode(PFTE_MODULATE);


    pfGeoState *gstate = new pfGeoState;
    gstate->setMode(PFSTATE_TRANSPARENCY, PFTR_OFF);
    gstate->setMode(PFSTATE_CULLFACE,PFCF_OFF);
    gstate->setMode(PFSTATE_ENLIGHTING, PF_ON);
    gstate->setMode(PFSTATE_ENTEXTURE, PF_ON);
    gstate->setMode(PFSTATE_ENTEXGEN, PF_OFF);
    gstate->setAttr(PFSTATE_FRONTMTL, (void *)mat);
    gstate->setAttr(PFSTATE_BACKMTL, (void *)mat);
    gstate->setAttr(PFSTATE_TEXTURE, (void *)tex);
    gstate->setAttr(PFSTATE_TEXENV, tev);

#if 1
    wash_mesh->setTextures(&gstate, 1, "Animations_small/land", 16);
    wash_mesh->setColor(gstate, 0.3, 0.25, 0.1, 1);
#else
    // use the same textures on land as on the water
    wash_mesh->setTextures(&gstate, 1, NULL, 0);
    wash_mesh->setColor(gstate, 0.1, 0.05, 0, 0.75);
#endif

    /* only one layer */
    xstep = (xmax-xmin)/(terrRes-1);
    zstep = (zmax-zmin)/(terrRes-1);


    z = zmin;

    int *lengths = (int *)new (sizeof(int)*PatchStep) pfMemory;

    for(i=0;i<PatchStep;i++)
	lengths[i] = PatchStep*2+2;

    for(i=0;i<terrRes-1;i+=PatchStep) {
	x = xmin;
	for(j=0;j<terrRes-1;j+=PatchStep) {
	    pfGeoSet *gset = new pfGeoSet;
	    int index;
		
	    zl = z;
	    
	    gset->setPrimType(PFGS_TRISTRIPS);
	    gset->setPrimLengths(lengths);
	    gset->setNumPrims(PatchStep);
		
	    pfVec3 *coords = (pfVec3 *)new (sizeof(pfVec3)*PatchStep * (PatchStep*2 + 2)) pfMemory;
	    pfVec3 *normals = (pfVec3 *)new (sizeof(pfVec3)*PatchStep * (PatchStep*2 + 2)) pfMemory;
	    pfVec2 *texcoords = (pfVec2 *)new (sizeof(pfVec2)*PatchStep * (PatchStep*2 + 2)) pfMemory;
		
	    index = 0;

	    for(u=0;u<PatchStep;u++) {
		
		xl = x;
		//if(zl>zmax+zstep*0.5)
		//    break;
				
		for(v=0;v<=PatchStep;v++) {
		  //if(xl > xmax+xstep*0.5)
		  //    break;
		  coords[index].set(xl, zl+zstep, terrain[(i+u+1)*terrRes+j+v].height);
		  texcoords[index].set(xl*TEX_SCALE, (zl+zstep)*TEX_SCALE);
		  normals[index++].set(terrain[(i+u+1)*terrRes+j+v].normal[0],
				       terrain[(i+u+1)*terrRes+j+v].normal[2],
				       terrain[(i+u+1)*terrRes+j+v].normal[1]);
		  
		  coords[index].set(xl, zl, terrain[(i+u)*terrRes+j+v].height);
		  texcoords[index].set(xl*TEX_SCALE, zl*TEX_SCALE);
		  normals[index++].set(terrain[(i+u)*terrRes+j+v].normal[0],
				       terrain[(i+u)*terrRes+j+v].normal[2],
				       terrain[(i+u)*terrRes+j+v].normal[1]);
		  xl += xstep;
		}
		zl += zstep;
	    }
	    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
	    gset->setAttr(PFGS_NORMAL3, PFGS_PER_VERTEX, normals, NULL);
	    gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, texcoords, NULL);
	
	    gset->setGState(gstate);
    
	    terr->addGSet(gset);
	    
	    x += PatchStep*xstep;
	}
	z += PatchStep*zstep;
    }

    root->addChild(terr);	    


    return root;
}
