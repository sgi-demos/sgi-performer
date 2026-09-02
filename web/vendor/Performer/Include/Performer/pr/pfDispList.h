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
// pfDispList.h
//
// $Revision: 1.200 $
// $Date: 2002/11/21 02:02:01 $
//
//

#ifndef __PF_DISPLIST_H__
#define __PF_DISPLIST_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfState.h>
#include <Performer/pr/pfByteBank.h>
#include <Performer/pr/pfGSetBank.h>
#include <Performer/pr/pfGeoMath.h>


void _pfSetCurrentBBox(const unsigned short wMinX, 
		       const unsigned short wMinY, 
		       const unsigned short wMaxX, 
		       const unsigned short wMaxY);


/*
  This structure is used in the cull for keeping track of each per geoset/
  per instance bounding box
*/
typedef struct _pfScreenSpaceBoundingBox {
  //Window coordinates
  unsigned short wMinX_, wMaxX_;
  unsigned short wMinY_, wMaxY_;
#if 0
  //object space coordinates
  pfVec3 oTopLeft_;
  pfVec3 oTopRight_;
  pfVec3 oBtmLeft_;
  pfVec3 oBtmRight_;
#endif
} _pfScreenSpaceBoundingBox;

/*
  This is the global screen space bounding box that the display list sets
*/
extern _pfScreenSpaceBoundingBox pfCurrentBBox;

// display list tokens 

/*---------------------------- pfApply* -----------------------------*/

#define DL_APPLYMTL		0			/* 0 */
#define DL_APPLYLM		1		

#define DL_APPLYTEX_0		2	
#define DL_APPLYTEX_1		3	
#define DL_APPLYTEX_2		4	
#define DL_APPLYTEX_3		5	

#define DL_APPLYTEV_0		6	
#define DL_APPLYTEV_1		7	
#define DL_APPLYTEV_2		8	
#define DL_APPLYTEV_3		9	

#define DL_APPLYCTAB		10	
#define DL_APPLYFOG		11	
#define DL_APPLYFRONTMTL	12
#define DL_APPLYBACKMTL		13
#define DL_APPLYHL		14
#define DL_APPLYFRUST		15
#define DL_APPLYLPSTATE		16

#define DL_APPLYTGEN_0		17
#define DL_APPLYTGEN_1		18
#define DL_APPLYTGEN_2		19
#define DL_APPLYTGEN_3		20

#define DL_APPLYLIGHTS		21
#define DL_LIGHTON		22
#define DL_LIGHTOFF		23
#define DL_APPLYTLOAD		24
#define DL_APPLYTLOAD_PARTIAL	25
#define DL_APPLYCLIPTEX		26
#define DL_APPLYVIRTUALCLIP     27
#define DL_APPLYCLIPCENTER     	28
#define DL_APPLYCOMBINER        29
#define DL_APPLY_END		29			/* 29 */

/*---------------------------- pfDraw* -----------------------------*/

#define DL_DRAW_BGN		(DL_APPLY_END + 1)	/* 30 */
#define DL_CLEAR		(DL_DRAW_BGN + 0)
#define DL_DRAWDL		(DL_DRAW_BGN + 1)
#define DL_DRAW_GSET_GSTATE	(DL_DRAW_BGN + 2)
#define DL_DRAW_GSET		(DL_DRAW_BGN + 3)
#define DL_DRAWGLOBJ		(DL_DRAW_BGN + 4)
#define DL_DRAW_HL_GSET		(DL_DRAW_BGN + 5)
#define DL_DRAWSTRING		(DL_DRAW_BGN + 6)	
#define DL_DRAW_END		(DL_DRAW_BGN + 6)	/* 36 */


/*---------------------------- Xforms -----------------------------*/

#define DL_XFORM_BGN		(DL_DRAW_END + 1)	/* 37 */
#define DL_PUSHMATID		(DL_XFORM_BGN + 0)
#define DL_SCALE		(DL_XFORM_BGN + 1)
#define DL_TRANS		(DL_XFORM_BGN + 2)
#define DL_ROT			(DL_XFORM_BGN + 3)
#define DL_PUSHMAT		(DL_XFORM_BGN + 4)
#define DL_POPMAT		(DL_XFORM_BGN + 5)
#define DL_MULTMAT		(DL_XFORM_BGN + 6)
#define DL_LOADMAT		(DL_XFORM_BGN + 7)
#define DL_BEGINSPRITE		(DL_XFORM_BGN + 8)
#define DL_ENDSPRITE		(DL_XFORM_BGN + 9)
#define DL_SPRITETRANS		(DL_XFORM_BGN + 10)
#define DL_BEGINSPRITE_SINCOS	(DL_XFORM_BGN + 11)
#define DL_BEGINSPRITE_MAT	(DL_XFORM_BGN + 12)
#define DL_SETCURRENTBBOX       (DL_XFORM_BGN + 13)
#define DL_XFORM_END		(DL_XFORM_BGN + 13)	/* 50 */


/*---------------------------- Modes -----------------------------*/

#define DL_MODES_BGN		(DL_XFORM_END + 1)	/* 51 */
#define DL_CULLFACE		(DL_MODES_BGN + 0)
#define DL_TRANSP		(DL_MODES_BGN + 1)
#define DL_AA			(DL_MODES_BGN + 2)
#define DL_DECAL		(DL_MODES_BGN + 3)
#define DL_DECALPLANE		(DL_MODES_BGN + 4)
#define DL_AFUNC		(DL_MODES_BGN + 5)
#define DL_ENABLE		(DL_MODES_BGN + 6)
#define DL_DISABLE		(DL_MODES_BGN + 7)
#define DL_SHADEMODEL		(DL_MODES_BGN + 8)	
#define DL_MSMASK		(DL_MODES_BGN + 9)	
#define DL_GLNORMALIZE		(DL_MODES_BGN + 10)	
#define DL_MODES_END		(DL_MODES_BGN + 10)	/* 61 */

/*---------------------------- State Management---------------------*/

#define DL_STATE_BGN		(DL_MODES_END + 1)	/* 62 */
#define DL_SAVEGS		(DL_STATE_BGN + 0)
#define DL_RESTOREGS		(DL_STATE_BGN + 1)
#define DL_FLUSHSTATE   	(DL_STATE_BGN + 2)
#define DL_OVERRIDE		(DL_STATE_BGN + 3)
#define DL_ORFLAG		(DL_STATE_BGN + 4)
#define DL_LOADGSTATE		(DL_STATE_BGN + 5)
#define DL_LOADSTATE		(DL_STATE_BGN + 6)
#define DL_GSETFILTER		(DL_STATE_BGN + 7)
#define DL_GLOVERRIDE		(DL_STATE_BGN + 8)
#define DL_GSTATE_TABLE		(DL_STATE_BGN + 9)
#define DL_PUSHSTATE		(DL_STATE_BGN + 10)
#define DL_POPSTATE		(DL_STATE_BGN + 11)
#define DL_SELECTSTATE		(DL_STATE_BGN + 12)
#define DL_BASICSTATE		(DL_STATE_BGN + 13)
#define DL_STATE_END		(DL_STATE_BGN + 13)	/* 75 */

/*---------------------------- Stats ------------------------------*/

#define DL_STATS_BGN		(DL_STATE_END + 1)	/* 76 */
#define DL_TIMESTAMP		(DL_STATS_BGN + 0)
#define DL_OPEN_GFXSTATS	(DL_STATS_BGN + 1)	
#define DL_CLOSE_GFXSTATS	(DL_STATS_BGN + 2)
#define DL_ENABLE_STATSHW	(DL_STATS_BGN + 3)
#define DL_DISABLE_STATSHW	(DL_STATS_BGN + 4)
#define DL_STATS_END		(DL_STATS_BGN + 4)	/* 80 */

/*---------------------------- Texture ---------------------------*/

#define DL_TEXTURE_BGN		(DL_STATS_END + 1)	/* 81 */
#define DL_FORMAT_TEX		(DL_TEXTURE_BGN + 0)
#define DL_LOAD_TEX		(DL_TEXTURE_BGN + 1)
#define DL_FILL_TEX		(DL_TEXTURE_BGN + 2)
#define DL_APPLY_TEX_LOD_BIAS	(DL_TEXTURE_BGN + 3)
#define DL_APPLY_TEX_MIN_LOD	(DL_TEXTURE_BGN + 4)
#define DL_APPLY_TEX_MAX_LOD	(DL_TEXTURE_BGN + 5)
#define DL_APPLY_TEXLOD_0	(DL_TEXTURE_BGN + 6)
#define DL_APPLY_TEXLOD_1	(DL_TEXTURE_BGN + 7)
#define DL_APPLY_TEXLOD_2	(DL_TEXTURE_BGN + 8)
#define DL_APPLY_TEXLOD_3	(DL_TEXTURE_BGN + 9)
#define DL_TEXTURE_END		(DL_TEXTURE_BGN + 9)	/* 90 */

/* --------------------------- Lpoint Special --------------------*/

#define DL_LPOINT_BGN		(DL_TEXTURE_END +1)	/* 91 */
#define DL_LOAD_MTEXTURE        (DL_LPOINT_BGN + 0)
#define DL_RASTER               (DL_LPOINT_BGN + 1)    
#define DL_CALLIGRAPHIC         (DL_LPOINT_BGN + 2)   
#define DL_SELECT_CALLIGRAPHIC	(DL_LPOINT_BGN + 3)
#define DL_LPOINT_END		(DL_LPOINT_BGN + 3)	/* 94 */


/* ----------------------- OpenGL State elements ---------------*/

#define DL_OGL_BGN              (DL_LPOINT_END + 1)     /* 95 */
#define DL_OGL_STENCILOP        (DL_OGL_BGN + 0 )  
#define DL_OGL_STENCILFUNC      (DL_OGL_BGN + 1 )  
#define DL_OGL_STENCILMASK      (DL_OGL_BGN + 2 )   
#define DL_OGL_BLENDFUNC        (DL_OGL_BGN + 3 )   
#define DL_OGL_DEPTHFUNC        (DL_OGL_BGN + 4 )   
#define DL_OGL_DEPTHRANGE       (DL_OGL_BGN + 5 )   
#define DL_OGL_DEPTHMASK        (DL_OGL_BGN + 6 )   
#define DL_OGL_COLORMASK        (DL_OGL_BGN + 7 )   
#define DL_OGL_COLORMATRIX      (DL_OGL_BGN + 8 )   
#define DL_OGL_SHADEMODEL       (DL_OGL_BGN + 9 )  
#define DL_OGL_BLENDEQ          (DL_OGL_BGN + 10)  
#define DL_OGL_BLENDCOLOR       (DL_OGL_BGN + 11)  
#define DL_OGL_PIXELBIAS        (DL_OGL_BGN + 12)  
#define DL_OGL_PIXELSCALE       (DL_OGL_BGN + 13)  
#define DL_OGL_PIXELMAP         (DL_OGL_BGN + 14)  
#define DL_OGL_DISENABLE        (DL_OGL_BGN + 15)
#define DL_OGL_MAPCOLOR         (DL_OGL_BGN + 16)
#define DL_OGL_END              (DL_OGL_BGN + 16) /* 111 */

/*---------------------------- Misc ------------------------------*/
/* ENDFRAME and RETURN !!!must!!!! be the last tokens */

#define DL_MISC_BGN		(DL_OGL_END + 1)	/* 112 */
#define DL_NPDIST		(DL_MISC_BGN + 0)
#define DL_MODELMAT		(DL_MISC_BGN + 1)
#define DL_INVMODELMAT		(DL_MISC_BGN + 2)
#define DL_VIEWMAT		(DL_MISC_BGN + 3)
#define DL_INVVIEWMAT		(DL_MISC_BGN + 4)
#define DL_PROJMAT		(DL_MISC_BGN + 5)
#define DL_TEXMAT		(DL_MISC_BGN + 6)
#define DL_APPLYTEXMAT_0	(DL_MISC_BGN + 7)
#define DL_APPLYTEXMAT_1	(DL_MISC_BGN + 8)
#define DL_APPLYTEXMAT_2	(DL_MISC_BGN + 9)
#define DL_APPLYTEXMAT_3	(DL_MISC_BGN + 10)
#define DL_APPLYTEXMAT_NOSTATE	(DL_MISC_BGN + 11)
#define DL_IDLETEX		(DL_MISC_BGN + 12)
#define DL_PIXSCALE		(DL_MISC_BGN + 13)
#define DL_CALLBACK		(DL_MISC_BGN + 14)
#define DL_FOGSCALE             (DL_MISC_BGN + 15)
#define DL_FOGOFFSET            (DL_MISC_BGN + 16)

#define DL_DRAW_GSET_C          (DL_MISC_BGN + 17)
#define DL_DRAW_GSET_CT         (DL_MISC_BGN + 18)
#define DL_DRAW_GSET_MT4        (DL_MISC_BGN + 19)	/* 131 */
#define DL_SKIP_GSET            (DL_MISC_BGN + 20)	/* 132 */
#define DL_SKIP_GSET_GSTATE     (DL_MISC_BGN + 21)	/* 133 */
#define DL_DRAW_SHADED_GSET     (DL_MISC_BGN + 22)
#define DL_DRAW_GSET_BB		(DL_MISC_BGN + 23)
#define DL_DRAW_GSET_GSTATE_BB	(DL_MISC_BGN + 24)

#define DL_STIPPLE_MASKS        (DL_MISC_BGN + 25)
#define DL_SET_INGSTATEFLAG	(DL_MISC_BGN + 26)
#define DL_RESET_INGSTATEFLAG	(DL_MISC_BGN + 27)

/* glPushAttrib and glPopAttrib for shader */
#define DL_GL_PUSH_ATTRIB       (DL_MISC_BGN + 28)
#define DL_GL_POP_ATTRIB        (DL_MISC_BGN + 29)

/* Restore OpenGL defaults for shader */
#define DL_APPLY_SHADER_STATE   (DL_MISC_BGN + 30)

#define DL_SKIPWORDS		(DL_MISC_BGN + 31)
#define DL_ENDFRAME		(DL_MISC_BGN + 32)	
#define DL_RETURN		(DL_MISC_BGN + 33)
#define DL_MISC_END		(DL_MISC_BGN + 33)	/* 145 */

/* ENDFRAME and RETURN !!!must!!!! be the last tokens */
#define _PFDL_NUMCMDS		(DL_MISC_END + 1)	/* 146 */

#if _PFDL_NUMCMDS != 146
#error _PFDL_NUMCMDS not what was expected
#endif

/* shares compileFlags with mode */
#define DLCOMPILE_MODE_GL	0x01
#define DLCOMPILE_MODE_MASK	0x0f
#define DLCOMPILE_NEEDSID   	0x100

/* Display list state */
#define PFDL_PRE_OPEN	1
#define PFDL_OPEN	2
#define PFDL_CLOSED	3

// This is the maximum number of values in any multi-value graphic state.
// For now, multi-texture is th eonly multi-value state (Yair).

#define PF_MAX_MULTIVALUE	PF_MAX_TEXTURES

// Maximum size of all display list commands
// Currently, CALLBACK, LOADMAT, MULTMAT are largest
// 64 words + 1 cmd + 1 func pointer + 1 size for pfDLCallback
#define DL_TAIL			68		

typedef union
{
#ifdef N64
    float		f2[2];
    int			I2[2];
    uint64_t            ui64;
#endif
  float   		f;
  int			i;
  uint32_t              ui32;
  void    		*p;
} DLElt;	// Display list element 
 	

typedef struct _DLLink
{
    DLElt          *data;
    long	   dataEnd;
    struct _DLLink *next;
} DLLink;



extern "C" {     // EXPORT to CAPI
/* ----------------------- pfDispList Tokens ----------------------- */

#define PFDL_FLAT	    0
#define PFDL_RING	    1
#define PFDL_END_OF_LIST    1
#define PFDL_END_OF_FRAME   2
#define PFDL_RETURN	    3

#define PF_MAX_BANKS		32

/* pfDListMode() */
#define PFDL_MODE_COMPILE_GL	    1

/* Preprocess token */
#define PFDL_PREPROCESS_LPSTATE 0x1


/* ----------------pfDispList Related Routines------------------ */

extern pfDispList* pfGetCurDList(void);
extern void    pfDrawGLObj(GLOBJECT _obj);
	    
#ifdef DLSTATS
    if (pfCurDLStats) pfCurDLStats->cmds[cmd]++;		
#endif
extern pfDispList* pfCurDrawDList;
} // extern "C" END of C include export


#define PFDISPLIST ((pfDispList*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFDISPLISTBUFFER ((pfDispList*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfDispList : public pfObject
{
    friend class pfDispListOptimizer;

public:
    // constructors and destructors
    // CAPI:basename DList
    pfDispList(int _type, int _size);
    virtual ~pfDispList();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *prefix, FILE *_file);
    virtual int		getGLHandle() const;

public:
    // sets and gets
    int		getSize() const { return size; }
    void	setMode(int _mode, int _val);
    int		getMode(int _mode) const;
    // CAPI:verb GetDListType
    int		getDListType() const { return dlType; }
	
public:
    // other functions
    // CAPI:verb
    int		draw();
    int         isEmpty();
    int		compile();
    int		preprocess(int flag);
    void	open();
    static void	close();
    int		append(const pfDispList *_src);
    void	reset();
    void	resetBanks();
    // CAPI:verb AddDListCmd
    static void	addCmd(int _cmd);
    // CAPI:noverb
    static void	callback(pfDListFuncType _callback, int _bytes, void* _data);


PFINTERNAL:  // XXX - should make protected???

    // Non-MP-safe functions. Use myId to distinguish among callers from 
    // different processes.
    pfGeoSet	*getTmpGSet(int myId);
    void        *getTmpBuffer(int myId, int size);

    int		dlType;	    // PFDL_RING, PFDL_FLAT 
    int		size;	    // Size of display list link in DLElt's 

    uint64_t	sv_known;   // bitfield indicating which state elements 
			    // are known or not 

    uint64_t	mv_known[PF_MAX_MULTIVALUE];   
			    // bitfield indicating which multi-value state 
			    // elements are known. 
			    // The elements of ``gs'' are valid only when the
			    // corresponding *_known bit is on.

    short	gsflag;	    // flag to indicate if in ApplyGState or not
    short	compileFlags;// bitmask indicating compile mode
    GLOBJECT	glIndex;	// callobj() identifier  
    int		dirty;
    pfGeoState	gs;	    // keeps track of state when compiling displist 
    pfGeoState	*curgs;	    // last geostate added to displist 
    int		xformDirty; // flag used in DLApplyGState
    uint64_t	gsModvec;   // flag used in DLApplyGState

    pfList	*gstab;	    // current geostate table 

    //
    // For PFDL_RING only. NOTE: It is OK for these int's to wrap.
    //
    volatile int    numRead;	// Number of complete commands read. 
    volatile int    numWritten;	// Number of complete commands written. 

    volatile DLElt  *headElt;	// Head element. Append commands after head ptr. 
    volatile DLElt  *tailElt;	// Tail element. Start drawing at tail ptr. 
    DLLink	    *curLink;	// Current link to resume drawing 
    DLLink	    *headLink;	// Head and tail link pointers. If 
    DLLink	    *tailLink;  // PFDL_RING, only tailLink is used 

    // Manage some temporary memory (for LightPoints, or other extensions)

    void	**tmpMemory;
    void	**freeTmpMemory;

    // Temporary buffers for alternate texture coordinate arrays. 
    //
    // pfByteBank supplies buffers of arbitrary size to be used during the 
    // course of a single frame. When a display list is reset, all the 
    // allocated buffers are implicitly returned to the free-list.
    // 
    // pfGSetBank supplies empty pfGeoSets to be used for temporary storage 
    // in a display list. For example, in order to add a geoset to a display
    // list multiple time with a different drawIndex, withdraw a geoset from 
    // the pfGSetBank, copy the original geoset contents to it, and change the
    // drawIndex on the temp geoset. When a display list is reset, all the
    // allocated geosets are implicitly returned to the free-list.
    //
    // These are probably redundant to tmpMemory and freeTmpMemory, but 
    // they are encapsulated in a class so handling is easier (Yair).
    // 
    // These banks should be called from within the CULL process generating 
    // this display list. They are not MP-safe so calling from other processes
    // will crash.

    pfByteBank	*byteBank;
    pfGSetBank	*gsetBank;

    // Additional temporary buffers and geosets. Banks are not MP-safe 
    // so need a separate bank to support each CULL_SIDEKICK process.

    int		numGSetBanks;
    pfGSetBank  *gsetBanks[PF_MAX_BANKS];

    int		numByteBanks;
    pfByteBank  *byteBanks[PF_MAX_BANKS];

    int		m_state; // Display list state. Can be any of:
			 // PFDL_PRE_OPEN - 
			 //    From top of frame until culler opens it.
			 // PFDL_OPEN - 
			 //    Between culler open and close.
			 // PFDL_CLOSED - 
			 //    After culler closes displist for this frame.
private:
    static pfType   *classType;
};


#endif // !__PF_DISPLIST_H__
