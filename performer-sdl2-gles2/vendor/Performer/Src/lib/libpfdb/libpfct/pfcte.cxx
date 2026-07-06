//
//
// Copyright 2001, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
// in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
// in similar or successor clauses in the FAR, or the DOD or NASA FAR
// Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
// 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
// PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
// GRAPHICS, INC.
//
// pfcte.c	Cliptexture Emulation Loader
//
//

/******************************************************************************/


#define MAXSUBDIV 100
static int SUBDIV = 1;

static double CURVATURE = 0;
static double LON0 = -.5;
static double LON1 = .5;
static double LAT0 = -.5;
static double LAT1 = .5;
static double S0 = 0.;
static double S1 = 1.;
static double T0 = 0.;
static double T1 = 1.;

/******************************************************************************/

#include <stdlib.h>
#include <math.h>
#if !defined( __linux__) && !defined(WIN32)
#include <mutex.h>
#endif

#ifdef WIN32
#include <windows.h>
//#include "wintypes.h"

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <Performer/pf.h>

#include <Performer/pr/pfGenericMatrix.h>

#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfDoubleDCS.h>
#include <Performer/pf/pfMPClipTexture.h>
#include <Performer/pr/pfClipTexture.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfFlux.h>
#include <Performer/pr/pfCycleBuffer.h>
#include <Performer/pf/pfTraverser.h>
#include <Performer/pr/pfTextureValidMap.h>
#include <Performer/pfutil/pfuClipCenterNode.h>
#include <Performer/pr/pfGeoMath.h>

/******************************************************************************/

#define FOR(i,n) for ((i) = 0; (i) < (n); ++(i))
#define round(x) floor((x)+.5)
#define log2(x) (log(x)*M_LOG2E)
#define intlog2(x) ((int)round(log2(x)))/* could make this faster if critical */
#define LERP(a,b,t) ((a) + (t)*((b)-(a)))
#define ATOF_GETENV(name, default) (getenv(name) ? atof(getenv(name)) : (default))
#define DTOR_ATOF_GETENV(name, default) (getenv(name) ? atof(getenv(name))*(M_PI/180.) : (default))

/******************************************************************************/

typedef struct CTE_CallbackParams
{
    pfClipTexture *cliptex;
    pfMPClipTexture *mpcliptex;    

    int nLevels, vSize, clipSize;
    
    pfGroup *geodesGroup;
    pfGeode *proxyGeode;
    
    int nDonuts;
    int invalidBorder;
    
} CTE_CallbackParams;


/******************************************************************************/

static int CTE_forceALL_INCull(pfTraverser *, void *)
{
    pfCullResult(PFIS_MAYBE | PFIS_TRUE | PFIS_ALL_IN);
    return PFTRAV_CONT;
}

/******************************************************************************/

static pfMPClipTexture *CTE_findMPClipTexture(pfClipTexture *cliptex)
{
    int npipes = pfGetMultipipe();
    int i;
    for (i = 0; i < npipes; ++i)
    {
        pfPipe *pipe = pfGetPipe(i);
        int n_mpcliptextures = pipe->getNumMPClipTextures();
        int j;
        for (j = 0; j < n_mpcliptextures; ++j)
        {
            pfMPClipTexture *mpcliptex = pipe->getMPClipTexture(j);
            if (mpcliptex->getClipTexture() == cliptex)
                return mpcliptex;
        }
    }
    return NULL;
}

/******************************************************************************/

/*
 * Calculate 10 vertices forming a triangle strip (8 triangles)
 * in the shape of a square with a smaller square cut out of the middle.
 */
static void
CTE_GetDonut(double centerx, double centery,
         float bigdiameter, float littlediameter,
	 int subdiv,
         pfVec2 verts[/* 8*subdiv + 2 */],
	 double s0, double t0,
	 double s1, double t1)
{
    double r0 = PF_MIN2(littlediameter*.5, .5);
    double r1 = PF_MIN2(bigdiameter*.5, .5);
    double c0x = PF_CLAMP(centerx, s0+r0, s1-r0);
    double c0y = PF_CLAMP(centery, t0+r0, t1-r0);
    double c1x = PF_CLAMP(centerx, s0+r1, s1-r1);
    double c1y = PF_CLAMP(centery, t0+r1, t1-r1);

    pfVec2d coarseVerts[10];
    PFSET_VEC2(coarseVerts[0], c0x-r0, c0y-r0);
    PFSET_VEC2(coarseVerts[1], c1x-r1, c1y-r1);
    PFSET_VEC2(coarseVerts[2], c0x+r0, c0y-r0);
    PFSET_VEC2(coarseVerts[3], c1x+r1, c1y-r1);
    PFSET_VEC2(coarseVerts[4], c0x+r0, c0y+r0);
    PFSET_VEC2(coarseVerts[5], c1x+r1, c1y+r1);
    PFSET_VEC2(coarseVerts[6], c0x-r0, c0y+r0);
    PFSET_VEC2(coarseVerts[7], c1x-r1, c1y+r1);
    PFSET_VEC2(coarseVerts[8], c0x-r0, c0y-r0);
    PFSET_VEC2(coarseVerts[9], c1x-r1, c1y-r1);

    int i;
    int j = 0;
    FOR (i, subdiv)
    {
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[0],
				   i/(double)subdiv, coarseVerts[2]);
	j++;
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[1],
				   i/(double)subdiv, coarseVerts[3]);
	j++;
    }
    FOR (i, subdiv)
    {
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[2],
				   i/(double)subdiv, coarseVerts[4]);
	j++;
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[3],
				   i/(double)subdiv, coarseVerts[5]);
	j++;
    }
    FOR (i, subdiv)
    {
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[4],
				   i/(double)subdiv, coarseVerts[6]);
	j++;
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[5],
				   i/(double)subdiv, coarseVerts[7]);
	j++;
    }
    FOR (i, subdiv+1)
    {
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[6],
				   i/(double)subdiv, coarseVerts[8]);
	j++;
	PFCOMBINE_VEC2(verts[j], 1-i/(double)subdiv, coarseVerts[7],
				   i/(double)subdiv, coarseVerts[9]);
	j++;
    }
}

/******************************************************************************/

static inline double sqr(double x) { return x*x; }
/* 1-cos(t), doesn't lose accuracy for small t */
static inline double cos1(double t) { return 2.*sqr(sin(.5*t)); }
/* 1-cos(a)*cos(b), doesn't lose accuracy for small a or b */
static inline double cos1(double a, double b) { return cos1(a) + cos(a)*cos1(b); }

static void
CTE_BendDonut(double curvature, int subdiv, pfVec2 tcoords[/*subdiv*/], pfVec3d vcoords[/*subdiv*/],
	double lon0, double lon1, double lat0, double lat1,
	double s0,   double s1,   double t0,   double t1)
{
    double c = curvature;
    int i;
    FOR (i, 8*subdiv+2)
    {
	double s = tcoords[i][PF_S];
	double t = tcoords[i][PF_T];
	if (c == 0) // XXX find the right condition!
	{
	    vcoords[i][PF_X] = LERP(LON0, LON1, s);
	    vcoords[i][PF_Y] = 0;
	    vcoords[i][PF_Z] = LERP(LAT0, LAT1, t);
	}
	else
	{
	    double lon = LERP(lon0, lon1, s) * c;
	    double lat = LERP(lat0, lat1, t) * c;

	    double z = sin(lat) / c;
	    double x = cos(lat)*sin(lon) / c;
	    double y = cos1(lat,lon) / c;

	    vcoords[i][PF_X] = x;
	    vcoords[i][PF_Y] = y;
	    vcoords[i][PF_Z] = z;

	    vcoords[i][PF_Y] -= 1/c; //XXX center at origin? WRONG! inconsistent
	    vcoords[i][PF_X] += .5;
	    vcoords[i][PF_Z] += .5;
	}
	// tcoords was in 0..1 x 0..1, changing to in s0..s1 x t0..t1
	tcoords[i][PF_S] = LERP(s0, s1, s);
	tcoords[i][PF_T] = LERP(t0, t1, t);
    }
}


/******************************************************************************/

//
// Post-app callback on the top node
// morphs the geometry to adapt to the current clip center.
//
static int
CTE_callbackGroupPostApp(pfTraverser *, void *arg)
{
    CTE_CallbackParams *cbData = (CTE_CallbackParams*)arg;
    if (cbData->mpcliptex == NULL)
    {
	cbData->mpcliptex = CTE_findMPClipTexture(cbData->cliptex);
	PFASSERTALWAYS(cbData->mpcliptex != NULL);
    }


    

    int centerS, centerT;
    cbData->mpcliptex->getCenter(&centerS, &centerT, NULL);
    double centerX = ((double)centerS / cbData->vSize - S0) / (S1 - S0);
    double centerY = ((double)centerT / cbData->vSize - T0) / (T1 - T0);


    pfGroup *geodesGroup = cbData->geodesGroup;
    int i, j, nDonuts = cbData->nDonuts;

    cbData->invalidBorder = cbData->mpcliptex->getInvalidBorder();


    /*printf("\n\nvirtSize=%d  clipSize=%d   invBorder=%d   nDonuts=%d\n\n", 
	cbData->vSize, cbData->clipSize, cbData->invalidBorder, nDonuts);*/





    int top = (centerT<cbData->vSize)? 1:0;
    int right = (centerS<cbData->vSize)? 1:0;


    
    FOR (i, nDonuts)
    {
	pfNode *child = geodesGroup->getChild(i);
	PFASSERTALWAYS(child->isOfType(pfGeode::getClassType()));
	pfGeode *geode = (pfGeode *)child;
	pfGeoSet *geoset = geode->getGSet(0);
	
	//struct DonutCullParams *cullParams = (struct DonutCullParams *)
	//	geode->getTravData(PFTRAV_CULL);	
	//float bigdiameter = cullParams->bigDiameter;
	//float littlediameter = cullParams->littleDiameter;

	double bigdiameter;
	double littlediameter;


	{
	    int align = 8<<(nDonuts-i);
	    
	    int goodarea = cbData->clipSize -(cbData->invalidBorder<<1);
	    
	    int bigdiam_i = (i==0)? cbData->vSize: ((goodarea<<(nDonuts-1-i)) - (align<<1));

	    int littlediam_i = (i==(nDonuts-1))? 0 : ((goodarea<<(nDonuts-2-i)) - align);

	    /*pfNotify( PFNFY_ALWAYS, PFNFY_PRINT, 
		"DONUT %d:goodarea=%d  bigdiam=%d  littlediam=%d  align=%d  ", 
		    i, goodarea, bigdiam_i, littlediam_i, align );*/
		
	    bigdiameter = (double)bigdiam_i/cbData->vSize;
	    littlediameter = (double)littlediam_i/cbData->vSize;
	    	    
	}


	pfVec3 *vcoords;
	pfVec2 *tcoords; // in double for now, demote later
	ushort *dummy;
	geoset->getAttrLists(PFGS_COORD3, (void **)&vcoords, &dummy);
	geoset->getAttrLists(PFGS_TEXCOORD2, (void **)&tcoords, &dummy);
	PFASSERTALWAYS(vcoords != NULL && tcoords != NULL && dummy == NULL);
	pfFlux *vcoords_flux, *tcoords_flux;
	
	vcoords_flux = pfFlux::getFlux(vcoords);
	tcoords_flux = pfFlux::getFlux(tcoords);
	vcoords = (pfVec3 *)vcoords_flux->getWritableData();
	tcoords = (pfVec2 *)tcoords_flux->getWritableData();

	pfVec2 tcoordsD[8*MAXSUBDIV+2];
	pfVec3d vcoordsD[8*MAXSUBDIV+2];



	
	CTE_GetDonut(centerX, centerY, bigdiameter, littlediameter, SUBDIV, tcoordsD, 0., 0., 1., 1.);

	CTE_BendDonut(CURVATURE, SUBDIV, tcoordsD, vcoordsD,
		  LON0, LON1, LAT0, LAT1,
		  S0, S1, T0, T1);

	// Subtract what the DCS added (namely, the eye in the parent space
	// of that DCS).  This will put the eye at (0,0,0).
	//pfDoubleDCS *dcs = params->dcs;
	//pfMatrix4d mat;
	//dcs->getMat(mat);
	
	FOR (j, (8*SUBDIV+2))
	{
	    //    PFSUB_VEC3(vcoords[j], vcoordsD[j], mat[3]);
	    PFCOPY_VEC3(vcoords[j], vcoordsD[j]);
	    PFCOPY_VEC2(tcoords[j], tcoordsD[j]);
	}
	
	//*(int32_t *)&tcoords[j] = 0; // use as a lock so only one cull process will convert it to single

	
	vcoords_flux->writeComplete();
	tcoords_flux->writeComplete();
	

	if( cbData->cliptex->isEmulated() )
	{
	    /* paolo: recompute tex-coord bbox */
	    /* NOTE: this is not mp safe! */	
    	    /*geoset->calcTexBBox();*/
	    
	    uint mid_diam = (cbData->vSize>>2) * (bigdiameter+littlediameter);
	    
	    uint cS = (right)? centerS + mid_diam : centerS - mid_diam;
	    uint cT = (top)? centerT + mid_diam : centerT - mid_diam;
	    
	    geoset->setTexBBox( cS, cT, 2, 2 );
	    
	}	

    }
    
    return PFTRAV_CONT;
}


/******************************************************************************/

PFPFB_DLLEXPORT pfNode * pfdLoadFile_cte(char *fileName)
{
    pfClipTexture *cliptex;

    /* Read the .ct file */
    char *cliptexFileName = fileName;
    cliptex = pfdLoadClipTexture(cliptexFileName);
    if (cliptex == NULL)
    {
        pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
                 "pfdLoadFile_ct: Couldn't load clip texture from file \"%s\"",
                 cliptexFileName);
	return NULL;
    }
    cliptex->setName(cliptexFileName);
    /*pfuGridifyClipTexture(cliptex);*/

    int vSize;
    cliptex->getVirtualSize(&vSize, NULL, NULL);

    int nLevels = intlog2(vSize) + 1;
    int clipSize = cliptex->getClipSize();
      
    char *e = getenv("_PFCT_VALID_MAP");
    if (e != NULL)
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
		 "pfdLoadFile_cte: creating a texture valid map");
	pfTextureValidMap *validMap = new pfTextureValidMap(
	    vSize, vSize,
	    PF_MIN2(2*clipSize, vSize), PF_MIN2(2*clipSize, vSize),
	    clipSize, clipSize,
	    8, 8); // XXX machine dependent
	PFASSERTALWAYS(validMap != NULL);
	validMap->setTracing(*e ? atoi(e) : 0);
	cliptex->setValidMap(validMap);
    }
  
    /*
     * Make a geostate referring to the clip texture;
     * this will be used for all geosets including the proxy.
     */
    pfGeoState *geostate = new pfGeoState();
    PFASSERTALWAYS(geostate != NULL);
    

/* findme! */
    geostate->setAttr(PFSTATE_TEXTURE, cliptex);
    geostate->setMode(PFSTATE_ENTEXTURE, TRUE);

    pfGeoSet *proxyGeoSet;
    pfGeode *proxyGeode;
    
    {
	static float square[4][2] = {{-.5,-.5},{.5,-.5},{.5,.5},{-.5,.5}};
	pfVec3 *proxyVertCoords =
		(pfVec3 *)pfMalloc(4*sizeof(pfVec3), pfGetSharedArena());
	pfVec2 *proxyTexCoords =
		(pfVec2 *)pfMalloc(4*sizeof(pfVec2), pfGetSharedArena());
	PFASSERTALWAYS(proxyVertCoords != NULL && proxyTexCoords != NULL);
	int i;
	FOR (i, 4)
	{
	    PFSET_VEC3(proxyVertCoords[i], square[i][0], 0.0f, square[i][1]);
	    //PFSET_VEC2(proxyTexCoords[i], square[i][0], square[i][1]);
	}
	
	PFSET_VEC2(proxyTexCoords[0], 0.0f, 0.0f );
	PFSET_VEC2(proxyTexCoords[1], 1.0f, 0.0f );
	PFSET_VEC2(proxyTexCoords[2], 1.0f, 1.0f );
	PFSET_VEC2(proxyTexCoords[3], 0.0f, 1.0f );
	
	
	
	
	
	
	
	
	proxyGeoSet = new pfGeoSet();
	PFASSERTALWAYS(proxyGeoSet != NULL);
	proxyGeoSet->setGState(geostate);
	proxyGeoSet->setPrimType(PFGS_QUADS);
	proxyGeoSet->setNumPrims(1);
	proxyGeoSet->setAttr(PFGS_COORD3, PFGS_PER_VERTEX,
			     proxyVertCoords, NULL);
	proxyGeoSet->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX,
			     proxyTexCoords, NULL);
	proxyGeode = new pfGeode();
	PFASSERTALWAYS(proxyGeode != NULL);
	proxyGeode->addGSet(proxyGeoSet);
    }

    pfGroup *geodesGroup = new pfGroup();
    PFASSERTALWAYS(geodesGroup != NULL);

    int nDonuts = cliptex->getNumClippedLevels()+1;
    int i;

    FOR (i, nDonuts)
    {
        int *lengths = (int *)pfMalloc(1*sizeof(*lengths), pfGetSharedArena());
	PFASSERTALWAYS(lengths != NULL);
        lengths[0] = (8*SUBDIV+2);
        pfVec4 *white = (pfVec4 *)pfMalloc(1*sizeof(*white), pfGetSharedArena());
        PFASSERTALWAYS(white != NULL);
        white->set(1,1,1,1);


        pfGeoSet *geoset = new pfGeoSet();
        PFASSERTALWAYS(geoset != NULL);
        geoset->setNumPrims(1);
        geoset->setPrimLengths(lengths);
        geoset->setPrimType(PFGS_TRISTRIPS);
	
	geoset->setAttr( PFGS_COORD3, PFGS_PER_VERTEX,
			    new pfFlux((8*SUBDIV+2)*sizeof(pfVec3),
			    PFFLUX_DEFAULT_NUM_BUFFERS+2),
			    NULL);
	geoset->setAttr( PFGS_TEXCOORD2, PFGS_PER_VERTEX,
			    //new pfFlux((8*SUBDIV+2)*sizeof(pfVec2d) + sizeof(uint32_t), // sic
			    new pfFlux((8*SUBDIV+2)*sizeof(pfVec2), PFFLUX_DEFAULT_NUM_BUFFERS+2),
			    NULL);

        geoset->setAttr(PFGS_COLOR4, PFGS_OVERALL, white, NULL);



	{
	    /*fluxCte->initData(&fluxedData);*/
	    
	    /* NEED TO INITIALIZE FLUXED DATA HERE! */
	    
	    
	    
	}



	/* paolo: findme */
        /* need to initialize tcoords, so that tex bbox can be computed */
        geoset->setGState(geostate);


        pfGeode *geode = new pfGeode();
        PFASSERTALWAYS(geode != NULL);
        geode->addGSet(geoset);


        geodesGroup->addChild(geode);
    }

    pfuClipCenterNode *clipCenterNode = pfuNewClipCenterNode();
    PFASSERTALWAYS(clipCenterNode != NULL);
    clipCenterNode->setTravFuncs(PFTRAV_CULL, CTE_forceALL_INCull, NULL);
    clipCenterNode->setClipTexture(cliptex);
    clipCenterNode->setRefNode(proxyGeode);
    clipCenterNode->addChild(geodesGroup);
    
    
    CTE_CallbackParams* cbData = (CTE_CallbackParams*)
	    pfMalloc(1*sizeof(CTE_CallbackParams), pfGetSharedArena());
    PFASSERTALWAYS(cbData);
    
    cbData->cliptex = cliptex;
    cbData->mpcliptex = NULL;    

    cbData->nLevels = nLevels;
    cbData->vSize = vSize;
    cbData->clipSize = clipSize;
    cbData->nDonuts = nDonuts;
    cbData->invalidBorder = cliptex->getInvalidBorder();
    
    cbData->geodesGroup = geodesGroup;
    cbData->proxyGeode = proxyGeode;
    
    pfGroup *callbackGroup = new pfGroup();
    PFASSERTALWAYS(callbackGroup != NULL);
    
    
    callbackGroup->addChild(clipCenterNode);
    callbackGroup->setTravFuncs(PFTRAV_APP, NULL, CTE_callbackGroupPostApp);
    callbackGroup->setTravData(PFTRAV_APP, cbData);

    pfSphere sph;
    sph.center.set(0.0f, 0.0f, 0.0f);
#ifdef WIN32
    sph.radius=fsqrt(0.5);
#else
    sph.radius=M_SQRT1_2;
#endif
    callbackGroup->setBound(&sph, PFBOUND_STATIC);

    return callbackGroup;
    
}

/******************************************************************************/

