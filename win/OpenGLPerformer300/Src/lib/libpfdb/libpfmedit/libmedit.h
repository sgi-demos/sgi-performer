/*--------------------------Message start-------------------------------

This software is copyright (C) 1994-1995 by Medit Productions


This software is provided under the following terms, if you do not
agree to all of these terms, delete this software now. Any use of
this software indicates the acceptance of these terms by the person
making said usage ("The User").

1) Medit Productions provides this software "as is" and without
warranty of any kind. All consequences of the use of this software
fall completely on The User.

2) This software may be copied without restriction, provided that an
unaltered and clearly visible copy of this message is placed in any
copies of, or derivatives of, this software.


---------------------------Message end--------------------------------*/




#ifndef __MEDITLIB__
#define __MEDITLIB__

#ifdef __cplusplus
extern "C" {
#endif


/* nb. libmedit.h must be included after "pf.h" or this will fail... */
#ifdef __PF_H__

#ifdef WIN32
#include "pfmedit.h"

#define fabsf fabs
#else
#include <Performer/pfdb/pfmedit.h>
#endif

/************************************************************************
Definitions for the IRIS Performer loader...
************************************************************************/

pfNode *LoadMedit(char *filename, int flags);	/* Load a medit file	*/
void SaveMedit(pfNode *data, char *filename);	/* Save a medit file	*/

#endif







/*----------------------------------------------------------------------*/
/*									*/
/* The following are the data structure definitions for	libmedit.a	*/
/* If you are not going to use libmedit, you can safely ignore this.	*/
/*									*/
/*----------------------------------------------------------------------*/



/************************************************************************
Maximum length allowed for names of Objects/LODs/sub objects/...
.../branches/materials.....etc...etc...
************************************************************************/
#define M_MaxName 128

/************************************************************************
Storage structures
nb. Fields labelled "Internal use while reading/writing" can be used
as temporary storage in your software, but will be overwritten if
you load/save a file...
************************************************************************/
/* Material */
typedef struct mmaterial_struct {
struct mmaterial_struct	*Next;		/* Link to next material in file	*/
char			*Name;		/* Name of this material		*/
float			Specular[3];	/* Specular properties of material	*/
float			Diffuse[3];	/* Diffuse properties of material	*/
float			Ambient[3];	/* Ambient properties of material	*/
float			Emissive[3];	/* Emissive properties of material	*/
float			Shine;		/* Material shininess			*/
float			Alpha;		/* Material transparency		*/
int			Id;		/* Internal use while reading/writing	*/
int 			Used;		/* Internal use while reading/writing	*/
unsigned int		Pack;		/* Internal use while reading/writing	*/
} MMaterial;
typedef MMaterial *MMaterialPtr;



/* Texture (nb. Filters use the GL filter values as defined in <gl.h>) */
typedef struct mtexture_struct {
struct mtexture_struct	*Next;		/* Link to next texture			*/
char			*File;		/* Pointer to filename of texture	*/
float			MinFilter;	/* Minification filter (default=MT_NULL)*/
float			MagFilter;	/* Magnification filter (    "      "  )*/
int			ClampX;		/* Flag - Clamp texture coords in X	*/
int			ClampY;		/* Flag - Clamp texture coords in Y	*/
int			Fast;		/* Flag - Use fast internal format	*/
int			Id;		/* Internal use while reading/writing	*/
} MTexture;
typedef MTexture *MTexturePtr;

/* texture filter choices, these are the same as in gl.h, but not */
/* everybody has gl.h these days, so we reproduce them here */
#define MT_NULL			0x000
#define MT_POINT		0x110
#define MT_BILINEAR		0x220
#define MT_BICUBIC		0x230
#define MT_SHARPEN		0x240

#define MT_MIPMAP_POINT		0x121
#define MT_MIPMAP_LINEAR	0x122
#define MT_MIPMAP_BILINEAR	0x123
#define MT_MIPMAP_TRILINEAR	0x124



/* Polygon bead */
typedef struct mpolybead_struct {
struct mpolybead_struct	*Next;		/* Link to next bead in polygon		*/
float			Coordinate[3];	/* Vertex's coordinate			*/
float			Normal[3];	/* Vertex's normal			*/
float			TextureCoord[2];/* Vertex's texture coordinate		*/
} MPolygonBead;
typedef MPolygonBead *MPolygonBeadPtr;



/* Polygon */
typedef struct mpolygon_struct {
struct mpolygon_struct	*Next;		/* Link to next polygon			*/
struct mpolygon_struct	*Child;		/* Link to sub polygon			*/
float			Normal[3];	/* Polygon's normal if flat lit		*/
MPolygonBeadPtr		FirstBead;	/* Pointer to first vertex in polygon	*/
MMaterialPtr		NormalMaterial;	/* Polygon's material			*/
MMaterialPtr		TextureMaterial;/* Polygon's material when textured	*/
MTexturePtr		Texture;	/* Polygon's texture			*/
unsigned int		Colour;		/* Colour of polygon for cpack()	*/
int			Lighting;	/* Polygon's lighting (see below)	*/
int			Flags;		/* Special types of polygon (see below)	*/
int			LightGroup;	/* Grouping for normal calculations	*/
int			Flag;		/* Internal use while reading/writing	*/
} MPolygon;
typedef MPolygon *MPolygonPtr;

enum { Unlit=0, Lit, Smooth };		/* Different types of lighting		*/
/* Individual bits in the "Flags" above */
#define MEnvironmentMap 1



/* Sub object */
typedef struct msubobj_struct {
struct msubobj_struct	*Next;		/* Link to next sub object		*/
char			*Name;		/* Name of this sub object		*/
MPolygonPtr		FirstPolygon;	/* Pointer to first polygon in subobject*/
int			Id;		/* Internal use while reading/writing	*/
int			Added;		/* Used by Performer loader		*/
} MSubObject;
typedef MSubObject *MSubObjectPtr;



/* Lod bead */
typedef struct mlodbead_struct {
struct mlodbead_struct	*Next;		/* Pointer to next bead in Lod		*/
MSubObjectPtr		SubObject;	/* Pointer to subobject			*/
} MLodBead;
typedef MLodBead *MLodBeadPtr;



/* Lod */
typedef struct mlod_struct {
struct mlod_struct	*Next;		/* Pointer to next Lod			*/
char			*Name;		/* Name of Lod				*/
float			SwitchOut;	/* Switch out distance of this Lod	*/
MLodBeadPtr		FirstBead;	/* Pointer to first bead of this Lod	*/
} MLod;
typedef MLod *MLodPtr;



/* Object */
typedef struct mobject_struct {
struct mobject_struct	*Next;		/* Pointer to next object		*/
char			*Name;		/* Name of this object			*/
MLodPtr			FirstLod;	/* Pointer to the first Lod		*/
MSubObjectPtr		FirstSubObject;	/* Pointer to the first sub object	*/
int			Attributes;	/* Object attribute flags		*/
int			Id;		/* Internal use while reading/writing	*/
int			MatBase;	/* Internal use while reading/writing	*/
} MeditObject;
typedef MeditObject *MeditObjectPtr;

/* Flags inside the object 'Attributes' field, you should &AND& the field with	*/
/* these numbers to see if the attribute is set...				*/
/* eg. if (o->Attributes & ObjBillboard) { It's a billboard }			*/

#define ObjBillboard 1



/* Scene tree branch */
#define BR struct branch_struct
typedef struct branch_struct {
struct branch_struct	*Across;	/* Links the whole tree together	*/
struct branch_struct	*Down;
char			*Name;		/* Name of branch			*/
MeditObjectPtr		Obj;		/* Object referenced by this node	*/
float			Trans[4][4];	/* Transformation matrix for object	*/
} MBranch;
typedef MBranch *MBranchPtr;



/* Entire file info... */
typedef struct mfile_struct {
MMaterialPtr		FirstMaterial; 	/* First material in the file		*/
MTexturePtr		FirstTexture;	/* First texture in the file		*/
MeditObjectPtr		FirstObject;	/* Points to the first object		*/
MBranchPtr		Scene;		/* Points to scene tree			*/
} MeditFile;
typedef MeditFile *MeditFilePtr;


/************************************************************************
Functions/external vars
************************************************************************/

/* ---- Global vars */
extern MeditFilePtr CurrentMeditFile;	/* Current database				*/
extern int MeditFileHadNormals;		/* Flag - TRUE if last file read has normals	*/
extern char Medit_TextureDir[4096];	/* "Default texture dir" of last file loaded	*/
extern char Medit_FilePath[4096];	/* Path of file being written (during export)	*/
 
/* ---- Errors returned by read/write */
extern int MeditError;			/* TRUE if an error occurred			*/
extern int MeditErrorType;		/* Error type					*/
extern char *MeditErrorList[];		/* Ascii error messages for MeditErrorType	*/


/* ---- File functions */
boolean ReadMedit(char*FileName);
boolean WriteMedit(char*FileName);


/* ---- Data creation functions */
MeditFilePtr	NewMeditFile(void);

MMaterialPtr	Medit_AddMaterial(char *Name);
MTexturePtr	Medit_AddTexture(char *FileName);

MeditObjectPtr	Medit_AddObject(char *Name);
MSubObjectPtr	Medit_AddSubObject(char *Name);
MLodPtr		Medit_AddLod(char *Name);
MLodBeadPtr	Medit_AddLodBead(MSubObjectPtr s);
MPolygonPtr	Medit_AddPolygon(MPolygonBeadPtr beads, MPolygonPtr parent);
MPolygonBeadPtr	Medit_AddPolygonBead(MPolygonBeadPtr *base, float coord[3], float texture[2]);
void 		Medit_SetObject(MeditObjectPtr, MSubObjectPtr);

MBranchPtr	Medit_AddBranch(MBranchPtr base, int direction);
void		Medit_AttachObject(MBranchPtr base, MeditObjectPtr o, float trans[4][4]);
#define ACROSS 0	/* Possible directions for Medit_AddBranch */
#define DOWN 1

/* Free up memory used by a file */
void		Medit_FreeFile(MeditFilePtr);

/* Set this to FALSE and Medit will switch to "Scene" mode when you load the file*/
extern	int	ObjectIsolated;


/************************************************************************
These routines convert the coordinate system of a file between IRIS GL
"y-is-up" and Performer/Medit "z-is-up". Use as necessary. (Medit changed
coordinate systems after version 1.0,  so these routines are part of
the loader to ensure backwards compatibility, so we might as well let you
use them as well...)
************************************************************************/
/* Convert database to GL coordinate system after a ReadMedit() */
void Medit_ConvertToGLCoordinates(void);

/* Convert database from GL coordinate system before a WriteMedit() */
void Medit_ConvertFromGLCoordinates(void);

#ifdef __cplusplus
}
#endif

#endif

