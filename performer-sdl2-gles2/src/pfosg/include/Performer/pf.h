/* ============================================================================
 *  Performer/pf.h - pfosg stage-A shim header (NOT the SGI original).
 *
 *  API-compatible C subset of the OpenGL Performer libpf interface,
 *  implemented over OpenSceneGraph + SDL2 (src/pfosg/pfosg.cpp).
 *
 *  Math types and the entire linmath C API come from the REAL
 *  <Performer/prmath.h> (self-contained in C-API mode); this header adds the
 *  libpf object handles, tokens, and function declarations the shim
 *  implements.  The shim's Performer/pfutil.h and Performer/pfui.h forward
 *  to the real headers the same way, so tokens and struct layouts are
 *  exactly the originals.
 * ==========================================================================*/
#ifndef PFOSG_PF_H
#define PFOSG_PF_H

#include <stddef.h>
#include <sys/types.h>      /* ushort, uint - used bare by sample code */

/* the real headers are Windows-flavored; neutralize their decorations and
 * force the C view of the API even in C++ translation units */
#ifndef __declspec
#define __declspec(x)
#endif
#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 0
#endif

#include <Performer/pfDLL.h>    /* REAL: DLLEXPORT (empty off Windows); the
                                   C-API branch of prmath.h relies on it */
#include <Performer/prmath.h>   /* REAL header: pfVec*, pfMatrix, C math API */

#include <X11/Xlib.h>           /* stub; samples use Display/Window */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PF_X
#define PF_X 0
#define PF_Y 1
#define PF_Z 2
#define PF_W 3
#endif

/* (pfCoord/pfSphere/pfBox/pfSeg/pfSegSet struct bodies, PFIS_MAX_SEGS, and
 * the entire linmath/frustum C API come from the real prmath.h above) */

/* ---- libpf object handles (osg objects / shim structs underneath) -------- */

typedef struct pfObject      pfObject;
typedef struct pfNode        pfNode;
typedef struct pfGroup       pfGroup;
typedef struct pfScene       pfScene;
typedef struct pfGeode       pfGeode;
typedef struct pfDCS         pfDCS;
typedef struct pfSCS         pfSCS;
typedef struct pfLOD         pfLOD;
typedef struct pfLightSource pfLightSource;
typedef struct pfPipe        pfPipe;
typedef struct pfPipeWindow  pfPipeWindow;
typedef struct pfChannel     pfChannel;
typedef struct pfGeoState    pfGeoState;
typedef struct pfGeoSet      pfGeoSet;
typedef struct pfTexture     pfTexture;
typedef struct pfTexEnv      pfTexEnv;
typedef struct pfEarthSky    pfEarthSky;

/* opaque handles referenced by the real pfutil.h/pfui.h declarations; the
 * shim implements only what the samples actually execute */
typedef struct pfASD              pfASD;
typedef struct pfCalligraphic     pfCalligraphic;
typedef struct pfClipTexture      pfClipTexture;
typedef struct pfCompositor       pfCompositor;
typedef struct pfDataPool         pfDataPool;
typedef struct pfDispList         pfDispList;
typedef struct pfFog              pfFog;
typedef struct pfFont             pfFont;
typedef struct pfFrameStats       pfFrameStats;
typedef struct pfHighlight        pfHighlight;
typedef struct pfImageCache       pfImageCache;
typedef struct pfImageTile        pfImageTile;
typedef struct pfLPointState     pfLPointState;
typedef struct pfLight            pfLight;
typedef struct pfLightModel       pfLightModel;
typedef struct pfLightPoint       pfLightPoint;
typedef struct pfList             pfList;
typedef struct pfPath             pfPath;
typedef struct pfMPClipTexture    pfMPClipTexture;
typedef struct pfMaterial         pfMaterial;
typedef struct pfPartition        pfPartition;
typedef struct pfPipeVideoChannel pfPipeVideoChannel;
typedef struct pfSequence         pfSequence;
typedef struct pfString           pfString;
typedef struct pfSwitch           pfSwitch;
typedef struct pfTexGen           pfTexGen;
typedef struct pfText             pfText;
typedef struct pfTraverser        pfTraverser;
typedef struct pfVideoChannel     pfVideoChannel;
typedef struct pfWindow           pfWindow;
typedef void*  pfWSConnection;
typedef void*  pfWSWindow;
typedef void*  pfWSDrawable;
typedef int    pfFBConfig;
typedef struct pfVirtualClipTexLimits pfVirtualClipTexLimits;
typedef int (*pfReadImageTileFuncType)(pfImageTile* it, int ntexels);

#define PF_MAXSTRING 300

/* flag helpers (exact pr.h definitions; must be macros — an implicit
 * function declaration here silently corrupts flag words) */
#define PFFLAG_SET(flag, mask)      ((flag) |= (mask))
#define PFFLAG_UNSET(flag, mask)    ((flag) &= ~(mask))
#define PFFLAG_BOOL_SET(flag, mask, val) \
            (val ? PFFLAG_SET(flag, mask) : PFFLAG_UNSET(flag, mask))
#define PFFLAG_BOOL_GET(flag, mask) (((flag) & (mask)) ? 1 : 0)

#define PF_OFF 0
#define PF_ON  1

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

/* ---- init / process model ------------------------------------------------ */

#define PFMP_DEFAULT       (-1)
#define PFMP_APPCULLDRAW   0
#define PFMP_FORK_ISECT    0x01
#define PFMP_FORK_CULL     0x02
#define PFMP_FORK_DRAW     0x04
#define PFMP_FORK_DBASE    0x08
#define PFMP_FORK_LPOINT   0x10
#define PFMP_FORK_COMPUTE  0x20
#define PFMP_APP_CULL_DRAW (PFMP_FORK_CULL | PFMP_FORK_DRAW)
#define PFMP_APP_CULLDRAW  (PFMP_FORK_CULL)
#define PFMP_APPCULL_DRAW  (PFMP_FORK_DRAW)
#define PFMP_APPCULL_DL_DRAW 0x201
#define PFMP_CULLoDRAW     0x10000
#define PFMP_CULL_DL_DRAW  0x20000

extern int    pfInit(void);
extern int    pfMultiprocess(int mode);
extern int    pfMultipipe(int npipes);
extern int    pfGetMultipipe(void);
extern int    pfMultithread(int pipe, int stage, int nprocs);
extern int    pfConfig(void);
extern void   pfExit(void);
extern int    pfFrame(void);
extern int    pfSync(void);
extern int    pfAppFrame(void);
extern void   pfApp(void);
extern void   pfCull(void);
extern void   pfDraw(void);
extern double pfGetTime(void);
extern void   pfInitClock(double t);
extern float  pfFrameRate(float rate);
extern float  pfGetFrameRate(void);
extern int    pfGetFrameCount(void);
extern int    pfFieldRate(int fields);
extern int    pfGetFieldRate(void);
extern void   pfPhase(int phase);
extern int    pfGetPhase(void);
extern void*  pfGetSharedArena(void);
extern void*  pfMalloc(size_t nbytes, void* arena);
extern void*  pfCalloc(size_t numelem, size_t elsize, void* arena);
extern void   pfFree(void* ptr);
extern int    pfDelete(void* obj);
extern int    pfGetId(void* obj);
extern int    pfIsOfType(void* obj, void* type);
extern void   pfFilePath(const char* path);
extern const char* pfGetFilePath(void);
extern int    pfFindFile(const char* file, char path[PF_MAXSTRING], int amode);
extern const char* pfGetMachString(void);
extern int    pfQueryFeature(int which, int* dst);
extern int    pfMQueryFeature(int* which, int* dst);
extern int    pfQuerySys(int which, int* dst);
extern int    pfMultithreadParami(int pipe, int param, int val);
extern unsigned int pfGetMPBitmask(void);

/* phase tokens */
#define PFPHASE_FLOAT       0
#define PFPHASE_LOCK        1
#define PFPHASE_FREE_RUN    2
#define PFPHASE_LIMIT       3
#define PFPHASE_MODE_MASK   0xf
#define PFPHASE_SPIN_DRAW   0x10000

/* cull-sidekick (accepted, no sidekick process exists) */
#define PFSK_BACKFACE_CULL   0x00000001
#define PFSK_FRUSTUM_CULL    0x00000002
#define PFSK_CULL_DONE       1
#define PFSK_CULL_FRAME_DONE 2
#define PFSK_SIDEKICK_DONE   3
#define PFLOAD_BALANCE       2
extern void pfChanCullSidekickMode(pfChannel* chan, int mode, int val);
extern void pfMultithreadParamf(int pipe, int param, float val);

/* pfQueryFeature tokens: everything answers "no/0" except basics */
#define PFQFTR_MULTISAMPLE       1
#define PFQFTR_TAG_CLEAR         2
#define PFQFTR_TEXTURE           3
#define PFQFTR_STIPPLE           4
#define PFQFTR_TRANSPARENCY      5
#define PFQFTR_MTL_CMODE         6
#define PFQFTR_FOG_SPLINE        7
#define PFQFTR_ALPHA_FUNC_COMPARE_REF 8
#define PFQFTR_LIGHT_CLR_SPECULAR 9
#define PFQFTR_GANGDRAW          10
#define PFQFTR_HYPERPIPE         11
#define PFQFTR_STEREO_IN_WINDOW  12
#define PFQFTR_LIGHTPOINT        13
#define PFQFTR_DISPLACE_POLYGON  14
#define PFQFTR_POLYMODE          15
#define PFQFTR_FOG_LAYERED       16
#define PFQFTR_LMODEL_ATTENUATION 17
#define PFQFTR_TEXTURE_DETAIL    18
#define PFQFTR_TEXTURE_SHARPEN   19
#define PFQFTR_READ_WSDRAWABLE   20
#define PFQFTR_CALLIGRAPHIC      21
#define PFQFTR_TEXTURE_PROJECTIVE 22
#define PFQFTR_MULTITEXTURE      23
#define PFQFTR_TEXTURE_CLIPMAP   24
#define PFQFTR_VSYNC             25
#define PFQFTR_VSYNC_SET         26
#define PFQFTR_FAST              1
#define PFQFTR_TRUE              2

/* global enables */
#define PFEN_LIGHTING    1
#define PFEN_TEXTURE     2
#define PFEN_FOG         3
#define PFEN_WIREFRAME   4
#define PFEN_COLORTABLE  5
#define PFEN_HIGHLIGHTING 6
#define PFEN_LPOINTSTATE 7
#define PFEN_TEXGEN      8
extern void pfEnable(int target);
extern void pfDisable(int target);
extern void pfOverride(unsigned long long mask, int val);

/* immediate-mode state (DRAW-callback helpers; mostly no-ops on OSG) */
extern void pfPushState(void);
extern void pfPopState(void);
extern void pfBasicState(void);
extern void pfPushMatrix(void);
extern void pfPushIdentMatrix(void);
extern void pfPopMatrix(void);
extern void pfMultMatrix(pfMatrix m);
extern void pfRotate(int axis, float degrees);
extern void pfClear(int which, const pfVec4 col);
extern void pfCullFace(int cull);
extern void pfAntialias(int type);
extern void pfDrawString(const pfString* string);
extern void pfApplyMtl(pfMaterial* mtl);
extern void pfApplyLModel(pfLightModel* lm);
/* pfApplyFrust: declared by prmath.h */

#define PFCL_COLOR 0x1
#define PFCL_DEPTH 0x2
#define PFAA_ON 1
#define PFAA_OFF 0

/* ---- notification -------------------------------------------------------- */

#define PFNFY_ALWAYS 0
#define PFNFY_FATAL  1
#define PFNFY_WARN   2
#define PFNFY_NOTICE 3
#define PFNFY_INFO   4
#define PFNFY_DEBUG  5
#define PFNFY_FP_DEBUG 6

#define PFNFY_USAGE    1
#define PFNFY_RESOURCE 2
#define PFNFY_SYSERR   3
#define PFNFY_ASSERT   4
#define PFNFY_PRINT    5
#define PFNFY_INTERNAL 6
#define PFNFY_MORE     -1

extern void pfNotify(int severity, int error, const char* format, ...);
extern void pfNotifyLevel(int severity);
extern int  pfGetNotifyLevel(void);

/* ---- scene graph --------------------------------------------------------- */

extern pfScene*       pfNewScene(void);
extern pfGroup*       pfNewGroup(void);
extern pfGeode*       pfNewGeode(void);
extern pfDCS*         pfNewDCS(void);
extern pfSCS*         pfNewSCS(pfMatrix mat);
extern pfLOD*         pfNewLOD(void);
extern pfLightSource* pfNewLSource(void);
extern pfPartition*   pfNewPart(void);
extern pfText*        pfNewText(void);
extern pfSwitch*      pfNewSwitch(void);

extern int  (pfAddChild)(pfGroup* group, pfNode* child);
extern int  (pfRemoveChild)(pfGroup* group, pfNode* child);
extern pfNode* (pfGetChild)(pfGroup* group, int index);
extern int  (pfGetNumChildren)(pfGroup* group);
extern int  (pfGetNumParents)(pfNode* node);
extern int  (pfGetNodeBSphere)(pfNode* node, pfSphere* sphere);
extern void (pfNodeBSphere)(pfNode* node, pfSphere* sphere, int mode);
extern void (pfNodeName)(pfNode* node, const char* name);
extern const char* (pfGetNodeName)(pfNode* node);
extern void (pfNodeTravMask)(pfNode* node, int which, unsigned int mask,
                             int setMode, int bitOp);
typedef int (*pfNodeTravFuncType)(pfTraverser* trav, void* userData);
extern void (pfNodeTravFuncs)(pfNode* node, int which,
                              pfNodeTravFuncType pre, pfNodeTravFuncType post);
extern void (pfNodeTravData)(pfNode* node, int which, void* data);
extern pfNode* (pfGetTravNode)(pfTraverser* trav);
extern pfNode* (pfClone)(pfNode* node, int mode);
extern int  (pfFlatten)(pfNode* node, int mode);

extern void (pfDCSTrans)(pfDCS* dcs, float x, float y, float z);
extern void (pfDCSRot)(pfDCS* dcs, float h, float p, float r);
extern void (pfDCSScale)(pfDCS* dcs, float s);
extern void (pfDCSMat)(pfDCS* dcs, pfMatrix m);
extern void (pfDCSCoord)(pfDCS* dcs, pfCoord* coord);
extern void (pfGetDCSMat)(pfDCS* dcs, pfMatrix m);

typedef float (*pfLODEvalFuncType)(pfLOD* lod, pfChannel* chan, pfMatrix* m);
extern void (pfLODRange)(pfLOD* lod, int index, float range);
extern void (pfLODUserEvalFunc)(pfLOD* lod, pfLODEvalFuncType func);

extern void (pfBuildPart)(pfPartition* part);
extern void (pfUpdatePart)(pfPartition* part);
extern void (pfPartVal)(pfPartition* part, int which, float val);
extern void (pfPartAttr)(pfPartition* part, int which, void* attr);
#define PFPART_FINENESS    0
#define PFPART_MIN_SPACING 1
#define PFPART_MAX_SPACING 2
#define PFPART_ORIGIN      3

/* casts as the original C API */
#define pfAddChild(g, c)        (pfAddChild)((pfGroup*)(g), (pfNode*)(c))
#define pfRemoveChild(g, c)     (pfRemoveChild)((pfGroup*)(g), (pfNode*)(c))
#define pfGetChild(g, i)        (pfGetChild)((pfGroup*)(g), (i))
#define pfGetNumChildren(g)     (pfGetNumChildren)((pfGroup*)(g))
#define pfGetNumParents(n)      (pfGetNumParents)((pfNode*)(n))
#define pfGetNodeBSphere(n, s)  (pfGetNodeBSphere)((pfNode*)(n), (s))
#define pfNodeBSphere(n, s, m)  (pfNodeBSphere)((pfNode*)(n), (s), (m))
#define pfNodeName(n, s)        (pfNodeName)((pfNode*)(n), (s))
#define pfGetNodeName(n)        (pfGetNodeName)((pfNode*)(n))
#define pfNodeTravMask(n, w, m, sm, op) \
    (pfNodeTravMask)((pfNode*)(n), (w), (m), (sm), (op))
#define pfNodeTravFuncs(n, w, pre, post) \
    (pfNodeTravFuncs)((pfNode*)(n), (w), (pre), (post))
#define pfNodeTravData(n, w, d) (pfNodeTravData)((pfNode*)(n), (w), (d))
#define pfClone(n, m)           (pfClone)((pfNode*)(n), (m))
#define pfFlatten(n, m)         (pfFlatten)((pfNode*)(n), (m))
#define pfDCSTrans(d, x, y, z)  (pfDCSTrans)((pfDCS*)(d), (x), (y), (z))
#define pfDCSRot(d, h, p, r)    (pfDCSRot)((pfDCS*)(d), (h), (p), (r))
#define pfDCSScale(d, s)        (pfDCSScale)((pfDCS*)(d), (s))
#define pfDCSMat(d, m)          (pfDCSMat)((pfDCS*)(d), (m))
#define pfDCSCoord(d, c)        (pfDCSCoord)((pfDCS*)(d), (c))
#define pfGetDCSMat(d, m)       (pfGetDCSMat)((pfDCS*)(d), (m))
#define pfLODRange(l, i, r)     (pfLODRange)((pfLOD*)(l), (i), (r))
#define pfLODUserEvalFunc(l, f) (pfLODUserEvalFunc)((pfLOD*)(l), (f))
#define pfBuildPart(p)          (pfBuildPart)((pfPartition*)(p))
#define pfUpdatePart(p)         (pfUpdatePart)((pfPartition*)(p))
#define pfPartVal(p, w, v)      (pfPartVal)((pfPartition*)(p), (w), (v))
#define pfPartAttr(p, w, a)     (pfPartAttr)((pfPartition*)(p), (w), (a))

/* node "bound" modes */
#define PFBOUND_DYNAMIC 0
#define PFBOUND_STATIC  1

/* ---- text / string / font ------------------------------------------------- */

#define PFSTR_JUSTIFY 1
#define PFSTR_LEFT    0
#define PFSTR_CENTER  1
#define PFSTR_MIDDLE  1
#define PFSTR_RIGHT   2
#define PFSTR_DRAWSTYLE 2
#define PFSTR_EXTRUDED  2

extern pfString* pfNewString(void* arena);
extern void pfStringFont(pfString* str, pfFont* font);
extern void pfStringMode(pfString* str, int mode, int val);
extern void pfStringColor(pfString* str, float r, float g, float b, float a);
extern void pfStringString(pfString* str, const char* text);
extern void pfStringMat(pfString* str, pfMatrix m);
extern void pfFlattenString(pfString* str);
extern const pfBox* pfGetStringBBox(pfString* str);
extern int  (pfAddString)(pfText* text, pfString* str);
#define pfAddString(t, s)     (pfAddString)((pfText*)(t), (s))

/* ---- geosets -------------------------------------------------------------- */

#define PFGS_POINTS          0
#define PFGS_LINES           1
#define PFGS_LINESTRIPS      2
#define PFGS_TRIS            3
#define PFGS_QUADS           4
#define PFGS_TRISTRIPS       5
#define PFGS_FLAT_LINESTRIPS 6
#define PFGS_FLAT_TRISTRIPS  7
#define PFGS_POLYS           8
#define PFGS_TRIFANS         9
#define PFGS_FLAT_TRIFANS    10

#define PFGS_COORD3    0
#define PFGS_COLOR4    1
#define PFGS_NORMAL3   2
#define PFGS_TEXCOORD2 3

#define PFGS_OFF        0
#define PFGS_OVERALL    1
#define PFGS_PER_PRIM   2
#define PFGS_PER_VERTEX 3

extern pfGeoSet* pfNewGSet(void* arena);
extern void pfGSetPrimType(pfGeoSet* gset, int type);
extern int  pfGetGSetPrimType(pfGeoSet* gset);
extern void pfGSetNumPrims(pfGeoSet* gset, int n);
extern int  pfGetGSetNumPrims(pfGeoSet* gset);
extern void pfGSetPrimLengths(pfGeoSet* gset, int* lengths);
extern void pfGSetAttr(pfGeoSet* gset, int attr, int binding,
                       void* alist, ushort* ilist);
extern void pfGetGSetAttrLists(pfGeoSet* gset, int attr,
                               void** alist, ushort** ilist);
extern void pfGSetGState(pfGeoSet* gset, pfGeoState* gstate);
extern pfGeoState* pfGetGSetGState(pfGeoSet* gset);
extern int  (pfAddGSet)(pfGeode* geode, pfGeoSet* gset);
extern pfGeoSet* (pfGetGSet)(pfGeode* geode, int index);
extern int  (pfGetNumGSets)(pfGeode* geode);
#define pfAddGSet(g, gs)   (pfAddGSet)((pfGeode*)(g), (gs))
#define pfGetGSet(g, i)    (pfGetGSet)((pfGeode*)(g), (i))
#define pfGetNumGSets(g)   (pfGetNumGSets)((pfGeode*)(g))

/* ---- textures -------------------------------------------------------------- */

#define PFTEX_MINFILTER 0
#define PFTEX_MAGFILTER 1

#define PFTEX_POINT     0
#define PFTEX_BILINEAR  1
#define PFTEX_TRILINEAR 2
#define PFTEX_MIPMAP    3
#define PFTEX_MIPMAP_LINEAR 4

extern pfTexture* pfNewTex(void* arena);
extern int  pfLoadTexFile(pfTexture* tex, const char* fileName);
extern void pfTexFilter(pfTexture* tex, int which, int filter);
extern void pfGetTexImage(pfTexture* tex, uint** image, int* comp,
                          int* sx, int* sy, int* sz);

#define PFTE_MODULATE 0
#define PFTE_BLEND    1
#define PFTE_DECAL    2
#define PFTE_ALPHA    3

extern pfTexEnv* pfNewTEnv(void* arena);
extern void pfTEnvMode(pfTexEnv* tev, int mode);
extern void pfTEnvBlendColor(pfTexEnv* tev, float r, float g, float b, float a);

/* ---- materials / lights / fog ---------------------------------------------- */

#define PFMTL_FRONT 0
#define PFMTL_BACK  1
#define PFMTL_BOTH  2
#define PFMTL_AMBIENT  1
#define PFMTL_DIFFUSE  2
#define PFMTL_SPECULAR 3
#define PFMTL_EMISSION 4
#define PFMTL_CMODE_AMBIENT_AND_DIFFUSE 1
#define PFMTL_CMODE_AD PFMTL_CMODE_AMBIENT_AND_DIFFUSE
#define PFMTL_CMODE_OFF 0

extern pfMaterial* pfNewMtl(void* arena);
extern void pfMtlColor(pfMaterial* mtl, int which, float r, float g, float b);
extern void pfMtlColorMode(pfMaterial* mtl, int side, int mode);

extern pfLight* pfNewLight(void* arena);
extern void pfLightPos(pfLight* light, float x, float y, float z, float w);
extern void pfLightOn(pfLight* light);
extern void pfLightOff(pfLight* light);
extern void pfLightColor(pfLight* light, int which, float r, float g, float b);

extern pfLightModel* pfNewLModel(void* arena);
extern void pfLModelAmbient(pfLightModel* lm, float r, float g, float b);
extern void pfLModelLocal(pfLightModel* lm, int local);

extern void (pfLSourceColor)(pfLightSource* ls, int which,
                             float r, float g, float b);
extern void (pfLSourcePos)(pfLightSource* ls,
                           float x, float y, float z, float w);
#define pfLSourceColor(l, w, r, g, b) \
    (pfLSourceColor)((pfLightSource*)(l), (w), (r), (g), (b))
#define pfLSourcePos(l, x, y, z, w) \
    (pfLSourcePos)((pfLightSource*)(l), (x), (y), (z), (w))
#define PFLT_AMBIENT  1
#define PFLT_DIFFUSE  2
#define PFLT_SPECULAR 3

#define PFFOG_OFF     0
#define PFFOG_PIX_LIN 1
#define PFFOG_PIX_EXP 2
#define PFFOG_PIX_EXP2 3
#define PFFOG_PIX_SPLINE 4
#define PFFOG_VTX_LIN 5

extern pfFog* pfNewFog(void* arena);
extern void pfFogType(pfFog* fog, int type);
extern void pfFogColor(pfFog* fog, float r, float g, float b);
extern void pfFogRange(pfFog* fog, float onset, float opaque);

/* ---- geostate --------------------------------------------------------------*/

#define PFSTATE_TRANSPARENCY 1
#define PFSTATE_ALPHAFUNC    2
#define PFSTATE_ENLIGHTING   3
#define PFSTATE_ENTEXTURE    4
#define PFSTATE_CULLFACE     5
#define PFSTATE_ENTEXMAT     6
#define PFSTATE_ALPHAREF     7
#define PFSTATE_FRONTMTL     8
#define PFSTATE_TEXTURE      9
#define PFSTATE_TEXENV       10
#define PFSTATE_TEXMAT       11
#define PFSTATE_ENFOG        12
#define PFSTATE_ENWIREFRAME  13
#define PFSTATE_BACKMTL      14
#define PFSTATE_FOG          15
#define PFSTATE_LIGHTMODEL   16
#define PFSTATE_LIGHTS       17
#define PFSTATE_ENLPOINTSTATE 18
#define PFSTATE_LPOINTSTATE  19
#define PFSTATE_ANTIALIAS    20
#define PFSTATE_ENTEXGEN     21
#define PFSTATE_TEXGEN       22
#define PFSTATE_ENHIGHLIGHTING 23
#define PFSTATE_HIGHLIGHT    24

#define PFTR_OFF          0
#define PFTR_ON           1
#define PFTR_HIGH_QUALITY 2
#define PFTR_FAST         3
#define PFTR_BLEND_ALPHA  4
#define PFTR_MS_ALPHA     5
#define PFTR_NO_OCCLUDE   0x100

#define PFAF_OFF      0
#define PFAF_NEVER    1
#define PFAF_LESS     2
#define PFAF_EQUAL    3
#define PFAF_LEQUAL   4
#define PFAF_GREATER  5
#define PFAF_NOTEQUAL 6
#define PFAF_GEQUAL   7
#define PFAF_ALWAYS   8

#define PFCF_OFF   0
#define PFCF_BACK  1
#define PFCF_FRONT 2
#define PFCF_BOTH  3

extern pfGeoState* pfNewGState(void* arena);
extern void pfGStateMode(pfGeoState* gs, int mode, int val);
extern void pfGStateVal(pfGeoState* gs, int which, float val);
extern void pfGStateAttr(pfGeoState* gs, int which, void* attr);
extern void (pfSceneGState)(pfScene* scene, pfGeoState* gs);
extern pfGeoState* pfMakeBasicGState(pfGeoState* gs);
extern void pfMakeBasicState(void);
#define pfSceneGState(s, g) (pfSceneGState)((pfScene*)(s), (g))

/* ---- earth-sky --------------------------------------------------------------*/

#define PFES_BUFFER_CLEAR 1

#define PFES_TAG      0
#define PFES_FAST     1
#define PFES_SKY      2
#define PFES_SKY_GRND 3
#define PFES_SKY_CLEAR 4

#define PFES_CLEAR     0
#define PFES_SKY_TOP   1
#define PFES_SKY_BOT   2
#define PFES_GRND_FAR  3
#define PFES_GRND_NEAR 4
#define PFES_HORIZ     5

#define PFES_GRND_HT   0
#define PFES_GENERAL     381   /* real pf.h values */
#define PFES_HORIZ_ANGLE 317

extern pfEarthSky* pfNewESky(void);
extern void pfESkyMode(pfEarthSky* esky, int mode, int val);
extern void pfESkyAttr(pfEarthSky* esky, int attr, float val);
extern void pfESkyColor(pfEarthSky* esky, int which,
                        float r, float g, float b, float a);
extern void pfESkyFog(pfEarthSky* esky, int which, pfFog* fog);

/* ---- pipe / window / channel ---------------------------------------------- */

#define PFPWIN_TYPE_X       1
#define PFWIN_TYPE_X        1
#define PFPWIN_TYPE_PBUFFER 2
#define PFPWIN_TYPE_STATS   4
#define PFWIN_TYPE_OVERLAY  8
#define PFWIN_TYPE_NOPORT   16

typedef void (*pfPWinFuncType)(pfPipeWindow* pw);
typedef void (*pfStageFuncType)(int pipe, unsigned int stage);

extern pfPipe*       pfGetPipe(int index);
extern int           pfGetPipeNum(void);   /* "which pipe is this process" */
extern int           pfGetId(void*);
extern void          pfPipeScreen(pfPipe* pipe, int screen);
extern void          pfGetPipeSize(pfPipe* pipe, int* xs, int* ys);
extern void          pfPipeSwapFunc(pfPipe* pipe, void* func);
extern const char*   pfPipeWSConnectionName(pfPipe* pipe, const char* name);
extern int           pfGetPipeDrawCount(pfPipe* pipe);
extern void          pfPipeIncrementalStateChanNum(pfPipe* pipe, int num);
extern int           pfGetPipeNumMPClipTextures(pfPipe* pipe);
extern void          pfStageConfigFunc(int pipe, unsigned int stage,
                                       pfStageFuncType func);
extern void          pfConfigStage(int pipe, unsigned int stage);
extern int           pfIsectFunc(void (*func)(void*));

#define PFPROC_APP   0x1
#define PFPROC_CULL  0x2
#define PFPROC_DRAW  0x4
#define PFPROC_ISECT 0x8
#define PFPROC_DBASE 0x10
#define PFPROC_CLOCK 0x20
#define PFPROC_LPOINT 0x0080

/* window framebuffer-config attribute tokens (real pf.h values) */
#define PFFB_RGBA         4
#define PFFB_DOUBLEBUFFER 5
#define PFFB_RED_SIZE     8
#define PFFB_ALPHA_SIZE   11
#define PFFB_DEPTH_SIZE   12
#define PFFB_STENCIL_SIZE 13
#define PFFB_SAMPLE_BUFFER 100000
#define PFFB_SAMPLES      100001

/* highlight draw modes (real pr.h values) */
#define PFHL_LINES      0x001
#define PFHL_LINES_R    (0x002 | PFHL_LINES)
#define PFHL_FILL       0x0010
#define PFHL_SKIP_BASE  0x0200
#define PFHL_POINTS     0x0400
#define PFHL_NORMALS    0x0800
#define PFHL_BBOX_LINES 0x1000
#define PFHL_BBOX_FILL  0x2000
#define PFHL_LINESPAT   (0x004 | PFHL_LINES)
#define PFHL_FILLPAT    (0x0020 | PFHL_FILL)
#define PFHL_FILLTEX    (0x080 | PFHL_FILL)
/* node paths (pfPath = growable list of node pointers) */
extern pfPath* pfNewPath(void);

/* data pools (single-process shim: heap-backed; locks are no-ops) */
extern volatile void* pfDPoolFind(pfDataPool* dpool, int id);
extern int  pfDPoolLock(void* dpmem);
extern void pfDPoolUnlock(void* dpmem);

/* GUI-panel support (libpfutil gui.c) */
extern void pfAddChan(pfPipeWindow* pw, pfChannel* chan);
extern void pfApplyHlight(pfHighlight* hl);
extern void pfHlightLineWidth(pfHighlight* hl, float width);
extern void pfHlightPntSize(pfHighlight* hl, float size);
extern int  pfGetHyperpipe(pfPipe* p);
extern float pfGetSwitchVal(const pfSwitch* sw);
extern volatile void* pfDPoolAlloc(pfDataPool* dpool, unsigned int size,
                                   int id);
extern int  pfDPoolFree(pfDataPool* dpool, void* dpmem);
extern int  pfChanPick(pfChannel* chan, int mode, float px, float py,
                       float radius, pfHit** pickList[]);
extern void pfGetChanOutputOrigin(pfChannel* chan, int* x, int* y);
extern void pfGetChanOutputSize(pfChannel* chan, int* xs, int* ys);
extern pfWindow* pfGetCurWin(void);
extern void pfGetWinSize(const pfWindow* win, int* xs, int* ys);

/* class-type queries (pfIsOfType cookies) */
extern pfType* pfGetNodeClassType(void);
extern pfType* pfGetGroupClassType(void);
extern pfType* pfGetGeodeClassType(void);
extern pfType* pfGetSCSClassType(void);
extern pfType* pfGetSwitchClassType(void);
extern const char* pfGetTypeName(const void* data);
extern pfGroup* (pfGetParent)(const pfNode* node, int i);
extern void (pfGetSCSMat)(pfSCS* scs, pfMatrix m);
extern void* (pfGetNodeTravData)(pfNode* node, int trav);
extern void (pfGetNodeTravFuncs)(pfNode* node, int trav,
                                 pfNodeTravFuncType* pre,
                                 pfNodeTravFuncType* post);

/* pfList (heap-backed) */
extern pfList* pfNewList(int eltSize, int listLength, void* arena);
extern void  pfResetList(pfList* list);
extern void  pfAdd(pfList* list, void* elt);
extern void* pfGet(const pfList* list, int index);
extern int   pfGetNum(const pfList* list);
extern int   pfSearch(const pfList* list, void* elt);
extern int   pfCopy(void* dst, void* src);

/* immediate-mode transforms (DRAW-phase only) */
extern void pfScale(float x, float y, float z);
extern void pfTranslate(float x, float y, float z);

#define PFHL_FGCOLOR    0x1
#define PFHL_BGCOLOR    0x2
extern void pfHlightMode(pfHighlight* hl, unsigned int mode);
extern void pfHlightColor(pfHighlight* hl, unsigned int which,
                          float r, float g, float b);
extern pfHighlight* pfNewHlight(void* arena);

/* pfPrint verbosity */
#define PFPRINT_VB_OFF    0
#define PFPRINT_VB_ON     1
#define PFPRINT_VB_NOTICE 1
#define PFPRINT_VB_INFO   2
#define PFPRINT_VB_DEBUG  3
extern void pfPrint(void* obj, unsigned long long which, int verbose,
                    void* file);

/* compositors (IRIX video hardware; accepted and ignored) */
#define PFLOAD_COEFF 1
#define PFCOMP_TYPE  0
#define PFCOMP_2x2   1
#define PFCOMP_4x1   2
#define PFCOMP_1x4   3
extern pfCompositor* pfNewCompositor(void);
extern void pfCompositorAddChild(pfCompositor* c, int pipe);
extern void pfCompositorVal(pfCompositor* c, int which, float val);
extern void pfCompositorMode(pfCompositor* c, int mode, int val);
extern int  pfGetCompositorMode(pfCompositor* c, int mode);
extern void pfCompositorViewport(pfCompositor* c,
                                 float l, float r, float b, float t);
extern void pfCompositorReconfig(pfCompositor* c);
extern void pfCompositorMasterPipe(pfCompositor* c, pfPipe* pipe);
extern void pfCompositorChannelClipped(pfCompositor* c, int chan, int val);
extern void pfHyperpipe(int npipes);
extern void pfHyperpipe2D(pfCompositor* c);

extern pfPipeWindow* pfNewPWin(pfPipe* pipe);
extern pfPipeWindow* pfGetPipePWin(pfPipe* pipe, int index);
extern int  pfGetPWinIndex(pfPipeWindow* pw);
extern void pfPWinIndex(pfPipeWindow* pw, int index);
extern void pfPWinType(pfPipeWindow* pw, int type);
extern int  pfGetPWinType(pfPipeWindow* pw);
extern void pfPWinName(pfPipeWindow* pw, const char* name);
extern void pfPWinOriginSize(pfPipeWindow* pw, int x, int y, int xs, int ys);
extern void pfGetPWinSize(pfPipeWindow* pw, int* xs, int* ys);
extern void pfGetPWinOrigin(pfPipeWindow* pw, int* x, int* y);
extern int  pfGetPWinScreen(pfPipeWindow* pw);
extern int  pfOpenPWin(pfPipeWindow* pw);
extern void pfClosePWin(pfPipeWindow* pw);
extern void pfPWinConfigFunc(pfPipeWindow* pw, pfPWinFuncType func);
extern void pfConfigPWin(pfPipeWindow* pw);
extern void pfPWinMode(pfPipeWindow* pw, int mode, int val);
extern void pfPWinFBConfig(pfPipeWindow* pw, void* fbconfig);
extern void pfPWinFBConfigAttrs(pfPipeWindow* pw, int* attrs);
extern void pfPWinFBConfigId(pfPipeWindow* pw, int id);
extern void pfSwapPWinBuffers(pfPipeWindow* pw);
extern int  pfGetPWinSelect(pfPipeWindow* pw);
extern void pfSelectPWin(pfPipeWindow* pw);
extern pfPipe* pfGetPWinPipe(pfPipeWindow* pw);
extern void* pfGetPWinOverlayWin(pfPipeWindow* pw);
extern void* pfGetCurWSConnection(void);
extern unsigned long pfGetPWinWSWindow(pfPipeWindow* pw);
extern void pfSelectWin(pfWindow* win);
extern void pfCloseWin(pfWindow* win);
extern pfWindow* pfOpenNewNoPortWin(const char* name, int screen);

/* traversals */
#define PFTRAV_APP   0
#define PFTRAV_CULL  1
#define PFTRAV_DRAW  2
#define PFTRAV_ISECT 3
#define PFTRAV_LPOINT 4

#define PFTRAV_SELF     0x1
#define PFTRAV_DESCEND  0x2
#define PFTRAV_IS_CACHE 0x4
#define PFTRAV_CONT     0
#define PFTRAV_PRUNE    1
#define PFTRAV_TERM     2
#define PF_SET 0
#define PF_AND 1
#define PF_OR  2

typedef void (*pfChanFuncType)(pfChannel* chan, void* userData);

extern pfChannel* pfNewChan(pfPipe* pipe);
extern pfPipe*    pfGetChanPipe(pfChannel* chan);
extern pfPipeWindow* pfGetChanPWin(pfChannel* chan);
extern void pfChanScene(pfChannel* chan, pfScene* scene);
extern void pfChanFOV(pfChannel* chan, float fovh, float fovv);
extern void pfGetChanFOV(pfChannel* chan, float* fovh, float* fovv);
extern void pfChanNearFar(pfChannel* chan, float nearDist, float farDist);
extern void pfGetChanNearFar(pfChannel* chan, float* nearDist, float* farDist);
extern void pfChanView(pfChannel* chan, pfVec3 xyz, pfVec3 hpr);
extern void pfGetChanView(pfChannel* chan, pfVec3 xyz, pfVec3 hpr);
extern void pfChanViewMat(pfChannel* chan, pfMatrix mat);
extern void pfGetChanViewMat(pfChannel* chan, pfMatrix mat);
extern void pfGetChanOffsetViewMat(pfChannel* chan, pfMatrix mat);
extern void pfChanViewOffsets(pfChannel* chan, pfVec3 xyz, pfVec3 hpr);
extern void pfChanESky(pfChannel* chan, pfEarthSky* esky);
extern void pfChanTravFunc(pfChannel* chan, int trav, pfChanFuncType func);
extern void pfChanTravMode(pfChannel* chan, int trav, int mode);
extern int  pfGetChanTravMode(pfChannel* chan, int trav);
extern void pfChanTravMask(pfChannel* chan, int trav, unsigned int mask);
extern void pfClearChan(pfChannel* chan);
extern void pfDrawChanStats(pfChannel* chan);
extern void pfChanViewport(pfChannel* chan, float l, float r, float b, float t);
extern void pfGetChanViewport(pfChannel* chan, float* l, float* r,
                              float* b, float* t);
extern void pfGetChanOrigin(pfChannel* chan, int* x, int* y);
extern void pfGetChanSize(pfChannel* chan, int* xs, int* ys);
extern void pfChanShare(pfChannel* chan, unsigned int mask);
extern unsigned int pfGetChanShare(pfChannel* chan);
extern void pfChanAutoAspect(pfChannel* chan, int which);
extern void pfChanProjMode(pfChannel* chan, int mode);
extern void pfChanBinOrder(pfChannel* chan, int bin, int order);
extern void pfChanLODAttr(pfChannel* chan, int attr, float val);
extern float pfGetChanLODAttr(pfChannel* chan, int attr);
extern void pfChanStressFilter(pfChannel* chan, float frac, float low,
                               float high, float s, float max);
extern void pfChanStatsMode(pfChannel* chan, unsigned int mode,
                            unsigned int val);
extern pfFrameStats* pfGetChanFStats(pfChannel* chan);
extern void pfChanCullPtope(pfChannel* chan, void* ptope);
extern void pfGetChanBaseFrust(pfChannel* chan, pfFrustum* frust);
extern void pfMakeSimpleChan(pfChannel* chan, float fov);
extern void pfMakeOrthoChan(pfChannel* chan, float l, float r,
                            float b, float t);
extern void pfAttachChan(pfChannel* master, pfChannel* slave);
extern pfPipeVideoChannel* pfGetChanPVChan(pfChannel* chan);

#define PFCHAN_LOD_SCALE  1
#define PFCHAN_LOD_FADE   2
#define PFCHAN_LOD_STRESS_PIX_LIMIT 3
#define PFLOD_SCALE  1
#define PFLOD_FADE   3
#define PFLOD_STRESS_PIX_LIMIT 4
#define PFFRUST_SIMPLE 0
#define PFFRUST_ORTHOGONAL 1
#define PFFRUST_PERSPECTIVE 2

/* share masks */
#define PFCHAN_FOV              0x1
#define PFCHAN_VIEW             0x2
#define PFCHAN_VIEW_OFFSETS     0x4
#define PFCHAN_NEARFAR          0x8
#define PFCHAN_SCENE            0x10
#define PFCHAN_EARTHSKY         0x20
#define PFCHAN_STRESS           0x40
#define PFCHAN_LOD              0x80
#define PFCHAN_SWAPBUFFERS      0x100
#define PFCHAN_APPFUNC          0x200
#define PFCHAN_CULLFUNC         0x400
#define PFCHAN_DRAWFUNC         0x800
#define PFCHAN_STATS_DRAWMODE   0x1000
#define PFCHAN_VIEWPORT         0x2000
#define PFCHAN_SWAPBUFFERS_HW   0x4000
#define PFCHAN_CULL_VOLUME      0x8000

#define PFDRAW_ON 1
#define PFDRAW_OFF 0

/* pfChanTravMode CULL modes */
#define PFCULL_VIEW 0x1
#define PFCULL_SORT 0x2
#define PFCULL_GSET 0x4
#define PFCULL_IGNORE_LSOURCES 0x8
#define PFCULL_ALL  0x7
#define PFSORT_NO_ORDER   (-1)
#define PFSORT_OPAQUE_BIN 0
#define PFSORT_TRANSP_BIN 1

/* stats: everything accepted, nothing recorded */
#define PFSTATS_ENGFX      0x1
#define PFSTATS_OFF        0
#define PFSTATS_ON         1
#define PFSTATS_SET        3   /* real prstats.h values */
#define PFFSTATS_ENPFTIMES 0x2
#define PFFSTATS_ENDB      0x4
#define PFFSTATS_ENCULL    0x8
#define PFSTATS_ENTEXLOAD  0x10
#define PFFSTATS_BUF_PREV  1
#define PFFSTATS_PFTIMES_HIST 2
#define PFSTATS_ALL        (~((unsigned int)0))
#define PFSTATS_DEFAULT    2
#define PFFSTATS_CLASSES_START 100
#define PFFSTATS_PFTIMES   (4 + PFFSTATS_CLASSES_START)
#define PFFSTATS_PFTIMES_BASIC 0x01000000
#define PFWIN_GFX_WIN     (-1)
#define PFWIN_STATS_WIN   (-3)
#define PFSTATS_EN_BITS   16
#define PFFSTATS_PFTIMES_MASK 0x03000000
#define PFFSTATS_BUF_AVG  (0x2000u << PFSTATS_EN_BITS)
#define PFCSTATS_DRAW     1
#define PFSTATSHW_GFXPIPE_FILL 2
#define PFSTATSHW_GFXPIPE_FILL_DEPTHCMP    0x0004
#define PFSTATSHW_GFXPIPE_FILL_TRANSPARENT 0x0008
#define PFSTATS_GFX 1
#define PFSTATS_GFX_TSTRIP_LENGTHS 0x0002
#define PFSTATS_GFX_ATTR_COUNTS    0x0004
#define PFSTATS_ENCALLIG  0x0040
#define PFSK_OPTIMIZATION 0
#define PFSK_POLICY       1
#define PFPART_DEBUG      2
#define PFCHAN_PROJ_VIEWPORT 1
#define PFCHAN_PROJ_WINDOW   2
#define PFFSTATS_UPDATE_SECS 3000
#define PFSTATSHW_ENGFXPIPE_TIMES 0x0001
#define PFSTATSHW_ENGFXPIPE_FILL  0x0002
#define PFFSTATS_ENGFXPFTIMES (PFFSTATS_ENPFTIMES | PFSTATSHW_ENGFXPIPE_TIMES)
extern unsigned int pfFStatsClass(pfFrameStats* fs, unsigned int mask, int val);
extern unsigned int pfFStatsClassMode(pfFrameStats* fs, int cls,
                                      unsigned int mask, int val);
extern void pfFStatsAttr(pfFrameStats* fs, int attr, float val);
extern unsigned int pfGetFStatsClass(pfFrameStats* fs, unsigned int mask);
extern void pfFStatsCountNode(pfFrameStats* fs, int cls, pfNode* node);
extern double pfGetFrameTimeStamp(void);

/* frustum object C API: declared by prmath.h */

/* video channels (accepted; no DVR) */
extern pfPipeVideoChannel* pfGetPipeVChan(pfPipe* pipe, int index);
extern void pfPVChanDVRMode(pfPipeVideoChannel* pvc, int mode);
extern int  pfGetPVChanDVRMode(pfPipeVideoChannel* pvc);
extern void pfPVChanOutputSize(pfPipeVideoChannel* pvc, int xs, int ys);
extern void pfGetPVChanOutputSize(pfPipeVideoChannel* pvc, int* xs, int* ys);
extern void pfGetPVChanSize(pfPipeVideoChannel* pvc, int* xs, int* ys);
extern void pfGetPVChanScale(pfPipeVideoChannel* pvc, float* x, float* y);
extern void pfPVChanStressFilter(pfPipeVideoChannel* pvc, float frameFrac,
                                 float lowLoad, float highLoad,
                                 float pipeLoadScale, float minScale,
                                 float maxScale);
#define PFPVC_DVR_OFF    0
#define PFPVC_DVR_MANUAL 0x1
#define PFPVC_DVR_AUTO   0x2

/* ---- intersection ----------------------------------------------------------- */

#define PFTRAV_IS_PRIM      0x1
#define PFTRAV_IS_NORM      0x2
#define PFTRAV_IS_CULL_BACK 0x4
#define PFTRAV_IS_POINT     0x8
#define PFTRAV_IS_UV        0x10
#define PFTRAV_IS_PATH      0x400

/* pick traversal modes (LOD/switch/sequence: current child = 0) */
#define PFPK_M_NEAREST 0x0
#define PFTRAV_LOD_CUR 0x0
#define PFTRAV_SW_CUR  0x0
#define PFTRAV_SEQ_CUR 0x0

#define PFQHIT_FLAGS 0
#define PFQHIT_POINT 1
#define PFQHIT_NORM  2
#define PFQHIT_XFORM 3
#define PFQHIT_PATH  23

/* pfQuerySys */
#define PFQSYS_MAX_DBL_RGB_BITS 0x200630

extern int (pfNodeIsectSegs)(pfNode* node, pfSegSet* segSet, pfHit** hits[]);
#define pfNodeIsectSegs(n, s, h) (pfNodeIsectSegs)((pfNode*)(n), (s), (h))
extern int pfQueryHit(pfHit* hit, int which, void* dst);
extern int pfChanNodeIsectSegs(pfChannel* chan, pfNode* node,
                               pfSegSet* segSet, pfHit** hits[]);

#ifdef __cplusplus
}
#endif

#endif /* PFOSG_PF_H */
