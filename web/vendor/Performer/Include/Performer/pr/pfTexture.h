//
//
// Copyright 1995, Silicon Graphis, Inc.
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
// pfTexture.h
//
// $Revision: 1.160 $
// $dDateRevision: 1.34 $
// 
//

#ifndef __PF_TEXTURE_H__
#define __PF_TEXTURE_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfList.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/image.h>

//////////////////////////////////// pfTexture ////////////////////////////////////


extern "C" {     // EXPORT to CAPI
/* ----------------------- pfTexture Tokens ----------------------- */

#define PFTEX_DEFAULT			(0x7fff)
#define PFTEX_LEVEL_BASE		(PFTEX_DEFAULT-1)
#define PFTEX_LEVEL_ALL			(PFTEX_DEFAULT-2)

/* Bitmasks to create min/mag filters for pfTexFilter 
 *  LE/GE | add/mod | interp | bicubic/mipmap/detail/sharp | alpha/color | /min/mag
 */
#define	PFTEX_FAST			0x80000000
#define PFTEX_MAGFILTER			0x00000001
#define PFTEX_MINFILTER			0x00000002
#define PFTEX_COLOR			0x00000004
#define PFTEX_ALPHA			0x00000008
#define PFTEX_MIPMAP			0x00000010
#define PFTEX_SHARPEN			0x00000020
#define PFTEX_DETAIL			0x00000040
#define PFTEX_MODULATE			0x00000100
#define PFTEX_ADD			0x00000200
#define PFTEX_POINT			0x00001000
#define PFTEX_LINEAR			0x00002000
#define PFTEX_BILINEAR			0x00004000
#define PFTEX_TRILINEAR			0x00008000
#define PFTEX_QUADLINEAR		0x00010000
#define PFTEX_CLIPMAP			0x00020000
#define PFTEX_MIPMAP_POINT		(PFTEX_MIPMAP | PFTEX_POINT)
#define PFTEX_MIPMAP_LINEAR		(PFTEX_MIPMAP | PFTEX_LINEAR)
#define PFTEX_MIPMAP_BILINEAR		(PFTEX_MIPMAP | PFTEX_BILINEAR)
#define PFTEX_CLIPMAP_TRILINEAR		(PFTEX_CLIPMAP | PFTEX_TRILINEAR)
#define PFTEX_MIPMAP_TRILINEAR		(PFTEX_MIPMAP | PFTEX_TRILINEAR)
#define PFTEX_MIPMAP_QUADLINEAR		(PFTEX_MIPMAP | PFTEX_QUADLINEAR)
#define PFTEX_MAGFILTER_COLOR		(PFTEX_MAGFILTER | PFTEX_COLOR)
#define PFTEX_MAGFILTER_ALPHA		(PFTEX_MAGFILTER | PFTEX_ALPHA)
#define PFTEX_SHARPEN_COLOR		(PFTEX_SHARPEN | PFTEX_COLOR)
#define PFTEX_SHARPEN_ALPHA		(PFTEX_SHARPEN | PFTEX_ALPHA)
#define PFTEX_DETAIL_COLOR		(PFTEX_DETAIL | PFTEX_COLOR)
#define PFTEX_DETAIL_ALPHA		(PFTEX_DETAIL | PFTEX_ALPHA)
#define PFTEX_DETAIL_LINEAR		(PFTEX_DETAIL | PFTEX_LINEAR)
#define PFTEX_MODULATE_DETAIL		(PFTEX_DETAIL | PFTEX_MODULATE)
#define PFTEX_ADD_DETAIL		(PFTEX_DETAIL | PFTEX_ADD)


/* IRIS GL only filters */
#define PFTEX_BICUBIC			0x00000080
#define PFTEX_LEQUAL			0x00000400
#define PFTEX_GEQUAL			0x00000800
#define PFTEX_BICUBIC_LEQUAL		(PFTEX_BICUBIC | PFTEX_LEQUAL)
#define PFTEX_BICUBIC_GEQUAL		(PFTEX_BICUBIC | PFTEX_GEQUAL)
#define PFTEX_BICUBIC_LEQUAL		(PFTEX_BICUBIC | PFTEX_LEQUAL)
#define PFTEX_BICUBIC_GEQUAL		(PFTEX_BICUBIC | PFTEX_GEQUAL)
#define PFTEX_BILINEAR_LEQUAL		(PFTEX_BILINEAR | PFTEX_LEQUAL)
#define PFTEX_BILINEAR_GEQUAL		(PFTEX_BILINEAR | PFTEX_GEQUAL)

#define PFTEX_SHARPEN_SPLINE		2
#define PFTEX_DETAIL_SPLINE		3

#define PFTEX_MAX_SPLINE_POINTS		4

/* pfTexBorderType() tokens */
#define PFTEX_BORDER_NONE		0
#define PFTEX_BORDER_COLOR		1
#define PFTEX_BORDER_TEXELS		2

/* pfTexFormat() tokens */
#define PFTEX_INTERNAL_FORMAT		1
#define PFTEX_EXTERNAL_FORMAT		2
#define PFTEX_DETAIL_TEXTURE		3
#define PFTEX_GEN_MIPMAP_FORMAT		4
#define PFTEX_SUBLOAD_FORMAT		6
#define PFTEX_FAST_DEFINE		PFTEX_SUBLOAD_FORMAT
#define PFTEX_NO_TEXEL_OPS_FORMAT	7 /* no pixmode or scale/bias operations on texels */
#define PFTEX_4D_FORMAT			8
#define PFTEX_IMAGE_FORMAT		9

/* pfTexLoadMode() modes */
#define PFTEX_LOAD_LIST			1
#define PFTEX_LOAD_BASE			2
#define PFTEX_LOAD_SOURCE		3
#define PFTEX_LOAD_VIDEO_INTERLACED	4

/* pfTexLoadMode() values for PFTEX_LOAD_LIST */
#define PFTEX_LIST_APPLY		0
#define PFTEX_LIST_AUTO_IDLE		1
#define PFTEX_LIST_AUTO_SUBLOAD		2

/* pfTexLoadMode() values for PFTEX_LOAD_BASE */
#define PFTEX_BASE_APPLY		0
#define PFTEX_BASE_AUTO_SUBLOAD		1

/* pfTexLoadMode() values for PFTEX_LOAD_SOURCE */
#define PFTEX_SOURCE_IMAGE		0
#define PFTEX_SOURCE_FRAMEBUFFER	1
#define PFTEX_SOURCE_VIDEO		2
#define PFTEX_SOURCE_DMBUF		3
#define PFTEX_SOURCE_DMVIDEO		4

/* pfTexLoadMode() values for PFTEX_LOAD_VIDEO_INTERLACED */
#define PFTEX_VIDEO_INTERLACED_OFF	0
#define PFTEX_VIDEO_INTERLACED_ODD	1
#define PFTEX_VIDEO_INTERLACED_EVEN	2

/* pfTexLoadVal() */
#define PFTEX_LOAD_DMBUF		0
#define PFTEX_LOAD_VIDEO_VLSERVER	1
#define PFTEX_LOAD_VIDEO_VLPATH		2
#define PFTEX_LOAD_VIDEO_VLDRAIN	3


/* pfTex src/dest select tokens -- used for
 * pfTexLoadOrigin()
*/
#define PFTEX_ORIGIN_SOURCE		0
#define PFTEX_ORIGIN_DEST		1

/* more special external formats - currently IRIS GL only */
#define PFTEX_PACK_USHORT_5_6_5		0x8801
#define PFTEX_PACK_USHORT_4_4_4_4	0x8802

/* ------------------ pfTexture Related Functions--------------------- */

extern  pfTexture*   pfGetCurTex(void);
#if PF_C_API
extern void                 pfTexBorderColor(pfTexture* _tex, pfVec4 _clr);

#endif /* !PF_CPLUSPLUS_API */
}

#define PFTEXTURE ((pfTexture*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFTEXTUREBUFFER ((pfTexture*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfTexture : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename Tex
    pfTexture();
    virtual ~pfTexture();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions
    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);
    virtual int getGLHandle() const;
    virtual void deleteGLHandle();
    
public:
    // sets and gets
    // CAPI:basename 
    // CAPI:basename Tex
    void	setName(const char *_name);
    const char*	getName() const { return name; }
    void	setImage(unsigned int* _image, int _comp, int _sx, int _sy, int _sz);
    void	getImage(unsigned int** _image, int* _comp, int* _sx, int* _sy, int* _sz) const;
    // CAPI:private
    void	setBorderColor(pfVec4 _clr);
    // CAPI:public
    void	getBorderColor(pfVec4 *_clr);
    void	setBorderType(int _type);
    int		getBorderType(void);
    void	setFormat(int _format, int type);
    int		getFormat(int _format) const;
    void	setFilter(int _filt, int _type);
    int		getFilter(int _filt) const;
    void	setRepeat(int _wrap, int _type);
    int		getRepeat(int _wrap) const;
    // CAPI:virtual
    virtual void setLODRange(float _min, float _max);
    virtual void getLODRange(float *_min, float *_max);
    // CAPI:novirtual
    void	setLODBias(float _biasS, float _biasT, float _biasR); 
    void	getLODBias(float *_biasS, float *_biasT, float *_biasR); 
    void	setSpline(int _type, pfVec2 *_pts, float _clamp);
    void	getSpline(int _type, pfVec2 *_pts, float *_clamp) const;
    void	setDetail(int _l, pfTexture *_detail);
    void	getDetail(int *_l, pfTexture **_detail) const;
    pfTexture*	getDetailTex() const { return detail; }
    // CAPI:basename
    void	setDetailTexTile(int _j, int _k, int _m, int _n, int scram);
    void	getDetailTexTile(int *_j, int *_k, int *_m, int *_n, int *scram) const;
    
    // CAPI:basename Tex
    void	setList(pfList *_list);
    pfList*	getList() const { return texList; }
    void	setFrame(float _frame) { frame = _frame; }
    float	getFrame() const { return frame; }
    
    // CAPI:basename Tex
    void	setLoadImage(unsigned int* _image);
    unsigned int *getLoadImage() const;
    void	setLoadMode(int _mode, int _val);
    int		getLoadMode(int _mode) const;
    void	setLoadVal(int _mode, void* _val);
    void*	getLoadVal(int _mode) const;
    void	setLevel(int _level, pfTexture* _ltex);
    pfTexture*	getLevel(int _level);
    void	setLoadOrigin(int _which, int _xo, int _yo);
    void	getLoadOrigin(int _which, int *_xo, int *_yo);
    void	setLoadSize(int _xs, int _ys);
    void	getLoadSize(int *_xs, int *_ys) const {
	if (_xs != NULL) *_xs = ldXSize;
	if (_ys != NULL) *_ys = ldYSize;
    }
    void		setValidMap(pfTextureValidMap *_validMap);
    pfTextureValidMap 	*getValidMap() const { return validMap; }

    // CAPI:verb GetCurTexLODRange
    void	getCurLODRange(float *_min, float *_max);
    // CAPI:verb GetCurTexLODBias
    void	getCurLODBias(float *_biasS, float *_biasT, float *_biasR); 
 
    // CAPI:basename Tex
    // CAPI:verb TexAnisotropy
    void        setAnisotropy(const int _degree);
    // CAPI:verb GetTexAnisotropy
    int         getAnisotropy() const;

    // CAPI:basename Tex
public:
    // other functions
    // CAPI:verb
    void	apply();
    void	multiApply(int myID);
    // CAPI:verb ApplyTexMinLOD
    void	applyMinLOD(float _min);
    // CAPI:verb ApplyTexMaxLOD
    void        applyMaxLOD(float _max);
    // CAPI:verb ApplyTexLODBias
    void	applyLODBias(float _biasS, float _biasT, float _biasR);
    void	format();
    void	load();
    // CAPI:verb LoadTexLevel
    void	loadLevel(int _level);
    // CAPI:verb SubloadTex
    void	subload(int _source, uint *image, int _xsrc, int _ysrc, int _srcwid, int _xdst, int _ydst, int _xsize, int _ysize);
    // CAPI:verb SubloadTexLevel
    void	subloadLevel(int _source, uint *image, int _xsrc, int _ysrc, int _srcwid, int _xdst, int _ydst, int _xsize, int _ysize, int _level);
    // CAPI:verb LoadTexFile
    int		loadFile(const char* _fname);
    // CAPI:verb SaveTexFile
    int		saveFile(const char* _fname);
    // CAPI: verb SaveTexFileType
    int         saveFileType(const char* _fname, const char* _type);
    // CAPI:verb FreeTexImage
    void	freeImage();
    void        idle();
    // CAPI:verb IsTexLoaded
    int		isLoaded() const;
    // CAPI:verb IsTexFormatted
    int		isFormatted() const;
    

PFINTERNAL: // XXX - should make protected???

    int		    index;	// unique descriptor 
    int		    dirty;	// for changes after binding 
    short	    mode, flags, source, comp;
    int		    xSize, ySize, zSize;
    int		    srcXOrg, srcYOrg, srcZOrg, dstXOrg, dstYOrg, dstZOrg;
    int		    ldXSize, ldYSize, ldZSize;
    int		    dirtyParams;
    uint	    minfilterMask, magfilterMask; // our own bitmasks
    uint	    userMagFilter, userMinFilter; // shadow user set mask
    uint	    userAlphaMag, userColorMag;  // shadow user set mask
    ushort	    intformat, extformat, imgformat, compformat;
    ushort	    minfilter, magfilter; // gl tokens
    //--------------------------------
    ushort	    wrap, twrap, swrap, rwrap;
    //--------------------------------
    short	    numSplinePoints;
    ushort	    magMode;
    ushort	    bindTarget;
    ushort	    enableTarget;
    pfVec4	    borderClr;
    short	    borderType, borderWidth;
    short	    detailLevel;
    float	    detailclamp;
    float	    frame;	// frame index into texList sequence 
    float	    sharpclamp;
    pfVec2	    sharpspline[PFTEX_MAX_SPLINE_POINTS+4]; // room for clamp points
    pfVec2	    detailspline[PFTEX_MAX_SPLINE_POINTS+4]; // room for clamp points
    pfVec2	    userSharpSpline[PFTEX_MAX_SPLINE_POINTS];
    pfVec2	    userDetailSpline[PFTEX_MAX_SPLINE_POINTS];
    pfTexture	    *detail;
    pfList	    *texList;	// for sequence of textures 
    unsigned int    *image;	// base image
    unsigned int    *loadImage;	// alternative image source for texture subload loads
    unsigned int    *curImage;	// loadImage or else image
    pfList	    *maps;	// pfTexture maps for levels of the texture
    pfList	    *mapDirty;	// dirty flags for maps

    pfTexLOD	    *tlod;

    pfTextureValidMap   *validMap;	// bit array(s) of what's currently loaded

    char	    *name;
    short	    clipTexHw; // Does the current hardware support cliptexturing?
    short	    interlacedVideo;

    int             aniso_degree;

private:
    static pfType   *classType;
    float	    userDetailClamp;
    static int      defaultAnisoDegree;      
};

/////////////////////////// pfTexEnv ///////////////////////////////////////

extern "C" {     // EXPORT to CAPI
/* ----------------------- pfTexEnv Tokens ----------------------- */

/* pfTEnvComponent()  */
#define PFTE_COMP_OFF		0

/* fast default for pfTEnvMode */
#define PFTEV_FAST		0x80000000

/* ------------------ pfTexEnv Related Functions--------------------- */

extern  pfTexEnv*   pfGetCurTEnv(void);
}

#define PFTEXENV ((pfTexEnv*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFTEXENVBUFFER ((pfTexEnv*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfTexEnv : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename TEnv
    pfTexEnv();
    virtual ~pfTexEnv();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);
    virtual int getGLHandle() const { return index; }

public:
    // sets and gets
    // CAPI:basename 
    // CAPI:basename TEnv
    void      	setMode(int _mode);
    int		getMode() const { return userMode; }
    void	setComponent(int _comp);
    int		getComponent() const { return compsel; }
    void	setBlendColor(float _r, float _g, float _b, float _a);
    void	getBlendColor(float* _r, float* _g, float* _b, float* _a);

public:
    // other functions
    // CAPI:verb
    void	apply();
    void	multiApply(int myID);


private:
    int	    index;
    int     dlindex;    // dirty bit to mark whether dl has reserved a name
    int	    dirty;      // for changes after binding 
    int	    mode, userMode;
    pfVec4  blend;
    int	    compsel;	// Component select 
#ifdef USE_COMPILE
    int	    flags;	// compile
#endif // USE_COMPILE  
    
private:
    static pfType   *classType;
};

/////////////////////////////// pfTexGen ///////////////////////////////////////

extern "C" {     // EXPORT to CAPI
/* ------------------ pfTexGen Related Functions--------------------- */

extern  pfTexGen*   pfGetCurTGen(void);
}

#define PFTEXGEN ((pfTexGen*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFTEXGENBUFFER ((pfTexGen*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfTexGen : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename TGen
    pfTexGen();
    virtual ~pfTexGen();
public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);
    virtual int getGLHandle() const { return index; }

public:
    // sets and gets
    // CAPI:basename 
    // CAPI:basename TGen
    void     setMode(int _texCoord, int _mode);
    int      getMode(int _texCoord) const;
    void     setPoint(int _texCoord, float _x, float _y, float _z, float _dx, float _dy, float _dz);
    void     getPoint(int _texCoord, float *_x, float *_y, float *_z);
    void     setPlane(int _texCoord, float _x, float _y, float _z, float _d);
    void     getPlane(int _texCoord, float* _x, float* _y, float* _z, float* _d); 
	
public:
    // other functions
    // CAPI:verb
    void     apply();
    void     multiApply(int myID);


PFINTERNAL: // XXX - should make protected???
    int		index;
    int         dlindex;    // dirty bit to mark whether dl has reserved a name
    int	    	dirty;      // for changes after binding 
    int		mode[4], enable[4];
    pfVec4	plane[4];
    float	line[4][8];
    int	    	flags;	// compile, plane/point mode
    
private:
    static pfType   *classType;
};


/////////////////////////////// pfTexLOD ///////////////////////////////////////



extern "C" {     // EXPORT to CAPI
/* ------------------ pfTexLOD Related Functions--------------------- */

extern  pfTexLOD*   pfGetCurTLOD(void);
}

#define PFTEXLOD ((pfTexLOD*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFTEXLODBUFFER ((pfTexLOD*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfTexLOD : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename TLOD
    pfTexLOD();
    virtual ~pfTexLOD();
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
    // CAPI:basename 
    // CAPI:basename TLOD
    void	setRange(float _min, float _max);
    void	getRange(float *_min, float *_max);
    void	setBias(float _biasS, float _biasT, float _biasR); 
    void	getBias(float *_biasS, float *_biasT, float *_biasR); 
	
public:
    // other functions
    // CAPI:verb
    void     apply();
    void     multiApply(int myID);


PFINTERNAL: // XXX - should make protected???
    int		index;
    int		dirty;
    float	min, max;
    float	biasS, biasT, biasR;
    uint	flags;
    
private:
    static pfType   *classType;
};


void DLLEXPORT pfApplyTMat(pfMatrix *mat);
void DLLEXPORT pfApplyMultiTMat(pfMatrix *mat, int myID);

#endif	// !__PF_TEXTURE_H__
