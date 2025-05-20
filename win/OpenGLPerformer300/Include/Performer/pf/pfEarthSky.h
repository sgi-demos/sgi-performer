//
// Copyright 1995, Silicon Graphics, Inc.
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
// pfEarthSky.h	Earth / Sky include file 
//
// $Revision: 1.74 $
// $Date: 2002/05/30 00:33:56 $
//
#ifndef __PF_ESKY_H__
#define __PF_ESKY_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pf/pfUpdatable.h>
#include <Performer/pr/pfLinMath.h>


// XXX - Geometry is not multi-buffered. Pointers to pfFog etc. are 
// multibuffered but the actual pfFog data structure is not.

typedef struct
{
    pfVec3	 *coords;
    pfVec2	 *texCoords;
    pfVec4	 *colors;
    ushort	 *vindex[3];
    ushort	 *cindex[3];
    ushort	 *tindex;
    pfGeoSet    *sky;
    pfGeoSet    *horiz;
    pfGeoSet    *gnd;
} ESky;
    

typedef struct
{
    int 	 visMode;	// visiblity mode
    short 	 renderMode;	// render switch for cloud tops 
    short	 fogRenderMode;	// cahced key to do fog for sky
    float	 ctop, cbot;	// cloud ceiling and top z
    float	 tztop, tzbot;  // transition zone offsets 
    float	 gfz;		// ground fog height
    float	 gftz;		// ground fog transition zone height
    float	 gfbot;		// ground fog bottom
    float	 ftop;		// height of uppermost fog layer
    pfFog	*fog;		// references either vfog, gfog, or cfog.
} Visibility;

extern "C" {     // EXPORT to CAPI
/* ----------------------- pfEarthSky Tokens ----------------------- */

/* pfESkyMode() */
#define PFES_BUFFER_CLEAR	300
#define PFES_TAG		301
#define PFES_FAST		302
#define PFES_SKY		303
#define PFES_SKY_GRND		304
#define PFES_CLOUDS		305
#define PFES_OVERCAST		306
#define PFES_SKY_CLEAR		307
#define PFES_MULTIPASS_FOG	308
		   
/* pfESkyAttr() */
#define PFES_CLOUD_TOP		310
#define PFES_CLOUD_BOT		311
#define PFES_TZONE_TOP		312
#define PFES_TZONE_BOT		313
#define PFES_GRND_FOG_TOP	316
#define PFES_HORIZ_ANGLE	317
#define PFES_GRND_HT		318
#define PFES_GRND_FOG_BOT	319
#define PFES_GRND_FOG_TZONE	320
#define PFES_FOG_TOP		321
#define PFES_MPFOG_TOP		322
#define PFES_MPFOG_BOT		323
#define PFES_MPFOG_MINRADIUS	324
#define PFES_MPFOG_MAXRADIUS	325		   
#define PFES_TENT_DISTANCE_FACTOR	326		   

/* pfESkyColor()  */
#define PFES_SKY_TOP		350
#define PFES_SKY_BOT		351
#define PFES_HORIZ		352
#define PFES_GRND_FAR		353
#define PFES_GRND_NEAR		354
#define PFES_CLEAR		357

/* pfESkyFog() */
#define PFES_GRND		380
#define PFES_GENERAL		381
} // extern "C" END of C include export


// XXX - User fog should be multibuffered.
#define PFEARTHSKY ((pfEarthSky*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFEARTHSKYBUFFER ((pfEarthSky*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfEarthSky : public pfUpdatable
{
public:

    void setMode(int  mode, int  val)  {
        PFEARTHSKY->nb_setMode(mode, val);
    }

    int getMode(int  mode)  {
        return PFEARTHSKY->nb_getMode(mode);
    }

    void setAttr(int  mode, float val)  {
        PFEARTHSKY->nb_setAttr(mode, val);
    }

    float getAttr(int  mode)  {
        return PFEARTHSKY->nb_getAttr(mode);
    }

    void setColor(int  which, float r, float g, float b, float a)  {
        PFEARTHSKY->nb_setColor(which, r, g, b, a);
    }

    void getColor(int  which, float *r, float *g, float *b, float *a)  {
        PFEARTHSKY->nb_getColor(which, r, g, b, a);
    }

    void setFog(int  which, pfFog *fog)  {
        PFEARTHSKY->nb_setFog(which, fog);
    }

    pfFog* getFog(int  which)  {
        return PFEARTHSKY->nb_getFog(which);
    }

    void setFogDensities(int numpt, float *elevations, float *densities)  {
        PFEARTHSKY->nb_setFogDensities(numpt, elevations, densities);
    }

    void getFogDensities(int *numpt, float **elevations, float **densities)  {
        PFEARTHSKY->nb_getFogDensities(numpt, elevations, densities);
    }

    void setFogTextureElevations(int n, float *elev)  {
        PFEARTHSKY->nb_setFogTextureElevations(n, elev);
    }

    void getFogTextureElevations(int *n, float **elev)  {
        PFEARTHSKY->nb_getFogTextureElevations(n, elev);
    }

    void setFogTexture(pfTexture *tex)  {
        PFEARTHSKY->nb_setFogTexture(tex);
    }

    pfTexture* getFogTexture()  {
        return PFEARTHSKY->nb_getFogTexture();
    }

    void setFogTextureColorTable(int n, unsigned char *table)  {
        PFEARTHSKY->nb_setFogTextureColorTable(n, table);
    }

    void getFogTextureColorTable(int *n, unsigned char **table)  {
        PFEARTHSKY->nb_getFogTextureColorTable(n, table);
    }

    void loadFogTextureColorTable()  {
        PFEARTHSKY->nb_loadFogTextureColorTable();
    }

    void makeFogTexture()  {
        PFEARTHSKY->nb_makeFogTexture();
    }
public:		// Constructors/Destructors
    //CAPI:basename ESky
    //CAPI:updatable
    //CAPI:newargs
    pfEarthSky();
    virtual ~pfEarthSky()     {} // can't delete me

protected:
    pfEarthSky(pfBuffer *buf);
    pfEarthSky(const pfEarthSky *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:			// pfMemory virtual functions

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int  upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);
    virtual void	    pf_updateBufferList(pfBuffer *buf, int addRemove) {
        buf->pf_updateList(this, addRemove);
    }

PFINTERNAL:		// specific sets and gets
    void 		    nb_setMode(int  mode, int  val);
    int 		    nb_getMode(int  mode);
    void 		    nb_setAttr(int  mode, float val);
    float 		    nb_getAttr(int  mode);
    void 		    nb_setColor(int  which, float r, float g, float b, float a);
    void 		    nb_getColor(int  which, float *r, float *g, float *b, float *a);
    void		    nb_setFog(int  which, pfFog *fog);
    pfFog*		    nb_getFog(int  which);

    void		    nb_setFogDensities(int numpt, float *elevations, float *densities);
    void		    nb_getFogDensities(int *numpt, float **elevations, float **densities);
    void		    nb_setFogTextureElevations(int n, float *elev);
    void		    nb_getFogTextureElevations(int *n, float **elev);
    void		    nb_setFogTexture(pfTexture *tex);
    pfTexture* 		    nb_getFogTexture(void);
    void		    nb_setFogTextureColorTable(int n, unsigned char *table);
    void		    nb_getFogTextureColorTable(int *n, unsigned char **table);
    void		    nb_loadFogTextureColorTable(void);
    void		    nb_makeFogTexture(void);

private:
    int 	clearMode;	    
    float	gndz;
    float	horzwid;
    float	skyrs, skyrt;	// scales for sky texture 
    pfVec4	clearcolor;
    pfFog	*gfog;		// user fog for ground fog
    pfFog	*vfog;		// user fog for general visibility
    pfFog	*cfog;		
    pfFog	*def_cfog;	// fog for clouds, scud, tzones
    pfTexture   *ctex;		// cloud texture
    pfTexEnv	*ctev;		// cloud texture environment
    ESky	esky;
    Visibility	vis;
    int 	cloudType;
    int		frame;		// optimization of mp update
    pfChannel	*prevDrawChan;	// optimization of mp update
    PF_ULOCK_T  lock;		// lock for mp safe update

    int multipassFog;		// whether multipass fog is enabled

    pfTexture *fogtex;		// fog texture
    pfTexEnv *fogtenv;		// tenv for fog texture
    pfTexGen *fogtgen;		// texgen for fog texture

    pfTexture *userfogtex;	// fog texture supplied by user

    int numFogTexElevations;	// list of elevations for which we create 2D slices
    float *fogTexElevations;    //     of the 3D fog texture
    float *quantFogTexElevations;// quantized copy of fogTexElevations

    float fogTexTop;		// range of world space to which the fog texture
    float fogTexBottom;		//     is mapped
    float fogTexMinRadius;
    float fogTexMaxRadius;

    int numFogPoints;		// number of density/elevation control points
    float *fogDensities;	// list of densities 
    float *fogElevations;	// list of elevations

    unsigned char *teximage;	// buffer for fog texture image
    int fogTexImageBufSize;	// allocated size of image buffer

    int fogTexColorTableSize;	// size of tlut for fog texture
    unsigned char *fogTexColorTable;

    pfMaterial *fogmtl;		// material used for fog texture pass XXX
    pfLightModel *foglmodel;	// light model used for fog texture pass XXX

    float *fogmtab;		// table used to speedup recalc of fog texture
    int fogMTabBufSize;		// allocated size of table

    float  tentDistanceFactor;  // pull tent away from `far' plane by this 
			 	// factor

private:
    static pfType *classType;
public:
    void *_pfEarthSkyExtraData;
};

PF_UPDATABLE_REF(pfEarthSky, _pfEarthSkyRef);

#endif // !__PF_ESKY_H__


