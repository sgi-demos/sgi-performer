
// This file contains the public interface from the Performer
// header file Perormer/pfdu.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%section "libpfdu"


%{
#include <Performer/pfdu.h>
%}

/* read a file in any of the above 3D-formats */
extern pfNode* pfdLoadFile(const char *file);

/* read a file in any of the above 3D-formats */
extern int pfdStoreFile(pfNode *root, const char *file);

/* convert an in-memory representation to Performer */
extern pfNode* pfdConvertFrom(void *root, const char *ext);

/* convert a Performer subgraph to another in-memory respresentation */
extern void* pfdConvertTo(pfNode* root, const char *ext);

/* open DSO for the loader, useful before pfConfig() */
extern int pfdInitConverter(const char *_ext);

/* discard any database loaders that have been dynamically loaded */
extern int pfdExitConverter(const char *_ext);

/* locate and open a file using the IRIS Performer search path */
extern FILE* pfdOpenFile(const char *_file);

/* Add a file extension alias for db loaders to recognize */
extern void pfdAddExtAlias(const char *ext, const char *alias);

/* Set/Get the mode/attr/val of a particular loader */
extern void pfdConverterMode(const char *ext, int mode, int value);
extern int pfdGetConverterMode(const char *ext, int mode);
extern void pfdConverterAttr(const char *ext, int which, void *attr);
extern void* pfdGetConverterAttr(const char *ext, int which);
extern void pfdConverterVal(const char *ext, int which, float val);
extern float pfdGetConverterVal(const char *ext, int which);

/* user callback function registry */
extern int pfdRegisterUserFunc(void *func,
			       const char *name, const char *dso_name);
extern int pfdGetRegisteredUserFunc(void *func, char **name, char **dso_name);
extern int pfdIsRegisteredUserFunc(void *func);
extern void * pfdFindRegisteredUserFunc(char *name);

/* Simple summary statistics */
extern void pfdPrintSceneGraphStats(pfNode *node, double elapsedTime);

       /*------------------------ Haeberli Font Extensions------------------*/

#define PFDFONT_TEXTURED        0
#define PFDFONT_OUTLINED        1
#define PFDFONT_FILLED          2
#define PFDFONT_EXTRUDED        3
#define PFDFONT_VECTOR          4

extern pfFont*          pfdLoadFont(const char *ftype, const char *name, int style);
extern pfFont*          pfdLoadFont_type1(const char *name, int style);

	/*-------------------- Generate pfGeoSets ---------------------------*/

/* 3D primitives */
	/* Unit cube */
extern pfGeoSet * pfdNewCube(void *arena);

	/* Unit sphere */
extern pfGeoSet * pfdNewSphere(int ntris, void *arena);

	/* radius=1, from Z=-1 to Z=1 */
extern pfGeoSet * pfdNewCylinder(int ntris, void *arena);

	/* radius=1, from Z=0 to Z=1 */
extern pfGeoSet * pfdNewCone(int ntris, void *arena);

	/* CYLINDER without end caps and variable radii */
extern pfGeoSet * pfdNewPipe(float botRadius, float topRadius, int ntris, void *arena);

	/* Unit square base, from Z=0 to Z=1 */
extern pfGeoSet * pfdNewPyramid(void *arena);

	/* Z=0 to Z=1 */
extern pfGeoSet * pfdNewArrow(int ntris, void *arena);

	/* Z=-1 to Z=1 */
extern pfGeoSet * pfdNewDoubleArrow(int ntris, void *arena);

	/* Unit circle facing +Z, filled */
extern pfGeoSet * pfdNewCircle(int ntris, void *arena);

	/* Unit circle in Z=0 plane, lines */
extern pfGeoSet * pfdNewRing(int ntris, void *arena);


extern void pfdGSetColor(pfGeoSet *gset, float r, float g, float b, float a);

	/*-------------------- Mesh Triangles ---------------------------*/

#define PFDMESH_SHOW_TSTRIPS	0
#define PFDMESH_RETESSELLATE	1
#define PFDMESH_INDEXED		2
#define PFDMESH_MAX_TRIS	3
#define PFDMESH_LOCAL_LIGHTING	4

extern pfGeoSet* pfdMeshGSet(pfGeoSet *_gset);
extern void pfdMesherMode(int _mode, int _val);
extern int pfdGetMesherMode(int _mode);
extern void pfdShowStrips(pfGeoSet *gset);

	/*-------------------- Optimize Scene Graphs -----------------------------------*/

extern pfNode*	pfdCleanTree(pfNode *node, pfuTravFuncType doitfunc);
extern void	pfdReplaceNode(pfNode *oldn, pfNode *newn);
extern void	pfdInsertGroup(pfNode *oldn, pfGroup *grp);
extern void	pfdRemoveGroup(pfGroup *oldn);
extern pfNode*	pfdFreezeTransforms(pfNode *node, pfuTravFuncType doitfunc);

	/*-------------------- Breakup Scene Graphs -----------------------------------*/

extern pfNode* 	pfdBreakup(pfGeode *geode, float geodeSize, int stripLength, int geodeChild);

	/*-------------------- Generate Hierarchies --------------------------------*/

extern pfList * pfdTravGetGSets(pfNode *node);

extern pfGroup*	pfdSpatialize(pfGroup *group, float maxGeodeSize, int maxGeoSets);

	/*----------------- Combine pfLayers --------------------------*/

extern void		pfdCombineLayers(pfNode *node);

	/*----------------- Combine pfBillboards -----------------------*/

extern void		pfdCombineBillboards(pfNode *node, int sizeLimit);



