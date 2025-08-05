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

#include <Performer/pfdu.h>
#include <Performer/pr/pfGeoState.h>
#include "clipdemo.h"

// 10 x 10
#define XSIZE 10
#define YSIZE 10
//
#define SCALE 52428.8

int ctWidth, ctHeight;

static int numElevels = 16;
static int Offset = 0;


void incElevels(void)
{
    numElevels ++;
    if (numElevels > 16)
        numElevels = 16;
    else
        Shared->mpcliptex->setNumEffectiveLevels(numElevels);
}

void decElevels(void)
{
    numElevels --;
    if (numElevels < 1)
        numElevels = 1;
    else
        Shared->mpcliptex->setNumEffectiveLevels(numElevels);
}

void incOffset(void)
{
    Offset ++;
    if (Offset > 16)
        Offset = 16;
    else
        Shared->mpcliptex->setVirtualLODOffset(Offset);
}

void decOffset(void)
{
    Offset --;
    if (Offset < 0)
        Offset = 0;
    else
        Shared->mpcliptex->setVirtualLODOffset(Offset);
}

pfGroup *MakeClipDbase(char *fname, float download_time)
{
    int i, j;

    printf("###### Loading Clip texture %s\n", fname);
    Shared->clptex = pfdLoadClipTexture(fname);
    if(!Shared->clptex)
      return(NULL);
    Shared->clptex->getVirtualSize(&ctWidth, &ctHeight, NULL);
    //Shared->clptex->setFilter(PFTEX_MAGFILTER, PFTEX_POINT);
    Shared->clptex->setFilter(PFTEX_MAGFILTER, PFTEX_BILINEAR);

    printf("###### Clip texture Size, %d, %d\n", ctWidth, ctHeight);
    Shared->mpcliptex = new pfMPClipTexture();
    Shared->mpcliptex->setClipTexture(Shared->clptex);
    Shared->mpcliptex->setNumEffectiveLevels(numElevels);
    Shared->mpcliptex->setVirtualLODOffset(Offset);
    Shared->mpcliptex->setLODRange(0, 21);
    Shared->mpcliptex->setLODBias(0, 0, 0);
    Shared->mpcliptex->setDTRMode(PF_DTR_MEMLOAD | PF_DTR_TEXLOAD | PF_DTR_READSORT);
    Shared->mpcliptex->setTexLoadTime(download_time);
    Shared->mpcliptex->setCenter(0.5f*ctWidth, 0.5f*ctHeight, 0.0f);

    pfGeoState *Cgstate = new pfGeoState;
    Cgstate->setMode(PFSTATE_TRANSPARENCY, PFTR_OFF);
    Cgstate->setMode(PFSTATE_ENTEXTURE, PF_ON);
    Cgstate->setMode(PFSTATE_ENLIGHTING, PF_OFF);
    Cgstate->setMode(PFSTATE_CULLFACE, PFCF_OFF);
    Cgstate->setMode(PFSTATE_ENWIREFRAME, PF_OFF);
    Cgstate->setMode(PFSTATE_CULLFACE, PFCF_OFF);

    pfTexEnv *Ctenv = new pfTexEnv;
    Ctenv->setMode(GL_MODULATE);
    Cgstate->setMode(PFSTATE_ENTEXTURE, PF_ON);
    Cgstate->setAttr(PFSTATE_TEXENV, Ctenv);

    Cgstate->setAttr(PFSTATE_TEXTURE, Shared->clptex);

    pfGroup *terrain = new pfGroup;

    for (i = 0; i<XSIZE; i++)
        for (j = 0; j<YSIZE; j++)
        {
            pfGeode *geode = new pfGeode;
            // Set up geoset
            int *Clengths = (int *) new(sizeof(int)) pfMemory;
            pfVec4 *Ccolors = (pfVec4*) new(sizeof(pfVec4)) pfMemory;
            pfVec2 *Ctex = (pfVec2*) new(4*sizeof(pfVec2)) pfMemory;
            pfVec3 *Cvert = (pfVec3*) new ( 4*sizeof(pfVec3)) pfMemory;

            *Clengths = 4;
            Ccolors->set(1.0f, 1.0f, 1.0f, 1.0f);
                Ctex->set(      i/(float)XSIZE,      j/(float)YSIZE );
            (Ctex+1)->set(  (i+1)/(float)XSIZE,      j/(float)YSIZE );
            (Ctex+2)->set(      i/(float)XSIZE,  (j+1)/(float)YSIZE );
            (Ctex+3)->set(  (i+1)/(float)XSIZE,  (j+1)/(float)YSIZE );
                Cvert->set(      SCALE*i,      SCALE*j, 0.0f);
            (Cvert+1)->set(  SCALE*(i+1),      SCALE*j, 0.0f);
            (Cvert+2)->set(      SCALE*i,  SCALE*(j+1), 0.0f);
            (Cvert+3)->set(  SCALE*(i+1),  SCALE*(j+1), 0.0f);
            pfGeoSet *gset = new pfGeoSet;
            gset->setNumPrims(1);
            gset->setPrimType(PFGS_TRISTRIPS);
            gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, Cvert, NULL);
            gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, Ccolors, NULL);
            gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, Ctex, NULL);
            gset->setPrimLengths(Clengths);
            gset->setGState(Cgstate);

            geode->addGSet(gset);
            //clipCB(geode);
            terrain->addChild(geode);
        }

    return terrain;
}

void UpdateClipCenter(float eyex, float eyey)
{
    double s, t;

    s = eyex/((double)XSIZE*(double)SCALE);
    if (s < 0.0)
        s = 0.0;
    else if (s > 1.0)
        s = 1.0;
    t = eyey/((double)YSIZE*(double)SCALE);
    if (t < 0.0)
        t = 0.0;
    else if (t > 1.0)
        t = 1.0;

    Shared->mpcliptex->setCenter(s*ctWidth, t*ctHeight, 0.0f);

}
