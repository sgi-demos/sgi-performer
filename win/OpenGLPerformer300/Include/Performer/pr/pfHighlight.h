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
// pfHighlight.h
//
// $Revision: 1.34 $
// $Date: 2002/03/20 03:20:47 $
//
//
#ifndef __PF_HIGHTLIGHT_H__
#define __PF_HIGHTLIGHT_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfTexture.h>

#include <wintypes.h>



extern "C" {     // EXPORT to CAPI


/* ------------------ pfHighlight Tokens--------------------- */

/* pfGSetHlight() */
#define PFHL_INHERIT	    ((pfHighlight *) 0)
#define PFHL_ON		    ((pfHighlight *) 1)

/* pfHlightMode() */
#define PFHL_OFF	    0

#define PFHL_LINES	    0x001
#define PFHL_LINES_R	    (0x002 | PFHL_LINES) /* reverse fg and bf colors for lines */
#define PFHL_LINESPAT	    (0x004 | PFHL_LINES)
#define PFHL_LINESPAT2	    (0x008 | PFHL_LINESPAT) /* 2-pass patterning */
#define PFHL_LINESMASK	    0x00f

#define PFHL_FILL	    0x0010
#define PFHL_FILLPAT	    (0x0020 | PFHL_FILL)
#define PFHL_FILLPAT2	    (0x0040 | PFHL_FILLPAT) /* 2-pass patterning */
#define PFHL_FILLTEX	    (0x080 | PFHL_FILL)  /* tex overrides patterning  */
#define PFHL_FILL_R	    0x0100 /* reverse fg and bf colors for fill */
#define PFHL_FILLMASK	    0x01f0

#define PFHL_SKIP_BASE      0x0200
#define PFHL_POINTS	    0x0400
#define PFHL_NORMALS	    0x0800
#define PFHL_BBOX_LINES	    0x1000
#define PFHL_BBOX_FILL	    0x2000

#define PFHL_MASK	    0x3fff

/* pfHlightColor() */
#define PFHL_FGCOLOR	    0x1
#define PFHL_BGCOLOR	    0x2
/* pfHlightPat() */
#define PFHL_FGPAT    	    0x1
#define PFHL_BGPAT    	    0x2

/* ------------------ pfHighlight Related Functions--------------------- */

extern pfHighlight *pfGetCurHlight(void);

} // extern "C" END of C include export

#define PFHIGHLIGHT ((pfHighlight*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFHIGHLIGHTBUFFER ((pfHighlight*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfHighlight : public pfObject
{
public:
    // constructors and destructors
    // CAPI:basename Hlight
    pfHighlight();
    virtual ~pfHighlight();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(unsigned int _travMode, unsigned int _verbose, char *_prefix, FILE *_file);

public:
    // sets and gets
    void	 setMode(unsigned int _mode);
    unsigned int getMode() const { return mode; };
    pfGeoState	*getGState() const;
    void	 setGState(pfGeoState *_gstate);
    void	 setGStateIndex(int _id);
    int		 getGStateIndex() const;
    void	 setColor(unsigned int _which, float _r, float _g, float _b);
    void	 getColor(unsigned int _which, float *_r, float *_g, float *_b) const;
    void	 setAlpha(float _a);
    float	 getAlpha() const { return alpha; }
    void	 setNormalLength(float _len, float _bboxScale) { 
	normalLength = _len;
	normalScale = _bboxScale; 
    }
    void	 getNormalLength(float *_len, float *_bboxScale) const { 
	*_len = normalLength; 
	*_bboxScale = normalScale; 
    }
    void	 setLineWidth( float _width ) { lineWidth = _width; }
    float 	 getLineWidth() const { return lineWidth; }
    void	 setPntSize( float _size ) { pntSize = _size; }
    float 	 getPntSize() const { return pntSize; }
    void	 setLinePat(int _which, unsigned short _pat);
    unsigned short 	 getLinePat(int _which) const;
    void	 setFillPat( int _which, unsigned int *fillPat );
    void 	 getFillPat(int _which, unsigned int *_pat) const;
    void	 setTex(pfTexture *_tex);
    pfTexture	 *getTex() const { return texUser; };
    void	 setTEnv(pfTexEnv *_tev);
    pfTexEnv	 *getTEnv() const { return tevUser; };
    void	 setTGen(pfTexGen *_tgen);
    pfTexGen	 *getTGen() const { return tgenUser; };
    
public:
    // other functions
    // CAPI:verb
    void	apply();

    
private:
    friend class pfGeoSet;
    unsigned int	    mode;
    // signals multipass alg - may use later to 
    // let user have further control on alg
    pfVec4	    fgclr;
    pfVec4	    bgclr;
    pfVec4	   *lnfgclr, *lnbgclr;
    pfVec4	   *fillfgclr, *fillbgclr;
    pfGeoSet	   *hlgset; // special gset used when highlighting 
    pfMaterial	   *mtl;
    pfTexture	   *tex, *texUser; 
    pfTexEnv	   *tev, *tevUser;
    pfTexGen	   *tgen, *tgenUser; 
    GStateId	    gstate;		// Union of pfGeoState* and index 
    float	    alpha;
    float	    normalLength, normalScale; // for drawing normals
    float	    pntSize, lineWidth;
    int	            fgpatSize, bgpatSize;
    int	            prevGSetDrawIndex;
    short	    gsetDirty, backDirty;
    short	    fillPasses, linePasses;
    short	    fgLinePat, bgLinePat;
    unsigned int    fgFillPat[64], bgFillPat[64];

private:
    static pfType *classType;
};

extern "C" { /* export to C headers */
} // extern "C" END of C include export

#endif // !__PF_HIGHTLIGHT_H__
