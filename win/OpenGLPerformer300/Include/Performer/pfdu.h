/*
 * Copyright 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
 */

/*
 * file: pfdu.h
 * --------------
 *
 * $Revision: 1.125 $
 * $Date: 2002/11/13 23:26:16 $
 *
 * IRIS Performer database library header.
 */

#ifndef __PFDU_H__
#define __PFDU_H__


#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/objfnt.h>
#if PF_CPLUSPLUS_API
#include <Performer/pr/pfLinMath.h>
#endif
#include <Performer/pfdu-DLL.h>


#ifdef __cplusplus
extern "C" {
#endif

	/*---------------------- Database Conversions -----------------------*/

/* read a file in any of the above 3D-formats */
extern PFDUDLLEXPORT pfNode* pfdLoadFile(const char *file);

/* read a file in any of the above 3D-formats */
extern PFDUDLLEXPORT int pfdStoreFile(pfNode *root, const char *file);

/* convert an in-memory representation to Performer */
extern PFDUDLLEXPORT pfNode* pfdConvertFrom(void *root, const char *ext);

/* convert a Performer subgraph to another in-memory respresentation */
extern PFDUDLLEXPORT void* pfdConvertTo(pfNode* root, const char *ext);

/* open DSO for the loader, useful before pfConfig() */
extern PFDUDLLEXPORT int pfdInitConverter(const char *_ext);

/* discard any database loaders that have been dynamically loaded */
extern PFDUDLLEXPORT int pfdExitConverter(const char *_ext);

/* locate and open a file using the IRIS Performer search path */
extern PFDUDLLEXPORT FILE* pfdOpenFile(const char *_file);

/* Add a file extension alias for db loaders to recognize */
extern PFDUDLLEXPORT void pfdAddExtAlias(const char *ext, const char *alias);

/* Set/Get the mode/attr/val of a particular loader */
extern PFDUDLLEXPORT void pfdConverterMode(const char *ext, int mode, int value);
extern PFDUDLLEXPORT int pfdGetConverterMode(const char *ext, int mode);
extern PFDUDLLEXPORT void pfdConverterAttr(const char *ext, int which, void *attr);
extern PFDUDLLEXPORT void* pfdGetConverterAttr(const char *ext, int which);
extern PFDUDLLEXPORT void pfdConverterVal(const char *ext, int which, float val);
extern PFDUDLLEXPORT float pfdGetConverterVal(const char *ext, int which);

/* user callback function registry */
extern PFDUDLLEXPORT int pfdRegisterUserFunc(void *func,
			       const char *name, const char *dso_name);
extern PFDUDLLEXPORT int pfdGetRegisteredUserFunc(void *func, char **name, char **dso_name);
extern PFDUDLLEXPORT int pfdIsRegisteredUserFunc(void *func);
extern PFDUDLLEXPORT void * pfdFindRegisteredUserFunc(char *name);

/* Simple summary statistics */
extern PFDUDLLEXPORT void pfdPrintSceneGraphStats(pfNode *node, double elapsedTime);



	/*-------------------- Generate pfGeoSets ---------------------------*/

/* 3D primitives */
	/* Unit cube */
extern PFDUDLLEXPORT pfGeoSet * pfdNewCube(void *arena);
	/* Unit sphere */
extern PFDUDLLEXPORT pfGeoSet * pfdNewSphere(int ntris, void *arena);
	/* radius=1, from Z=-1 to Z=1 */
extern PFDUDLLEXPORT pfGeoSet * pfdNewCylinder(int ntris, void *arena);
	/* radius=1, from Z=0 to Z=1 */
extern PFDUDLLEXPORT pfGeoSet * pfdNewCone(int ntris, void *arena);
	/* CYLINDER without end caps and variable radii */
extern PFDUDLLEXPORT pfGeoSet * pfdNewPipe(float botRadius, float topRadius, int ntris, void *arena);
	/* Unit square base, from Z=0 to Z=1 */
extern PFDUDLLEXPORT pfGeoSet * pfdNewPyramid(void *arena);
	/* Z=0 to Z=1 */
extern PFDUDLLEXPORT pfGeoSet * pfdNewArrow(int ntris, void *arena);
	/* Z=-1 to Z=1 */
extern PFDUDLLEXPORT pfGeoSet * pfdNewDoubleArrow(int ntris, void *arena);

extern PFDUDLLEXPORT pfGeoSet * pfdNewTorus(float rMajor, float rMinor, int nDivMajor, int nDivMinor, void* arena);

/* 2D primitives */
	/* Unit circle facing +Z, filled */
extern PFDUDLLEXPORT pfGeoSet * pfdNewCircle(int ntris, void *arena);
	/* Unit circle in Z=0 plane, lines */
extern PFDUDLLEXPORT pfGeoSet * pfdNewRing(int ntris, void *arena);


#if !PF_CPLUSPLUS_API
extern PFDUDLLEXPORT void pfdXformGSet(pfGeoSet *gset, pfMatrix mat);
#else
extern PFDUDLLEXPORT void pfdXformGSet(pfGeoSet *gset, pfMatrix &mat);
#endif

extern PFDUDLLEXPORT void pfdGSetColor(pfGeoSet *gset, float r, float g, float b, float a);

	/*-------------------- Mesh Triangles ---------------------------*/

#define PFDMESH_SHOW_TSTRIPS	0
#define PFDMESH_RETESSELLATE	1
#define PFDMESH_INDEXED		2
#define PFDMESH_MAX_TRIS	3
#define PFDMESH_LOCAL_LIGHTING	4

extern PFDUDLLEXPORT pfGeoSet* pfdMeshGSet(pfGeoSet *_gset);
extern PFDUDLLEXPORT void pfdMesherMode(int _mode, int _val);
extern PFDUDLLEXPORT int pfdGetMesherMode(int _mode);
extern PFDUDLLEXPORT void pfdShowStrips(pfGeoSet *gset);

	/*-------------------- Optimize Scene Graphs -----------------------------------*/

extern PFDUDLLEXPORT pfNode*	pfdCleanTree(pfNode *node, pfuTravFuncType doitfunc);
extern PFDUDLLEXPORT void	pfdReplaceNode(pfNode *oldn, pfNode *newn);
extern PFDUDLLEXPORT void	pfdInsertGroup(pfNode *oldn, pfGroup *grp);
extern PFDUDLLEXPORT void	pfdRemoveGroup(pfGroup *oldn);
extern PFDUDLLEXPORT pfNode*	pfdFreezeTransforms(pfNode *node, pfuTravFuncType doitfunc);

	/*-------------------- Breakup Scene Graphs -----------------------------------*/

extern PFDUDLLEXPORT pfNode* 	pfdBreakup(pfGeode *geode, float geodeSize, int stripLength, int geodeChild);

	/*-------------------- Generate Hierarchies --------------------------------*/

extern PFDUDLLEXPORT pfList * pfdTravGetGSets(pfNode *node);

extern PFDUDLLEXPORT pfGroup*	pfdSpatialize(pfGroup *group, float maxGeodeSize, int maxGeoSets);

	/*---------------------- Share pfGeoStates -------------------------------*/

typedef struct _pfdShare	pfdShare;

extern PFDUDLLEXPORT pfdShare*	pfdNewShare(void);
extern PFDUDLLEXPORT pfdShare*	pfdGetGlobalShare(void);
extern PFDUDLLEXPORT void		pfdSetGlobalShare(pfdShare *share);
extern PFDUDLLEXPORT int		pfdCleanShare(pfdShare *_share);
extern PFDUDLLEXPORT void		pfdDelShare(pfdShare *_share, int _deepDelete);
extern PFDUDLLEXPORT void		pfdPrintShare(pfdShare *_share);
extern PFDUDLLEXPORT int		pfdCountShare(pfdShare *_share);
extern PFDUDLLEXPORT pfList*		pfdGetSharedList(pfdShare *_share, pfType* _type);
extern PFDUDLLEXPORT pfObject*        pfdNewSharedObject(pfdShare *_share, pfObject *_object);
extern PFDUDLLEXPORT pfObject*	pfdFindSharedObject(pfdShare *_share, pfObject *_object);
extern PFDUDLLEXPORT int		pfdAddSharedObject(pfdShare *_share, pfObject *_object);
extern PFDUDLLEXPORT void		pfdMakeShared(pfNode *_node);
extern PFDUDLLEXPORT void		pfdMakeSharedScene(pfScene *_scene);
extern PFDUDLLEXPORT int		pfdCleanShare(pfdShare *share);
extern PFDUDLLEXPORT int		pfdRemoveSharedObject(pfdShare *share, pfObject *object);
extern PFDUDLLEXPORT pfList*		pfdGetNodeGStateList(pfNode *node);


	/*----------------- Combine pfLayers --------------------------*/

extern PFDUDLLEXPORT void		pfdCombineLayers(pfNode *node);

	/*----------------- Combine pfBillboards -----------------------*/

extern PFDUDLLEXPORT void		pfdCombineBillboards(pfNode *node, int sizeLimit);

	/*-------------------- The Geometry Builder ---------------------------*/
	/*-- The state-ignorant geometry-only builder utility package --*/

typedef struct _pfdGeoBuilder   pfdGeoBuilder;

/* pfdPrim->flags */
#define PFD_CONCAVE		0x1
#define PFD_INDEXED		0x2

#ifdef __cplusplus
extern "C++" {  /* give structs containing Performer classes C++ linkage */
#endif

typedef struct _pfdPrim
{
    int    	flags;
    int    	nbind, cbind, tbind[PF_MAX_TEXTURES];

    float   	pixelsize;

    /* Non-indexed attributes - do not set if tri is indexed */
    pfVec3  	coords[3];
    pfVec3  	norms[3];
    pfVec4  	colors[3];
    pfVec2  	texCoords[PF_MAX_TEXTURES][3];

    /* Indexed attributes - do not set if tri is non-indexed */
    pfVec3  	*coordList;
    pfVec3  	*normList;
    pfVec4  	*colorList;
    pfVec2  	*texCoordList[PF_MAX_TEXTURES];

    /* Index lists - do not set if poly is non-indexed */
    ushort  	icoords[3];
    ushort  	inorms[3];
    ushort  	icolors[3];
    ushort  	itexCoords[PF_MAX_TEXTURES][3];

    int		numTextures;

    struct _pfdPrim	    *next;

} pfdPrim;

typedef struct _pfdGeom
{
    int    	flags;
    int    	nbind, cbind, tbind[PF_MAX_TEXTURES];

    int    	numVerts;
    short   	primtype;
    float   	pixelsize;

    /* Non-indexed attributes - do not set if poly is indexed */
    pfVec3  	*coords;
    pfVec3  	*norms;
    pfVec4  	*colors;
    pfVec2  	*texCoords[PF_MAX_TEXTURES];

    /* Indexed attributes - do not set if poly is non-indexed */
    pfVec3  	*coordList;
    pfVec3  	*normList;
    pfVec4  	*colorList;
    pfVec2  	*texCoordList[PF_MAX_TEXTURES];

    /* Index lists - do not set if poly is non-indexed */
    ushort  	*icoords;
    ushort  	*inorms;
    ushort  	*icolors;
    ushort  	*itexCoords[PF_MAX_TEXTURES];

    int		numTextures;

    struct _pfdGeom	    *next;

} pfdGeom;


#ifdef __cplusplus
}   /* End of C++ linkage */
#endif

typedef pfdGeom 	pfdPoly;
typedef pfdPrim 	pfdTri;

/* pfdGeoBldrMode() */
#define PFDGBLDR_AUTO_COLORS		PFDBLDR_AUTO_COLORS
#define PFDGBLDR_AUTO_NORMALS		PFDBLDR_AUTO_NORMALS
#define PFDGBLDR_AUTO_TEXTURE		PFDBLDR_AUTO_TEXTURE
#define PFDGBLDR_AUTO_ORIENT		PFDBLDR_AUTO_ORIENT
#define PFDGBLDR_MESH_ENABLE		PFDBLDR_MESH_ENABLE
#define PFDGBLDR_BUILD_LIMIT		PFDBLDR_BUILD_LIMIT
#define PFDGBLDR_SHARE_INDEX_LISTS	PFDBLDR_SHARE_INDEX_LISTS

/* PFDGBLDR_AUTO_COLORS modes */
#define PFDGBLDR_COLORS_PRESERVE 	PFDBLDR_COLORS_PRESERVE
#define PFDGBLDR_COLORS_MISSING		PFDBLDR_COLORS_MISSING
#define PFDGBLDR_COLORS_GENERATE	PFDBLDR_COLORS_GENERATE
#define PFDGBLDR_COLORS_DISCARD		PFDBLDR_COLORS_DISCARD

/* PFDGBLDR_AUTO_NORMALS modes */
#define PFDGBLDR_NORMALS_PRESERVE 	PFDBLDR_NORMALS_PRESERVE
#define PFDGBLDR_NORMALS_MISSING	PFDBLDR_NORMALS_MISSING
#define PFDGBLDR_NORMALS_GENERATE	PFDBLDR_NORMALS_GENERATE
#define PFDGBLDR_NORMALS_DISCARD	PFDBLDR_NORMALS_DISCARD

/* PFDGBLDR_AUTO_TEXTURE modes */
#define PFDGBLDR_TEXTURE_PRESERVE 	PFDBLDR_TEXTURE_PRESERVE
#define PFDGBLDR_TEXTURE_MISSING	PFDBLDR_TEXTURE_MISSING
#define PFDGBLDR_TEXTURE_GENERATE	PFDBLDR_TEXTURE_GENERATE
#define PFDGBLDR_TEXTURE_DISCARD	PFDBLDR_TEXTURE_DISCARD

/* PFDGBLDR_AUTO_ORIENT modes */
#define PFDGBLDR_ORIENT_PRESERVE 	PFDBLDR_ORIENT_PRESERVE
#define PFDGBLDR_ORIENT_NORMALS 	PFDBLDR_ORIENT_NORMALS
#define PFDGBLDR_ORIENT_VERTICES	PFDBLDR_ORIENT_VERTICES

/* this 256 is just something outside the PFDBLDR mode range */
#define PFDGBLDR_COLLAPSE_ATTRS		256

extern PFDUDLLEXPORT pfdGeom*      	pfdNewGeom(int numV);
extern PFDUDLLEXPORT void             pfdResizeGeom(pfdGeom *_geom, int numV);
extern PFDUDLLEXPORT void             pfdDelGeom(pfdGeom *_geom);
extern PFDUDLLEXPORT int		pfdReverseGeom(pfdGeom *_geom);
extern PFDUDLLEXPORT void		pfdCopyGeom(pfdGeom *dst, pfdGeom *src);

extern PFDUDLLEXPORT pfdGeoBuilder*   pfdNewGeoBldr(void);
extern PFDUDLLEXPORT void		pfdDelGeoBldr(pfdGeoBuilder* _bldr);
extern PFDUDLLEXPORT void		pfdGeoBldrMode(pfdGeoBuilder* _bldr, int mode, int val);
extern PFDUDLLEXPORT int		pfdGetGeoBldrMode(pfdGeoBuilder* _bldr, int mode);

extern PFDUDLLEXPORT int		pfdTriangulatePoly(pfdGeom *_pgon, pfdPrim *_triList);

extern PFDUDLLEXPORT void		pfdAddGeom(pfdGeoBuilder *_bldr, pfdGeom *_Geom, int num);

extern PFDUDLLEXPORT void		pfdAddLineStrips(pfdGeoBuilder *bldr, pfdGeom *lineStrips, int num);
extern PFDUDLLEXPORT void		pfdAddLines(pfdGeoBuilder *bldr, pfdGeom *lines);
extern PFDUDLLEXPORT void		pfdAddPoints(pfdGeoBuilder *bldr, pfdGeom *points);
extern PFDUDLLEXPORT void		pfdAddPoly(pfdGeoBuilder *_bldr, pfdGeom *_poly);

extern PFDUDLLEXPORT void		pfdAddIndexedLineStrips(pfdGeoBuilder *bldr, pfdGeom *lines, int num);
extern PFDUDLLEXPORT void		pfdAddIndexedLines(pfdGeoBuilder *bldr, pfdGeom *lines);
extern PFDUDLLEXPORT void		pfdAddIndexedPoints(pfdGeoBuilder *bldr, pfdGeom *points);
extern PFDUDLLEXPORT void		pfdAddIndexedPoly(pfdGeoBuilder *_bldr, pfdGeom *_poly);

extern PFDUDLLEXPORT void		pfdAddIndexedTri(pfdGeoBuilder *_bldr, pfdPrim *_tri);
extern PFDUDLLEXPORT void		pfdAddLine(pfdGeoBuilder *_bldr, pfdPrim *_line);
extern PFDUDLLEXPORT void		pfdAddPoint(pfdGeoBuilder *_bldr, pfdPrim *_Point);
extern PFDUDLLEXPORT void		pfdAddTri(pfdGeoBuilder *_bldr, pfdPrim *_tri);

extern PFDUDLLEXPORT int		pfdGetNumTris(pfdGeoBuilder *_bldr);
extern PFDUDLLEXPORT const pfList*	pfdBuildGSets(pfdGeoBuilder *_bldr);

extern PFDUDLLEXPORT void		pfdPrintGSet(pfGeoSet *gset);

	/*---------------------- The Scene Builder ---------------------------*/
	/*-- The state-savvy builder that's layered on pfdGeoBuilder --*/

typedef struct _pfdBuilder pfdBuilder;

/* Make extra state token exclusive from PFSTATE tokens */
#define PFDBLDR_START_NEW_STATES 0x80000000

/* pfdBldrMode() */
#define PFDBLDR_MESH_ENABLE		 	0
#define PFDBLDR_MESH_SHOW_TSTRIPS 	 	1
#define PFDBLDR_MESH_INDEXED 		 	2
#define PFDBLDR_MESH_MAX_TRIS 		 	3
#define PFDBLDR_MESH_RETESSELLATE 	 	4
#define PFDBLDR_MESH_LOCAL_LIGHTING	 	5
#define PFDBLDR_AUTO_COLORS 		 	10
#define PFDBLDR_AUTO_NORMALS 		 	11
#define PFDBLDR_AUTO_TEXTURE 		 	12
#define PFDBLDR_AUTO_ORIENT 		 	13
#define PFDBLDR_AUTO_ENABLES		 	14	
#define PFDBLDR_AUTO_DISABLE_TCOORDS_BY_STATE	15
#define PFDBLDR_AUTO_DISABLE_NCOORDS_BY_STATE	16
#define PFDBLDR_AUTO_LIGHTING_STATE_BY_NCOORDS 	17
#define PFDBLDR_AUTO_LIGHTING_STATE_BY_MATERIALS 18
#define PFDBLDR_AUTO_TEXTURE_STATE_BY_TEXTURES	19
#define PFDBLDR_AUTO_TEXTURE_STATE_BY_TCOORDS	20
#define PFDBLDR_AUTO_CMODE			21
#define PFDBLDR_BREAKUP 			30
#define PFDBLDR_BREAKUP_SIZE 			31
#define PFDBLDR_BREAKUP_BRANCH 			32
#define PFDBLDR_BREAKUP_STRIP_LENGTH 		33
#define PFDBLDR_SHARE_MASK 			34
#define PFDBLDR_ATTACH_NODE_NAMES 		35
#define PFDBLDR_DESTROY_DATA_UPON_BUILD		36
#define PFDBLDR_PF12_STATE_COMPATIBLE		37
#define PFDBLDR_BUILD_LIMIT			38
#define PFDBLDR_GEN_OPENGL_CLAMPED_TEXTURE_COORDS	39
#define PFDBLDR_SHARE_INDEX_LISTS		40
#define PFDBLDR_OPTIMIZE_COUNTS_NULL_ATTRS	50
#define PFDBLDR_PASS_REFERENCES			60

/* PFDBLDR_AUTO_COLORS modes */
#define PFDBLDR_COLORS_PRESERVE 	 0
#define PFDBLDR_COLORS_MISSING		 1
#define PFDBLDR_COLORS_GENERATE	 	 2
#define PFDBLDR_COLORS_DISCARD		 3

/* PFDBLDR_AUTO_NORMALS modes */
#define PFDBLDR_NORMALS_PRESERVE	 0
#define PFDBLDR_NORMALS_MISSING		 1
#define PFDBLDR_NORMALS_GENERATE	 2
#define PFDBLDR_NORMALS_DISCARD		 3

/* PFDBLDR_AUTO_TEXTURE modes */
#define PFDBLDR_TEXTURE_PRESERVE	 0
#define PFDBLDR_TEXTURE_MISSING		 1
#define PFDBLDR_TEXTURE_GENERATE	 2
#define PFDBLDR_TEXTURE_DISCARD		 3

/* PFDBLDR_AUTO_ORIENT modes */
#define PFDBLDR_ORIENT_PRESERVE		 0
#define PFDBLDR_ORIENT_VERTICES		 1
#define PFDBLDR_ORIENT_NORMALS 		 2

/* pfdBldrAttr() */
#define PFDBLDR_NODE_NAME_COMPARE 	 0
#define PFDBLDR_STATE_NAME_COMPARE 	 1
#define PFDBLDR_EXTENSOR_NAME_COMPARE 	 2

extern PFDUDLLEXPORT void               pfdInitBldr(void);
extern PFDUDLLEXPORT void               pfdExitBldr(void);
extern PFDUDLLEXPORT pfdBuilder *  	  pfdNewBldr(void);
extern PFDUDLLEXPORT void               pfdDelBldr(pfdBuilder *bldr);
extern PFDUDLLEXPORT void               pfdSelectBldr(pfdBuilder *bldr);
extern PFDUDLLEXPORT pfdBuilder *       pfdGetCurBldr(void);
extern PFDUDLLEXPORT void		  pfdBldrDeleteNode(pfNode *node);

extern PFDUDLLEXPORT void               pfdBldrMode(int mode, int val);
extern PFDUDLLEXPORT int                pfdGetBldrMode(int mode);
extern PFDUDLLEXPORT void               pfdBldrAttr(int which, void *attr);
extern PFDUDLLEXPORT void *             pfdGetBldrAttr(int which);

extern PFDUDLLEXPORT pfObject *         pfdGetTemplateObject(pfType *_type);
extern PFDUDLLEXPORT void		  pfdResetObject(pfObject *obj);
extern PFDUDLLEXPORT void		  pfdResetAllTemplateObjects(void);
extern PFDUDLLEXPORT void		  pfdMakeDefaultObject(pfObject *obj);
extern PFDUDLLEXPORT void               pfdResetBldrGeometry(void);
extern PFDUDLLEXPORT void               pfdResetBldrShare(void);
extern PFDUDLLEXPORT void		  pfdCleanBldrShare(void);
extern PFDUDLLEXPORT pfdShare *	  pfdGetBldrShare(void);
extern PFDUDLLEXPORT void		  pfdSetBldrShare(pfdShare *share);

extern PFDUDLLEXPORT void               pfdCaptureDefaultBldrState(void);
extern PFDUDLLEXPORT void               pfdResetBldrState(void);
extern PFDUDLLEXPORT void               pfdPushBldrState(void);
extern PFDUDLLEXPORT void               pfdPopBldrState(void);
extern PFDUDLLEXPORT void               pfdSaveBldrState(void *name);
extern PFDUDLLEXPORT void               pfdLoadBldrState(void *name);

extern PFDUDLLEXPORT void               pfdBldrGState(const pfGeoState *gstate);
extern PFDUDLLEXPORT const pfGeoState * pfdGetBldrGState(void);

extern PFDUDLLEXPORT void		  pfdBldrStateVal(uint64_t which, float val);
extern PFDUDLLEXPORT float		  pfdGetBldrStateVal(uint64_t which);
extern PFDUDLLEXPORT void               pfdBldrStateMode(uint64_t mode, int val);
extern PFDUDLLEXPORT int                pfdGetBldrStateMode(uint64_t mode);
extern PFDUDLLEXPORT void               pfdBldrStateAttr(uint64_t which, const void *attr);
extern PFDUDLLEXPORT const void *       pfdGetBldrStateAttr(uint64_t attr);
extern PFDUDLLEXPORT void               pfdBldrStateInherit(uint64_t mask);
extern PFDUDLLEXPORT uint64_t           pfdGetBldrStateInherit(void);

extern PFDUDLLEXPORT void               pfdSelectBldrName(void *name);
extern PFDUDLLEXPORT void *             pfdGetCurBldrName(void);

extern PFDUDLLEXPORT void               pfdAddBldrGeom(pfdGeom *p, int n);
extern PFDUDLLEXPORT void               pfdAddIndexedBldrGeom(pfdGeom *p, int n);

extern PFDUDLLEXPORT pfNode *           pfdBuild(void);
extern PFDUDLLEXPORT pfNode *           pfdBuildNode(void *name);

extern PFDUDLLEXPORT void 		  pfdDefaultGState(pfGeoState *def);
extern PFDUDLLEXPORT const pfGeoState*  pfdGetDefaultGState(void);
extern PFDUDLLEXPORT void	          pfdMakeSceneGState(pfGeoState *sceneGState, 
                                    pfList *gstateList, 
				    const pfGeoState *previousGlobalState);
extern PFDUDLLEXPORT void 	          pfdOptimizeGStateList(pfList *gstateList, 
                                    const pfGeoState *prevGlobalGState, 
				    const pfGeoState *newGlobalGState);

         /*----------------------- Clipmap and Icache Utilities ------------*/

extern PFDUDLLEXPORT pfImageCache *pfdLoadImageCache(const char *fileName, 
                                       pfTexture *texure,
                                       int level);

extern PFDUDLLEXPORT pfImageCache *pfdLoadImageCacheState(const char *fileName, 
					    pfTexture *texure,
					    int level,
					    pfuImgCacheConfig *state);

extern PFDUDLLEXPORT pfMPClipTexture *pfdLoadMPClipTexture(const char *fileName);
extern PFDUDLLEXPORT pfClipTexture *pfdLoadClipTexture(const char *fileName);

extern PFDUDLLEXPORT pfClipTexture *pfdLoadClipTextureState(const char *fileName,
					      pfuClipTexConfig *state);

extern PFDUDLLEXPORT int pfdImageCacheNodeUpdate(pfTraverser *trav, void *userData);
extern PFDUDLLEXPORT int pfdClipTextureNodeUpdate(pfTraverser *trav, void *userData);



	/*------------------------ Haeberli Font Extensions------------------*/

#define PFDFONT_TEXTURED	0
#define PFDFONT_OUTLINED	1
#define PFDFONT_FILLED		2
#define PFDFONT_EXTRUDED	3
#define PFDFONT_VECTOR		4

extern PFDUDLLEXPORT pfFont*		pfdLoadFont(const char *ftype, const char *name, int style);
extern PFDUDLLEXPORT pfFont*		pfdLoadFont_type1(const char *name, int style);

	/*------------------------ Texture Callbacks ------------------------*/

#define PFD_TEXGEN_OFF 		0
#define PFD_TEXGEN_LINEAR 	1
#define PFD_TEXGEN_CONTOUR 	2
#define PFD_TEXGEN_REFLECTIVE 	3

extern PFDUDLLEXPORT int pfdPreDrawTexgenExt(pfTraverser *trav, void *data);
extern PFDUDLLEXPORT int pfdPostDrawTexgenExt(pfTraverser *trav, void *data);
extern PFDUDLLEXPORT int pfdPreDrawReflMap(pfTraverser *trav, void *data);
extern PFDUDLLEXPORT int pfdPostDrawReflMap(pfTraverser *trav, void *data);
extern PFDUDLLEXPORT int pfdPreDrawContourMap(pfTraverser *trav, void *data);
extern PFDUDLLEXPORT int pfdPostDrawContourMap(pfTraverser *trav, void *data);
extern PFDUDLLEXPORT int pfdPreDrawLinearMap(pfTraverser *trav, void *data);
extern PFDUDLLEXPORT int pfdPostDrawLinearMap(pfTraverser *trav, void *data);
extern PFDUDLLEXPORT void pfdTexgenParams(const float *newParamsX, const float *newParamsY);

	/*------------------------ Function Extensors ------------------------*/

/* This structure fully defines a performer functionality extension */
typedef struct _pfdExtensorType
{
    void	(*initialize)(void *data);
    int		(*compare)(void *data1, void *data2);
    long	(*copy)(void *dst, void *src);
    void	(*deletor)(void *data);
    pfList*	callbacks;
    pfList*	shareList;
    long	share;
    int		token;
    long	dataSize;
    void*	name;

} pfdExtensorType;

/* Instance of an Extensor which has been defined using the above def.*/
typedef struct _pfdExtensor
{
    pfdExtensorType *extensorType;
    int 	token;
    void 	*data;
    int 	mode;

} pfdExtensor;

#define PFDEXT_APP_FUNC 	       	0
#define PFDEXT_CULL_PREFUNC 		1
#define PFDEXT_CULL_POSTFUNC 		2
#define PFDEXT_DRAW_PREFUNC 		3
#define PFDEXT_DRAW_POSTFUNC 		4
#define PFDEXT_GSTATE_PREFUNC 		5
#define PFDEXT_GSTATE_POSTFUNC 		6
#define PFDEXT_GSET_PREFUNC 		7
#define PFDEXT_GSET_POSTFUNC 		8

/* Define minimal Extensor as state in the builder */
/* Note a token will be created for you and passed back if the token */
/* arg is NULL */
extern PFDUDLLEXPORT int pfdAddState(void *name, long dataSize, void (*initialize)(void *data), void (*deletor)(void *data), int (*compare)(void *data1, void *data2), long (*copy)(void *dst, void *src), int token);

/* Extend Extensor Definition through callbacks listed above */
/* Specify function callbacks for an Extensor */
extern PFDUDLLEXPORT void pfdStateCallback(int stateToken, int whichCBack, pfNodeTravFuncType callback);
extern PFDUDLLEXPORT pfNodeTravFuncType pfdGetStateCallback(int stateToken, int which);

/* Look up the builder state token to use for a registered name */
extern PFDUDLLEXPORT int pfdGetStateToken(void *name);

/* Get the next Unique State token that can be used as a valid */
/* token for user state in the builder */
extern PFDUDLLEXPORT int pfdGetUniqueStateToken(void);

/* Create Generic Extensors and Extensor Definitions */
/* Note the builder creates these automatically based on user */
/* definition of extensors through pfdAddState and appropriately */
/* creates instances of an Extensor based on the current user state */
extern PFDUDLLEXPORT pfdExtensor* pfdNewExtensor(int which);
extern PFDUDLLEXPORT pfdExtensorType* pfdNewExtensorType(int token);

/* Needed to share Extensors and do internal extensor caching */
extern PFDUDLLEXPORT int pfdCompareExtensor(void *a, void *b);

/* Compare a list of Extensors */
extern PFDUDLLEXPORT int pfdCompareExtraStates(void *lista, void *listb);
extern PFDUDLLEXPORT void pfdCopyExtraStates(pfList *dst, pfList *src);

/* Find an instance of an Extensor in the Builder's current User State */
extern PFDUDLLEXPORT pfdExtensor* pfdGetExtensor(int token);

/* Find a Extensor Definitions in the Builder */
extern PFDUDLLEXPORT pfdExtensorType* pfdGetExtensorType(int token);

/* Share Arbitrary data */
extern PFDUDLLEXPORT void *pfdUniqifyData(pfList *dataList, const void *data, long dataSize, void *(*newData)(long), int (*compare)(void *,void *), long (*copy)(void *, void *), int *compareResult);


/* ----------------------- ASD terrain mesh constructor ----------*/

extern PFDUDLLEXPORT pfASD *pfdBuildASD(unsigned int numx, unsigned int numy, pfVec3 *data, int mode, int inputlods, int buildpaging, char *prename, char *confname, char *pagename, int *lookahead);
extern PFDUDLLEXPORT void pfdStoreASD(pfASD *asd, FILE *f);
extern PFDUDLLEXPORT void pfdBreakTiles(int numlods, pfASDLODRange *lods, int numverts, pfASDVert *verts, int numfaces, pfASDFace *faces, int numfaces0, char *prename, char *conf);
extern PFDUDLLEXPORT void pfdPagingLookahead(char *confname, char *pagename, int *lookahead);
extern PFDUDLLEXPORT void pfdWriteFile(int ***FACE, int **numf, int ***V, int **numv, pfASDFace *faces, pfASDVert *verts, pfBox *facebounds, int lod, int x, int y, char *prename);
extern PFDUDLLEXPORT pfASD * pfdLoadConfig(char *fname, char *pagename);

extern PFDUDLLEXPORT void pfdProcessASDTiles(char *fname, char *pagename);

/*---------------------- ASD vertex projection ---------------------------*/

extern PFDUDLLEXPORT void pfdAlignVerticesToASD(pfASD *asd, pfVec3 *_v, int _nofVertices, float *_base, float *_down, float *_azimuth, unsigned long _opcode, pfVec3 *_v_aligned);

extern PFDUDLLEXPORT void pfdProjectVerticesOnASD( pfASD *asd, pfVec3 *_v, int _nofVertices, float *_projection, float *_down, pfVec3 *_v_projected);

/*---------------------- ASD vertex projection ---------------------------*/

extern PFDUDLLEXPORT void pfdExtractGraphTriangles(pfNode *node, pfGeoSet *gset, unsigned long flags);

/*---------------------- ASD Clipring computation -----------------------*/
extern PFDUDLLEXPORT void pfdASDClipring(pfASD *asd, int numrings);
/* numrings is the number of additional rings inside the smallest LODRange */

/* ----------------------- pfi image file generation --------------------*/

typedef struct _pfdImage {
    int xsize;
    int ysize;
    int zsize;
    int num_comp;
    double ****image;
    int format;
    int texel_size;
    int alignment;
    int packed_size;
    unsigned char *packed;
    struct _pfdImage *mipmaps;
    struct _pfdImage *next;
} pfdImage;

extern PFDUDLLEXPORT pfdImage *pfdLoadImage(const char *_fileName);
extern PFDUDLLEXPORT pfdImage *pfdLoadImage_sgi(const char *_fileName);
extern PFDUDLLEXPORT pfdImage *pfdLoadImage_pfi(const char *_fileName);
extern PFDUDLLEXPORT void pfdDelImage(pfdImage *_image);
extern PFDUDLLEXPORT void pfdStoreImage(pfdImage *_image, const char *_fileName);
extern PFDUDLLEXPORT void pfdStoreImage_sgi(pfdImage *_image, const char *_fileName);
extern PFDUDLLEXPORT void pfdStoreImage_pfi(pfdImage *_image, const char *_fileName);
extern PFDUDLLEXPORT void pfdImagePack(pfdImage *_image, int format);
extern PFDUDLLEXPORT void pfdImageFromPfTexture(pfdImage *_image, pfTexture *pftex);
extern PFDUDLLEXPORT void pfdImageToPfTexture(pfdImage *_image, pfTexture *pftex);
extern PFDUDLLEXPORT void pfdImageGenMipmaps(pfdImage *_image,
			       int *ksize, double ***kernel, int *wrap);
extern PFDUDLLEXPORT void pfdImageDelMipmaps(pfdImage *_image);
extern PFDUDLLEXPORT void pfdImageAlignment(pfdImage *_image, int alignment);

/* ----------------------- ASDGen --------------------------------------*/

typedef struct _pfdASDGenerator pfdASDGenerator;
typedef float   (*pfd_grid_elevation_func)(void *user_data, int x, int y);


extern PFDUDLLEXPORT pfdASDGenerator	*pfdNewASDGen (void);

extern PFDUDLLEXPORT void 	pfdASDGenerate (
			pfdASDGenerator 	*gen);

extern PFDUDLLEXPORT void  	pfdASDGenPlaneGeometry (
                        pfdASDGenerator         *gen,
                        float                   x_base,
                        float                   y_base,
                        float                   x_span,
                        float                   y_span);

extern PFDUDLLEXPORT void  	pfdASDGenSphereGeometry (
                        pfdASDGenerator         *gen,
                        float                   radius,
                        PFVEC3                  center,
                        float                   x_base,
                        float                   y_base,
                        float                   x_span,
                        float                   y_span);

extern PFDUDLLEXPORT void  	pfdASDGenElevationFunc (
                        pfdASDGenerator         *gen,
                        pfd_grid_elevation_func func, 
                        void                    *user_data);

extern PFDUDLLEXPORT void  pfdASDGenPrune (
                        pfdASDGenerator         *gen,
                        int                     mode,
                        float                   epsilon);

extern PFDUDLLEXPORT void  pfdASDGenGridSize (
                        pfdASDGenerator         *gen,
                        int                     x_size,
                        int                     y_size);

extern PFDUDLLEXPORT void  pfdASDGenTileSize (
                        pfdASDGenerator         *gen,
                        int                     size);

extern PFDUDLLEXPORT void  pfdASDGenNofLevels (
                        pfdASDGenerator         *gen,
                        int                     n);

extern PFDUDLLEXPORT void  pfdASDGenTempFile (
                        pfdASDGenerator         *gen,
                        char                    *tmp);

extern PFDUDLLEXPORT void  pfdASDGenOutputFile (
                        pfdASDGenerator         *gen,
                        char                    *out);

extern PFDUDLLEXPORT void  pfdASDGenLookahead (
                        pfdASDGenerator         *gen,
                        int                     x,
                        int                     y);

extern PFDUDLLEXPORT	void  pfdASDGenAddSegment (
                        pfdASDGenerator         *gen,
                        PFVEC3                  v0,
                        PFVEC3                  v1);

extern PFDUDLLEXPORT	void  pfdASDGenAddTriangle (
                        pfdASDGenerator         *gen,
                        PFVEC3                  v0,
                        PFVEC3                  v1,
                        PFVEC3                  v2);

/*************************** Load and save isl appearances *************************/
extern PFDUDLLEXPORT islAppearance *pfdLoadAppearance(const char *filename);
extern PFDUDLLEXPORT int pfdStoreAppearance(const char *filename, islAppearance *app);
/* get the filename that belongs to an isl appearance */
extern PFDUDLLEXPORT char *pfdGetAppearanceFilename(islAppearance *app);


#ifdef __cplusplus
}
#endif

#endif /* __PFDU_H__ */
