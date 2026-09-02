//
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
// pfGeoSet.h
//
// $Revision: 1.177 $
// $Date: 2002/11/20 01:01:10 $
//
//

#ifndef __PF_GEOSET_H__
#define __PF_GEOSET_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfGeoMath.h>
#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfStats.h>
#include <Performer/pr/pfState.h>
#include <Performer/pr/pfVideoChannel.h>
#include <Performer/pr/pfFlux.h>
#include <Performer/pr/pfCycleBuffer.h>
#include <Performer/pr/pfWindow.h>


/* Another field for geoset flags. This one needs to be visible, though */
#define FLAGS_SHADED_GSET       0x10000000

/* 
 * attrBindings looks like:
 * ---------------------------------------------------------------------------
 * 			|index flag| color bind | normal bind | texcoord bind |
 * ---------------------------------------------------------------------------
 *   7           6            5          4, 3          2, 1             0 
 * 
 *	NINDEXMASK = TRUE for non-indexed geoset
 * 	bindings are PFGS_OFF, PFGS_OVERALL, PFGS_PER_PRIM, PFGS_PER_VERTEX
*/

#define TEXCOORDSHIFT		    0
#define TEXCOORDBITS		    1
#define TEXCOORDMASK		    (0x1 << TEXCOORDSHIFT)
#define NORMALSHIFT		    TEXCOORDBITS
#define NORMALBITS		    2
#define NORMALMASK		    (0x3 << NORMALSHIFT)
#define COLORSHIFT		    (NORMALSHIFT + NORMALBITS)
#define COLORBITS		    2
#define COLORMASK		    (0x3 << COLORSHIFT)
#define NINDEXSHIFT		    (COLORSHIFT + COLORBITS)
#define NINDEXBITS		    1
#define NINDEXMASK		    (0x1 << NINDEXSHIFT)
#define BINDBITS		    (TEXCOORDBITS + NORMALBITS + COLORBITS + \
				     NINDEXBITS)

/* 
   To save memory bandwidth, texattrbindings for 4 textures can be packed 
   into a single byte. The tokens below behave just like the ones above, only
   that the two that go into texattrbindings have values which will fit into
   two bits/texture

   If we ever move to more texture units, the structure of geoset will need to 
   change a little. The macros below will still work, but the tex_flags field and
   the texAttrBindings field must be made larger to accomodate all the texture units.
   Currently, each variable uses 2 bits / texture.
*/

#define SMALL_TEXCOORDSHIFT         0
#define SMALL_TEXCOORDBITS          1
#define SMALL_TEXCOORDMASK          (0x1 << SMALL_TEXCOORDSHIFT)

#define SMALL_NINDEXSHIFT           (SMALL_TEXCOORDSHIFT + SMALL_TEXCOORDBITS)
#define SMALL_NINDEXBITS            1
#define SMALL_NINDEXMASK            (0x1 << SMALL_NINDEXSHIFT)
/* How much to shift between textures */
#define SMALL_TEXTURE_SHIFT         (SMALL_TEXCOORDBITS + SMALL_NINDEXBITS)

/*
  Likewise, texture flags which normally go in pfGeoSet::flags are duplicated for multi-texture in
  the variable tex_flags. For four texture units, these can be tightly packed into a byte. 
*/

#define SMALL_TEXFLAG_SHIFT           2
#define SMALL_CBUF_TEXCOORDS_0        0x1
#define SMALL_FLUX_TEXCOORDS_0        0x2
#define SMALL_TEXFLAG_MASK_0          (SMALL_CBUF_TEXCOORDS_0 | SMALL_FLUX_TEXCOORDS_0)
#define SMALL_MB_TEXCOORDS_0          (SMALL_CBUF_TEXCOORDS_0 | SMALL_FLUX_TEXCOORDS_0)

#define SMALL_CBUF_TEXCOORDS_1        (SMALL_CBUF_TEXCOORDS_0 << SMALL_TEXFLAG_SHIFT)
#define SMALL_FLUX_TEXCOORDS_1        (SMALL_FLUX_TEXCOORDS_0 << SMALL_TEXFLAG_SHIFT)
#define SMALL_TEXFLAG_MASK_1          (SMALL_TEXFLAG_MASK_0 << SMALL_TEXFLAG_SHIFT)
#define SMALL_MB_TEXCOORDS_1          (SMALL_MB_TEXCOORDS_0 << SMALL_TEXFLAG_SHIFT)

#define SMALL_CBUF_TEXCOORDS_2        (SMALL_CBUF_TEXCOORDS_1 << SMALL_TEXFLAG_SHIFT)
#define SMALL_FLUX_TEXCOORDS_2        (SMALL_FLUX_TEXCOORDS_1 << SMALL_TEXFLAG_SHIFT)
#define SMALL_TEXFLAG_MASK_2          (SMALL_TEXFLAG_MASK_1 << SMALL_TEXFLAG_SHIFT)
#define SMALL_MB_TEXCOORDS_2          (SMALL_MB_TEXCOORDS_1 << SMALL_TEXFLAG_SHIFT)

#define SMALL_CBUF_TEXCOORDS_3        (SMALL_CBUF_TEXCOORDS_2 << SMALL_TEXFLAG_SHIFT)
#define SMALL_FLUX_TEXCOORDS_3        (SMALL_FLUX_TEXCOORDS_2 << SMALL_TEXFLAG_SHIFT)
#define SMALL_TEXFLAG_MASK_3          (SMALL_TEXFLAG_MASK_2 << SMALL_TEXFLAG_SHIFT)
#define SMALL_MB_TEXCOORDS_3          (SMALL_MB_TEXCOORDS_2 << SMALL_TEXFLAG_SHIFT)

/* masks for marking a specific texture units texture coordinates as cyclebuffered */
extern unsigned char small_texflags_cbuf[];
/* masks for marking a specific texture units texture coordinates as fluxed */
extern unsigned char small_texflags_flux[];
/* masks used to zero out a particular texture inits texture flags */
extern unsigned char small_texflags_zeromask[];
/* masks used to determine if texcoords are multibuffered */
extern unsigned char small_texflags_mbtexcoords[];


typedef struct 
{
    pfPlane	plane;
    ushort	proj1;
    ushort	proj2;
    float	denom;
    int         numTris;
} _pfTriCache;






typedef struct __pfCteGSetData _pfCteGSetData;



typedef struct _pfGSetExtraData
{
    _pfTriCache* cache;
    _pfCteGSetData* cte;
    
} _pfGSetExtraData;

struct pfAppearanceRecord;

/* This is a copy of the original gstateId structure */
typedef union
{
  pfGeoState	*ref;
  int		index;
  pfAppearanceRecord *arec;
    
} GStateId;


// Exported Tokens
extern "C" {     // EXPORT to CAPI

/* ----------------------- pfGeoSet Tokens ----------------------- */

/* primititve token numbers in gsdraw.m must match these numbers */
/* pfGSetPrimType() */
#define PFGS_POINTS	    	0
#define PFGS_LINES	    	1
#define PFGS_LINESTRIPS	    	2	
#define PFGS_TRIS	    	3
#define PFGS_QUADS	    	4
#define PFGS_TRISTRIPS	    	5
#define PFGS_FLAT_LINESTRIPS	6
#define PFGS_FLAT_TRISTRIPS	7
#define PFGS_POLYS		8
#define PFGS_TRIFANS		9
#define PFGS_FLAT_TRIFANS	10
#define PFGS_NUM_PRIMS		11

/* pfGSetAttr(), attribute types */
#define PFGS_COLOR4	    0
#define PFGS_NORMAL3	    1
#define PFGS_TEXCOORD2	    2
#define PFGS_COORD3	    3
#define PFGS_PACKED_ATTRS   5 /* needed for pfGSetDrawMode too */ 
#define PFGS_TEXCOORD3	    6 /* for vert arrrays only */

/* pfGSetAttr(), binding types */
#define PFGS_OFF	    0
#define PFGS_OVERALL	    1
#define PFGS_PER_PRIM	    2
#define PFGS_PER_VERTEX	    3

/* pfGSetDrawMode() */
#define PFGS_FLATSHADE	    	2
#define PFGS_WIREFRAME	    	3
#define PFGS_COMPILE_GL     	4
/* #define PFGS_PACKED_ATTRS  5 */
#define PFGS_DRAW_GLOBJ     	6

/* packed attr formats for pfGSetAttr() with PACKED_ATTRS */
	/* these are the fast path formats */
#define PFGS_PA_OFF		0
#define PFGS_PA_C4UBN3ST2FV3F	1
#define PFGS_PA_C4UBN3ST2F	2
#define PFGS_PA_C4UBT2F		3
#define PFGS_PA_T2S_FORMATS	4
	/* these are additional useful formats */
#define PFGS_PA_GEN_FORMATS	4
#define PFGS_PA_C4UBN3ST2SV3F	4
#define PFGS_PA_C4UBN3ST2S	5
#define PFGS_PA_C4UBT2S		6
#define PFGS_PA_T3_FORMATS	7
#define PFGS_PA_C4UBN3ST3FV3F	7
#define PFGS_PA_C4UBN3ST3F	8
#define PFGS_PA_C4UBT3F		9
#define PFGS_PA_T3S_FORMATS	10
#define PFGS_PA_C4UBN3ST3SV3F	10
#define PFGS_PA_C4UBN3ST3S	11
#define PFGS_PA_C4UBT3S		12
#define PFGS_PA_T2D_FORMATS	13
#define PFGS_PA_C4UBN3ST2DV3F	13
#define PFGS_PA_C4UBN3ST2D	14

/* pfQueryGSet() */
#define PFQGSET_NUM_TRIS	1
#define PFQGSET_NUM_VERTS	2
#define PFQGSET_NUM_QUERIES	2

/* pfGSetIsectMask() */
#define PFIS_ALL_MASK		0xffffffff /* all active mask */
#define PFIS_PICK_MASK		0x80000000 /* reserved for picking */
#define PFIS_SET_PICK		0x80000000 /* picking intersection set flag */ 
    
/* pfGSetPassFilter() */
#define PFGS_TEX_GSET            0x1
#define PFGS_NONTEX_GSET         0x2
#define PFGS_EMISSIVE_GSET       0x4
#define PFGS_NONEMISSIVE_GSET    0x8
#define PFGS_LAYER_GSET          0x10
#define PFGS_NONLAYER_GSET       0x20
#define PFGS_GSET_MASK           0xfff

/* pfGSetBBox() */
#define PFBOUND_STATIC		1
#define PFBOUND_DYNAMIC		2

/* Function for turning a pfFlux data buffer into a pfGeoSet */
extern DLLEXPORT int pfFluxedGSetInit(pfFluxMemory *_fmem);

}




#define PFGS_SPRITE	0x01
#define PFGS_COLORTABLE	0x02
#define PFGS_HIGHLIGHT	0x04
#define PFGS_FILTER	0x08
#define PFGS_DECALPLANE	0x10


#define PFGS_CALLBACK   0x01
#define PFGS_OPTIMIZE   0x02

#define PFGS_IS_CALLBACK	(type_mask & PFGS_CALLBACK)

typedef struct _prIsectTrav prIsectTrav;

//CAPI:private
typedef void (*DrawFuncType)(pfGeoSet *, char *, char *, char *, char *);

extern DrawFuncType *_pfGSetDrawFuncs;
extern DrawFuncType *_pfGSetDirectDrawFuncs;
extern DrawFuncType *_pfGSetGenDrawFuncs;
extern DrawFuncType _pfGSetRasterDrawFuncs[];

// TRUE if we're building geoset display list
extern int	_pfBuildingGSetObject;	

// 30 Words : 4 words
#define PFGEOSET ((pfGeoSet*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFGEOSETBUFFER ((pfGeoSet*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfGeoSet : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename GSet
    pfGeoSet();
    virtual ~pfGeoSet();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_obj) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *prefix, FILE *_file);
    virtual int getGLHandle() const;

    
public:
    // sets and gets
    
    void	setNumPrims(int _n);
    int		getNumPrims() const { return numPrims; }

    void	setPrimType(int _type);
    int		getPrimType() const;

    void	setPrimLengths(int *_lengths);
    int*	getPrimLengths() const { return lengths; }
    int		getPrimLength(int i) const { return (lengths ? PF_ABS(lengths[i]) : 1); }

    void	setAttr(int _attr, int _bind, void* _alist, unsigned short* _ilist);
    void	setMultiAttr(int _attr, int _index, int _bind, void* _alist, unsigned short* _ilist);
    int		getAttrBind(int _attr) const;
    int		getMultiAttrBind(int _attr, int _index) const;

    void	getAttrLists(int _attr, void** _alist, unsigned short** _ilist) const;    
    void	getMultiAttrLists(int _attr, int _index, void** _alist, unsigned short** _ilist) const;    
    int		getAttrRange(int _attr, int *_min, int *_max) const;
    int		getMultiAttrRange(int _attr, int _index, int *_min, int *_max) const;

    void	setDrawMode(int _mode, int _val);
    int		getDrawMode(int _mode) const;

    void	setGState(pfGeoState *_gstate);
    pfGeoState*	getGState() const; 

    void	setGStateIndex(int _id);
    int		getGStateIndex() const;

    void	setHlight(pfHighlight *_hlight);
    pfHighlight*getHlight() const { return  hlight; }

    void	setDecalPlane(pfPlane *_plane);
    pfPlane 	*getDecalPlane() const { return  decalPlane; }

    void	setLineWidth(float _width) { lineWidth = _width; }
    float	getLineWidth() const { return lineWidth; }

    void	setPntSize(float _s) { pntSize = _s; }
    float	getPntSize() const { return pntSize; }

    void	setIsectMask(unsigned int _mask, int _setMode, int _bitOp);
    unsigned int getIsectMask() const { return mask; }

    void	setDrawBin(short bin) {
	drawBin = bin;
    }
    int		getDrawBin(void) const {
	return drawBin;
    }
    void	setDrawOrder(unsigned int order) {
	drawOrder = order;
    }
    unsigned int getDrawOrder(void) const {
	return drawOrder;
    }


  void setAppearance(islAppearance *appearance_);
  islAppearance* getAppearance() const;
 

  int isShaded() { return flags & FLAGS_SHADED_GSET; }

  void computeShaderBoundingBox(const pfMatrix *model, 
				const pfMatrix *invView, 
				const int width, const int height,
				const int xo, const int yo,
				const pfFrustum *viewFrust);
  
  void computeShaderBoundingBox(const pfMatrix *model, 
				const pfMatrix *invView, 
				const int width, const int height,
				const int xo, const int yo,
				const pfFrustum *viewFrust,
				_pfScreenSpaceBoundingBox *dst);

  
    int getNumTextures() {
	return (num_textures);
    }

    //CAPI:verb GSetOptimize
    void setOptimize(int _state) { 
		type_mask = _state ? (type_mask | PFGS_OPTIMIZE) : 
				      (type_mask & ~PFGS_OPTIMIZE); 
    }

    //CAPI:verb GetGSetOptimize
    int getOptimize(void) { return (type_mask & PFGS_OPTIMIZE); } 

    
    //CAPI:verb GSetBBox
    void	setBound(pfBox* _box, int _mode);
    //CAPI:verb GetGSetBBox
    int		getBound(pfBox* _box);
    //CAPI:verb GSetBBoxFlux
    void	setBoundFlux(pfFlux* _flux);
    //CAPI:verb GetGSetBBoxFlux
    pfFlux*	getBoundFlux(void);

    //CAPI:verb HideGSetStripPrim
    void	hideStripPrim(int i) { 
	if (lengths && lengths[i] > 0) lengths[i] = -lengths[i];
    }
    //CAPI:verb UnhideGSetStripPrim
    void	unhideStripPrim(int i) { 
	if (lengths && lengths[i] < 0) lengths[i] = -lengths[i];
    }
    //CAPI:verb IsGSetStripPrimHidden
    int		isStripPrimHidden(int i) { 
	return ((lengths && (lengths[i] >= 0)) ? FALSE : TRUE);
    }





    /* CLIPTEXTURE EMULATION API */
    
    /* the following function will allocate (or free) cte data */
    //CAPI:verb GSetUpdateCteRefs
    void	updateCteRefs(void);

    /* the following func calculates texcoord bbox only if cte data is there */
    //CAPI:verb GSetCalcTexBBox
    void	calcTexBBox(void);

    //CAPI:verb GSetTexBBox_i
    void	setTexBBox( uint centerS, uint centerT, uint halfwidth, uint halfheight );
    
    //CAPI:verb GSetTexBBox_f
    void	setTexBBox( float minS, float maxS, float minT, float maxT  );
    
    //CAPI:verb GetGSetTexBBox_i
    int		getTexBBox( uint* centerS, uint* centerT, uint* halfwidth, uint* halfheight );
    
    //CAPI:verb GetGSetTexBBox_f
    int		getTexBBox( float* minS, float* maxS, float* minT, float* maxT  );
    
    //CAPI:verb GSetCteAttr
    void 	setCteAttr( int which, void* val );

    //CAPI:verb GetGSetCteAttr
    void* 	getCteAttr( int which );


    // Methods for setting pfGeoSet members with no reference count checks, 
    // and with no impact on any other pfGeoSet internal state (e.g. no 
    // recompute of draw index).

    void	quickCopy(pfGeoSet *src);

    // CAPI:verb pfGSetQuickAttr
    void	quickSetAttr(int _attr, void* _alist, unsigned short* _ilist);

    // CAPI:verb pfGSetQuickMultiAttr
    void	quickSetMultiAttr(int _attr, int _index, void* _alist, unsigned short* _ilist);

    // CAPI:verb pfGSetQuickPrimLengths
    void	quickSetPrimLengths(int *_lengths) {lengths = _lengths;}

    void	quickReset(int extRefOnly);
    
	
public:
    // other functions
    // CAPI:verb
    void	draw();
    void	drawBB();

    // CAPI:public
    // CAPI:verb

    void	compile();
    int		query(unsigned int _which, void *_dst) const;
    int		mQuery(unsigned int *_which, void *_dst) const;
    // CAPI:verb GSetIsectSegs
    int		isect(pfSegSet *segSet, pfHit **hits[]);
    // CAPI:verb DrawHlightedGSet
    void	drawHlightOnly();

public:
    static void	setPassFilter(uint _mask);
    static uint	getPassFilter() {
	return pfGeoSet::filterMask;
    }



PFINTERNAL: // XXX - should make protected???

    ushort 	drawIndex;	// Index into current draw function table 
    ushort	directDrawIndex;// Bypasses genDrawGSet, errDrawGSet etc.
    ushort	buildDrawIndex; // For rebuilding display lists
    ushort	wireDrawIndex; // For drawing wireframe for pfEnable(PFEN_WIREFRAME)
    
    short 	drawBin;
    unsigned char	subDrawIndex;	// For indexing sub-tables for vert arrays
    unsigned char	packedFormat;   // format of packed vertex attributes array
    
    unsigned int drawOrder;	// order in a bin if PF_SORT_DRAW_ORDER
    
    int 	flags;		// flags: flat shading & bbox valid, VA flags
#ifdef LARGE_TEX_FLAGS
    int		tex_flags[PF_MAX_TEXTURES];
#else
    unsigned char tex_flags;
#endif
    unsigned char  num_textures;//number of textures. Shouldn't need more than
                                //255
#ifdef LARGE_TEXATTRBINDINGS
    ushort	texAttrBindings[PF_MAX_TEXTURES];	
#else
    unsigned char texAttrBindings; //four textures will fit with 2 bits/texture
#endif
				// Bitfield indicating binding of 
				// multiTexture attributes. attrBindings holds 
				// PFGS_PER_VERTEX if any of the texture units
				// binds PFGS_PER_VERTEX. PF_OFF otherwise.

    unsigned char type_mask;    // Bits in use: 
				// 0: determine whether this
				//    geoset is a pfGeoSetCB. If it is, we 
				//    should never override the settings of 
				//    its draw indices.
				// 1: determine whether this geoset should
			 	//    be optimized by CULL_SIDEKICK.

    int		numPrims;	// Number of primitives, e.g. - num quads 
    
    ushort	primType;	// Type of primitive represented 
    ushort	attrBindings;	// Bitfield indicating binding of attributes

    
				// Bitfield containing flux/cycle-buf mode
				// of each texCoord element.

    pfVec4    	*colors;	// Base address of color list 
    pfVec3    	*norms;		// Base address of normal list 
    pfVec2    	*texCoords[PF_MAX_TEXTURES];	
				// Base address of texture coordinate list 
    pfVec3    	*coords;	// Base address of coordinate list 

    ushort    	*cindex;	// List of color indexes 
    ushort    	*nindex;	// List of normal indexes 
    
    ushort    	*tindex[PF_MAX_TEXTURES];	
				// List of texture coordinate indexes 
    ushort    	*vindex;	// List of coordinate indexes 

#ifdef ENABLE_STRIDES
    short	cstride;	// Byte stride of colors 
    short	nstride;	// Byte stride of normals 
    short	tstride[PF_MAX_TEXTURES];	
				// Byte stride of texture coordinates 
    short	vstride;	// Byte stride of coordinates 
#endif

    int		*lengths;	// List of strip lengths 

    float	lineWidth;
    float	pntSize;

    GStateId	gstate;		// Union of pfGeoState* and index 
    
    void	*packedAttrs; // packed vertex attributes array

    // put non performance stuff down here 
    int		dirty;		// dirty value for multipipe object management 
    GLOBJECT	glIndex;	// callobj() identifier 
    
    pfBox	bbox;		// bounding box 

    int		mask;		// isection mask



#if 0    
    _pfTriCache	*cache;		// cach of normals & projections 
#endif
    _pfGSetExtraData* extraData;



    int		numVerts;	// number of verts for stats and vert arrays
    int		numGLPrims;	// total number of GL prims for stats 
    pfHighlight *hlight;	// highlight structure 
    pfPlane 	*decalPlane;	// reference plane for decal
    
    //ushort      shaderDrawIndex;// Draw function shaded geoset uses

    /* Texture flags are stored in this field for multitexture. Flags for all 4 possible 
       texture units can be stored in one byte */
    
    static uint filterMask;	// For filtering emissive, transparent etc.

private:
    static pfType *classType;
private:
};


#define PFHIT ((pfHit*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFHITBUFFER ((pfHit*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfHit : public pfObject
{
public:
    // constructors and destructors
    pfHit();
    virtual ~pfHit();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

public:
    // sets and gets
    // CAPI:noverb
    // CAPI:private
    virtual void        setUserData(void *data) { setUserData(0, data); }
    virtual void        setUserData(int _slot, void *data);
	
public:
    // other functions
    // CAPI:verb
    int		query(unsigned int _which, void *_dst) const; 
    int		mQuery(unsigned int *_which, void *_dst) const;


protected:
    pfSegSet	*segSet;	// segSet for intersection 
    int		segnum;		// index of the segment 
    int		flags;		// type of isection and validity bits 
    pfSeg	seg;		// segment being intersected as clipped 
    pfVec3	point;		// point of intersection, if calculated 
    pfVec3	normal;		// normal at intersection point 
    pfVec3	verts[3];	// vertices of the triangle 
    int		tri;		// index of triangle in the primitve 
    int		prim;		// index of primitive in the geoset 
    pfGeoSet*	gset;		// geoset of intersection 
    pfString*	string;		// string of intersection
    int		charIndex;	// which character in string was hit
    void*	node;		// node pointer 
    const char*	name;		// name string 
    pfMatrix	xform;		// transformation to world coords
    pfList*	path;		// path to intersected geode 
    int		faceIndex;	// Face Id in pfASD.

private:
    static pfType *classType;
};


//////////////////////////////////////////////////////////////////////////////
// pfHits - interface to the manipulation of pfHit arrays.
//////////////////////////////////////////////////////////////////////////////


extern "C" {     // EXPORT to CAPI
/* ------------------ pfHit Tokens --------------------- */

#define PFQHIT_FLAGS		1
#define PFQHIT_SEGNUM		2
#define PFQHIT_SEG		3
#define PFQHIT_POINT		4
#define PFQHIT_NORM		5
#define PFQHIT_VERTS		6
#define PFQHIT_TRI		7
#define PFQHIT_PRIM		8
#define PFQHIT_GSET		9
#define PFQHIT_STRING		10
#define PFQHIT_CHARINDEX	11
#define PFQHIT_FACEINDEX	12


/* pfHit flag values */
#define PFHIT_NONE		0x00 /* no intersection */
#define PFHIT_ISECT		0x01 /* intersection */
#define PFHIT_POINT		0x02 /* point info valid */
#define PFHIT_NORM		0x04 /* normal info valid */
#define PFHIT_TRI		0x08 /* triangle index valid */
#define PFHIT_PRIM		0x10 /* primitive index valid */
#define PFHIT_VERTS		0x20 /* triangle vertex info valid */
}

#endif // !__PF_GEOSET_H__
