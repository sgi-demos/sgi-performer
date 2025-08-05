
// This file contains the public interface from the Performer
// header file Performer/pr.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%section "libpr"


%{
#include <Performer/pr.h>
%}


#define PF_OFF		0
#define PF_ON		1

/* Texture coordinates */    
#define PF_S		0
#define PF_T		1
#define PF_R		2
#define PF_Q		3

/* Coordinate spaces */
#define PF_EYESPACE 0
#define PF_WORLDSPACE 1
#define PF_OBJSPACE 2


class	pfType;
class	pfObject;
class	pfMemory;
class	pfCycleMemory;
class	pfCycleBuffer;
class	pfFluxMemory;
class	pfFlux;
class	pfEngine;
class	pfList;
class 	pfImageTile;
class	pfQueue;

/* graphics */
class	pfImageCache;
class   pfClipTexture;
class	pfDispList;
class	pfGeoSet;
class	pfGeoState;
class	pfColortable;
class	pfFont;
class	pfMaterial;
class	pfLight;
class	pfLightModel;
class	pfTexture;
class	pfTexEnv;
class	pfTexLoad;
class	pfFog;
class	pfHighlight;
class	pfLPointState;
class	pfTexGen;
class	pfTexLOD;
class	pfTextureValidMap;
class	pfSprite;
class	pfState;
class	pfStats;
class	pfString;
class	pfVideoChannel;
class	pfWindow;
class   pfCalligraphic;
/* utility */
class	pfDataPool;
class	pfFile;

/*------------------------ Shared Memory ----------------------*/
 
extern int		pfInitArenas(void);
extern int		pfFreeArenas(void);

extern PF_USPTR_T* 	pfGetSemaArena(void);
extern void 		pfSemaArenaSize(size_t size);
extern size_t		pfGetSemaArenaSize(void);
extern void		pfSemaArenaBase(void *base);
extern void*		pfGetSemaArenaBase(void);

extern void*    	pfGetSharedArena(void);
extern void 		pfSharedArenaSize(size_t size);
extern size_t		pfGetSharedArenaSize(void);
extern void		pfSharedArenaBase(void *base);
extern void*		pfGetSharedArenaBase(void);

extern void	        pfTmpDir(char *dir);
extern const char *	pfGetTmpDir(void);

/* --------------------------- pfObject --------------------------------*/

/* pfOverride() Modes, pfGStateMode(), pfGStateInherit() */
#define PFSTATE_ENLIGHTING    	0x1
#define PFSTATE_ENTEXTURE     	0x4
#define PFSTATE_TRANSPARENCY  	0x10
#define PFSTATE_ALPHAFUNC	0x20
#define PFSTATE_ENFOG	    	0x40
#define PFSTATE_ANTIALIAS	0x100
#define PFSTATE_CULLFACE	0x200
#define PFSTATE_ENCOLORTABLE   	0x400
#define PFSTATE_DECAL	    	0x4000
#define PFSTATE_SHADEMODEL	0x8000
#define PFSTATE_ENWIREFRAME	0x10000
#define PFSTATE_ENHIGHLIGHTING	0x100000
#define PFSTATE_ENLPOINTSTATE	0x400000
#define PFSTATE_ENTEXGEN	0x1000000
#define PFSTATE_ENTEXMAT	0x4000000
#define PFSTATE_ENTEXLOD	0x10000000

/* pfGStateVal()  */
#define PFSTATE_ALPHAREF	0x40000

/* pfOverride() Attributes, pfGStateAttr(), pfGStateInherit() */
#define PFSTATE_FRONTMTL 	0x2
#define PFSTATE_TEXTURE       	0x8
#define PFSTATE_TEXENV	    	0x80
#define PFSTATE_COLORTABLE    	0x800
#define PFSTATE_BACKMTL       	0x1000
#define PFSTATE_FOG	    	0x2000
#define PFSTATE_LIGHTMODEL	0x20000
#define PFSTATE_LIGHTS	    	0x80000
#define PFSTATE_HIGHLIGHT	0x200000
#define PFSTATE_LPOINTSTATE	0x800000
#define PFSTATE_TEXGEN		0x2000000
#define PFSTATE_TEXMAT		0x8000000
#define PFSTATE_TEXLOD		0x20000000
#define PFSTATE_DECALPLANE	0x40000000

#define PFSTATE_ALL		\
      (	PFSTATE_ENLIGHTING    	| \
	PFSTATE_ENTEXTURE     	| \
	PFSTATE_TRANSPARENCY  	| \
	PFSTATE_ALPHAFUNC	| \
	PFSTATE_ENFOG	    	| \
	PFSTATE_ANTIALIAS	| \
	PFSTATE_CULLFACE	| \
	PFSTATE_ENCOLORTABLE   	| \
	PFSTATE_DECAL	    	| \
	PFSTATE_SHADEMODEL	| \
	PFSTATE_ENWIREFRAME	| \
	PFSTATE_ENHIGHLIGHTING	| \
	PFSTATE_ENLPOINTSTATE	| \
	PFSTATE_ENTEXGEN	| \
	PFSTATE_ENTEXLOD	| \
	PFSTATE_ENTEXMAT	| \
	PFSTATE_ALPHAREF	| \
	PFSTATE_FRONTMTL 	| \
	PFSTATE_TEXTURE       	| \
	PFSTATE_TEXENV	    	| \
	PFSTATE_COLORTABLE    	| \
	PFSTATE_BACKMTL       	| \
	PFSTATE_FOG	    	| \
	PFSTATE_LIGHTMODEL	| \
	PFSTATE_LIGHTS	    	| \
	PFSTATE_HIGHLIGHT	| \
	PFSTATE_LPOINTSTATE	| \
	PFSTATE_TEXGEN 		| \
	PFSTATE_TEXLOD 		| \
	PFSTATE_TEXMAT 		| \
	PFSTATE_DECALPLANE)


/*
 * ------------------------------------------------------------------------
 *   Alignment supplier (e.g. terrain) constants and callback definitions
 */
#define PR_QUERY_POSITION       0x0001
#define PR_QUERY_NORMAL         0x0002
#define PR_QUERY_COLOR          0x0004
#define PR_QUERY_Z              0x0008
#define PR_QUERY_FRAME          0x0010
#define PR_QUERY_RECORD_FRAME   0x0020
#define PR_QUERY_TRI_TEXTURE    0x0040
#define PR_QUERY_TRI_COLOR      0x0080
#define PR_QUERY_TRI_NORMAL     0x0100
#define PR_QUERY_TRI_COORD      0x0200

#define PR_ALIGN_TRANSLATE              0x0001
#define PR_ALIGN_AXIS_BILLBOARD         0x0002
#define PR_ALIGN_NORMAL                 0x0004
#define PR_ALIGN_AZIMUTH                0x0008

#define PR_QUERY_DIRECTIONAL            0x0001


/*---------------------------------- Draw Modes ----------------------------*/


/* pfTransparency() */
#define PFTR_OFF	    	0
#define PFTR_ON	    		1
#define PFTR_FAST    		2
#define PFTR_HIGH_QUALITY  	3
#define PFTR_BLEND_ALPHA	4
#define PFTR_MS_ALPHA		5
#define PFTR_MS_ALPHA_MASK	6

#define PFTR_NO_OCCLUDE		0x100

/* pfAntialias() */
#define PFAA_OFF	    	0
#define PFAA_ON	    	    	1
#define PFAA_MSPOINT		2
#define PFAA_MSAREA		3
#define PFAA_MSCENTER_POINT	4
#define PFAA_SMOOTH		5

/* pfDecal() */
#define PFDECAL_OFF	    		0
#define PFDECAL_LAYER	    		2
#define PFDECAL_BASE	    		3
#define PFDECAL_BASE_FAST  		4
#define PFDECAL_BASE_HIGH_QUALITY   	5
#define PFDECAL_BASE_STENCIL		6
#define PFDECAL_BASE_DISPLACE		7
#define PFDECAL_LAYER_FAST  		8
#define PFDECAL_LAYER_HIGH_QUALITY   	9
#define PFDECAL_LAYER_STENCIL		10
#define PFDECAL_LAYER_DISPLACE		11
#define PFDECAL_LAYER_DISPLACE_AWAY	12
#define PFDECAL_MODE_MASK		0x000f
#define PFDECAL_PLANE			0x0010
#define PFDECAL_ENABLE_MASK		0x001f
#define PFDECAL_LAYER_0			0x0000
#define PFDECAL_LAYER_1			0x0100
#define PFDECAL_LAYER_2			0x0200
#define PFDECAL_LAYER_3			0x0300
#define PFDECAL_LAYER_4			0x0400
#define PFDECAL_LAYER_5			0x0500
#define PFDECAL_LAYER_6			0x0600
#define PFDECAL_LAYER_7			0x0700
#define PFDECAL_LAYER_MASK		0xff00
#define PFDECAL_LAYER_SHIFT		8
#define PFDECAL_LAYER_OFFSET		0x10000

/* pfEnable()/pfDisable() */
#define PFEN_LIGHTING	    	2
#define PFEN_TEXTURE	    	3
#define PFEN_FOG	    	4
#define PFEN_WIREFRAME		5
#define PFEN_COLORTABLE		6
#define PFEN_HIGHLIGHTING	7
#define PFEN_TEXGEN		8
#define PFEN_LPOINTSTATE	9
#define PFEN_TEXMAT		10
#define PFEN_TEXLOD		11

/* pfClear() */
#define PFCL_COLOR		0x01
#define PFCL_DEPTH		0x02
#define PFCL_MSDEPTH		0x04
#define PFCL_STENCIL		0x08
#define PFCL_DITHER		0x10

/* pfGLOverride() */
#define PFGL_OFF                    0

#define PFGL_TRANSPARENCY           10
#define PFGL_TRANSP_MSALPHA         11
#define PFGL_TRANSP_BLEND           12

#define PFGL_DECAL                  20
#define PFGL_DECAL_STENCIL          21
#define PFGL_DECAL_DISPLACE         22
#define PFGL_DECAL_PLANE            23


extern void	pfShadeModel(int _model);
extern int	pfGetShadeModel(void);
extern void	pfTransparency(int _type);
extern int	pfGetTransparency(void);
extern void	pfAlphaFunc(float _ref, int _func);
extern void	pfGetAlphaFunc(float* _ref, int* _func);
extern void	pfAntialias(int _type);
extern int	pfGetAntialias(void);
extern void	pfDecal(int _mode);
extern void	pfApplyDecalPlane(pfPlane *_plane);
extern pfPlane	*pfGetCurDecalPlane(void);
extern int	pfGetDecal(void);
extern void	pfCullFace(int _cull);
extern int	pfGetCullFace(void);
extern void 	pfApplyTMat(pfMatrix *mat);
extern void 	pfClear(int _which, const pfVec4 *_col);
extern void     pfGLOverride(int _mode, float _val);
extern float    pfGetGLOverride(int _mode);

/*------------------------ GL Matrix Stack ----------------------------*/

extern double		pfGetTime(void);
extern pid_t		pfInitClock(double _time);
extern void		pfWrapClock(void);
extern void		pfClockName(char *_name);
extern const char*	pfGetClockName(void);

#define	PFCLOCK_DEFAULT   0
#define PFCLOCK_APPWRAP   1
#define	PFCLOCK_FORKWRAP  2
#define	PFCLOCK_NOWRAP    3
extern void		pfClockMode(int _mode);
extern int		pfGetClockMode(void);

/*----------------------------------- File Paths --------------------------------*/

extern void 		pfFilePath(const char* _path);
extern const char* 	pfGetFilePath(void);
extern int		pfFindFile(const char* _file, char _path[PF_MAXSTRING], int _amode);


/*------------------------------- Video Clock Routines ---------------------*/

extern int		pfStartVClock(void);
extern void		pfStopVClock(void);
extern void 		pfInitVClock(int _ticks);
extern void		pfVClockOffset(int _offset);
extern int		pfGetVClockOffset(void);
extern int 		pfGetVClock(void);
extern int 		pfVClockSync(int _rate, int _offset);

/*----------------------- pfVideoChannel Routines ------------------------*/

extern pfVideoChannel	*pfGetCurVChan(void);

/*----------------------------- pfWindow Routines ------------------------*/

extern void		prInitGfx(void);
extern pfWindow		*pfGetCurWin(void);

/*------------------------- Window System Routines -----------------------*/

extern void 		pfCloseWSConnection(pfWSConnection _dsp);

extern pfFBConfig	pfChooseFBConfig(pfWSConnection _dsp, int _screen, int *_attr);
extern pfFBConfig	pfChooseFBConfigData(void **dst, pfWSConnection dsp, int screen, int *attr, void *arena);
extern void 		pfSelectWSConnection(pfWSConnection);
extern pfWSConnection	pfOpenWSConnection(const char *_str, int _shared);
extern pfWSConnection	pfOpenScreen(int _screen, int _shared);
extern pfWSConnection	pfGetCurWSConnection(void);
extern const char*	pfGetWSConnectionName(pfWSConnection);
extern void		pfGetScreenSize(int screen, int *_x, int *_y);
extern int		pfGetNumScreenVChans(int scr);

/*-------------------------------- Query Features -----------------------*/

/* 
 * feature query return bitmasks 
 */
#define PFQFTR_FALSE	0x0	/* feature is not available */
#define PFQFTR_TRUE	0x1	/* feature is available on system */
#define PFQFTR_FAST_BIT	0x2	/* feature is reasonable to use in real-time */
#define PFQFTR_FAST	(PFQFTR_TRUE | PFQFTR_FAST_BIT)	

/* 
 * feature queries
 */
#define PFQFTR_BIT	    		    0x800000	    /* int */
#define PFQFTR_VSYNC	    		    0x800110	    /* int */
#define PFQFTR_VSYNC_SET		    0x800120	    /* int */
#define PFQFTR_GANGDRAW			    0x800210	    /* int */
#define PFQFTR_HYPERPIPE		    0x800230	    /* int */
#define PFQFTR_STEREO_IN_WINDOW		    0x800310	    /* int */
#define PFQFTR_TAG_CLEAR		    0x800320	    /* int */
#define PFQFTR_XSGIVC		    	    0x800330	    /* int */
#define PFQFTR_PBUFFER		    	    0x800340	    /* int */
#define PFQFTR_DMEDIA		    	    0x800350	    /* int */
#define PFQFTR_VLDMEDIA		    	    0x800360	    /* int */
#define PFQFTR_INTERLACE	    	    0x800370	    /* int */

#define PFQFTR_MULTISAMPLE		    0x800410	    /* int */
#define PFQFTR_MULTISAMPLE_TRANSP	    0x800420	    /* int */
#define PFQFTR_MULTISAMPLE_ROUND_POINTS	    0x800430	    /* int */
#define PFQFTR_MULTISAMPLE_STENCIL	    0x800440	    /* int */

#define PFQFTR_COLOR_ABGR		    0x800510	    /* int */

#define PFQFTR_DISPLACE_POLYGON		    0x800610	    /* int */
#define PFQFTR_POLYMODE			    0x800710	    /* int */
#define PFQFTR_TRANSPARENCY	            0x800810	    /* int */
#define PFQFTR_MTL_CMODE	            0x800910	    /* int */
    
#define PFQFTR_FOG_SPLINE		    0x800a10	    /* int */
#define PFQFTR_FOG_LAYERED		    0x800a20	    /* int */

#define PFQFTR_ALPHA_FUNC		    0x800b10	    /* int */
#define PFQFTR_ALPHA_FUNC_COMPARE_REF	    0x800b20	    /* int */

#define PFQFTR_BLENDCOLOR		    0x800c10	    /* int */
#define PFQFTR_BLEND_FUNC_SUBTRACT	    0x800c20	    /* int */
#define PFQFTR_BLEND_FUNC_MINMAX	    0x800c30	    /* int */

#define PFQFTR_FRAMEZOOM	    	    0x800d10	    /* int */
#define PFQFTR_GLSPRITE		    	    0x800d20	    /* int */
#define PFQFTR_LIGHTPOINT	    	    0x800d30	    /* int */
#define PFQFTR_DVR	    	    	    0x800d40	    /* int */
#define PFQFTR_DECAL_PLANE    	    	    0x800d50	    /* int */

/* some more OpenGL vs IRIS GL distinctions */
#define PFQFTR_LMODEL_ATTENUATION	    0x800e10    /* int */
#define PFQFTR_LIGHT_ATTENUATION	    0x800e20    /* int */
#define PFQFTR_LIGHT_CLR_SPECULAR	    0x800e30    /* int */

#define PFQFTR_TEXTURE			    0x801110	    /* int */
#define PFQFTR_TEXTURE_16BIT_IFMTS	    0x801210	    /* int */
#define PFQFTR_TEXTURE_SUBTEXTURE	    0x801310	    /* int */
#define PFQFTR_TEXTURE_TRILINEAR	    0x801410	    /* int */
#define PFQFTR_TEXTURE_DETAIL		    0x801510	    /* int */
#define PFQFTR_TEXTURE_SHARPEN		    0x801610	    /* int */
#define PFQFTR_TEXTURE_3D		    0x801710	    /* int */
#define PFQFTR_TEXTURE_PROJECTIVE	    0x801810	    /* int */
#define PFQFTR_TEXTURE_EDGE_CLAMP    	    0x801910	    /* int */
#define PFQFTR_TEXTURE_CLIPMAP    	    0x801920	    /* int */
#define PFQFTR_TEXTURE_5551                 0x801930	    /* int */
#define PFQFTR_TEXTURE_MINFILTER_BILINEAR_CMP 0x801940	    /* int */
#define PFQFTR_TEXTURE_SHADOW               0x801950        /* int */
#define PFQFTR_TEXTURE_LOD_RANGE            0x801960        /* int */
#define PFQFTR_TEXTURE_LOD_BIAS             0x801961        /* int */

#define PFQFTR_READ_MSDEPTH_BUFFER	    0x802110	    /* int */
#define PFQFTR_COPY_MSDEPTH_BUFFER	    0x802120	    /* int */
#define PFQFTR_READ_TEXTURE_MEMORY	    0x802210	    /* int */
#define PFQFTR_COPY_TEXTURE_MEMORY	    0x802220	    /* int */

#define PFQFTR_CALLIGRAPHIC                 0x803010        /* int */

#define PFQFTR_PIPE_STATS                   0x804010        /* int */



/* Feature set tokens */

#define PFFTR_BIT			    0x400000
#define PFFTR_MACH			    0x400110
#define PFFTR_VSYNC			    0x400120
#define PFFTR_VSYNC_SET		    	    0x400130
#define PFFTR_MULTISAMPLE		    0x400140
#define PFFTR_MULTISAMPLE_ROUND_POINTS	    0x400150
#define PFFTR_ALPHA_FUNC_ALL		    0x400160
#define PFFTR_DISPLACE_POLYGON		    0x400170
#define PFFTR_POLYMODE			    0x400180
#define PFFTR_FOG_SPLINE		    0x400190
#define PFFTR_FOG_LAYERED		    0x400191
#define PFFTR_GANGDRAW			    0x4001a0
#define PFFTR_HYPERPIPE		    	    0x4001b0
#define PFFTR_FAST_TRANSPARENCY		    0x4001c0	
#define PFFTR_FAST_MTL_CMODE		    0x4001d0	
#define PFFTR_PBUFFER			    0x4001e0
#define PFFTR_TEXTURE_FAST		    0x401110
#define PFFTR_TEXTURE_16BIT_IFMTS	    0x401120
#define PFFTR_TEXTURE_SUBTEXTURE	    0x401130
#define PFFTR_TEXTURE_3D		    0x401140
#define PFFTR_TEXTURE_DETAIL		    0x401150
#define PFFTR_TEXTURE_SHARPEN		    0x401160
#define PFFTR_TEXTURE_OBJECT		    0x401170
#define PFFTR_TEXTURE_PROJECTIVE	    0x401180
#define PFFTR_TEXTURE_TRILINEAR	    	    0x401190
#define PFFTR_TEXTURE_EDGE_CLAMP    	    0x4011a0
#define PFFTR_TEXTURE_CLIPMAP    	    0x4011b0
#define PFFTR_TEXTURE_5551                  0x4011c0
#define PFFTR_TEXTURE_MINFILTER_BILINEAR_CMP 0x4011d0
#define PFFTR_TEXTURE_SHADOW                0x4011e0
#define PFFTR_TEXTURE_LOD_RANGE             0x4011f0
#define PFFTR_TEXTURE_LOD_BIAS              0x4011f1
#define PFFTR_FRAMEZOOM		    	    0x402100
#define PFFTR_GLSPRITE		    	    0x402110
#define PFFTR_LIGHTPOINT	    	    0x402120
#define PFFTR_TAG_CLEAR			    0x402140
#define PFFTR_DECAL_PLANE                   0x402150
#define PFFTR_XSGIVC		    	    0x402210	
#define PFFTR_DVR	    	    	    0x402220
#define PFFTR_CALLIGRAPHIC                  0x403010
#define PFFTR_PIPE_STATS                    0x404010
#define PFFTR_DMEDIA		    	    0x404020
#define PFFTR_VLDMEDIA		    	    0x404030
#define PFFTR_INTERLACE	    		    0x404040

#define _PFFTR_NO_STATE_DLS		    0x410110
#define _PFFTR_NO_MIPMAPS		    0x410120


extern const char *pfGetMachString(void);
extern const char *pfGetRelString(void);
extern float 	pfGetIRIXRelease(void);
extern int	pfQueryFeature(int _which, int *_dst);
extern int	pfMQueryFeature(int *_which, int *_dst);
extern void	pfFeature(int _which, int _val);

/*-------------------------------- Query System ----------------------------*/

/* 
 * absolute config queries - returns 1 value 
 */
#define PFQSYS_BIT			    0x200000	/* int */
#define PFQSYS_MAJOR_VERSION		    0x200101	/* int */
#define PFQSYS_MINOR_VERSION		    0x200102	/* int */
#define PFQSYS_MAINT_VERSION		    0x200103	/* int */
#define PFQSYS_GL			    0x200110	/* int */
#define PFQSYS_IRIX_MAJOR_VERSION	    0x200120	/* int */
#define PFQSYS_IRIX_MINOR_VERSION	    0x200121	/* int */

#define PFQSYS_NUM_CPUS			    0x200210	/* int */
#define PFQSYS_NUM_CPUS_AVAILABLE	    0x200220    /* int */
#define PFQSYS_NUM_SCREENS		    0x200310	/* int */
#define PFQSYS_SIZE_PIX_X		    0x200410   	/* int */
#define PFQSYS_SIZE_PIX_Y		    0x200510   	/* int */

#define PFQSYS_MAX_SNG_RGB_BITS	    	    0x200610   	/* int */
#define PFQSYS_MAX_SNG_ALPHA_BITS    	    0x200620   	/* int */
#define PFQSYS_MAX_DBL_RGB_BITS	    	    0x200630   	/* int */
#define PFQSYS_MAX_DBL_ALPHA_BITS	    0x200640   	/* int */
#define PFQSYS_MAX_SNG_CI_BITS		    0x200650   	/* int */
#define PFQSYS_MAX_DBL_CI_BITS		    0x200660   	/* int */
#define PFQSYS_MAX_SNG_OVERLAY_CI_BITS	    0x200670   	/* int */
#define PFQSYS_MAX_DBL_OVERLAY_CI_BITS	    0x200680   	/* int */
#define PFQSYS_MAX_DEPTH_BITS		    0x200690   	/* int */
#define PFQSYS_MIN_DEPTH_VAL		    0x2006a0   	/* int */
#define PFQSYS_MAX_DEPTH_VAL		    0x2006b0   	/* int */
#define PFQSYS_MAX_STENCIL_BITS		    0x2006c0   	/* int */
#define PFQSYS_MAX_MS_SAMPLES		    0x2006d0    /* int */
#define PFQSYS_MAX_MS_DEPTH_BITS	    0x2006e0    /* int */
#define PFQSYS_MAX_MS_STENCIL_BITS	    0x2006f0	/* int */
#define PFQSYS_MAX_LIGHTS		    0x200810    /* int */
#define PFQSYS_TEXTURE_MEMORY_BYTES	    0x200920    /* int */
#define PFQSYS_MAX_TEXTURE_SIZE		    0x200930    /* int */
#define PFQSYS_MAX_CLIPTEXTURE_SIZE	    0x200940	/* int */
#define PFQSYS_CLIPTEXTURE_CENTER_ALIGNMENT 0x200950	/* int */
#define PFQSYS_MIN_CLIPTEXTURE_INVALID_BORDER 0x200960	/* int */
#define PFQSYS_TEX_SUBLOAD_ALIGNMENT_S      0x200970	/* int */
#define PFQSYS_TEX_SUBLOAD_ALIGNMENT_T      0x200971	/* int */
#define PFQSYS_TEX_SUBLOAD_ALIGNMENT_R      0x200972	/* int */
#define PFQSYS_CALLIGRAPHIC_PIPE0	    0x200980    /* int */
#define PFQSYS_CALLIGRAPHIC_PIPE1	    0x200981    /* int */
#define PFQSYS_CALLIGRAPHIC_PIPE2	    0x200982    /* int */

extern int	pfQuerySys(int _which, int *_dst);
extern int	pfMQuerySys(int *_which, int *_dst);

#define PFQPERF_DEFAULT_TEXLOAD_TABLE       0x01
#define PFQPERF_USER_TEXLOAD_TABLE          0x02
#define PFQPERF_CUR_TEXLOAD_TABLE           0x04
extern int      pfQueryPerf(int _which, void *_dst);
extern int      pfPerf(int _which, void *_dst);


