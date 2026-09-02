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

#include <Performer/pr/pfImageCache.h>
#include <Performer/pr/pfClipTexture.h>
#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pfutil.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#define FOR(i,n) for ((i) = 0; (i) < (n); ++(i))
#define INRANGE(foo,bar,baz) ((foo(bar))&&((bar)baz))
#define log2(x) (log(x)*M_LOG2E)
#define round(x) floor((x)+.5)

#define MAXLEVELS 32


static void
hsv_to_rgb(float h, float s, float v, float *R, float *G, float *B)
{
    int segment;
    float *rgb[3], major, minor, middle, frac;

    rgb[0] = R;
    rgb[1] = G;
    rgb[2] = B;

    while (h < 0)
        h++;
    while (h >= 1)
        h--;

    segment = (int)(h*6);

    frac = (6*h)-segment;
    if (segment % 2)
        frac = 1 - frac;

    major = v;
    minor = (1-s)*v;
    middle = frac * major + (1-frac) * minor;

    *rgb[(segment+1)/2 % 3] = major;
    *rgb[(segment+4)/2 % 3] = minor;
    *rgb[(7-segment) % 3] = middle;
}


class pfImageCacheViewer : public pfObject
{
public:
    pfImageCacheViewer()
    {
	tileShrink = .5;
	coordMap = NULL;
	coordMapArg = NULL;
	freeCoordMapArg = NULL;

	geostate = new(getArena()) pfGeoState;
	assert(geostate != NULL);
	geostate->setMode(PFSTATE_ENTEXTURE, PF_ON);

	int level;
	FOR (level, MAXLEVELS)
	{
	    geosets[level] = new(getArena()) pfGeoSet;
	    assert(geosets[level] != NULL);
	    pfRef(geosets[level]);
	    /* geosets[level]->setGState(geostate); */

	    nquads[level] = 0;
	    geosets[level]->setNumPrims(nquads[level]);
	    geosets[level]->setPrimType(PFGS_QUADS);
	    nquads_alloced[level] = 0;
	}
    }
    ~pfImageCacheViewer()
    {
	int level;
	setCoordMap(NULL,NULL,NULL);
	FOR (level, MAXLEVELS)
	{
	    set_nquads_alloced(level, 0);
	    assert(pfGetRef(geosets[level]) == 1);
	    pfMemory::unrefDelete(geosets[level]);
	}
    }
    void setTileShrink(float newTileShrink)
    {
	tileShrink = newTileShrink;
    }
    void setCoordMap(void (*newCoordMap)(double,double,double*,double*,void*),
		     void *newCoordMapArg,
		     void (*newFreeCoordMapArg)(void*))
    {
	if (freeCoordMapArg != NULL)
	    (*freeCoordMapArg)(coordMapArg);
	coordMap = newCoordMap;
	coordMapArg = newCoordMapArg;
	freeCoordMapArg = newFreeCoordMapArg;
    }

    void updateLevelFromImageCache(int level, pfImageCache *icache)
    {
	if (icache == NULL)
	{
	    nquads[level] = 0;
	    geosets[level]->setNumPrims(nquads[level]);
	    return;
	}

	int orgS, orgT, orgR; /* origin of icache, in tiles from ll of image */
	int nTilesS, nTilesT, nTilesR;
	int s, t, r; /* in tiles */
	int tileWidth, tileHeight, tileDepth;
	int imageWidth, imageHeight, imageDepth;
	icache->getMemRegionOrg(&orgS, &orgT, &orgR);
	icache->getMemRegionSize(&nTilesS, &nTilesT, &nTilesR);
	icache->getImageSize(&imageWidth, &imageHeight, &imageDepth);
	pfImageTile *itile = icache->getProtoTile();
	itile->getSize(&tileWidth, &tileHeight, &tileDepth);
	float logimageWidth = log2(imageWidth);

	if (nTilesS*nTilesT*nTilesR > nquads_alloced[level])
	    set_nquads_alloced(level, nTilesS*nTilesT*nTilesR);
	
	nquads[level] = 0;
	float z = level / 1000.;

	FOR (r, nTilesR)
	FOR (t, nTilesT)
	FOR (s, nTilesS)
	{
	    itile = icache->getTile(orgS+s, orgT+t, orgR+r);
	    int loading, valid, useDefault;
	    if (!(loading = itile->isLoading()))
		if (!(valid = itile->isValid()))
		    useDefault = itile->getDefaultTileMode();

	    double center[2];
	    int tileOriginS, tileOriginT, tileOriginR;
	    itile->getOrigin(&tileOriginS,&tileOriginT,&tileOriginR);
	    PFSET_VEC2(center, (tileOriginS+.5*tileWidth)/imageWidth,
			       (tileOriginT+.5*tileHeight)/imageHeight);
	    double vradiusS = tileShrink*(.5*tileWidth)/imageWidth;
	    double vradiusT = tileShrink*(.5*tileHeight)/imageHeight;
	    double tradiusS = (.5*tileWidth)/imageWidth;
	    double tradiusT = (.5*tileHeight)/imageHeight;
	    double double_vc[4][2];
	    double double_tc[4][2];
	    PFSET_VEC2(double_vc[0],center[PF_X]-vradiusS,center[PF_Y]-vradiusT);
	    PFSET_VEC2(double_vc[1],center[PF_X]+vradiusS,center[PF_Y]-vradiusT);
	    PFSET_VEC2(double_vc[2],center[PF_X]+vradiusS,center[PF_Y]+vradiusT);
	    PFSET_VEC2(double_vc[3],center[PF_X]-vradiusS,center[PF_Y]+vradiusT);
	    PFSET_VEC2(double_tc[0],center[PF_X]-tradiusS,center[PF_Y]-tradiusT);
	    PFSET_VEC2(double_tc[1],center[PF_X]+tradiusS,center[PF_Y]-tradiusT);
	    PFSET_VEC2(double_tc[2],center[PF_X]+tradiusS,center[PF_Y]+tradiusT);
	    PFSET_VEC2(double_tc[3],center[PF_X]-tradiusS,center[PF_Y]+tradiusT);
	    int i;

	    if (coordMap != NULL)
		FOR (i, 4)
		    coordMap(double_vc[i][0], double_vc[i][1], &double_vc[i][0], &double_vc[i][1], coordMapArg);

	    FOR (i, 4)
	    {
		PFSET_VEC3(vc[level][nquads[level]][i],
			   double_vc[i][PF_X],
			   -z,
			   double_vc[i][PF_Y]);
		PFCOPY_VEC2(tc[level][nquads[level]][i], double_tc[i]);
	    }
	    FOR (i, 4)
	    {
		assert(INRANGE(0 <=, double_vc[i][PF_X], <= 1));
		assert(INRANGE(0 <=, double_vc[i][PF_Y], <= 1));
	    }

	    float hue, sat, val;
	    if (loading)
	    {
		hue = (logimageWidth-3)/6.0f;
		sat = .5;
		val = 1.;
	    }
	    else if (valid)
	    {
		hue = (logimageWidth-3)/6.0f;
		sat = .9;
		val = 1.;
	    }
	    else if (useDefault)
	    {
		static float defaultSat = -1, defaultVal = -1;
		if (sat < 0)
		{
		    char *e = getenv("_GEOFLY_DEFAULT_SAT");
		    defaultSat = (e ? *e ? atof(e) : .5 : .5);
		    e = getenv("_GEOFLY_DEFAULT_VAL");
		    defaultVal = (e ? *e ? atof(e) : .3 : .3);
		    printf("default sat = %g, val = %g\n", sat, val);
		}
		hue = (logimageWidth-3)/6.0f;
		sat = defaultSat;
		val = defaultVal;
	    }
	    else
	    {
		/* waiting on this one! white */
		hue = 1;
		sat = 0;
		val = 1;
	    }

	    float red,green,blue;
	    hsv_to_rgb(hue,sat,val,&red,&green,&blue);
	    FOR (i, 4)
		PFSET_VEC4(colors[level][nquads[level]][i], red,green,blue,1.0f);

	    ++nquads[level];
	}
	geosets[level]->setNumPrims(nquads[level]);
    }

    void updateFromClipTexture(pfClipTexture *cliptex)
    {
	int w,h,d;
	int clipSize = cliptex->getClipSize();
	cliptex->getVirtualSize(&w,&h,&d);
	int nlevels = (int)round(log2(w));
	int level;
	FOR (level, nlevels)
	{
	    pfObject *obj = cliptex->getLevel(nlevels-1 - level);

	    if (obj != NULL
	     && obj->isOfType(pfImageCache::getClassType())
	     && (1<<level) >= clipSize) /* don't draw big hidden quads */
		    updateLevelFromImageCache(level, (pfImageCache *)obj);
	    else
		    updateLevelFromImageCache(level, NULL);
	}

	geosets[level]->getGState()->setAttr(PFSTATE_TEXTURE, cliptex);
    }

    /*
     * Draw all levels from coarsest to finest,
     * scaled so that the virtual image takes up the range 0..1 x 0..1.
     * The hue of a tile is chosen to match the gridification color
     * of its level (see gridify.C).
     * Tiles that are resident in memory are drawn fully saturated;
     * tiles that are scheduled to be read but not yet resident
     * are drawn with low saturation.
     */
    void draw()
    {
	int level;
	FOR (level, MAXLEVELS)
	{
	    if (nquads[level] != 0)
	    {
		/*
		if (pfGetNotifyLevel() >= PFNFY_DEBUG)
		{
		    fprintf(stderr, "Drawing geoset for level %d:\n", level);
		    pfPrint(geosets[level], 0, PFPRINT_VB_DEBUG, stderr);
		}
		*/
		int levels_to_enable_texture = 1<<11; /* make this a member */
		if (levels_to_enable_texture & (1<<level))
		{
		    geosets[level]->getGState()->setMode(PFSTATE_ENTEXTURE, PF_ON);
		    geosets[level]->getGState()->apply(); /* XXX ??? */
		}

		geosets[level]->draw();

		if (levels_to_enable_texture & (1<<level))
		{
		    geosets[level]->getGState()->setMode(PFSTATE_ENTEXTURE, PF_OFF);
		    geosets[level]->getGState()->apply(); /* XXX ??? */
		}
	    }
	}
    }



private:
    void set_nquads_alloced(int level, int new_nquads_alloced)
    {
	if (new_nquads_alloced != nquads_alloced[level])
	{
	    if (nquads_alloced[level] != 0)
	    {
		geosets[level]->setAttr(PFGS_COORD3, PFGS_PER_VERTEX,
			NULL, NULL);
		geosets[level]->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX,
			NULL, NULL);
		geosets[level]->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX,
			NULL, NULL);
		assert(pfGetRef(geosets[level]) == 1);
		pfMemory::unrefDelete(geosets[level]);
		assert(pfGetRef(vc[level]) == 1);
		pfMemory::unrefDelete(vc[level]);
		assert(pfGetRef(tc[level]) == 1);
		pfMemory::unrefDelete(tc[level]);
		assert(pfGetRef(colors[level]) == 1);
		pfMemory::unrefDelete(colors[level]);
	    }

	    nquads_alloced[level] = new_nquads_alloced;

	    if (new_nquads_alloced > 0)
	    {
		vc[level] = (pfVec3 (*)[4])pfMalloc(nquads_alloced[level] * sizeof(*vc[level]), getArena());
		assert(vc[level] != NULL);
		pfRef(vc[level]);

		tc[level] = (pfVec2 (*)[4])pfMalloc(nquads_alloced[level] * sizeof(*tc[level]), getArena());
		assert(tc[level] != NULL);
		pfRef(tc[level]);

		colors[level] = (pfVec4 (*)[4])pfMalloc(nquads_alloced[level] * sizeof(*colors[level]), getArena());
		assert(colors[level] != NULL);
		pfRef(colors[level]);

	    }
	    geosets[level]->setAttr(PFGS_COORD3, PFGS_PER_VERTEX,
				    vc[level], NULL);
	    geosets[level]->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX,
				    tc[level], NULL);
	    geosets[level]->setAttr(PFGS_COLOR4, PFGS_PER_VERTEX,
				    colors[level], NULL);
	}
    }

private:
    float tileShrink;	/* 1 --> don't shrink, 0 --> shrink to nothing */
    void (*coordMap)(double, double, double*, double*, void*); /* warp from image 0..1 to displayed 0..1
			   so that the area of interest may be drawn bigger */
    void *coordMapArg; /* closure for coordMap function */
    void (*freeCoordMapArg)(void*); /* function to free coordMapArg */

    pfGeoSet *geosets[MAXLEVELS];
    pfGeoState *geostate;
    int nquads_alloced[MAXLEVELS];
    int nquads[MAXLEVELS];
    pfVec3 (*vc[MAXLEVELS])[4];
    pfVec2 (*tc[MAXLEVELS])[4];
    pfVec4 (*colors[MAXLEVELS])[4];
};

//
// External C api...
//
extern "C" {

PFUDLLEXPORT pfImageCacheViewer *pfuNewImageCacheViewer(void)
{
    return new(pfGetSharedArena()) pfImageCacheViewer;
}
PFUDLLEXPORT void pfuDrawImageCacheViewer(pfImageCacheViewer *viewer)
{
    viewer->draw();
}
PFUDLLEXPORT
void pfuUpdateImageCacheViewerFromClipTexture(pfImageCacheViewer *viewer,
					 pfClipTexture *cliptex)
{
    viewer->updateFromClipTexture(cliptex);
}
PFUDLLEXPORT
void pfuImageCacheViewerTileShrink(pfImageCacheViewer *viewer, float tileShrink)
{
    viewer->setTileShrink(tileShrink);
}
PFUDLLEXPORT
void pfuImageCacheViewerCoordMap(
	pfImageCacheViewer *viewer,
	void (*newCoordMap)(double,double,double*,double*,void*),
        void *newCoordMapArg,
        void (*newFreeCoordMapArg)(void*))
{
    viewer->setCoordMap(newCoordMap, newCoordMapArg, newFreeCoordMapArg);
}

} // end extern "C"
