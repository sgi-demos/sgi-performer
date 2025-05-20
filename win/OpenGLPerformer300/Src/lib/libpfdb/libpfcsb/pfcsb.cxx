/*
 * Copyright 1996, 1997 Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * This source code ("Source Code") was originally derived from a
 * code base owned by Silicon Graphics, Inc. ("SGI")
 * 
 * LICENSE: SGI grants the user ("Licensee") permission to reproduce,
 * distribute, and create derivative works from this Source Code,
 * provided that: (1) the user reproduces this entire notice within
 * both source and binary format redistributions and any accompanying
 * materials such as documentation in printed or electronic format;
 * (2) the Source Code is not to be used, or ported or modified for
 * use, except in conjunction with OpenGL Performer; and (3) the
 * names of Silicon Graphics, Inc.  and SGI may not be used in any
 * advertising or publicity relating to the Source Code without the
 * prior written permission of SGI.  No further license or permission
 * may be inferred or deemed or construed to exist with regard to the
 * Source Code or the code base of which it forms a part. All rights
 * not expressly granted are reserved.
 * 
 * This Source Code is provided to Licensee AS IS, without any
 * warranty of any kind, either express, implied, or statutory,
 * including, but not limited to, any warranty that the Source Code
 * will conform to specifications, any implied warranties of
 * merchantability, fitness for a particular purpose, and freedom
 * from infringement, and any warranty that the documentation will
 * conform to the program, or any warranty that the Source Code will
 * be error free.
 * 
 * IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
 * LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
 * ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
 * SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
 * OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
 * PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
 * OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
 * USE, THE SOURCE CODE.
 * 
 * Contact information:  Silicon Graphics, Inc., 
 * 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
 * or:  http://www.sgi.com
 */

/*
 *  pfCsb.c $Revision: 1.20 $
 */

#ifdef __linux__
int __cmpdi2;
#endif

#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfMaterial.h>

#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfBillboard.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfLOD.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfSwitch.h>
#include <Performer/pf/pfGeode.h>

#include <Performer/pfdu.h>

#include "pfcsb.h"

#if defined(__linux_has_ifl__) || defined(sgi)
#include <ifl/iflFile.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#ifdef sgi
#include <bstring.h>
#else
#include <string.h>
#endif
#ifndef WIN32
#include <inttypes.h>
#endif
#include <malloc.h>
#ifndef WIN32
#include <dlfcn.h>
#endif


// Magic numbers used in csb files.
#define CSB_MAGIC_NUMBER	0xdb0c3d00
#define CSB_MAGIC_NUMBER_E      0x003d0cdb	// opposite endian
#define CSB_MAGIC_ENCODED       0xdb0c3d01
#define CSB_MAGIC_ENCODED_E     0x013d0cdb	// opposite endian

// current version number
#define CURRENT_OPTIMIZER_VERSION "1.0"

// DEBUG DEFINES
//#define DEBUG_NODE
//#define PRINT_SCENE

// typedefs for sized integers and floats.
// It is important that the fields in a csb file are of a well known size.

#ifdef WIN32
typedef unsigned char uint8;
typedef short int16;
typedef __int64 int64;
#else
typedef int8_t		int8;
typedef uint8_t		uint8;
typedef int16_t		int16;
typedef uint16_t	uint16;
typedef int32_t		int32;
typedef uint32_t	uint32;
typedef int64_t		int64;
typedef uint64_t	uint64;
typedef intptr_t	intp;
typedef uintptr_t	uintp;
#endif
typedef float		float32;
typedef double		float64;

#ifndef sgi
typedef unsigned char uchar_t;
#endif

// typedef for ClearCoat facility
typedef void (*ClearCoatGenFunc)(uchar_t*, int, int, int, int, uchar_t*, float, float, float);


//--------------------------------------------------------------------------

// Structures, tables, and defines matching cosmo3d classes.

/*
 *  ---------- for csObject ----------
 */

#define OBJECT_DATA


/*
 *  ---------- for csContainer ----------
 */

#define CONTAINER_DATA	\
    OBJECT_DATA		\
    int32 name_size;	\
    int32 udata;

typedef struct {
    CONTAINER_DATA
} container_t;


/*
 *  ---------- for csNode ----------
 */

#define NODE_DATA	\
    CONTAINER_DATA	\
    int32 bound_mode;	\
    int32 sphere_bound;

static const int nbm_table[] =		/* node bound mode */
{2,
    PFBOUND_STATIC,
    PFBOUND_DYNAMIC,
};

typedef struct {
    NODE_DATA
} node_t;


/*
 *  ---------- for csGroup ----------
 */

#define GROUP_DATA	\
    NODE_DATA		\
    int32 num_children;

typedef struct {
    GROUP_DATA
} group_t;


/*
 *  ---------- for csTransform ----------
 */

#define TRANSFORM_DATA_894      \
    GROUP_DATA                  \
    csFloat mat[4][4];

typedef struct {
    TRANSFORM_DATA_894
} transform_894_t;

#define TRANSFORM_DATA          \
    GROUP_DATA                  \
    csFloat translation[3];     \
    csFloat rotation[4];        \
    csFloat scale[3];           \
    csFloat scaleOrientation[4];\
    csFloat center[3];          \

typedef struct {
    TRANSFORM_DATA
} transform_t;


/*
 *  ---------- for csSwitch ----------
 */

#define SWITCH_DATA	\
    GROUP_DATA		\
    int32 which_child;

#define CSB_SWITCH_NO_CHILDREN	-1
#define CSB_SWITCH_ALL_CHILDREN	-2

typedef struct {
    SWITCH_DATA
} switch_t;

/*
 *  ---------- for csLOD ----------
 */
#define CS_MAX_LOD_RANGES       (10)   // from csLOD.h

#define CSLOD_DATA	\
    SWITCH_DATA		\
    float32 center[3]; \
    float32 ranges[CS_MAX_LOD_RANGES+1]; \
    int32 numRanges; \
    int32 offset;

typedef struct {
    CSLOD_DATA
} cslod_t;

/*
 *  ---------- for csShape ----------
 */

#define SHAPE_DATA	\
    NODE_DATA		\
    int32 appearance;	\
    int32 num_geometry;

typedef struct {
    SHAPE_DATA
} shape_t;

/*
 *  ---------- for csBillboard ----------
 */

#define BILLBOARD_DATA_893      \
    GROUP_DATA                  \
    int num_position;           \
    int mode;                   \
    float32 axis[3];

#define BILLBOARD_DATA          \
    BILLBOARD_DATA_893          \
    int forward;                \
    int up;

static const int bbm_table[] =          /* billboard mode */
{3,
/*
    (int)csBillboard::AXIAL,
    (int)csBillboard::POINT_SCREEN,
    (int)csBillboard::POINT_OBJECT,
*/
};
static const int bbv_table[] =          /* billboard vector (forward & up) */
{6,
/*
    (int)csBillboard::POS_X_AXIS,
    (int)csBillboard::POS_Y_AXIS,
    (int)csBillboard::POS_Z_AXIS,
    (int)csBillboard::NEG_X_AXIS,
    (int)csBillboard::NEG_Y_AXIS,
    (int)csBillboard::NEG_Z_AXIS,
*/
};

typedef struct {
    BILLBOARD_DATA_893
} billboard_t_893;

typedef struct {
    BILLBOARD_DATA
} billboard_t;

/*
 *  ---------- for csEnvironment ----------
 */

#define ENVIRONMENT_DATA        \
    GROUP_DATA                  \
    int num_light;              \
    int fog;

typedef struct {
    ENVIRONMENT_DATA
} environment_t;


/*
 *  ---------- for csLight ----------
 */
#define LIGHT_DATA              \
    NODE_DATA                   \
    int on;                     \
    float32 intensity;          \
    float32 ambientIntensity;   \
    float32 color[3];

typedef struct {
    LIGHT_DATA
} light_t;

/*
 *  ---------- for csDirectionalLight ----------
 */

#define DIRECTIONAL_LIGHT_DATA  \
    LIGHT_DATA                  \
    float32 direction[3];

typedef struct {
    DIRECTIONAL_LIGHT_DATA
} directional_light_t;


/*
 *  ---------- for csPointLight ----------
 */

#define POINT_LIGHT_DATA        \
    LIGHT_DATA                  \
    float32 location[3];        \
    float32 radius;             \
    float32 attenuation[3];

typedef struct {
    POINT_LIGHT_DATA
} point_light_t;


/*
 *  ---------- for csSpotLight ----------
 */

#define SPOT_LIGHT_DATA         \
    POINT_LIGHT_DATA            \
    float32 direction[3];       \
    float32 exponent;           \
    float32 cutOffAngle;

typedef struct {
    SPOT_LIGHT_DATA
} spot_light_t;


/*
 *  ---------- for csFog ------------
 */

#define FOG_DATA                \
    NODE_DATA                   \
    int on;                     \
    int mode;                   \
    float32 density;            \
    float32 start;              \
    float32 end;                \
    float32 index;              \
    float32 color[4];

typedef struct {
    FOG_DATA
} fog_t;

static const int fgm_table[] =          /* fog mode */
{3,
/*
    (int)csFog::LINEAR_FOG,
    (int)csFog::EXP_FOG,
    (int)csFog::EXP2_FOG,
*/
};



/*
 *  ---------- for all csNode types ----------
 */
#define N_GROUP			0
#define N_SHAPE			1
#define N_SWITCH		2
#define N_TRANSFORM		3
#define N_LOD			4
#define N_BILLBOARD		5
#define N_ENVIRONMENT           6
#define N_DIRECTIONAL_LIGHT     7
#define N_POINT_LIGHT           8
#define N_SPOT_LIGHT            9
#define N_FOG                   10
#define N_NUMNODE_TYPE  	11

typedef union {
    node_t node;
    group_t group;
    transform_t transform;
    transform_894_t transform_894;
    switch_t swtch;
    shape_t shape;
    cslod_t cslod;
    billboard_t billboard;
    environment_t environment;
    light_t light;
    directional_light_t directional_light;
    point_light_t point_light;
    spot_light_t spot_light;
    fog_t fog;
} all_nodes_t;


/*
 *  ---------- for csGeometry ----------
 */

#define GEOMETRY_DATA	\
    CONTAINER_DATA	\
    int32 bound_mode;	\
    int32 box_bound;

static const int gbm_table[] =		/* geometry bound mode */
{2,
    PFBOUND_STATIC,
    PFBOUND_DYNAMIC,
};

typedef struct {
    int32 list_id;
    int32 list_item;
} geometry_ref_t;

typedef struct {
    GEOMETRY_DATA
} geometry_t;


/*
 *  ---------- for csBox ----------
 */

#define BOX_DATA	\
    GEOMETRY_DATA	\
    float32 size[3];	\
    float32 center[3];

typedef struct {
    BOX_DATA
} box_t;


/*
 *  ---------- for csCone ----------
 */

#define CONE_DATA		\
    GEOMETRY_DATA		\
    float32 bottom_radius;	\
    float32 height;		\
    int32 side;			\
    int32 bottom;		\
    float32 center[3];

typedef struct {
    CONE_DATA
} cone_t;


/*
 *  ---------- for csCylinder ----------
 */

#define CYLINDER_DATA	\
    GEOMETRY_DATA	\
    float32 radius;	\
    float32 height;	\
    int32 side;		\
    int32 top;		\
    int32 bottom;	\
    float32 center[3];

typedef struct {
    CYLINDER_DATA
} cylinder_t;


/*
 *  ---------- for csSphere ----------
 */

#define SPHERE_DATA	\
    GEOMETRY_DATA	\
    float32 radius;	\
    float32 center[3];

typedef struct {
    SPHERE_DATA
} sphere_t;


/*
 *  ---------- for csSprite ----------
 */

#define SPRITE_DATA		\
    GEOMETRY_DATA		\
    float32 bottom_left[3];	\
    float32 top_right[3];	\
    float32 position[3];	\
    float32 normal[3];		\
    float32 axis[3];		\
    int32 mode;

static const int32 spm_table[] =		/* sprite mode */
{3,
    PFBB_AXIAL_ROT,
    PFBB_POINT_ROT_EYE,
    PFBB_POINT_ROT_WORLD,
};

typedef struct {
    SPRITE_DATA
} sprite_t;


/*
 *  ---------- for csGeoSet ----------
 */

#define GEOSET_DATA		\
    GEOMETRY_DATA		\
    int32 prim_count;		\
    int32 normal_bind;		\
    int32 color_bind;		\
    int32 tex_coord_bind;	\
    int32 coords;		\
    int32 normals;		\
    int32 colors;		\
    int32 tex_coords;		\
    int32 coord_index;		\
    int32 normal_index;		\
    int32 color_index;		\
    int32 tex_coord_index;	\
    int32 cull_face;

static const int gnb_table[] =		/* geoset normal bind */
{4,
    PFGS_OFF,
    PFGS_OVERALL,
    PFGS_PER_PRIM,
    PFGS_PER_VERTEX, 
};

static const int gcb_table[] =		/* geoset color bind */
{4,
    PFGS_OFF,
    PFGS_OVERALL,
    PFGS_PER_PRIM,
    PFGS_PER_VERTEX,
};

static const int gtb_table[] =		/* geoset tex coord bind */
{2,
    PFGS_OFF,
    PFGS_PER_VERTEX,
};

static const int ccf_table[] =		/* context cull face */
{2,
    //(int)csContext::NO_CULL,
    //(int)csContext::FRONT_CULL,
    //(int)csContext::BACK_CULL,
    //(int)csContext::BOTH_CULL,
};

typedef struct {
    GEOSET_DATA
} geoset_t;


/*
 *  ---------- for csLineSet ----------
 */

#define LINESET_DATA		\
    GEOSET_DATA			\
    float32 width;

typedef struct {
    LINESET_DATA
} lineset_t;


/*
 *  ---------- for csLineStripSet ----------
 */

#define LINESTRIPSET_DATA	\
    GEOSET_DATA			\
    float32 width;		\
    int32 num_lengths;

typedef struct {
    LINESTRIPSET_DATA
} linestripset_t;


/*
 *  ---------- for csPointSet ----------
 */

#define POINTSET_DATA		\
    GEOSET_DATA			\
    float32 size;

typedef struct {
    POINTSET_DATA
} pointset_t;


/*
 *  ---------- for csPolySet ----------
 */

#define POLYSET_DATA		\
    GEOSET_DATA			\
    int32 num_lengths;

typedef struct {
    POLYSET_DATA
} polyset_t;


/*
 *  ---------- for csQuadSet ----------
 */

#define QUADSET_DATA		\
    GEOSET_DATA

typedef struct {
    QUADSET_DATA
} quadset_t;


/*
 *  ---------- for csTriSet ----------
 */

#define TRISET_DATA		\
    GEOSET_DATA

typedef struct {
    TRISET_DATA
} triset_t;


/*
 *  ---------- for csTriStripSet ----------
 */

#define TRISTRIPSET_DATA	\
    GEOSET_DATA			\
    int32 num_lengths;

typedef struct {
    TRISTRIPSET_DATA
} tristripset_t;


/*
 *  ---------- for csTriFanSet ----------
 */

#define TRIFANSET_DATA	\
    GEOSET_DATA			\
    int32 num_lengths;

typedef struct {
    TRIFANSET_DATA
} trifanset_t;


/*
 *  -- for csCoordSet, csColorSet, csNormalSet, csTexCoordSet, and csIndexSet -
 */

#define ARRAYSET_DATA		\
    CONTAINER_DATA		\
    int32 array_type;		\
    int32 size;			\
    int32 count;

#define ARRAY_TYPE_UNKNOWN	-1
#define ARRAY_TYPE_1F		 0
#define ARRAY_TYPE_2F		 1
#define ARRAY_TYPE_3F		 2
#define ARRAY_TYPE_4F		 3
#define ARRAY_TYPE_1I		 4
#define ARRAY_TYPE_2I		 5
#define ARRAY_TYPE_3I		 6
#define ARRAY_TYPE_4I		 7
#define ARRAY_TYPE_1S		 9
#define ARRAY_TYPE_2S		10
#define ARRAY_TYPE_3S		11
#define ARRAY_TYPE_4S		12

typedef struct {
    ARRAYSET_DATA
} arrayset_t;


/*
 *  ---------- for csAppearance ----------
 */

// csAppearance data stored in CSB prior to version -995
#define APPEARANCE_DATA_A_32    \
    CONTAINER_DATA              \
    unsigned int inherit;

#define APPEARANCE_DATA_A_64    \
    CONTAINER_DATA              \
    int64 inherit;

#define APPEARANCE_DATA_B       \
    int tex;                    \
    int tex_enable;             \
    int tex_mode;               \
    float32 tex_bcolor[4];      \
    int tenv;                   \
    int tgen;                   \
    int tgen_enable;            \
    int mtl;                    \
    int light_enable;           \
    int shade_model;            \
    int transp_enable;          \
    int transp_mode;            \
    int alpha_func;             \
    float32 alpha_ref;          \
    float32 bcolor[4];          \
    int src_bfunc;              \
    int dst_bfunc;              \

#define APPEARANCE_DATA_C_1     \
    uint8 color_mask[4];        

#define APPEARANCE_DATA_C_2     \
    unsigned int color_mask[4]; 

#define APPEARANCE_DATA_D       \
    int depth_func;             \
    unsigned int depth_mask;    \
    int fog_enable;             \
    int poly_mode;

#define APPEARANCE_DATA_E       \
    int line_stipple_pattern;   \
    int line_stipple_factor;    \
    float32 tex_transform[16];  \
    int back_mtl;

#define APPEARANCE_DATA_F	\
    int     depth_enable;       \

// csAppearance data added with versions -895 and -894
#define APPEARANCE_DATA_G	\
    int     fog_mode;           \
    float32 fog_density;        \
    float32 fog_start;          \
    float32 fog_end;            \
    float32 fog_index;          \
    float32 fog_color[4];

// csAppearance data added with version -893
#define APPEARANCE_DATA_H       \
    int     material_mode_enable;

// csAppearance data stored in CSB prior to version -995
#define APPEARANCE_DATA_996     \
    APPEARANCE_DATA_A_32        \
    APPEARANCE_DATA_B           \
    APPEARANCE_DATA_C_1         \
    APPEARANCE_DATA_D

// csAppearance data stored in CSB prior to version -990
#define APPEARANCE_DATA_991     \
    APPEARANCE_DATA_A_32        \
    APPEARANCE_DATA_B           \
    APPEARANCE_DATA_C_2         \
    APPEARANCE_DATA_D

// csAppearance data stored in CSB prior to version -895
#define APPEARANCE_DATA_896     \
    APPEARANCE_DATA_991         \
    APPEARANCE_DATA_E

// csAppearance data stored in CSB prior to version -894
#define APPEARANCE_DATA_895     \
    APPEARANCE_DATA_896         \
    APPEARANCE_DATA_F

// csAppearance data stored in CSB prior to version -893
#define APPEARANCE_DATA_894     \
    APPEARANCE_DATA_A_64        \
    APPEARANCE_DATA_B           \
    APPEARANCE_DATA_C_2         \
    APPEARANCE_DATA_D           \
    APPEARANCE_DATA_E           \
    APPEARANCE_DATA_F		\
    APPEARANCE_DATA_G

// csAppearance data stored in CSB prior to version -890
#define APPEARANCE_DATA_893     \
    APPEARANCE_DATA_894         \
    APPEARANCE_DATA_H

// current csAppearance data stored in CSB
// note fog data is removed (appearance_data_g)
#define APPEARANCE_DATA         \
    APPEARANCE_DATA_A_64        \
    APPEARANCE_DATA_B           \
    APPEARANCE_DATA_C_2         \
    APPEARANCE_DATA_D           \
    APPEARANCE_DATA_E           \
    APPEARANCE_DATA_F		\
    APPEARANCE_DATA_H


static const int ctxm_table[] =		/* context.tex_mode */
{4,
    //(int)csContext::FAST_TEX,
    //(int)csContext::NICE_TEX,
    //(int)csContext::NON_PERSP_TEX,
    //(int)csContext::PERSP_TEX,
};

static const int cte_table[] =		/* context.tenv */
{5,
    PFTE_MODULATE,
    PFTE_BLEND,
    PFTE_REPLACE,
    PFTE_ADD,
    PFTE_DECAL,
};

static const int csm_table[] =		/* context.shade_model */
{2,
    PFSM_FLAT,
    PFSM_GOURAUD,
};

static const int ctrm_table[] =		/* context.transp_mode */
{4,
    PFTR_FAST,                  // csContext::FAST_TRANSP
    PFTR_HIGH_QUALITY,          // csContext::NICE_TRANSP
    PFTR_BLEND_ALPHA,           // csContext::BLEND_TRANSP
    PFTR_FAST,                  // csContext::SCREEN_DOOR_TRANSP
};

static const int caf_table[] =		/* context.alpha_func */
{8,
    PFAF_NEVER,
    PFAF_LESS,
    PFAF_EQUAL,
    PFAF_LEQUAL,
    PFAF_GREATER,
    PFAF_NOTEQUAL,
    PFAF_GEQUAL,
    PFAF_ALWAYS,
};


static const int csbf_table[] =		/* context.src_bfunc */
{9,
    //(int)csContext::ZERO_SBLEND,
    //(int)csContext::ONE_SBLEND,
    //(int)csContext::DST_COLOR_SBLEND,
    //(int)csContext::ONE_MINUS_DST_COLOR_SBLEND,
    //(int)csContext::SRC_ALPHA_SATURATE_SBLEND,
    //(int)csContext::SRC_ALPHA_SBLEND,
    //(int)csContext::ONE_MINUS_SRC_ALPHA_SBLEND,
    //(int)csContext::DST_ALPHA_SBLEND,
    //(int)csContext::ONE_MINUS_DST_ALPHA_SBLEND,
};

static const int cdbf_table[] =		/* context.dst_bfunc */
{8,
    //(int)csContext::ZERO_DBLEND,
    //(int)csContext::ONE_DBLEND,
    //(int)csContext::SRC_COLOR_DBLEND,
    //(int)csContext::ONE_MINUS_SRC_COLOR_DBLEND,
    //(int)csContext::SRC_ALPHA_DBLEND,
    //(int)csContext::ONE_MINUS_SRC_ALPHA_DBLEND,
    //(int)csContext::DST_ALPHA_DBLEND,
    //(int)csContext::ONE_MINUS_DST_ALPHA_DBLEND,
};

static const int cdf_table[] =		/* context.depth_func */
{8,
    //(int)csContext::NEVER_DFUNC,
    //(int)csContext::LESS_DFUNC,
    //(int)csContext::EQUAL_DFUNC,
    //(int)csContext::LEQUAL_DFUNC,
    //(int)csContext::GREATER_DFUNC,
    //(int)csContext::NOTEQUAL_DFUNC,
    //(int)csContext::GEQUAL_DFUNC,
    //(int)csContext::ALWAYS_DFUNC,
};

static const int cpm_table[] =		/* context.poly_mode */
{3,
    PFGS_POINTS,                   //POINT_PMODE,
    PFSTATE_ENWIREFRAME,           // Line mode
    PF_OFF,                              //(int)csContext::FILL_PMODE,
};

static const int cmm_table[] =          /* context.material_mode */
{2,
    //(int)csContext::NO_COLOR_MATERIAL,
    //(int)csContext::COLOR_MATERIAL,
};

#define NO_COLOR_MATERIAL 	1
#define COLOR_MATERIAL 		2


typedef struct {
    APPEARANCE_DATA
} appearance_t;

typedef struct {
    APPEARANCE_DATA_893
} appearance893_t;

typedef struct {
    APPEARANCE_DATA_894
} appearance894_t;

typedef struct {
    APPEARANCE_DATA_895
} appearance895_t;

typedef struct {
    APPEARANCE_DATA_896
} appearance896_t;

typedef struct {
    APPEARANCE_DATA_991
} appearance991_t;

typedef struct {
    APPEARANCE_DATA_996
} appearance996_t;


/*
 *  ---------- for csMaterial ----------
 */
#define MATERIAL_DATA		\
    CONTAINER_DATA		\
    float32 ambient[3];		\
    float32 diffuse[3];		\
    float32 specular[3];	\
    float32 emissive[3];	\
    float32 shininess;		\
    float32 transparency;	\
    int32 ambient_index;	\
    int32 diffuse_index;	\
    int32 specular_index;

typedef struct {
    MATERIAL_DATA
} material_t;


/*
 *  ---------- for csTexture ----------
 */
#define TEXTURE_DATA		\
    CONTAINER_DATA		\
    int32 file_name_size;	\
    int32 format;		\
    int32 repeat_s;		\
    int32 repeat_t;		\
    int32 min_filter;		\
    int32 mag_filter;

static const int32 txmnf_table[] =		/* texture.min_filter */
{3,
    PFTEX_POINT,
    PFTEX_LINEAR,
    PFTEX_MIPMAP,
};

static const int32 txmgf_table[] =		/* texture.mag_filter */
{2,
    PFTEX_POINT,
    PFTEX_LINEAR,
};

static const int32 txr_table[] =		/* texture.repeat */
{2,
    PFTEX_CLAMP,
    PFTEX_REPEAT,
};


static const int32 txf_table[] =		/* texture.format */
{45,
    PFTEX_LUMINANCE,         //  1(int32)csTexture::LUMINANCE,
    PFTEX_LUMINANCE_ALPHA,   //  2(int32)csTexture::LUMINANCE_ALPHA,
    PFTEX_RGB_5,             //  3(int32)csTexture::RGB,
    PFTEX_RGBA_4,            //  4(int32)csTexture::RGBA
    PFTEX_PACK_8,             // 5(int32)csTexture::TEXEL8,         /* PFTEX_PACK8 */
    PFTEX_PACK_16,            // 6(int32)csTexture::TEXEL16,
    PFTEX_UNSIGNED_INT,      // 7(int32)csTexture::TEXEL32,
    PFTEX_LUMINANCE,         // 8(int32)csTexture::LUMINANCE,	// PFTEX/* COLOR_INDEX, */
    PFTEX_LUMINANCE,         // 9(int32)csTexture::LUMINANCE,	/* COLOR_INDEX4, */
    PFTEX_LUMINANCE,         // 10(int32)csTexture::LUMINANCE,	/* COLOR_INDEX8, */
    PFTEX_LUMINANCE,         // 11(int32)csTexture::LUMINANCE,	/* COLOR_INDEX12, */
    PFTEX_LUMINANCE,         // 12(int32)csTexture::LUMINANCE,	/* COLOR_INDEX16, */
#ifdef OPENGL_EXTENSIONS
    GL_ALPHA4_EXT,            // 13(int32)csTexture::ALPHA4,
    GL_ALPHA8_EXT,            // 14(int32)csTexture::ALPHA8,
    GL_ALPHA12_EXT,           // 15(int32)csTexture::ALPHA12,
    GL_ALPHA16_EXT,           // 16(int32)csTexture::ALPHA16,
    GL_LUMINANCE4_EXT,               // 17 (int32)csTexture::LUMINANCE4,
    PFTEX_I_8,               // 18(int32)csTexture::LUMINANCE8,             /* PFTEX_I_8 */
    GL_LUMINANCE12_EXT,              //19(int32)csTexture::LUMINANCE12,
    PFTEX_I_16,              //20(int32)csTexture::LUMINANCE16,            /* PFTEX_I_16 */
    GL_LUMINANCE4_ALPHA4_EXT,    //21(int32)csTexture::LUMINANCE4_ALPHA4,
    GL_LUMINANCE6_ALPHA2_EXT,    //22(int32)csTexture::LUMINANCE6_ALPHA2,
    PFTEX_IA_8,              //23(int32)csTexture::LUMINANCE8_ALPHA8,      /* PFTEX_IA_8 */
    PFTEX_I_12A_4,           //24(int32)csTexture::LUMINANCE12_ALPHA4,     /* PFTEX_I_12A_4 */
    PFTEX_IA_12,             //25(int32)csTexture::LUMINANCE12_ALPHA12,    /* PFTEX_IA_12 */
    GL_LUMINANCE16_ALPHA16_EXT,   //26(int32)csTexture::LUMINANCE16_ALPHA16,
    PFTEX_I_8,               //27(int32)csTexture::INTENSITY,
    GL_INTENSITY4_EXT,               //28(int32)csTexture::INTENSITY4,
    PFTEX_I_8,               //29(int32)csTexture::INTENSITY8,
    GL_INTENSITY12_EXT,              //30(int32)csTexture::INTENSITY12,
    PFTEX_I_16,              //31(int32)csTexture::INTENSITY16,
    GL_RGB2_EXT,               //32(int32)csTexture::RGB2,
    PFTEX_RGB_4,               //33(int32)csTexture::RGB4,             /* PFTEX_RGB_4 */
    PFTEX_RGB_5,               //34(int32)csTexture::RGB5,             /* PFTEX_RGB_5 */
    PFTEX_RGB_8,               //35(int32)csTexture::RGB8,
    GL_RGB10_EXT,              //36(int32)csTexture::RGB10,
    PFTEX_RGB_12,              //37(int32)csTexture::RGB12,            /* PFTEX_RGB_12 */
    GL_RGB16_EXT,              //38(int32)csTexture::RGB16,
    GL_RGBA2_EXT,              //39(int32)csTexture::RGBA2,
    PFTEX_RGBA_4,              //40(int32)csTexture::RGBA4,           /* PFTEX_RGBA_4 */
    PFTEX_RGB5_A1,             //41(int32)csTexture::RGB5_A1,         /* PFTEX_RGB5_A1 */
    PFTEX_RGBA_8,              //42(int32)csTexture::RGBA8,           /* PFTEX_RGBA_8 */
    GL_RGB10_A2_EXT,           //43(int32)csTexture::RGB10_A2,
    PFTEX_RGBA_12,             //44(int32)csTexture::RGBA12,
    GL_RGBA16_EXT,             //45(int32)csTexture::RGBA16,
#else
    PFTEX_LUMINANCE,          //(int32)csTexture::LUMINANCE,    /* ALPHA4, */
    PFTEX_LUMINANCE,          //(int32)csTexture::LUMINANCE,	/* ALPHA8, */
    PFTEX_LUMINANCE,          //(int32)csTexture::LUMINANCE,	/* ALPHA12, */
    PFTEX_LUMINANCE,          //(int32)csTexture::LUMINANCE,	/* ALPHA16, */
    PFTEX_LUMINANCE,          //(int32)csTexture::LUMINANCE,	/* LUMINANCE4, */
    PFTEX_LUMINANCE,          //(int32)csTexture::LUMINANCE,	/* LUMINANCE8, */
    PFTEX_LUMINANCE,          //(int32)csTexture::LUMINANCE,	/* LUMINANCE12, */
    PFTEX_LUMINANCE,          //(int32)csTexture::LUMINANCE,	/* LUMINANCE16, */
    PFTEX_LUMINANCE_ALPHA,    //(int32)csTexture::LUMINANCE_ALPHA,	/* LUMINANCE4_ALPHA4, */
    PFTEX_LUMINANCE_ALPHA,    //(int32)csTexture::LUMINANCE_ALPHA,	/* LUMINANCE6_ALPHA2, */
    PFTEX_LUMINANCE_ALPHA,    //(int32)csTexture::LUMINANCE_ALPHA,	/* LUMINANCE8_ALPHA8, */
    PFTEX_LUMINANCE_ALPHA,    //(int32)csTexture::LUMINANCE_ALPHA,	/* LUMINANCE12_ALPHA4, */
    PFTEX_LUMINANCE_ALPHA,    //(int32)csTexture::LUMINANCE_ALPHA,	/* LUMINANCE12_ALPHA12, */
    PFTEX_LUMINANCE_ALPHA,    //(int32)csTexture::LUMINANCE_ALPHA,	/* LUMINANCE16_ALPHA16, */
    PFTEX_LUMINANCE,          //(int32)csTexture::LUMINANCE,	/* INTENSITY, */
    PFTEX_LUMINANCE,          //(int32)csTexture::LUMINANCE,	/* INTENSITY4, */
    PFTEX_LUMINANCE,          //(int32)csTexture::LUMINANCE,	/* INTENSITY8, */
    PFTEX_LUMINANCE,          //(int32)csTexture::LUMINANCE,	/* INTENSITY12, */
    PFTEX_LUMINANCE,          //(int32)csTexture::LUMINANCE,	/* INTENSITY16, */
    PFTEX_RGB,                //(int32)csTexture::RGB,		/* RGB2, */
    PFTEX_RGB,                //(int32)csTexture::RGB,		/* RGB4, */
    PFTEX_RGB,                //(int32)csTexture::RGB,		/* RGB5, */
    PFTEX_RGB,                //(int32)csTexture::RGB,		/* RGB8, */
    PFTEX_RGB,                //(int32)csTexture::RGB,		/* RGB10, */
    PFTEX_RGB,                //(int32)csTexture::RGB,		/* RGB12, */
    PFTEX_RGB,                //(int32)csTexture::RGB,		/* RGB16, */
    PFTEX_RGBA,               //(int32)csTexture::RGBA,		/* RGBA2, */
    PFTEX_RGBA,               //(int32)csTexture::RGBA,		/* RGBA4, */
    PFTEX_RGBA,               //(int32)csTexture::RGBA,		/* RGB5_A1, */
    PFTEX_RGBA,               //(int32)csTexture::RGBA,		/* RGBA8, */
    PFTEX_RGBA,               //(int32)csTexture::RGBA,		/* RGB10_A2, */
    PFTEX_RGBA,               //(int32)csTexture::RGBA,		/* RGBA12, */
    PFTEX_RGBA,               //(int32)csTexture::RGBA,		/* RGBA16, */
#endif
};

typedef struct {
    TEXTURE_DATA
} texture_t;


/*
 *  ---------- for csTexGen ----------
 */
#define TEXGEN_DATA		\
    CONTAINER_DATA		\
    int32 mode_s;		\
    int32 mode_t;		\
    int32 mode_r;		\
    int32 mode_q;		\
    float32 plane_s[4];		\
    float32 plane_t[4];		\
    float32 plane_r[4];		\
    float32 plane_q[4];

static const int32 tgm_table[] =		/* texgen.mode */
{5,
    PFTG_OFF,
    PFTG_EYE_LINEAR,
    PFTG_EYE_LINEAR_IDENT,
    PFTG_OBJECT_LINEAR,
    PFTG_SPHERE_MAP,
};

typedef struct {
    TEXGEN_DATA
} texgen_t;


/*
 *  ---------- for csEngine ----------
 */

typedef struct {
    int list_id;
    int list_item;
} engine_ref_t;


/*
 *  ------ for csInterpolator -------
 */

#define INTERPOLATOR_DATA       \
    CONTAINER_DATA              \
    float32 fraction;           \
    int num_keys;               \
    int num_keyValues;


typedef struct {
    INTERPOLATOR_DATA
} interpolator_t;


/*
 *  ---- for csMorphEng -----
 */

#define MORPHENG_DATA           \
    CONTAINER_DATA              \
    int num_counts;             \
    int num_indices;            \
    int num_weights;            \
    int num_inputs;

typedef struct {
    MORPHENG_DATA
} morpheng_t;

/*
 *  ---- for csSelectorEng -----
 */

#define SELECTENG_DATA          \
    CONTAINER_DATA              \
    int selector;               \
    int num_inputs;

typedef struct {
    SELECTENG_DATA
} selecteng_t;


/*
 *  ---- for csTransformEng -----
 */

#define TRANSENG_DATA           \
    CONTAINER_DATA              \
    int transform_type; \
    int num_counts;             \
    int num_indices;            \
    int num_matrices;           \
    int num_inputs;

typedef struct {
    TRANSENG_DATA
} transeng_t;

/*
static const int tet_table[] =         // transformeng.transform_type 
{2,
    (int)csTransformEng::POINT,
    (int)csTransformEng::VECTOR,
};
*/


/*
 *  -------- for csSpline ---------
 */

#define SPLINE_DATA             \
    CONTAINER_DATA              \
    float32 fraction;           \
    float32 basis[16];          \
    int num_keys;

typedef struct {
    SPLINE_DATA
} spline_t;


/*
 *  -------- for csTimeSensor ---------
 */

#define TIMESENSOR_DATA         \
    CONTAINER_DATA              \
    float64 cycle_interval;     \
    int enabled;                \
    int loop;                   \
    float64 start_time;         \
    float64 stop_time;

typedef struct {
    TIMESENSOR_DATA
} timesensor_t;


/*
 *  Maps Container to list/index
 */
typedef struct {
    int32	which_list;
    int32	index;
} ctr_ref_t;


/*
 * Field connection record
 */
typedef struct {
    ctr_ref_t	*from_ctr;
    int16	from_field;
    void	*to_ctr;   // csContainer	*to_ctr; to void AS
    int16	to_field;
} connection_t;


/*
 *  lists
 */
#define L_MTL		 0
#define L_TEX		 1
#define L_TEXGEN	 2
#define L_APP		 3
#define L_VSET		 4
#define L_CSET		 5
#define L_NSET		 6
#define L_TSET		 7
#define L_ISET		 8
#define L_LINESET	 9
#define L_LINESTRIPSET	10
#define L_POINTSET	11
#define L_POLYSET	12
#define L_QUADSET	13
#define L_TRISET	14
#define L_TRISTRIPSET	15
#define L_GEOM		16
#define L_NODE		17
#define L_BOX		18
#define L_CONE		19
#define L_CYLINDER	20
#define L_SPHERE	21
#define L_SPRITE	22
#define L_TRIFANSET	23
#define L_TRANSFORM	24
#define L_CUSTOM	25	// For custom classes derived from csNode
#define L_CONNECTIONS	26	// List of connections between fields
#define L_ENGINE        27
#define L_NORMINTERP    28
#define L_ORIENTINTERP  29
#define L_POSINTERP     30
#define L_SCALARINTERP  31
#define L_COLORINTERP   32
#define L_COORDINTERP   33
#define L_MORPHENG3F    34
#define L_MORPHENG4F    35
#define L_SELECTENG3F   36
#define L_SELECTENG4F   37
#define L_TRANSENG3F    38
#define L_SPLINE        39
#define L_TIMESENSOR    40
// L_COUNT == 41 in csCsb.h
#define L_EOF		-1


// class local to file

#define CSD_DUMMY_TYPE 	(csType *)-1
#define CSD_CUSTOMNAME	 32


class csdCustomObject
{
   public:
      csdCustomObject(pfType *_type, const char *_name,
                        csdDescendCustomObjFuncType_csb _descend_func,
                        csdStoreCustomObjFuncType_csb _store_func,
                        csdNewCustomObjFuncType_csb _new_func,
                        csdLoadCustomObjFuncType_csb _load_func);
      ~csdCustomObject(void) {};
      void addCustomObject(csdCustomObject *);
      csdCustomObject *findCustomObject(const char *name);
      csdCustomObject *findCustomObject(pfType *type);
      csdDescendCustomObjFuncType_csb descend_func;
      csdStoreCustomObjFuncType_csb store_func;
      csdNewCustomObjFuncType_csb new_func;
      csdLoadCustomObjFuncType_csb load_func;
      pfType *getType(void);

   private:
      pfType *type;
      char name[CSD_CUSTOMNAME];
      csdCustomObject *next;   // For linked list, later use binary tree
};

// GLOBALS

static csdCustomObject  *headCustomList = NULL;

// LOCAL FUNCTIONS

static int32 *set_buf_size(int size, csdFile_csb *csb);
void mask_to_bitmask(unsigned int mask, int64* bm);
void mask_to_bitmask64(int endian_flip, int64 mask, int64* bm);
void endian_flip(void *buf, size_t size);
size_t endian_fread(void *ptr, size_t size, size_t nitems, FILE *stream);
static void load_csb(csdFile_csb *csb);
static void free_load_csb(csdFile_csb *csb);
static void free_list( int list_id, csdFile_csb *csb );

   // Load functions
   
static pfMaterial *load_mtl(int id, csdFile_csb *csb);
static pfTexture *load_tex(int id, csdFile_csb *csb);
static pfTexGen *load_texgen(int id, csdFile_csb *csb);
static pfGeoState *load_appearance(int id, csdFile_csb *csb);
static pfVec3 *load_vset(int id, csdFile_csb *csb);
static pfVec4 *load_cset(int id, csdFile_csb *csb);
static pfVec3 *load_nset(int id, csdFile_csb *csb);
static void *load_tset(int id, csdFile_csb *csb);
static int32 *load_iset(int id, csdFile_csb *csb);
static void     load_bpc_data(pfGeoSet *gset, csdFile_csb *csb);
static pfGeoSet *load_lineset(int id, csdFile_csb *csb);
static pfGeoSet *load_linestripset(int id, csdFile_csb *csb);
static pfGeoSet *load_pointset(int id, csdFile_csb *csb);
static pfGeoSet *load_polyset(int id, csdFile_csb *csb);
static pfGeoSet *load_quadset(int id, csdFile_csb *csb);
static pfGeoSet *load_triset(int id, csdFile_csb *csb);
static pfGeoSet *load_tristripset(int id, csdFile_csb *csb);
static void *load_geometry(int id, csdFile_csb *csb);
static void *load_engine(int id, csdFile_csb *csb);
static void *load_norminterp(int id, csdFile_csb *csb);
static void *load_orientinterp(int id, csdFile_csb *csb);
static void *load_posinterp(int id, csdFile_csb *csb);
static void *load_scalarinterp(int id, csdFile_csb *csb);
static void *load_colorinterp(int id, csdFile_csb *csb);
static void *load_coordinterp(int id, csdFile_csb *csb);
static void *load_morpheng3f(int id, csdFile_csb *csb);
static void *load_morpheng4f(int id, csdFile_csb *csb);
static void *load_selecteng3f(int id, csdFile_csb *csb);
static void *load_selecteng4f(int id, csdFile_csb *csb);
static void *load_transeng3f(int id, csdFile_csb *csb);
static void *load_spline(int id, csdFile_csb *csb);
static void *load_timesensor(int id, csdFile_csb *csb);
static pfNode *load_node(int id, csdFile_csb *csb);
static pfGeoSet *load_box(int id, csdFile_csb *csb);
static pfGeoSet *load_cone(int id, csdFile_csb *csb);
static pfGeoSet *load_cylinder(int id, csdFile_csb *csb);
static pfGeoSet *load_sphere(int id, csdFile_csb *csb);
static pfBillboard *load_sprite(int id, csdFile_csb *csb);
static pfGeoSet *load_trifanset(int id, csdFile_csb *csb);
static void *load_connection(int id, csdFile_csb *csb);

   // Load helper functions
static void load_name(void *container, int name_size, csdFile_csb *csb);
static void load_node_name(pfNode *node, int name_size, csdFile_csb *csb);
static void set_geoset_data( pfGeoSet *gset,  geoset_t *g,  csdFile_csb *csb);
static void set_node_data(pfNode *node, node_t *g, csdFile_csb *csb);
static void fix_tristrip_normals( pfGeoSet *gset,  csdFile_csb *csb  );
static void convert_to_flat_shade( pfGeoSet *gset, csdFile_csb *csb  );
static void fix_linestrip_normals( pfGeoSet *gset,  csdFile_csb *csb  );
static void produceClearCoat(pfTexture* tex, float index, float contrastScale, float contrastBias);

// node loader stuff
static pfGroup *pfGroup_new(void);
static void csdLoadGroupInfo(void *obj, all_nodes_t *n, int structsize, csdFile_csb *csb);
static int pfGroup_load(void *obj, csdFile_csb *csb);
static pfDCS *pfDCS_new(void);
static void csdLoadTransformInfo(void *obj, all_nodes_t *n,int structsize,csdFile_csb *csb);
static int csTransform_load(void *obj, csdFile_csb *csb);
static pfSwitch *pfSwitch_new(void);
static void csdLoadSwitchInfo(void *node, all_nodes_t *n, int structsize,csdFile_csb *csb);
static int csSwitch_load(void *obj, csdFile_csb *csb);
static pfLOD *pfLOD_new(void);
static void csdLoadLODInfo(void *node, all_nodes_t *n, int structsize,csdFile_csb *csb);
static int csLOD_load(void *obj, csdFile_csb *csb);
static pfGeode *pfGeode_new(void);
int csShape_load(void *obj, csdFile_csb *csb);
static int csEnvironment_load(void *obj, csdFile_csb *csb);
static int csBillboard_load(void *obj, csdFile_csb *csb);
static int csPointLight_load(void *obj, csdFile_csb *csb);
static int csSpotLight_load(void *obj, csdFile_csb *csb);
static int csDirectionalLight_load(void *obj, csdFile_csb *csb);
static int csFog_load(void *obj, csdFile_csb *csb);
static void csRegisterTypes();

   // debug functions
static void print_gset_data( geoset_t *g );
static void print_appearance_data ( appearance_t *a );
static void print_material( material_t *m );
static void print_lod( cslod_t *l );

   // load function table
   
static void *(*load_func[L_COUNT])(int id, csdFile_csb *csb) =
{
    (void *(*)(int, csdFile_csb *))load_mtl,
    (void *(*)(int, csdFile_csb *))load_tex,
    (void *(*)(int, csdFile_csb *))load_texgen,
    (void *(*)(int, csdFile_csb *))load_appearance,
    (void *(*)(int, csdFile_csb *))load_vset,
    (void *(*)(int, csdFile_csb *))load_cset,
    (void *(*)(int, csdFile_csb *))load_nset,
    (void *(*)(int, csdFile_csb *))load_tset,
    (void *(*)(int, csdFile_csb *))load_iset,
    (void *(*)(int, csdFile_csb *))load_lineset,
    (void *(*)(int, csdFile_csb *))load_linestripset,
    (void *(*)(int, csdFile_csb *))load_pointset,
    (void *(*)(int, csdFile_csb *))load_polyset,
    (void *(*)(int, csdFile_csb *))load_quadset,
    (void *(*)(int, csdFile_csb *))load_triset,
    (void *(*)(int, csdFile_csb *))load_tristripset,
    (void *(*)(int, csdFile_csb *))load_geometry,
    (void *(*)(int, csdFile_csb *))load_node,
    (void *(*)(int, csdFile_csb *))load_box,
    (void *(*)(int, csdFile_csb *))load_cone,
    (void *(*)(int, csdFile_csb *))load_cylinder,
    (void *(*)(int, csdFile_csb *))load_sphere,
    (void *(*)(int, csdFile_csb *))load_sprite,
    (void *(*)(int, csdFile_csb *))load_trifanset,
    NULL,
    NULL,
    (void *(*)(int, csdFile_csb *))load_connection,
    (void *(*)(int, csdFile_csb *))load_engine,
    (void *(*)(int, csdFile_csb *))load_norminterp,
    (void *(*)(int, csdFile_csb *))load_orientinterp,
    (void *(*)(int, csdFile_csb *))load_posinterp,
    (void *(*)(int, csdFile_csb *))load_scalarinterp,
    (void *(*)(int, csdFile_csb *))load_colorinterp,
    (void *(*)(int, csdFile_csb *))load_coordinterp,
    (void *(*)(int, csdFile_csb *))load_morpheng3f,
    (void *(*)(int, csdFile_csb *))load_morpheng4f,
    (void *(*)(int, csdFile_csb *))load_selecteng3f,
    (void *(*)(int, csdFile_csb *))load_selecteng4f,
    (void *(*)(int, csdFile_csb *))load_transeng3f,
    (void *(*)(int, csdFile_csb *))load_spline,
    (void *(*)(int, csdFile_csb *))load_timesensor,
};


/*
 *  #define style psudo functions.
 */
#define GET_BUF(s) (((s) <= csb->buf_size)? csb->buf : set_buf_size((s), csb))

#define COPY3(dst, src)		\
(				\
    (dst)[0] = (src)[0],	\
    (dst)[1] = (src)[1],	\
    (dst)[2] = (src)[2]		\
)

// Load a csb file
PFCSB_DLLEXPORT pfNode *pfdLoadFile_csb(const char *fileName)
{
    static const char func_name[] = "pfdLoadFile_csb";
    csdFile_csb *csb;
    pfNode *root;
    char path[PF_MAXSTRING];

    csb = (csdFile_csb *)pfMalloc( sizeof(csdFile_csb),  pfGetSharedArena() );
    bzero(csb, sizeof(csdFile_csb));
    csb->func_name = func_name;
    csb->arena = pfGetSharedArena();
    //csb->arena = NULL;

    csRegisterTypes();
    
    /*
     *  Find the csb file.
     */
    
    if( !pfFindFile( fileName,  path, R_OK) )
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,"%s:  Could not find \"%s\".\n",
		func_name, fileName );
	free(csb);
	return(NULL);
    }

    /*
     *  open the csb file for reading
     */
    if ((csb->fp = fopen(path, "rb")) == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,"%s:  Could not open \"%s\" for reading.\n",
		 func_name, fileName );
	free(csb);
	return(NULL);
    }
    else
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "%s:  Loading \"%s\".\n", 
	         func_name, fileName);
    }

    /*
     *  Initialize csb structure.
     */
    set_buf_size(512, csb);

    /*
     *  Load the csb file.
     */
    load_csb(csb);
    root = csb->root;

    /*
     *  Close the csb file.
     */
    fclose(csb->fp);

    /*
     *  Link any remaining parts of the scene graph that needs to be linked
     */
    //link_csb(csb);   // doesn't do anything right now

    /*
     *  Free temporary memory used in loading the csb file.
     */
    free_load_csb(csb);

    return(root);
    
}  // end static void load_csb()


// used in GETBUF(s) macro and pfdLoadFile_csb

static int32 *set_buf_size(int size, csdFile_csb *csb)
{
    if (csb->buf_size < size)
    {
	if (csb->buf)
	    free(csb->buf);
	csb->buf = (int32 *)pfMalloc(size * sizeof(int32), csb->arena);
	csb->buf_size = size;
    }

    return(csb->buf);
    
}   // end set_buf_size()


/*
 * ------------------ misc utilities -------------------
 */

void mask_to_bitmask(unsigned int mask, int64* bm)
{
    int i;

    for (i = 0; i < 32; i++)
    {
        if (mask & (1 << i))
	    *bm |= ((int64)1 << i);
    }
    // upper 32 bits should inherit since CSB file didn't contain these fields
    for (i = 32; i < 64; i++)
	*bm |= ((int64)1 << i);
}


void mask_to_bitmask64(int endian_flip, int64 mask, int64* bm)
{
    if (endian_flip)
    {
        int64 left32, right32;

        // this is a double word so it needs an extra flip
        left32 = mask >> 32;
        right32 = mask && 0xFFFFFFFF;
        *bm = right32 << 32;
        *bm |= left32;
    }
    else
        *bm = mask;
}


void endian_flip(void *buf, size_t size)
{
    char *cbuf;
    int i, offset;
    char a, b;

    cbuf = (char *)buf;

    for (i = 0; (i+4) <= size; i += 4)
    {
	a = cbuf[i];
	b = cbuf[i+1];
	cbuf[i] = cbuf[i+3];
	cbuf[i+1] = cbuf[i+2];
	cbuf[i+2] = b;
	cbuf[i+3] = a;
    }

    if (offset = ((i+4) % 4))
    {
	if (offset == 2)
	{
	   a = cbuf[i];
	   cbuf[i] 	= cbuf[i+1];
	   cbuf[i+1] 	= a;
	} else if (offset == 3)
	{
	   a = cbuf[i];
	   cbuf[i] = cbuf[i+2];
	   cbuf[i+2] = a;
	}
    }
    
}   // end endian_flip()


size_t endian_fread(void *ptr, size_t size, size_t nitems, FILE *stream)
{
    size_t r;

    r = fread(ptr, size, nitems, stream);
    endian_flip((void *)ptr, size * nitems);
    return(r);
    
}   // end endian_fread()


static void load_csb(csdFile_csb *csb)
{
    int32 buf[4];
    int i;

    /*
     *  load header
     */
    fread(buf, sizeof(int32), 4, csb->fp);

    if (buf[0] == CSB_MAGIC_NUMBER_E ||
	buf[0] == CSB_MAGIC_ENCODED_E)
    {
	csb->endian_flip = TRUE;
	CSB_FREAD = endian_fread;
	endian_flip(buf, sizeof(int32) * 4);
    }
    else
	CSB_FREAD = fread;

    if (buf[0] != CSB_MAGIC_NUMBER)
    {
	csb->read_error = TRUE;
	pfNotify(PFNFY_WARN, PFNFY_PRINT,"%s: Unrecognized magic number 0x%08x.\n",
		csb->func_name, buf[0] );
	return;
    }
    csb->version = buf[1];

    if (csb->version < CSB_VERSION_NUMBER)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,"%s: Old version number found %d.\n",
		  csb->func_name, buf[1] );
    }

    /*
     *  load lists
     */
    while (csb->root == NULL && !csb->read_error)
    {
	if (CSB_FREAD(buf, sizeof(int32), 3, csb->fp) != 3)
	{
	    csb->read_error = TRUE;
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "%s: Unexpected EOF encountered.\n",
		    csb->func_name );
	    return;
	}

	if (buf[0] == L_EOF)
	{
	    if( buf[1] != L_NODE )
	    {
	        csb->read_error = TRUE;
		pfNotify(PFNFY_WARN, PFNFY_PRINT, "%s: Must return a pfNode nota %d .\n",
		    csb->func_name,  buf[1] );
		return;
	    }
	    else
	    {
	        
	       csb->root = (pfNode *)csb->rl_list[buf[1]][buf[2]];
	       #ifdef PRINT_SCENE
	        FILE *fp;
	        fp = fopen("scene.out",  "w" );
	       printf("loaded tree struct \n" );
	       pfPrint(csb->root, (PFTRAV_SELF|PFTRAV_DESCEND),
	                     PFPRINT_VB_DEBUG, fp);
	        fclose(fp);
		#endif
	    }
	}
	else if (buf[0] < L_COUNT) 
	{
            int32 listId;
	    // loading known csObjects, or objects subclassed off csNode
 	    listId = buf[0]; 
	    csb->curListLength = buf[1]; 
	    
	    csb->rl_list[listId] = (void **)pfMalloc(sizeof(void **) *
						     csb->curListLength, 
						     csb->arena);
						     
	    // keep track of the number of items in each list					     
	    csb->l_num[listId] = csb->curListLength;
	    
	    if( listId == L_GEOM )
	    {
		csb->geo_ref = (int32 *)pfMalloc( sizeof(int32)*csb->curListLength, 
		                                   csb->arena );
		for( int k = 0; k < csb->curListLength; k++ )
		{
		    csb->geo_ref[k] = 0;
		}
	    }
	    
	    if( listId == L_ISET )
	    {
		csb->iset_size = (int32 *)pfMalloc( sizeof(int32)*csb->curListLength, 
		                                   csb->arena );
		for( int k = 0; k < csb->curListLength; k++ )
		{
		    csb->iset_size[k] = 0;
		}
	    }

	    if ( listId == L_APP )
	    {
		csb->gstate_mtl_mode = (int32 *)pfMalloc( sizeof(int32)*csb->curListLength, 
		                                   csb->arena );
		for( int k = 0; k < csb->curListLength; k++ )
		{
		    csb->gstate_mtl_mode[k] = 0;
		}
	    }
						       
           // pfNotify( PFNFY_NOTICE, PFNFY_PRINT, "list %d size %d \n",  
	   //          listId,  csb->curListLength );
		     						       
	    for (i = 0; i < csb->curListLength && !csb->read_error; i++)
	    {
               csb->rl_list[listId][i] = load_func[listId](i, csb);
            }
		
	    // L_CONNECTIONS contains NULLs...
	} else
	{
	   pfNotify( PFNFY_WARN,  PFNFY_PRINT,  "Uknown list %d \n", buf[0] );  
	}
	
    }   // end while stuff to read
    
}   // end load_csb()

// used in pfdLoadFile_csb()

static void free_load_csb(csdFile_csb *csb)
{
    // call pfDelete on all vertex, normal, color and tex
    // arrays since if there was apfGeoSet in index mode we may have
    // fltttened them
    
    free_list( L_VSET,  csb );
    free_list( L_CSET,  csb );
    free_list( L_NSET,  csb );
    free_list( L_TSET,  csb );
    
    // free up anything allocated in the csdFile
    
    pfDelete(csb->buf);
    pfDelete(csb->geo_ref );
    pfDelete(csb->iset_size );
    pfDelete(csb->gstate_mtl_mode );
    pfDelete(csb);
    
}  //end free_load_csb


static void free_list( int list_id, csdFile_csb *csb )
{
    for( int i = 0; i < csb->l_num[list_id]; i++ )
    {
	pfDelete( csb->rl_list[list_id][i] );
    }

}   // end free_list()

//--------------------------------------------------------------------------

// Load functions


static pfMaterial *load_mtl(int /*id*/, csdFile_csb *csb)
{
   pfMaterial *mtl;
   material_t m;

   CSB_FREAD(&m, sizeof(material_t), 1, csb->fp);
   //printf("material id = %d \n",  id );
   //print_material( &m );
   
   mtl = new pfMaterial();
   mtl->setColor(PFMTL_AMBIENT,  m.ambient[0], m.ambient[1], m.ambient[2]);
   mtl->setColor(PFMTL_DIFFUSE,  m.diffuse[0], m.diffuse[1], m.diffuse[2]);
   mtl->setColor(PFMTL_SPECULAR, m.specular[0], m.specular[1], m.specular[2]);
   mtl->setColor(PFMTL_EMISSION,  m.emissive[0], m.emissive[1], m.emissive[2]); 
   mtl->setShininess(m.shininess*(1.0f/0.0078125f) );
   
   //convert VRML alpha to perfomer
   mtl->setAlpha(1.0 - m.transparency);
   // fprintf( stderr, "mtl transp = %f\n", m.transparency );

   if( m.name_size != -1 )
   {
       load_name( mtl,  m.name_size,  csb );
   }

   return(mtl);
 
}   // end load_mtl()

static void produceClearCoat(pfTexture *tex, float index, float contrastScale,
	float contrastBias)
{
    static int genInited = FALSE;
    static ClearCoatGenFunc genFunc = NULL;

    if (! genInited) {
#ifndef WIN32
	void* dso = dlopen(NULL, RTLD_LAZY);
	genFunc = (ClearCoatGenFunc)dlsym(dso, "generateClearCoat");
	dlclose(dso);
#else
	pfNotify(PFNFY_WARN,PFNFY_PRINT,"ClearCoat stuff from libpfcsb not yet ported to win32!");
#endif
	genInited = TRUE;
    }

    if (genFunc != NULL) {
	double start = pfGetTime();

	int channels;
	int width;
	int height;
	int depth;
	unsigned char *orig_bytes;

	tex->getImage((uint **)&orig_bytes, &channels, &width, &height, &depth);

	if (depth != 1) {
	    pfNotify(PFNFY_WARN, PFNFY_USAGE,
		     "libpfcsb::produceClearCoat: internal error - "
		     "unsupported z (depth) of texture %i", depth);
	    return;
	}

	pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
		 "libpfcsb::produceClearCoat: producing %.2f ClearCoat map "
		 "[%s] (%ix%i)", index, tex->getName(), width, height);
	if ((contrastScale != 1.0f) || (contrastBias != 0.0f))
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
		     "libpfcsb::produceClearCoat: using contrast scale "
		     "%.2f bias %.2f", contrastScale, contrastBias);

	int newchannels;
	int format;
	int pfformat;
	switch ( channels ) {
	case 1:
	    format = GL_LUMINANCE;
	    pfformat = PFTEX_IA_8;
	    newchannels = 2;
	    break;
	case 2:
	    format = GL_LUMINANCE_ALPHA;
	    pfformat = PFTEX_IA_8;
	    newchannels = 2;
	    break;
	case 3:
	    format = GL_RGB;
	    pfformat = PFTEX_RGBA_8;
	    newchannels = 4;
	    break;
	case 4:
	    format = GL_RGBA;
	    pfformat = PFTEX_RGBA_8;
	    newchannels = 4;
	    break;
	default:
	    pfNotify(PFNFY_WARN, PFNFY_USAGE,
		     "libpfcsb::produceClearCoat: internal error - "
		     "unsupported number of channels %i", channels);
	    return;
	} // switch channels

	unsigned char *bytes = (unsigned char *)
	    pfMalloc( width*height*newchannels );

	(*genFunc)(orig_bytes, width, height, format, GL_UNSIGNED_BYTE,
			  bytes, index, contrastScale, contrastBias);

	tex->setImage( (uint *)bytes, newchannels, width, height, depth );
	tex->setFormat(PFTEX_INTERNAL_FORMAT, pfformat);

	double total = pfGetTime() - start;
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "ClearCoat calculation took %f "
		"seconds", total);
    } else
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		"libpfcsb::produceClearCoat: ClearCoat disabled.");
} // end produceClearCoat

static pfTexture *load_tex(int /*id*/, csdFile_csb *csb)
{
    pfTexture *tex;
    texture_t t;
    unsigned int *image2;
    uint *image_ptr;
    int comp;
    int ns,  nt,  nr;
    int            channels;
    int            xRes;
    int            yRes;
#if defined(__linux_has_ifl__) || defined(sgi)
    iflStatus      sts;
    iflFile*       file;
    iflSize        dims;
#endif
    char	   filePath[512];
    
    // read texture info and file name
    CSB_FREAD(&t, sizeof(texture_t), 1, csb->fp);
 
    fread((char *)csb->buf, t.file_name_size, 1,  csb->fp);
    ((char *)csb->buf)[t.file_name_size] = '\0';
    
    if (!pfFindFile((char *)csb->buf, filePath, R_OK))
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "libpfcsb::load_tex: Could not find file \"%s\"", (char *)csb->buf);
	return NULL;
    }

    tex = new pfTexture();

    // see if we can read the texture
    if (!tex->loadFile(filePath))
    { // else try IFL
#if defined(__linux_has_ifl__) || defined(sgi)

	// Ask IFL to do its thing
	file = iflFile::open(filePath, O_RDONLY, &sts );
	if (sts != iflOKAY)
	{
	       pfDelete(tex);
	       return NULL;    
       }
    
  
	// Open the file and find out about it's dimensions
	file->getDimensions(dims);

	channels = dims.c;
	xRes     = dims.x;
	yRes     = dims.y;

	image2 = (unsigned int *)pfMalloc( sizeof(char)*xRes*yRes*channels, csb->arena );
	if( image2 == NULL )
	{
	    pfNotify( PFNFY_WARN,  PFNFY_PRINT, "error pfMalloc() load_tex %s \n",(char *)csb->buf );
	}
		
	pfNotify( PFNFY_INFO, PFNFY_PRINT, 
	       "Reading texture map, %s, %d %d, comp=%d\n",  (char *)csb->buf, xRes, yRes, channels );

	iflConfig cfg(iflUChar, iflInterleaved);
	sts = file->getTile(0, 0, 0, dims.x, dims.y, 1, image2, &cfg);
	if (sts != iflOKAY)
	{
	    pfNotify( PFNFY_WARN,  PFNFY_PRINT, "iflFile::getTile load_tex %s \n",(char *)csb->buf );
	}

	// set the textures image
	tex->setImage( image2,  channels,  xRes,  yRes,  1 );
#else
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "Only .rgb files are supported at this time.\n");
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "Skipping texture: %s\n", filePath);
	return NULL;
#endif
    }
    tex->setName( (char *)csb->buf );
    // for debugging show what we set
    
    tex->getImage( &image_ptr,  &comp,  &ns,  &nt,  &nr );
    pfNotify(PFNFY_INFO,PFNFY_PRINT,
    "tex image %s %p   comp = %d  ns = %d nt = %d nr = %d \n", 
    	tex->getName(), image_ptr,  comp,  ns,  nt,  nr );

    // the format index reported by the csb file is not 
    // makeing any sense, but the default for
    // the tex file is working, so for now rely on the default
    //tex->setFormat(PFTEX_INTERNAL_FORMAT, txf_table[t.format] );
    
    tex->setFormat(PFTEX_EXTERNAL_FORMAT, PFTEX_PACK_8 );
    tex->setRepeat( PFTEX_WRAP_S, txr_table[t.repeat_s] );
    tex->setRepeat( PFTEX_WRAP_T, txr_table[t.repeat_t] );
    tex->setFilter( PFTEX_MINFILTER, txmnf_table[t.min_filter]);
    tex->setFilter( PFTEX_MAGFILTER, txmgf_table[t.mag_filter]);
    
    // debug info
    //printf("repeat S: %d   T: %d  \n", t.repeat_s,  t.repeat_t ); 
    //printf("filter MIN: %d   MAG %d \n",  t.min_filter, t.mag_filter );  
    //printf("texture format is %d \n", t.format );  
    //printf("act FORMAT %d \n", tex->getFormat(PFTEX_INTERNAL_FORMAT) );
    //printf("search for fomat \n");
    //for( i = 0; i < txf_table[0]; i++ )
    //{
    //   if( txf_table[i] == tex->getFormat(PFTEX_INTERNAL_FORMAT) )
    //   {
    //      printf("act FORMAT is table entry %d \n", i );
    //   }
    //}
	  
     if( t.name_size != -1 )
     {
	 load_name( tex,  t.name_size,  csb );
	 ((char *)csb->buf)[t.name_size] = '\0';
	 tex->setName( (char *)csb->buf );
	 float index, contrastScale, contrastBias;
	 int n = sscanf((char *)csb->buf + t.file_name_size,
			"_i%f_cs%f_cb%f", &index,
			&contrastScale, &contrastBias);
	 if (n == 3) {
	     fprintf(stderr, "*** ClearCoat i%f cs%f cb%f\n", index,
		     contrastScale, contrastBias);
	     produceClearCoat(tex, index, contrastScale, contrastBias);
	 }
     }

    return(tex);
    
}   // end load_tex()

static pfTexGen *load_texgen(int /*id*/, csdFile_csb *csb)
{
    pfTexGen *texgen;
    texgen_t t;
    
    CSB_FREAD(&t, sizeof(texgen_t), 1, csb->fp);

    texgen = new pfTexGen();
    
    texgen->setMode( PF_S,   tgm_table[t.mode_s] );
    texgen->setMode( PF_T,   tgm_table[t.mode_t] );
    texgen->setMode( PF_R,   tgm_table[t.mode_q] );
    texgen->setMode( PF_Q,   tgm_table[t.mode_r] );
    texgen->setPlane( PF_S,  t.plane_s[0], t.plane_s[1], t.plane_s[2], t.plane_s[3] );
    texgen->setPlane( PF_T,  t.plane_t[0], t.plane_t[1], t.plane_t[2], t.plane_t[3] );
    texgen->setPlane( PF_R,  t.plane_r[0], t.plane_r[1], t.plane_r[2], t.plane_r[3] );
    texgen->setPlane( PF_Q,  t.plane_q[0], t.plane_q[1], t.plane_q[2], t.plane_q[3] );
    
    if( t.name_size != -1 )
    {
        load_name( texgen,  t.name_size,  csb );
    }
    
    return(texgen);
    
}   // end load_texgen()

/*
 * loading appearances
 */

#define OLD_OLD_LIGHT_ENABLE            0
#define OLD_OLD_MATERIAL                1
#define OLD_OLD_MATERIAL_MODE_ENABLE    2
#define OLD_OLD_TEX_ENABLE              3
#define OLD_OLD_TEXTURE                 4
#define OLD_OLD_TRANSP_ENABLE           5
#define OLD_OLD_CULL_FACE               6
#define OLD_OLD_ALPHA_FUNC              7
#define OLD_OLD_ALPHA_REF               8
#define OLD_OLD_TRANSP_MODE             9
#define OLD_OLD_TEX_MODE                10
#define OLD_OLD_TEX_ENV                 11
#define OLD_OLD_TEX_BLEND_COLOR         12
#define OLD_OLD_TEX_GEN_ENABLE          13
#define OLD_OLD_TEX_GEN                 14
#define OLD_OLD_SHADE_MODEL             15
#define OLD_OLD_FOG_ENABLE              16
#define OLD_OLD_BLEND_COLOR             17
#define OLD_OLD_SRC_BLEND_FUNC          18
#define OLD_OLD_DST_BLEND_FUNC          19
#define OLD_OLD_COLOR_MASK              20
#define OLD_OLD_DEPTH_FUNC              21
#define OLD_OLD_DEPTH_MASK              22
#define OLD_OLD_POLY_MODE               23
#define OLD_OLD_LINE_STIPPLE_PATTERN    24
#define OLD_OLD_LINE_STIPPLE_FACTOR     25
#define OLD_OLD_TEX_TRANSFORM           26
#define OLD_OLD_BACK_MATERIAL           27

#define OLD_TEXTURE                     0
#define OLD_TEX_ENABLE                  1
#define OLD_TEX_MODE                    2
#define OLD_TEX_BLEND_COLOR             3
#define OLD_TEX_ENV                     4
#define OLD_TEX_GEN                     5
#define OLD_TEX_GEN_ENABLE              6
#define OLD_MATERIAL                    7
#define OLD_LIGHT_ENABLE                8
#define OLD_SHADE_MODEL                 9
#define OLD_TRANSP_ENABLE               10
#define OLD_TRANSP_MODE                 11
#define OLD_ALPHA_FUNC                  12
#define OLD_ALPHA_REF                   13
#define OLD_BLEND_COLOR                 14
#define OLD_SRC_BLEND_FUNC              15
#define OLD_DST_BLEND_FUNC              16
#define OLD_COLOR_MASK                  17
#define OLD_DEPTH_FUNC                  18
#define OLD_DEPTH_MASK                  19
#define OLD_FOG_ENABLE                  20
#define OLD_POLY_MODE                   21
#define OLD_LINE_STIPPLE_PATTERN        22
#define OLD_LINE_STIPPLE_FACTOR         23
#define OLD_TEX_TRANSFORM               24
#define OLD_BACK_MATERIAL               25
#define OLD_DEPTH_ENABLE                26
#define OLD_FOG_MODE                    27
#define OLD_FOG_DENSITY                 28
#define OLD_FOG_START                   29
#define OLD_FOG_END                     30
#define OLD_FOG_INDEX                   31
#define OLD_FOG_COLOR                   32
#define OLD_MATERIAL_MODE_ENABLE        33

#define TEXTURE				0
#define TEX_ENABLE 			1
#define TEX_MODE			2
#define TEX_BLEND_COLOR			3
#define TEX_ENV				4
#define TEX_GEN				5
#define TEX_GEN_ENABLE			6
#define LIGHT_ENABLE			7
#define SHADE_MODEL			8
#define TRANSP_ENABLE			9
#define TRANSP_MODE			10
#define ALPHA_FUNC			11
#define ALPHA_REF			12
#define BLEND_COLOR			13
#define SRC_BLEND_FUNC			14
#define DST_BLEND_FUNC			15
#define COLOR_MASK			16
#define DEPTH_FUNC			17
#define DEPTH_MASK			18
#define FOG_ENABLE			19
#define POLY_MODE			20
#define LINE_STIPPLE_PATTERN		21
#define LINE_STIPPLE_FACTOR		22
#define TEX_TRANSFORM			23
#define MATERIAL			24
#define BACK_MATERIAL			25
#define DEPTH_ENABLE			26
#define MATERIAL_MODE_ENABLE		27

typedef struct {
    int lightEnable;
    int material;
    int materialModeEnable;
    int texEnable;
    int texture;
    int transpEnable;
    int cullFace;
    int alphaFunc;
    int alphaRef;
    int transpMode;
    int texMode;
    int texEnv;
    int texBlendColor;
    int texGenEnable;
    int texGen;
    int shadeModel;
    int fogEnable;
    int blendColor;
    int srcBlendFunc;
    int dstBlendFunc;
    int colorMask;
    int depthFunc;
    int depthMask;
    int polyMode;
    int lineStipplePattern;
    int lineStippleFactor;
    int texTransform;
    int backMaterial;
    int depthEnable;
    int fogMode;
    int fogDensity;
    int fogStart;
    int fogEnd;
    int fogIndex;
    int fogColor;
} appearance_local_t;

static int
bitOff(int64 mask, int bit)
{
    if (bit == -1) return 0;
    return ((mask & ((int64)1 << bit)) == 0);
}

static void
set_appearance_local(int64 mask, appearance_local_t *local, int tex, int version)
{
    if (version < CSB_VERSION_REORDERD_INHERTANCE_MASK)
    {
	local->lightEnable 	  = bitOff(mask, OLD_OLD_LIGHT_ENABLE);
	local->material		  = bitOff(mask, OLD_OLD_MATERIAL);
	local->materialModeEnable = bitOff(mask, OLD_OLD_MATERIAL_MODE_ENABLE);
	local->transpEnable	  = bitOff(mask, OLD_OLD_TRANSP_ENABLE);
	local->cullFace		  = bitOff(mask, OLD_OLD_CULL_FACE);
	local->alphaFunc	  = bitOff(mask, OLD_OLD_ALPHA_FUNC);
	local->alphaRef		  = bitOff(mask, OLD_OLD_ALPHA_REF);
	local->transpMode	  = bitOff(mask, OLD_OLD_TRANSP_MODE);
	local->texMode		  = bitOff(mask, OLD_OLD_TEX_MODE);
	local->texEnv		  = bitOff(mask, OLD_OLD_TEX_ENV);
	local->texBlendColor	  = bitOff(mask, OLD_OLD_TEX_BLEND_COLOR);
	local->texGenEnable	  = bitOff(mask, OLD_OLD_TEX_GEN_ENABLE);
	local->texGen		  = bitOff(mask, OLD_OLD_TEX_GEN);
	local->shadeModel	  = 1; // old version ignores inheritance bit
	local->fogEnable	  = bitOff(mask, OLD_OLD_FOG_ENABLE);
	local->blendColor	  = bitOff(mask, OLD_OLD_BLEND_COLOR);
	local->srcBlendFunc	  = bitOff(mask, OLD_OLD_SRC_BLEND_FUNC);
	local->dstBlendFunc	  = bitOff(mask, OLD_OLD_DST_BLEND_FUNC);
	local->colorMask	  = bitOff(mask, OLD_OLD_COLOR_MASK);
	local->depthFunc	  = bitOff(mask, OLD_OLD_DEPTH_FUNC);
	local->depthMask	  = bitOff(mask, OLD_OLD_DEPTH_MASK);
	local->polyMode		  = bitOff(mask, OLD_OLD_POLY_MODE);
	local->lineStipplePattern = bitOff(mask, OLD_OLD_LINE_STIPPLE_PATTERN);
	local->lineStippleFactor  = bitOff(mask, OLD_OLD_LINE_STIPPLE_FACTOR);
	local->texTransform	  = bitOff(mask, OLD_OLD_TEX_TRANSFORM);
	local->backMaterial	  = bitOff(mask, OLD_OLD_BACK_MATERIAL);
	local->depthEnable	  = 0;
	local->fogMode		  = 0;
	local->fogDensity	  = 0;
	local->fogStart		  = 0;
	local->fogEnd		  = 0;
	local->fogIndex		  = 0;
	local->fogColor		  = 0;

	if (tex != -1)
	{
	    local->texEnable	  = 1;
	    local->texture	  = 1;
	}
	else
	{
	    local->texEnable	  = bitOff(mask, OLD_OLD_TEX_ENABLE);
	    local->texture 	  = bitOff(mask, OLD_OLD_TEXTURE);
	}
    }
    else if (version < CSB_VERSION_MOVED_MATERIAL_BIT)
    {
	local->lightEnable 	  = bitOff(mask, OLD_LIGHT_ENABLE);
	local->material		  = bitOff(mask, OLD_MATERIAL);
	local->materialModeEnable = bitOff(mask, OLD_MATERIAL_MODE_ENABLE);
	local->texEnable	  = bitOff(mask, OLD_TEX_ENABLE);
	local->texture		  = bitOff(mask, OLD_TEXTURE);
	local->transpEnable	  = bitOff(mask, OLD_TRANSP_ENABLE);
	local->cullFace		  = 0;
	local->alphaFunc	  = bitOff(mask, OLD_ALPHA_FUNC);
	local->alphaRef		  = bitOff(mask, OLD_ALPHA_REF);
	local->transpMode	  = bitOff(mask, OLD_TRANSP_MODE);
	local->texMode		  = bitOff(mask, OLD_TEX_MODE);
	local->texEnv		  = bitOff(mask, OLD_TEX_ENV);
	local->texBlendColor	  = bitOff(mask, OLD_TEX_BLEND_COLOR);
	local->texGenEnable	  = bitOff(mask, OLD_TEX_GEN_ENABLE);
	local->texGen		  = bitOff(mask, OLD_TEX_GEN);
	local->shadeModel	  = bitOff(mask, OLD_SHADE_MODEL);
	local->fogEnable	  = bitOff(mask, OLD_FOG_ENABLE);
	local->blendColor	  = bitOff(mask, OLD_BLEND_COLOR);
	local->srcBlendFunc	  = bitOff(mask, OLD_SRC_BLEND_FUNC);
	local->dstBlendFunc	  = bitOff(mask, OLD_DST_BLEND_FUNC);
	local->colorMask	  = bitOff(mask, OLD_COLOR_MASK);
	local->depthFunc	  = bitOff(mask, OLD_DEPTH_FUNC);
	local->depthMask	  = bitOff(mask, OLD_DEPTH_MASK);
	local->polyMode		  = bitOff(mask, OLD_POLY_MODE);
	local->lineStipplePattern = bitOff(mask, OLD_LINE_STIPPLE_PATTERN);
	local->lineStippleFactor  = bitOff(mask, OLD_LINE_STIPPLE_FACTOR);
	local->texTransform	  = bitOff(mask, OLD_TEX_TRANSFORM);
	local->backMaterial	  = bitOff(mask, OLD_BACK_MATERIAL);
	local->depthEnable	  = bitOff(mask, OLD_DEPTH_ENABLE);
	local->fogMode		  = bitOff(mask, OLD_FOG_MODE);
	local->fogDensity	  = bitOff(mask, OLD_FOG_DENSITY);
	local->fogStart		  = bitOff(mask, OLD_FOG_START);
	local->fogEnd		  = bitOff(mask, OLD_FOG_END);
	local->fogIndex		  = bitOff(mask, OLD_FOG_INDEX);
	local->fogColor		  = bitOff(mask, OLD_FOG_COLOR);
   } 
   else
   {
	local->lightEnable 	  = bitOff(mask, LIGHT_ENABLE);
	local->material		  = bitOff(mask, MATERIAL);
	local->materialModeEnable = bitOff(mask, MATERIAL_MODE_ENABLE);
	local->texEnable	  = bitOff(mask, TEX_ENABLE);
	local->texture		  = bitOff(mask, TEXTURE);
	local->transpEnable	  = bitOff(mask, TRANSP_ENABLE);
	local->cullFace		  = 0;
	local->alphaFunc	  = bitOff(mask, ALPHA_FUNC);
	local->alphaRef		  = bitOff(mask, ALPHA_REF);
	local->transpMode	  = bitOff(mask, TRANSP_MODE);
	local->texMode		  = bitOff(mask, TEX_MODE);
	local->texEnv		  = bitOff(mask, TEX_ENV);
	local->texBlendColor	  = bitOff(mask, TEX_BLEND_COLOR);
	local->texGenEnable	  = bitOff(mask, TEX_GEN_ENABLE);
	local->texGen		  = bitOff(mask, TEX_GEN);
	local->shadeModel	  = bitOff(mask, SHADE_MODEL);
	local->fogEnable	  = bitOff(mask, FOG_ENABLE);
	local->blendColor	  = bitOff(mask, BLEND_COLOR);
	local->srcBlendFunc	  = bitOff(mask, SRC_BLEND_FUNC);
	local->dstBlendFunc	  = bitOff(mask, DST_BLEND_FUNC);
	local->colorMask	  = bitOff(mask, COLOR_MASK);
	local->depthFunc	  = bitOff(mask, DEPTH_FUNC);
	local->depthMask	  = bitOff(mask, DEPTH_MASK);
	local->polyMode		  = bitOff(mask, POLY_MODE);
	local->lineStipplePattern = bitOff(mask, LINE_STIPPLE_PATTERN);
	local->lineStippleFactor  = bitOff(mask, LINE_STIPPLE_FACTOR);
	local->texTransform	  = bitOff(mask, TEX_TRANSFORM);
	local->backMaterial	  = bitOff(mask, BACK_MATERIAL);
	local->depthEnable	  = bitOff(mask, DEPTH_ENABLE);
	local->fogMode		  = 0;
	local->fogDensity	  = 0;
	local->fogStart		  = 0;
	local->fogEnd		  = 0;
	local->fogIndex		  = 0;
	local->fogColor		  = 0;
   }
}



static pfGeoState *load_appearance_bitmask32(appearance895_t& a, csdFile_csb *csb)
{
    int64 inherit_mask;
    appearance_local_t local;

    pfGeoState* app = new pfGeoState;
    
    mask_to_bitmask(a.inherit, &inherit_mask);
    set_appearance_local(inherit_mask, &local, a.tex, csb->version);

    // if the texture is NULL then don't use it
    if (local.texture && a.tex != -1 && csb->rl_list[L_TEX][a.tex] != NULL)
    {
	if (local.texEnv)
	{
	    pfTexEnv *tenv = new pfTexEnv();
            tenv->setMode(cte_table[a.tenv]);
	    if (local.texBlendColor)
	    {
        	tenv->setBlendColor(a.tex_bcolor[0], a.tex_bcolor[1],
                		    a.tex_bcolor[2], a.tex_bcolor[3] );
	    }
            app->setAttr(PFSTATE_TEXENV, tenv);
	}
        app->setAttr(PFSTATE_TEXTURE,
                     (pfTexture *)csb->rl_list[L_TEX][a.tex]);

	if (local.texEnable)
	    app->setMode(PFSTATE_ENTEXTURE, a.tex_enable == 1 ? PF_ON : PF_OFF);
    }

    // if (local.texMode)
    // app->setTexMode((csContext::TexModeEnum)ctxm_table[a.tex_mode]);

    if (local.texGen && a.tgen != -1)
        app->setAttr(PFSTATE_TEXGEN, csb->rl_list[L_TEXGEN][a.tgen]);

    if (local.texGenEnable && a.tgen_enable == 1)
        app->setMode(PFSTATE_ENTEXGEN, PF_ON);

    if (local.material && a.mtl != -1)
        app->setAttr(PFSTATE_FRONTMTL,
                     (pfMaterial *)csb->rl_list[L_MTL][a.mtl]);

    if (local.lightEnable) 
	app->setMode(PFSTATE_ENLIGHTING, a.light_enable == 1 ? PF_ON : PF_OFF);
    else
	// XXX need to explicitly set lighting on rather than inherit
	// from global geostate since perfly does not handle this correctly
	app->setMode(PFSTATE_ENLIGHTING, PF_ON);
	

    if (local.shadeModel)
	app->setMode( PFSTATE_SHADEMODEL, csm_table[a.shade_model] );

    if (local.alphaFunc)
	app->setMode( PFSTATE_ALPHAFUNC, caf_table[a.alpha_func] );

    if (local.alphaRef)
	app->setVal( PFSTATE_ALPHAREF, a.alpha_ref  );

    if (local.transpEnable && a.transp_enable == 1)
	app->setMode( PFSTATE_TRANSPARENCY,  ctrm_table[a.transp_mode] );

    // if (local.blendColor)
    //     app->setBlendColor(a.bcolor[0], a.bcolor[1], a.bcolor[2], a.bcolor[3]);
    // if (local.srcBlendFunc)
    //     app->setSrcBlendFunc((csContext::SrcBlendFuncEnum)csbf_table[a.src_bfunc]);
    // if (local.dstBlendFunc)
    //     app->setDstBlendFunc((csContext::DstBlendFuncEnum)cdbf_table[a.dst_bfunc]);

    if (csb->version < -995)
    {
        appearance996_t *a996 = (appearance996_t*) &a;
	// if (local.colorMask)
        // app->setColorMask(a996->color_mask[0], a996->color_mask[1],
        //                   a996->color_mask[2], a996->color_mask[3]);
	// if (local.depthFunc)
        // app->setDepthFunc((csContext::DepthFuncEnum)cdf_table[a996->depth_func]);
	// if (local.depthMask)
        // app->setDepthMask(a996->depth_mask);

	if (local.fogEnable)
	    app->setMode(PFSTATE_ENFOG, a996->fog_enable == 1 ? PF_ON : PF_OFF);

	if (local.polyMode) 
	    if (cpm_table[a996->poly_mode] == PFSTATE_ENWIREFRAME)
		app->setMode(PFSTATE_ENWIREFRAME, PF_ON);
	    else
		app->setMode(PFSTATE_ENWIREFRAME, PF_OFF);
    } 
    else 
    {
	// if (local.colorMask)
        // app->setColorMask(a.color_mask[0], a.color_mask[1],
        //                  a.color_mask[2], a.color_mask[3]);
	// if (local.depthFunc)
        // app->setDepthFunc((csContext::DepthFuncEnum)cdf_table[a.depth_func]);
	// if (local.depthMask)
        // app->setDepthMask(a.depth_mask);

	if (local.fogEnable)
	    app->setMode(PFSTATE_ENFOG, a.fog_enable == 1 ? PF_ON : PF_OFF);

	if (local.polyMode) 
	    if (cpm_table[a.poly_mode] == PFSTATE_ENWIREFRAME)
		app->setMode(PFSTATE_ENWIREFRAME, PF_ON);
	    else
		app->setMode(PFSTATE_ENWIREFRAME, PF_OFF);
    }

    if (csb->version >= CSB_VERSION_TEX_STIP_N_TWOSIDE)
    {
	if (local.backMaterial && a.back_mtl != -1)
	    app->setAttr(PFSTATE_BACKMTL,
                         (pfMaterial *)csb->rl_list[L_MTL][a.back_mtl]);

	// if (local.lineStipplePattern)
	// app->setLineStipplePattern(a.line_stipple_pattern);
	// if (local.lineStippleFactor)
	// app->setLineStippleFactor(a.line_stipple_factor);

	if (local.texTransform)
	{
	    pfMatrix *tex_transform;
	    tex_transform = (pfMatrix *)pfMalloc(sizeof(pfMatrix), csb->arena);
	    tex_transform->set(a.tex_transform);
	    app->setAttr(PFSTATE_TEXMAT, tex_transform);
	    app->setMode(PFSTATE_ENTEXMAT, PFTR_ON);
	}
    }

    if (csb->version >= CSB_VERSION_ADDED_DEPTH_ENABLE)
    {
	// if (local.depthEnable)
        // app->setDepthEnable(a.depth_enable);
    }

    return app;
}



static pfGeoState *load_appearance_bitmask64(int id, appearance_t& a, csdFile_csb *csb)
{
    int64 inherit_mask;
    appearance_local_t local;

    pfGeoState* app = new pfGeoState;

    mask_to_bitmask64(csb->endian_flip, a.inherit, &inherit_mask);
    set_appearance_local(inherit_mask, &local, a.tex, csb->version);

    // if the texture is NULL then don't use it
    if (local.texture && a.tex != -1 && csb->rl_list[L_TEX][a.tex] != NULL)
    {
	if (local.texEnv)
	{
	    pfTexEnv *tenv = new pfTexEnv();
	    tenv->setMode(cte_table[a.tenv]);
	    tenv->setBlendColor(a.tex_bcolor[0], a.tex_bcolor[1],
			        a.tex_bcolor[2], a.tex_bcolor[3] );
	    app->setAttr(PFSTATE_TEXENV, tenv);
	}

	app->setAttr(PFSTATE_TEXTURE, 
		     (pfTexture *)csb->rl_list[L_TEX][a.tex]);

	if (local.texEnable)
	    app->setMode(PFSTATE_ENTEXTURE, a.tex_enable == 1 ? PF_ON : PF_OFF);
    }

    // if (local.texMode)
    // app->setTexMode((csContext::TexModeEnum)ctxm_table[a.tex_mode]);

    if (local.texGen && a.tgen != -1)
        app->setAttr(PFSTATE_TEXGEN, csb->rl_list[L_TEXGEN][a.tgen]);

    if (local.texGenEnable && a.tgen_enable == 1)
        app->setMode(PFSTATE_ENTEXGEN, PF_ON);

    if (local.material && a.mtl != -1)
        app->setAttr(PFSTATE_FRONTMTL,
                     (pfMaterial *)csb->rl_list[L_MTL][a.mtl]);

    if (local.lightEnable)
        app->setMode(PFSTATE_ENLIGHTING, a.light_enable ? PF_ON : PF_OFF);
    else
	// XXX need to explicitly set lighting on rather than inherit
	// from global geostate since perfly does not handle this correctly
	app->setMode(PFSTATE_ENLIGHTING, PF_ON);

    if (local.shadeModel) 
	app->setMode( PFSTATE_SHADEMODEL, csm_table[a.shade_model] );

    if (local.alphaFunc)
	app->setMode( PFSTATE_ALPHAFUNC, caf_table[a.alpha_func] );

    if (local.alphaRef)
	app->setVal( PFSTATE_ALPHAREF, a.alpha_ref  );

    if (local.transpEnable && a.transp_enable == 1)
	app->setMode( PFSTATE_TRANSPARENCY,  ctrm_table[a.transp_mode] );

    // if (local.blendColor)
    // app->setBlendColor(a.bcolor[0], a.bcolor[1], a.bcolor[2], a.bcolor[3]);
    // if (local.srcBlendFunc)
    // app->setSrcBlendFunc((csContext::SrcBlendFuncEnum)csbf_table[a.src_bfunc]);
    // if (local.dstBlendFunc)
    // app->setDstBlendFunc((csContext::DstBlendFuncEnum)cdbf_table[a.dst_bfunc]);

    // if (local.colorMask)
    // app->setColorMask(a.color_mask[0], a.color_mask[1],
    //                  a.color_mask[2], a.color_mask[3]);
    // if (local.depthFunc)
    // app->setDepthFunc((csContext::DepthFuncEnum)cdf_table[a.depth_func]);
    // if (local.depthMask)
    // app->setDepthMask(a.depth_mask);

    if (local.fogEnable)
        app->setMode(PFSTATE_ENFOG, a.fog_enable == 1 ? PF_ON : PF_OFF);

    if (local.polyMode) 
	if (cpm_table[a.poly_mode] == PFSTATE_ENWIREFRAME)
	    app->setMode(PFSTATE_ENWIREFRAME, PF_ON);
	else
	    app->setMode(PFSTATE_ENWIREFRAME, PF_OFF);

    if (local.backMaterial && a.back_mtl != -1)
        app->setAttr(PFSTATE_BACKMTL,
                     (pfMaterial *)csb->rl_list[L_MTL][a.back_mtl]);
    else if(!local.backMaterial && a.mtl != -1) {
        // Ref bug # 642531, set back material to front material if none
        // is specified. Also turn off backface culing.
        app->setMode(PFSTATE_CULLFACE, PFCF_OFF);
        app->setAttr(PFSTATE_BACKMTL,
                     (pfMaterial *)csb->rl_list[L_MTL][a.mtl]);
    }


    // if (local.lineStipplePattern)
    // app->setLineStipplePattern(a.line_stipple_pattern);
    // if (local.lineStippleFactor)
    // app->setLineStippleFactor(a.line_stipple_factor);

    if (local.texTransform)
    {
	pfMatrix *tex_transform;
	tex_transform = (pfMatrix *)pfMalloc(sizeof(pfMatrix), csb->arena);
	tex_transform->set(a.tex_transform);
	app->setAttr(PFSTATE_TEXMAT, tex_transform);
    }

    // if (local.depthEnable)
    // app->setDepthEnable(a.depth_enable);

    if (csb->version >= CSB_VERSION_ADDED_MATERIAL_MODE)
    {
	if (local.materialModeEnable)
	    csb->gstate_mtl_mode[id] = a.material_mode_enable;
	else
	    csb->gstate_mtl_mode[id] = NO_COLOR_MATERIAL;
    }

    return app;
}


static pfGeoState *load_appearance(int id, csdFile_csb *csb)
{
    pfGeoState *app;
    appearance_t a;
    appearance893_t a893;  // versions since fog added have larger appearance_t struct

    if (csb->version < -995) {
        CSB_FREAD(&a, sizeof(appearance996_t), 1, csb->fp);
    } else if (csb->version < CSB_VERSION_TEX_STIP_N_TWOSIDE) {
        CSB_FREAD(&a, sizeof(appearance991_t), 1, csb->fp);
    } else if (csb->version < CSB_VERSION_ADDED_DEPTH_ENABLE) {
        CSB_FREAD(&a, sizeof(appearance896_t), 1, csb->fp);
    } else if (csb->version < CSB_VERSION_BITMASK64_FOG) {
        CSB_FREAD(&a, sizeof(appearance895_t), 1, csb->fp);
    } else if (csb->version < CSB_VERSION_ADDED_MATERIAL_MODE) {
        CSB_FREAD(&a893, sizeof(appearance894_t), 1, csb->fp);
    } else if (csb->version < CSB_VERSION_FOG) {
        CSB_FREAD(&a893, sizeof(appearance893_t), 1, csb->fp);
    } else {
        CSB_FREAD(&a, sizeof(appearance_t), 1, csb->fp);
    }

    // versions before -894 had a 32 bit inherit mask, when it changed
    // to 64 bit the entire appearance_t structure changed since it
    // just happens to be the first element. argh.  -trina
    if (csb->version < CSB_VERSION_BITMASK64_FOG)
    {
        appearance895_t *a895 = (appearance895_t*) &a;
        app = load_appearance_bitmask32(*a895, csb);
    }
    else
    {
        // Versions containing FOG elements have appearance_t structs
        // which do not map nicely into the current appearance_t which
        // has no FOG elements, need to map old to new.
        if (csb->version < CSB_VERSION_FOG)
        {
            // copy all elements up to and including depth_enable
            memcpy(&a, &a893, sizeof(appearance_t)-sizeof(int));
            // skip fog elements, copy material_mode_enable if there
            if (csb->version >= CSB_VERSION_ADDED_MATERIAL_MODE)
                a.material_mode_enable = a893.material_mode_enable;
        }

        app = load_appearance_bitmask64(id, a, csb);
    }

    if( a.name_size != -1 )
	load_name( app,  a.name_size,  csb );

    return(app);
    
}   // end load_appearance


static pfVec3 *load_vset(int /*id*/, csdFile_csb *csb)
{
    pfVec3 *vset;
    arrayset_t s;
   
    CSB_FREAD(&s, sizeof(arrayset_t), 1, csb->fp);
   
    if( s.array_type == ARRAY_TYPE_3F )
    {
	vset = (pfVec3 *)pfMalloc( sizeof(pfVec3)*s.size,
                                   csb->arena );
	CSB_FREAD(vset, sizeof(pfVec3), s.size, csb->fp); 
    }
    else
    {
	csb->read_error = TRUE;
    }
   
    if( s.name_size != -1 )
    {
 	load_name(vset,  s.name_size,  csb );
    }

    return( vset );
   
}   // end load_vset()

static pfVec4 *load_cset(int /*id*/, csdFile_csb *csb)
{
    pfVec4 *cset;
    arrayset_t s;
    pfVec4 *v4_ptr;
    pfVec3 *v3_ptr;
    
    //printf("load cset \n" );
    
    CSB_FREAD(&s, sizeof(arrayset_t), 1, csb->fp);
    
    if( s.array_type == ARRAY_TYPE_4F )
    {
        cset = (pfVec4 *)pfMalloc( sizeof(pfVec4)*s.size,  
	                           csb->arena );
	CSB_FREAD(cset, sizeof(pfVec4), s.size, csb->fp);
    }
    else if( s.array_type == ARRAY_TYPE_3F )
    {
        pfNotify( PFNFY_WARN,  PFNFY_NOTICE,
	           "Perfomer does not support 3 component color \n \
		   converting to 4 component \n" );

	// read in the 3 componente array
        pfVec3 *tmp_cset = (pfVec3 *)pfMalloc( sizeof(pfVec3)*s.size,  
	                                       csb->arena );
        CSB_FREAD(tmp_cset, sizeof(pfVec3), s.size, csb->fp);
	
	// convert to a 4 component array
	cset =  (pfVec4 *)pfMalloc( sizeof(pfVec4)*s.size,
	                            csb->arena );
	v4_ptr = cset;
	v3_ptr = tmp_cset;
	for( int i = 0; i < s.size; i++ )
	{
	    v4_ptr->set( v3_ptr->vec[0], v3_ptr->vec[1],  v3_ptr->vec[2],  1.0 );
	    v4_ptr++;
	    v3_ptr++; 
	}
	
	pfDelete(tmp_cset);
    }
    else
    {
        csb->read_error = TRUE;
    }
    
    if( s.name_size != -1 )
    {
       load_name(cset,  s.name_size,  csb );
    }

    return( cset );
    
}   // end load_cset()

static pfVec3 *load_nset(int /*id*/, csdFile_csb *csb)
{
    pfVec3 *nset;
    arrayset_t s;
    
    CSB_FREAD(&s, sizeof(arrayset_t), 1, csb->fp);
    
    if (s.array_type == ARRAY_TYPE_3F)
    {
       nset = (pfVec3 *)pfMalloc( sizeof(pfVec3)*s.size, 
                                  csb->arena );
       CSB_FREAD(nset, sizeof(pfVec3), s.size, csb->fp);
    }
    else
    {
	   csb->read_error = TRUE;
    }
    
    if( s.name_size != -1 )
    {
       load_name(nset,  s.name_size,  csb );
    }

    //printf("loaded nset with %d normals \n",  s.count );
    return( nset );
    
}   // end load_nset()

static void *load_tset(int /*id*/, csdFile_csb *csb)
{
    pfVec2 *tset;
    arrayset_t s;

    CSB_FREAD(&s, sizeof(arrayset_t), 1, csb->fp);

    if (s.array_type == ARRAY_TYPE_2F)
    {
       tset = (pfVec2 *)pfMalloc( sizeof(pfVec2)*s.size,
                                  csb->arena );
       CSB_FREAD(tset, sizeof(pfVec2), s.size, csb->fp);
    }
    else
    {
	csb->read_error = TRUE;
    }

    if( s.name_size != -1 )
    {
       load_name(tset,  s.name_size,  csb );
    }

    return( tset );
    
}   // end load_tset()

static int32 *load_iset(int id, csdFile_csb *csb)
{
    int32 *iset;
    arrayset_t s;
    
    CSB_FREAD(&s, sizeof(arrayset_t), 1, csb->fp);
    
    // remeber size of iset
    csb->iset_size[id] = s.size;
    
    if( s.array_type == ARRAY_TYPE_1I )
    {
	iset = (int32 *)pfMalloc( sizeof(int32)*s.size,
	                          csb->arena );
	CSB_FREAD(iset, sizeof(int32), s.size, csb->fp);
    }
    else
    {
	csb->read_error = TRUE;
    }
    
    if( s.name_size != -1 )
    {
       load_name(iset,  s.name_size,  csb );
    }

    //printf("loaded iset %d size = %d  \n",  id, s.size );
  
    return( iset );
    
}   // end load_iset()



static void load_bpc_data(pfGeoSet * /*gset*/, csdFile_csb *csb)
{
    if (csb->version < CSB_VERSION_BPCULL_DATA)
        return;

    int prim_count;
    CSB_FREAD(&prim_count, sizeof(int), 1, csb->fp);

    unsigned char mode;
    CSB_FREAD(&mode, sizeof(unsigned char), 1, csb->fp);
    // XXX
    // gset->setBPCullPrimNormModeEnable(mode);

    CSB_FREAD(&mode, sizeof(unsigned char), 1, csb->fp);
    // XXX
    // gset->setBPCullDynamicBuildMode(mode);

    if (prim_count >= 0)
    {
        // XXX
        // gset->setBPCullPoint(new csVec3f[prim_count]);
        // CSB_FREAD(gset->getBPCullPoint(),  sizeof(csVec3f), prim_count, csb->fp);
        // gset->setBPCullCenter(new csVec3f[prim_count]);
        // CSB_FREAD(gset->getBPCullCenter(), sizeof(csVec3f), prim_count, csb->fp);
        // gset->setBPCullDelta(new csFloat[prim_count]);
        // CSB_FREAD(gset->getBPCullDelta(),  sizeof(csFloat), prim_count, csb->fp);

        char* vec_buf = new char[sizeof(pfVec3) * prim_count];
        float* float_buf = new float[prim_count];
        CSB_FREAD(vec_buf, sizeof(pfVec3), prim_count, csb->fp);
        CSB_FREAD(vec_buf, sizeof(pfVec3), prim_count, csb->fp);
        CSB_FREAD(float_buf,  sizeof(float), prim_count, csb->fp);
        delete [] vec_buf;
        delete [] float_buf;
    }
}


static pfGeoSet *load_lineset(int /*id*/, csdFile_csb *csb)
{
    pfGeoSet *lineset;
    lineset_t s;
    
    CSB_FREAD(&s, sizeof(lineset_t), 1, csb->fp);
    
    lineset = new pfGeoSet();
    
    set_geoset_data(lineset, (geoset_t *)&s, csb);
    lineset->setPrimType( PFGS_LINES );
    lineset->setLineWidth(s.width);
    
    load_bpc_data(lineset, csb);

    if( s.name_size != -1 )
    {
       load_name(lineset,  s.name_size,  csb );
    }

    return(lineset);
    
}   // end load_lineset()

static pfGeoSet *load_linestripset(int /*id*/, csdFile_csb *csb)
{
    pfGeoSet *linestripset;
    linestripset_t s;
    int32 *lengths;
   
    CSB_FREAD(&s, sizeof(linestripset_t), 1, csb->fp);
    
    linestripset = new pfGeoSet();
    
    set_geoset_data(linestripset, (geoset_t *)&s, csb);
    linestripset->setPrimType( PFGS_LINESTRIPS );
    linestripset->setLineWidth(s.width);
 
    lengths = (int32 *)pfMalloc( sizeof(int32)*s.num_lengths, 
                                 csb->arena );
    CSB_FREAD(lengths, sizeof(int32), s.num_lengths, csb->fp);
    linestripset->setPrimLengths( lengths );
    
    load_bpc_data(linestripset, csb);

    if( s.name_size != -1 )
    {
       load_name(linestripset,  s.name_size,  csb );
    }

    return(linestripset);

}   // end load_linestripset()

static pfGeoSet *load_pointset(int /*id*/, csdFile_csb *csb)
{
    pfGeoSet *pointset;
    pointset_t s;
    
    CSB_FREAD(&s, sizeof(pointset_t), 1, csb->fp);
    
    pointset = new pfGeoSet();
    
    set_geoset_data(pointset, (geoset_t *)&s, csb);
    pointset->setPrimType( PFGS_POINTS );
    pointset->setPntSize(s.size);
    
    load_bpc_data(pointset, csb);

    if( s.name_size != -1 )
    {
       load_name(pointset,  s.name_size,  csb );
    }

    return( pointset );
    
}   // end load_pointset()

static pfGeoSet *load_polyset(int /*id*/, csdFile_csb *csb)
{
    pfGeoSet *polyset;
    polyset_t s;
    int32 *lengths;
    
    CSB_FREAD(&s, sizeof(polyset_t), 1, csb->fp);
    
    polyset = new pfGeoSet();
    set_geoset_data(polyset, (geoset_t *)&s, csb);
    polyset->setPrimType( PFGS_POLYS );
    
    lengths = (int32 *)pfMalloc( sizeof(int32)*s.num_lengths, 
                                 csb->arena );
    CSB_FREAD(lengths, sizeof(int32), s.num_lengths, csb->fp);
    polyset->setPrimLengths( lengths );

    load_bpc_data(polyset, csb);

    if (s.name_size != -1)
    {
        load_name(polyset, s.name_size, csb);
    }

    return( polyset );
    
}   // end load_polyset()

static pfGeoSet *load_quadset(int /*id*/, csdFile_csb *csb)
{
    pfGeoSet *quadset;
    quadset_t s;
    
    CSB_FREAD(&s, sizeof(quadset_t), 1, csb->fp);
    
    quadset = new pfGeoSet();
    set_geoset_data(quadset, (geoset_t *)&s, csb);
    quadset->setPrimType( PFGS_QUADS );
    
    //printf("quadset loaded \n" );
    //print_gset_data( (geoset_t *)&s );
    
    load_bpc_data(quadset, csb);

    if( s.name_size != -1 )
    {
       load_name(quadset,  s.name_size,  csb );
    }

    return( quadset );
    
}   // end load_quadset()

static pfGeoSet *load_triset(int /*id*/, csdFile_csb *csb)
{
    pfGeoSet *triset;
    triset_t s;
    
    CSB_FREAD(&s, sizeof(triset_t), 1, csb->fp);
    
    triset = new pfGeoSet();
    
    set_geoset_data(triset, (geoset_t *)&s, csb);
    triset->setPrimType( PFGS_TRIS );

    load_bpc_data(triset, csb);

    if( s.name_size != -1 )
    {
	load_name( triset, s.name_size, csb );
    }

    return(triset);
    
}   // end load_triset()

static pfGeoSet *load_tristripset(int /*id*/, csdFile_csb *csb)
{
    pfGeoSet *tristripset;
    tristripset_t s;
    int32 *lengths;
    int32 sum;

    //printf("load tristripset \n");

    CSB_FREAD(&s, sizeof(tristripset_t), 1, csb->fp);
 
    //print_gset_data( (geoset_t *)&s );
    //printf("num_lengths = %d \n",  s.num_lengths );

    lengths = (int32 *)pfMalloc( sizeof(int32)*s.num_lengths, 
                                  csb->arena );
    if( s.num_lengths && lengths == NULL )
    {
	pfNotify( PFNFY_NOTICE, PFNFY_PRINT, 
                  "pfMalloc error \n");
    }
				    
    CSB_FREAD(lengths, sizeof(int32), s.num_lengths, csb->fp);
   
     	         
    tristripset = new pfGeoSet;
    if( tristripset == NULL )
    {
	pfNotify( PFNFY_NOTICE, PFNFY_PRINT, 
                  "load_tristrip: error new pfGeoSet \n");	
    }

    set_geoset_data( tristripset, (geoset_t *)&s, csb );
    tristripset->setPrimType( PFGS_TRISTRIPS );
    tristripset->setPrimLengths( lengths );
    
    load_bpc_data(tristripset, csb);

    if( s.name_size != -1 )
    {
	load_name( tristripset, s.name_size, csb );
    }

    return(tristripset);
    
}   // end load_tristripset()

static void *load_geometry(int /*id*/, csdFile_csb *csb)
{
    void *geometry;
    geometry_ref_t g;
   
    CSB_FREAD(&g, sizeof(geometry_ref_t), 1, csb->fp);
    //printf("load geometry refrence id =%d \n",  id );
   
    if (g.list_id != -1)
    {
        //printf("list id = %d \n",  g.list_id );
        //printf("list_item = %d \n",  g.list_item );
	geometry = (void *)csb->rl_list[g.list_id][g.list_item];
    }
    else
    {
        printf("list id = %d \n",  g.list_id );
	geometry = NULL;
    }

   return(geometry);
       
}   // end load_geometry()


static void *load_engine(int /* id */, csdFile_csb *csb)
{
    engine_ref_t e;

    CSB_FREAD(&e, sizeof(engine_ref_t), 1, csb->fp);

    /*
    if (e.list_id != -1)
    {
        engine = (csEngine *)csb->rl_list[e.list_id][e.list_item];
        if (csb->engine_list != NULL)
            csb->engine_list->append(engine);
    }
    else
        engine = NULL;
    */

    return(NULL);
}


#define LOAD_INTERPOLATOR(size)                                         \
    interpolator_t i;                                                   \
    float32 *k, *kv;                                                    \
                                                                        \
    CSB_FREAD(&i, sizeof(interpolator_t), 1, csb->fp);                  \
                                                                        \
    k = new float32[i.num_keys];					\
    CSB_FREAD(k, sizeof(float32), i.num_keys, csb->fp);                 \
    delete k;								\
									\
    kv = new float32[i.num_keyValues * size];				\
    CSB_FREAD(kv, sizeof(float32), i.num_keyValues * size, csb->fp);    \
    delete kv;								\
                                                                        \
    if (i.name_size != -1)                                              \
        load_name(interp, i.name_size, csb);                            \


static void *load_norminterp(int /* id */, csdFile_csb *csb)
{
    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csNormalInterpolator");

    void *interp = NULL;
    LOAD_INTERPOLATOR(3);
    return(interp);
}

static void *load_posinterp(int /* id */, csdFile_csb *csb)
{
    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csPositionInterpolator");

    void *interp = NULL;
    LOAD_INTERPOLATOR(3);
    return(interp);
}

static void *load_scalarinterp(int /* id */, csdFile_csb *csb)
{
    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csScalarInterpolator");

    void *interp = NULL;
    LOAD_INTERPOLATOR(1);
    return(interp);
}

static void *load_colorinterp(int /* id */, csdFile_csb *csb)
{
    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csColorInterpolator");

    void *interp = NULL;
    LOAD_INTERPOLATOR(3);
    return(interp);
}

static void *load_coordinterp(int /* id */, csdFile_csb *csb)
{
    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csCoordinateInterpolator");

    void *interp = NULL;
    LOAD_INTERPOLATOR(3);
    return(interp);
}


static void *load_orientinterp(int /* id */, csdFile_csb *csb)
{
    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csOrientationInterpolator");

    void *interp = NULL;

    interpolator_t i;
    float32 *k, *kv;

    CSB_FREAD(&i, sizeof(interpolator_t), 1, csb->fp);

    k = new float32[i.num_keys];
    CSB_FREAD(k, sizeof(float32), i.num_keys, csb->fp);
    delete [] k;

    kv = new float32[i.num_keyValues * 4];
    CSB_FREAD(kv, sizeof(float32), i.num_keyValues * 4, csb->fp);
    delete [] kv;

    if (i.name_size != -1)
        load_name(interp, i.name_size, csb);

    return(interp);
}


#define LOAD_MORPHENG(size)                                             \
    morpheng_t m;                                                       \
    int *c, *i;                                                 	\
    float32 *w, *in;                                                    \
                                                                        \
    CSB_FREAD(&m, sizeof(morpheng_t), 1, csb->fp);                      \
                                                                        \
    c = new int[m.num_counts];						\
    CSB_FREAD(c, sizeof(int), m.num_counts, csb->fp);                   \
    delete [] c;							\
                                                                        \
    i = new int[m.num_indices];						\
    CSB_FREAD(i, sizeof(int), m.num_indices, csb->fp);          	\
    delete [] i;							\
                                                                        \
    w = new float32[m.num_weights];					\
    CSB_FREAD(w, sizeof(float32), m.num_weights, csb->fp);              \
    delete [] w;							\
                                                                        \
    in = new float32[m.num_inputs * size ];		 		\
    CSB_FREAD(in, sizeof(float32), m.num_inputs * size, csb->fp);       \
    delete [] in;							\
									\
    if (m.name_size != -1)                                              \
        load_name(morph, m.name_size, csb);                             \


static void *load_morpheng3f(int /* id */, csdFile_csb *csb)
{
    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csMorphEng3f");

    void *morph = NULL;
    LOAD_MORPHENG(3);
    return (morph);
}

static void *load_morpheng4f(int /* id */, csdFile_csb *csb)
{
    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csMorphEng4f");

    void *morph = NULL;
    LOAD_MORPHENG(4);
    return (morph);
}

static void *load_selecteng3f(int /* id */, csdFile_csb *csb)
{
    selecteng_t s;
    float32 *in;
    void *select = NULL;

    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csSelectEng3f");

    CSB_FREAD(&s, sizeof(selecteng_t), 1, csb->fp);

    in = new float32[s.num_inputs * 3];
    CSB_FREAD(in, sizeof(float32), s.num_inputs * 3, csb->fp);
    delete in;

    if (s.name_size != -1)
        load_name(select, s.name_size, csb);

    return(select);
}

static void *load_selecteng4f(int /* id */, csdFile_csb *csb)
{
    selecteng_t s;
    float32 *in;
    void *select = NULL;

    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csSelectEng4f");

    CSB_FREAD(&s, sizeof(selecteng_t), 1, csb->fp);

    in = new float32[s.num_inputs * 4];
    CSB_FREAD(in, sizeof(float32), s.num_inputs * 4, csb->fp);
    delete [] in;

    if (s.name_size != -1)
        load_name(select, s.name_size, csb);

    return(select);
}

static void *load_transeng3f(int /* id */, csdFile_csb *csb)
{
    transeng_t t;
    int *c, *i;
    float32 *m, *in;
    void *trans = NULL;

    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csTransformEng3f");

    CSB_FREAD(&t, sizeof(transeng_t), 1, csb->fp);

    c = new int[t.num_counts];
    CSB_FREAD(c, sizeof(int), t.num_counts, csb->fp);
    delete [] c;

    i = new int[t.num_indices];
    CSB_FREAD(i, sizeof(int), t.num_indices, csb->fp);
    delete [] i;

    m = new float32[t.num_matrices * 16];
    CSB_FREAD(m, sizeof(float32), t.num_matrices * 16, csb->fp);
    delete [] m;

    in = new float32[t.num_inputs * 3];
    CSB_FREAD(in, sizeof(float32), t.num_inputs * 3, csb->fp);
    delete in;

    if (t.name_size != -1)
        load_name(trans, t.name_size, csb);

    return(trans);
}

static void *load_spline(int /* id */, csdFile_csb *csb)
{
    spline_t s;
    float32 *k;
    void *spline = NULL;

    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csSpline");

    CSB_FREAD(&s, sizeof(spline_t), 1, csb->fp);

    k = new float32[s.num_keys];
    CSB_FREAD(k, sizeof(float32), s.num_keys, csb->fp);
    delete [] k;

    if (s.name_size != -1)
        load_name(spline, s.name_size, csb);

    return(spline);
}

static void *load_timesensor(int /* id */, csdFile_csb *csb)
{
    timesensor_t t;
    void *sensor = NULL;

    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csTimeSensor");

    CSB_FREAD(&t, sizeof(timesensor_t), 1, csb->fp);

    if (t.name_size != -1)
        load_name(sensor, t.name_size, csb);

    return(sensor);
}


csdCustomObject *loadCustomHeader(char *customName, csdFile_csb *csb)
{
    CSB_FREAD(customName, CSD_CUSTOMNAME, 1, csb->fp);
    return (csdCustomObject *)(headCustomList->findCustomObject(customName));
    
}   // end loadCustomHeader()



static pfNode *load_node(int /*id*/, csdFile_csb *csb)
{
    int32 n_id;
    pfNode *node;
    pfType *node_type;
    all_nodes_t n;
    char customName[CSD_CUSTOMNAME];
    csdCustomObject *registeredObj = NULL;
    int i;
    int32 *children;
   
    node_type = pfNode::getClassType();
    CSB_FREAD(&n_id, sizeof(int32), 1, csb->fp);
    switch(n_id)
    {
	case L_CUSTOM:   // Must be a custom object 
	  if( registeredObj = loadCustomHeader(customName,csb) )
	  {
	     //printf("found a registered object \n");
	     node = (pfNode *)registeredObj->new_func();
	  }
	  else
	  {
              pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	           "csdLoadFile_csb: Node type %s not found in registered types\n", 
	           customName);
              return NULL;
	  }
	  break;
	case N_GROUP:
	  node = new pfGroup();
	  node_type = pfGroup::getClassType();
	  //printf("node is a Group \n");
	  break;
	case N_SHAPE : 
	  node = new pfGeode();
	  node_type = pfGeode::getClassType();
          //printf("node is a Geode \n");
	  break;
	case N_SWITCH: 
	  node = new pfSwitch();
	  node_type = pfSwitch::getClassType();
	  //printf("node is a Switch \n");
	  break;
	case N_TRANSFORM: 
	  node = new pfDCS();
	  node_type = pfDCS::getClassType();
	  //printf("node is a DCS \n");	  	  
	  break;
	case N_LOD: 
	   node = new pfLOD();
	   node_type = pfLOD::getClassType();
	   //printf("node is a LOD \n");
	  break;
        case N_BILLBOARD:
	   // XXX billboard is not implemented yet
           // node = new pfBillboard();
	   // node_type = pfBillboard::getClassType();
           // break;
	   node = new pfGroup();
	   csBillboard_load(node, csb);
	   return node;
        case N_ENVIRONMENT:
	   // XXX environment is not implemented yet
           // node = new csEnvironment;
	   // node_type = csEnvironment::getClassType();
           // break;
	   node = new pfGroup();
	   csEnvironment_load(node, csb);
	   return node;
        case N_DIRECTIONAL_LIGHT:
	   // XXX directional light is not implemented yet
           // node = new csDirectionalLight;
	   // node_type = csDirectionalLight::getClassType();
           // break;
	   csDirectionalLight_load(NULL, csb);
	   return NULL;
        case N_POINT_LIGHT:
	   // XXX point light is not implemented yet
           // node = new csPointLight;
	   // node_type = csPointLight::getClassType();
           // break;
	   csPointLight_load(NULL, csb);
	   return NULL;
        case N_SPOT_LIGHT:
	   // XXX spot light is not implemented yet
           // node = new csSpotLight;
	   // node_type = csSpotLight::getClassType();
           // break;
	   csSpotLight_load(NULL, csb);
	   return NULL;
        case N_FOG:
	   // XXX fog is not implemented yet
           // node = new csFog;
	   // node_type = csFog::getClassType();
           // break;
	   csFog_load(NULL, csb);
	   return NULL;

        default:
	break;
    } 

   if( csb->version >= CSB_VERSION_CUSTOMSTORELOAD )
   {
      //printf("version >= CSB_VERSION_CUSTOMSTORELOAD \n");
      if (registeredObj == NULL)
      {
         //printf("registers object == NULL looking by type \n");
         registeredObj = headCustomList->findCustomObject(node_type);
      }
      
      if (registeredObj)
      {
         //printf("registeredObj is non null loading \n");
         registeredObj->load_func(node,csb);
      }
      else
      {
         //printf("registeredObj is NULL ,  what to do \n");
         node = NULL;
      }
 
   }
   else
   { 
        // for older versions, there are only 5 node types - this is faster
        // need to check in order of derivation
        if (node_type->isDerivedFrom(pfLOD::getClassType()))
        {
            csLOD_load(node, csb);
        }
        else if (node_type->isDerivedFrom(pfSwitch::getClassType()))
        {
            csSwitch_load(node, csb);
        }
        else if (node_type->isDerivedFrom(pfDCS::getClassType()))
        {
            csTransform_load(node, csb);
        }
        else if (node_type->isDerivedFrom(pfGroup::getClassType()))
        {
            pfGroup_load(node, csb);
        }
        else if (node_type->isDerivedFrom(pfGeode::getClassType()))
        {
            csShape_load(node, csb);
        }
   }
    
   //printf("loaded node \n" );
   //pfPrint(node, (PFTRAV_SELF), PFPRINT_VB_DEBUG, NULL);

   return(node);
       
}   // end load_node()

static pfGeoSet *load_box(int /*id*/, csdFile_csb *csb)
{
    pfGeoSet *gset;  
    pfMatrix m;
    box_t b;
    
    //pfNotify( PFNFY_WARN, PFNFY_PRINT,  "just say no to csBox\n" );
   
    CSB_FREAD(&b, sizeof(box_t), 1, csb->fp);
   
    gset = pfdNewCube(csb->arena);
    
    m.makeTrans( b.center[0], b.center[1], b.center[2] );
    m.postScale( m,  b.size[0], b.size[1],  b.size[2] );
    
    pfdXformGSet( gset,  m );
    
    if (b.name_size != -1)
    {
	load_name(gset, b.name_size, csb);
    }
    
    return(gset);
   
}   // end load_box()

static pfGeoSet *load_cone(int /*id*/, csdFile_csb *csb)
{
    pfGeoSet *gset;
    pfMatrix m;
    cone_t c;  
   
    //pfNotify( PFNFY_WARN,  PFNFY_PRINT,  "just say no to csCone\n" );
   
    CSB_FREAD(&c, sizeof(cone_t), 1, csb->fp); 
   
    c.height /= 2.0f;

    gset = pfdNewCone( 400,  csb->arena );
    
    m.makeTrans( c.center[0], c.center[1], c.center[2] );
    m.postScale( m,  c.bottom_radius, c.bottom_radius,  c.height );

    pfdXformGSet( gset,  m );
  
    if (c.name_size != -1)
    {
	load_name(gset, c.name_size, csb);
    }
    
    return(NULL);
      
}   // end load_cone()

static pfGeoSet *load_cylinder(int /*id*/, csdFile_csb *csb)
{
    pfGeoSet *gset;
    pfMatrix m;
    cylinder_t c;
    
    //pfNotify( PFNFY_WARN,  PFNFY_PRINT,  "just say no to csCylinder\n" ); 
       
    CSB_FREAD(&c, sizeof(cylinder_t), 1, csb->fp);

    c.height /= 2.0f;
    
    gset = pfdNewCylinder( 400,  csb->arena );
    m.makeTrans( c.center[0], c.center[1], c.center[2] );
    m.postScale( m, c.radius, c.radius, c.height );
  
    pfdXformGSet( gset,  m );
  
    if (c.name_size != -1)
    {
	load_name(gset, c.name_size, csb);
    }

    return(gset);
    
}   // end load_cylinder()

static pfGeoSet *load_sphere(int /*id*/, csdFile_csb *csb)
{
    pfGeoSet *gset;
    pfMatrix m;
    sphere_t s;
    
    CSB_FREAD(&s, sizeof(sphere_t), 1, csb->fp);
    
    gset = pfdNewSphere( 500, csb->arena );
    
    m.makeTrans( s.center[0], s.center[1], s.center[2] );
    m.scale(  s.radius, m );
   
    pfdXformGSet( gset,  m );

    if (s.name_size != -1)
    {
	load_name(gset, s.name_size, csb);
    }

    return(gset);
    
}   // end load_sphere()

// how do children get added to a sprite ?
static pfBillboard *load_sprite(int /*id*/, csdFile_csb *csb)
{
   sprite_t s; 
   pfBillboard *sprite;
   
   CSB_FREAD(&s, sizeof(sprite_t), 1, csb->fp);
   
   pfNotify( PFNFY_WARN, PFNFY_PRINT,  "csSprite not supported\n" );

   sprite = new pfBillboard();
   
   return(sprite); 
   
}   // end load_sprite()

static pfGeoSet *load_trifanset(int /*id*/, csdFile_csb *csb)
{
    pfGeoSet *trifanset;
    trifanset_t s;
    int32 *lengths;
    pfVec3 *vert = NULL;
    pfVec3 *norm = NULL;
    pfVec2 *tex_coord = NULL;
    void *color = NULL;
 
    //printf("load trifan !!! \n");
    CSB_FREAD(&s, sizeof(trifanset_t), 1, csb->fp);

    lengths = (int32 *)pfMalloc( sizeof(int32)*s.num_lengths,  
                                 csb->arena );
    CSB_FREAD(lengths, sizeof(int32), s.num_lengths, csb->fp);

    trifanset = new pfGeoSet();
    set_geoset_data( trifanset, (geoset_t *)&s, csb );
  
    // finish up geoset info
    trifanset->setPrimType( PFGS_TRIFANS );
  
    trifanset->setPrimLengths( lengths );

    load_bpc_data(trifanset, csb);

    if (s.name_size != -1)
    {
	load_name(trifanset, s.name_size, csb);
    }

    return(trifanset);
   
}   // end load_trifanset()

static void *load_connection(int /*id*/, csdFile_csb *csb)
{
   int32   storage[6];
   //void   *fromCtr;
   //void   *toCtr;

   CSB_FREAD(storage, sizeof(int32), 6, csb->fp);

   //fromCtr = (void *) csb->rl_list[storage[0]][storage[1]];
   //toCtr = (void *) csb->rl_list[storage[3]][storage[4]];

   pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported: field connections\n");

   //fromCtr->connect( storage[2], toCtr, storage[5] );

   return(NULL);   // L_CONNECTIONS is NULLs after loading.
   
}   // end load_connection


// END MAIN LOAD FUNCTIONS
//-------------------------------------------------------------------------
// LOAD HELPER FUNCTIONS

static void load_name(void */*container*/, int name_size, csdFile_csb *csb)
{
    if (name_size > 0)
    {
        CSB_FREAD((char *)csb->buf, name_size, 1,  csb->fp);
        ((char *)csb->buf)[name_size] = '\0';
        //pfNotify( PFNFY_NOTICE, PFNFY_PRINT,  "load name %s \n",  csb->buf );
    }
}   // end load_name()

static void load_node_name(pfNode *node, int name_size, csdFile_csb *csb)
{
    if (name_size > 0)
    {
        CSB_FREAD((char *)csb->buf, name_size, 1,  csb->fp);
        ((char *)csb->buf)[name_size] = '\0';
	node->setName( (char *)csb->buf );
        //pfNotify( PFNFY_NOTICE, PFNFY_PRINT,  "load name %s \n",  csb->buf );
    }
}   // end load_name()


// cosmo seems to support mixed index and non indexed geo sets
// and performer does not so convert everything to non-indexed

static void set_geoset_data( pfGeoSet *gset,  geoset_t *g,  csdFile_csb *csb)
{
    int32 *index_ptr;
    int32 index_size;
    pfVec3 *vert = NULL;
    pfVec3 *norm = NULL;
    pfVec2 *tex_coord = NULL;
    pfVec4 *color = NULL;
 
    // find pointers to attribute lists
    if( g->coords != -1 )
    {
        vert = (pfVec3 *)csb->rl_list[L_VSET][g->coords];
    }
    if( g->normals != -1 )
    {
        norm = (pfVec3 *)csb->rl_list[L_NSET][g->normals];
    }
    if( g->tex_coords != -1 )
    {
       tex_coord = (pfVec2 *)csb->rl_list[L_TSET][g->tex_coords];
    }
    if( g->colors != -1 )
    {
	color = (pfVec4 *)csb->rl_list[L_CSET][g->colors];
    }
    
    // assign attribute lists
    if( g->coord_index != -1 )
    {
       index_ptr = (int32 *)csb->rl_list[L_ISET][g->coord_index];
       index_size = csb->iset_size[g->coord_index];
       pfVec3 *new_vert = (pfVec3 *)pfMalloc( sizeof(pfVec3)*index_size,
			    csb->arena );

      for( int i = 0; i < index_size; i ++ )
      {
         new_vert[i] = vert[*index_ptr];
	 index_ptr++;
	 
      }   // end for each new vertex
       
       gset->setAttr( PFGS_COORD3,  PFGS_PER_VERTEX,  new_vert,  
                           NULL );

       // delete old vertex array, we might use it or index again ?
       // I guess clean up all indexes at end of load ??
       // this is a leak here and at the 4 spots below
       // need to to refrence counting on arrays
       // but should be able to just delete all indexes
    }
    else
    {
	gset->setAttr( PFGS_COORD3,  PFGS_PER_VERTEX,  vert,  NULL );	
    }
    
    if( g->normal_index != -1)
    {
        index_ptr = (int32 *)csb->rl_list[L_ISET][g->normal_index];
	index_size = csb->iset_size[g->normal_index];
	
	pfVec3 *new_norm = (pfVec3 *)pfMalloc( sizeof(pfVec3)*index_size,
			                       csb->arena );

        for( int i = 0; i < index_size; i ++ )
        {
             new_norm[i] = norm[*index_ptr];
	     index_ptr++;
	 
        }   // end for each new normal
  
        gset->setAttr( PFGS_NORMAL3, gnb_table[g->normal_bind],  new_norm,  
	                    NULL );	 
    }
    else
    {
	if (g->normals != -1)
	    gset->setAttr( PFGS_NORMAL3, gnb_table[g->normal_bind],  norm,  
	                    NULL );
    }
 
    if( g->color_index != -1)
    {
        index_ptr = (int32 *)csb->rl_list[L_ISET][g->color_index];
	index_size = csb->iset_size[g->color_index];
	
	pfVec4 *new_color = (pfVec4 *)pfMalloc( sizeof(pfVec4)*index_size,
			                        csb->arena );

        for( int i = 0; i < index_size; i ++ )
        {
             new_color[i] = color[*index_ptr];
	     index_ptr++;
	 
        }   // end for each new normal

       gset->setAttr( PFGS_COLOR4, gcb_table[g->color_bind],  new_color,  
	                    NULL );
    }
    else
    {
	if (g->colors != -1)
	    gset->setAttr( PFGS_COLOR4, gcb_table[g->color_bind],  color,  
	                    NULL );
    }

    if( g->tex_coord_index != -1)
    {
        index_ptr = (int32 *)csb->rl_list[L_ISET][g->tex_coord_index];
	index_size = csb->iset_size[g->tex_coord_index];
	
	pfVec2 *new_tex_coord = (pfVec2 *)pfMalloc( sizeof(pfVec2)*index_size,
			                        csb->arena );

        for( int i = 0; i < index_size; i ++ )
        {
             new_tex_coord[i] = tex_coord[*index_ptr];
	     index_ptr++;
	 
        }   // end for each new normal

        gset->setAttr( PFGS_TEXCOORD2, gtb_table[g->tex_coord_bind], 
	                new_tex_coord, NULL );
    }
    else
    {
	if (g->tex_coords != -1)
	    gset->setAttr( PFGS_TEXCOORD2, gtb_table[g->tex_coord_bind], 
			   tex_coord, NULL );
    }
    
    // finish up geoset info

    gset->setNumPrims(g->prim_count );
    
}   // end set_geoset_data()

// set node bounding sphere mode

static void set_node_data(pfNode *node, node_t *g, csdFile_csb */*csb*/)
{
    node->setBound(NULL, nbm_table[g->bound_mode]);
    if (g->sphere_bound != -1)
    {
	// XXX;

    }
    // does nothing in the cscsb loader
    //set_container_data(node, (container_t *)g, csb);
    
}   // end set_node_data()

// convert a geo set to flat shaded.
// right now all geo sets are non-index

static void convert_to_flat_shade( pfGeoSet *gset, csdFile_csb *csb  )
{
   int prim_type;
 
   prim_type = gset->getPrimType();
  
   if( prim_type == PFGS_TRISTRIPS )
   {
       if( gset->getAttrBind(PFGS_NORMAL3) == PFGS_PER_VERTEX )
       {
           fix_tristrip_normals( gset, csb ); 
       }
       gset->setPrimType(PFGS_FLAT_TRISTRIPS);
   }
   
   if( prim_type == PFGS_LINESTRIPS )
   {
       if( gset->getAttrBind(PFGS_NORMAL3) == PFGS_PER_VERTEX )
       {
           fix_linestrip_normals( gset, csb );
       }
       gset->setPrimType(PFGS_FLAT_LINESTRIPS);
   }
   
}   // end convert_to_flat_shade(

static void fix_tristrip_normals( pfGeoSet *gset,  csdFile_csb *csb  )
{
   int i;
   int k;
   int sum;
   int prim_count;
   pfVec3 *n_ptr;
   pfVec3 *norm;
   ushort *index;
   int32 *lengths; 

   gset->getAttrLists( PFGS_NORMAL3, (void **)&n_ptr, &index  );
   lengths = gset->getPrimLengths(); 
   prim_count = gset->getNumPrims();
   
   sum = 0;
   for( i = 0; i < prim_count; i++ )
   {
       sum = sum + (PF_ABS(lengths[i]) - 2);
   }
   
   pfVec3 *new_n_ptr = (pfVec3 *)pfMalloc( sizeof(pfVec3)*sum, 
                                           csb->arena );
   norm = new_n_ptr;
					   
    for( i = 0; i < prim_count; i++ )
    {
        n_ptr++; 
	n_ptr++;
	
	for(k = 2; k < PF_ABS(lengths[i]); k++ )
	{
	    new_n_ptr->vec[0] = n_ptr->vec[0];
	    new_n_ptr->vec[1] = n_ptr->vec[1];
	    new_n_ptr->vec[2] = n_ptr->vec[2];
	    new_n_ptr++;
	    n_ptr++;
	}
    }
    
    // set pointer to new normal array and delete old one
    
    gset->setAttr( PFGS_NORMAL3, PFGS_PER_VERTEX, norm, NULL );
    pfDelete( n_ptr );
   
}   // end fix_tristrip_normals()

static void fix_linestrip_normals( pfGeoSet *gset,  csdFile_csb *csb  )
{
   int i;
   int k;
   int sum;
   int prim_count;
   pfVec3 *n_ptr;
   pfVec3 *norm;
   ushort *index;
   int32 *lengths; 
   
   gset->getAttrLists( PFGS_NORMAL3, (void **)&n_ptr, &index  );
   lengths = gset->getPrimLengths(); 
   prim_count = gset->getNumPrims();
   
   sum = 0;
   for( i = 0; i < prim_count; i++ )
   {
       sum = sum + (PF_ABS(lengths[i]) - 1);
   }
   
   pfVec3 *new_n_ptr = (pfVec3 *)pfMalloc( sizeof(pfVec3)*sum, 
                                           csb->arena );
   norm = new_n_ptr;
					   
    for( i = 0; i < prim_count; i++ )
    {
        n_ptr++; 
	
	for(k = 1; k < PF_ABS(lengths[i]); k++ )
	{
	    new_n_ptr->vec[0] = n_ptr->vec[0];
	    new_n_ptr->vec[1] = n_ptr->vec[1];
	    new_n_ptr->vec[2] = n_ptr->vec[2];
	    
	    new_n_ptr++;
	    n_ptr++;
	}
    }
    
    // set pointer to new normal array and delete old one
    
    gset->setAttr( PFGS_NORMAL3, PFGS_PER_VERTEX, norm, NULL );
    pfDelete( n_ptr );
   
}   // end fix_linestrip_normals()

//-------------------------------------------------------------------
// debug functions

static void print_gset_data( geoset_t *g )
{
    printf("prim_count = %d \n",  g->prim_count );
    printf("normal_bind = %d \n",  g->normal_bind );
    printf("color_bind = %d \n",  g->color_bind );
    printf("tex_coord_bind = %d\n",  g->tex_coord_bind );
    printf("coords = %d \n", g->coords );
    printf("normals = %d \n", g->normals );
    printf("colors = %d \n", g->colors );
    printf("tex_coords = %d \n", g->tex_coords );
    printf("coord_index = %d \n", g->coord_index );
    printf("normal_index = %d \n", g->normal_index );
    printf("color_index = %d \n", g->color_index );
    printf("tex_coord_index = %d \n", g->tex_coord_index );
    printf("cull_face = %d \n\n", g->cull_face );

}   // end print_geoset_data()

static void print_appearance_data ( appearance_t *a )
{
    printf("inherit = %X \n", a->inherit );
    printf("tex = %d \n", a->tex );
    printf("tex_enable = %d \n", a->tex_enable );
    printf("tex_mode = %d \n", a->tex_mode );
    printf("tex blend color %f \t %f \t %f \t %f \n", a->tex_bcolor[0],
            a->tex_bcolor[1], a->tex_bcolor[2], a->tex_bcolor[3] );
    printf("tenv = %d \n", a->tenv);
    printf("tgen = %d \n", a->tgen);
    printf("tgen_enable = %d \n", a->tgen_enable);
    printf("mtl = %d \n", a->mtl);
    printf("light_enable = %d \n", a->light_enable);
    printf("shade_model = %d \n", a->shade_model);
    printf("transp_enable = %d \n", a->transp_enable);
    printf("transp_mode = %d \n", a->transp_mode);
    printf("alpha_func = %d \n", a->alpha_func);
    printf("alpha_ref = %d \n", a->alpha_ref);
    printf("bcolor  %f \t %f \t %f \t %f \n",  a->bcolor[0], a->bcolor[1], 
            a->bcolor[2],  a->bcolor[3] );
    printf("src_bfunc = %d \n", a->src_bfunc);
    printf("dst_bfunc = %d \n", a->dst_bfunc);
    printf("color_mask = %d \t %d \t %d \t %d \n", a->color_mask[0], 
            a->color_mask[1], a->color_mask[2], a->color_mask[3] );
    printf("depth_func = %d \n", a->depth_func);
    printf("depth_mask = %d \n", a->depth_mask);
    printf("fog_enable = %d \n", a->fog_enable);
    printf("poly_mode = %d \n", a->poly_mode);
    printf("line_stipple_pattern = %d \n", a->line_stipple_pattern);
    printf("line_stipple_factor = %d \n", a->line_stipple_factor);
    //float32 tex_transform[16]
    printf("back_mtl = %d \n", a->back_mtl);
    
}   // end print_appearance_data()

static void print_material( material_t *m )
{
    printf("amient[%f, %f, %f] \n", m->ambient[0], m->ambient[1], m->ambient[2] );
    printf("diffuse[%f, %f, %f] \n", m->diffuse[0], m->diffuse[1], m->diffuse[2] );
    printf("specular[%f, %f, %f] \n", m->specular[0], m->specular[1], m->specular[2] );
    printf("emissive[%f, %f, %f] \n", m->emissive[0],  m->emissive[1], m->emissive[2] );
    printf("shininess = %f \n", m->shininess );
    printf("transparency = %f \n", m->transparency);
    //printf("ambient_index = %d \n", m->ambient_index );
    //printf("diffuse_index = %d \n", m->diffuse_index );
    //printf("specular_index = %d \n", m->specular_index );

}   // end print_material()

static void print_lod( cslod_t *l )
{
   printf("CSLOD_DATA \n");
   printf("   offset = %d \n", l->offset );
   printf("   numRanges = %d \n", l->numRanges );
   printf("   CS_MAX_LOD_RANGES+1 = %d \n", CS_MAX_LOD_RANGES+1 );
   for( int i = 0; i < CS_MAX_LOD_RANGES; i++ )
   {
      printf("   ranges[%d] = %f \n", i, l->ranges[i] );
   }
   printf("center = %f %f %f \n", l->center[0], l->center[1], l->center[2] );
   printf("SWITCH_DATA \n");
   printf("   which_child = %d \n", l->which_child );
   printf("GROUP_DATA \n");
   printf("   num_children = %d \n", l->num_children );
   printf("NODE_DATA \n");
   printf("   bound_mode = %d \n", l->bound_mode );
   printf("   sphere_bound = %d \n", l->sphere_bound );
   printf("CONTAINER_DATA \n");
   printf("   name_size = %d \n", l->name_size );
   printf("   udata = %d \n", l->udata );
   printf("OBJECT_DATA is NULL define \n" );

}   // end print_lod()


//--------------------------------------------------------------------------------
//---------- Custom Cosmo Objects -----------------------------------------------
//--------------------------------------------------------------------------------
 


csdCustomObject::csdCustomObject(pfType *_type, const char *_name,
                        csdDescendCustomObjFuncType_csb _descend_func,
                        csdStoreCustomObjFuncType_csb _store_func,
                        csdNewCustomObjFuncType_csb _new_func,
                        csdLoadCustomObjFuncType_csb _load_func) 
: type(_type), 
  descend_func(_descend_func), store_func(_store_func),
  new_func(_new_func),load_func(_load_func)
{
     strcpy(name,(char *)_name), 
     next = NULL;
     
}   // end csdCustomObject::csdCustomObject()

void csdCustomObject::addCustomObject(csdCustomObject *obj)
{
    //Insert into list;
    csdCustomObject *tmp = this->next;
    this->next = obj;
    obj->next = tmp;

}   //end csdCustomObject::addCustomObject

csdCustomObject *csdCustomObject::findCustomObject(const char *name)
{
   csdCustomObject *list = headCustomList;
   while (list)
   {
	if (!strcmp(list->name,name))
	     return list;
	
	else 
	    list = list->next;
   }
   
   return NULL;
   
}   // end csdCustomObject::findCustomObject() by name

csdCustomObject *csdCustomObject::findCustomObject(pfType *_type)
{
   csdCustomObject *list = headCustomList;
   while (list)
   { 
	if (list->type == _type)
	{
	     return list;
	}
	else
	{
	    list = list->next;
	}
   }
   
   return NULL;
   
}   // end csdCustomObject::findCustomObject() by type

pfType *csdCustomObject::getType(void)
{
    return type;
}


/* 
 * Registering custom objects with Cosmo3D
 */
int csdAddCustomObj_csb(pfType *type, const char *name,
			csdDescendCustomObjFuncType_csb descend_func,
			csdStoreCustomObjFuncType_csb store_func,
			csdNewCustomObjFuncType_csb new_func,
			csdLoadCustomObjFuncType_csb load_func)
{

    csdCustomObject *customObj = new csdCustomObject(type,name,descend_func,
			store_func,new_func,load_func); 

    if (headCustomList)
    {
	csdCustomObject *oldObj; 
    	if (oldObj = headCustomList->findCustomObject(type))
	{
    	     // Check if type is already in list
    	     // If so, then user is overriding the default type
	     oldObj->descend_func 	= descend_func; 
	     oldObj->store_func 	= store_func; 
	     oldObj->new_func 		= new_func; 
	     oldObj->load_func 		= load_func; 

	     delete customObj;
	} else
	{
    	     headCustomList->addCustomObject(customObj);
	}

    } else
    {
	headCustomList = customObj;
    }
	
    return 0;
}


int csdDeleteCustomObj_csb(pfType *type)
{
    // XXX Remove type from list of custom objs 

    return -1;
}

//----------------------------------------------------------------------------
// Node loading functions
//----------------------------------------------------------------------------

// ********* Registering csGroup 

static pfGroup *pfGroup_new(void)
{
   return new pfGroup();
   
}   // end pfGroup_new()

static void csdLoadGroupInfo(void *obj, all_nodes_t *n, int structsize, csdFile_csb *csb)
{
    int32 *children;
    int32 num_children;
    int32 i;
    pfGroup *group = (pfGroup *)obj;
    pfNode *node_ptr;
    
#ifdef DEBUG_NODE
printf( "csdLoadGroupInfo \n");
#endif
    
    CSB_FREAD( n, structsize, 1, csb->fp);
    num_children = n->group.num_children;
    
#ifdef DEBUG_NODE
printf("group has %d children \n",  num_children );
#endif

    // Get buffer for children and read in indexes
    children = GET_BUF(num_children);
    CSB_FREAD(children, sizeof(int32), num_children, csb->fp);
    
    for (i = 0; i < num_children; i++)
    {
#ifdef DEBUG_NODE
printf("group adding child %d nod list id %d \n",  i,  children[i] );
#endif
	if (children[i] >= 0 && children[i] <= csb->curListLength)
	{
	    if (csb->rl_list[L_NODE][children[i]] != NULL)
		group->addChild((pfNode *)csb->rl_list[L_NODE][children[i]]);
	}
    } 
}

static int pfGroup_load(void *obj, csdFile_csb *csb)
{
    all_nodes_t n;
#ifdef DEBUG_NODE
printf("load group \n");
#endif
    csdLoadGroupInfo(obj,&n,sizeof(group_t),csb);
    load_node_name((pfNode *) obj, n.node.name_size, csb);

    return 0;

}   // end pfGroup_load()

// ********* Registering csTransform 
static pfDCS *pfDCS_new(void)
{
   return new pfDCS();
   
}   // end pfDCS_new()

static void csdLoadTransformInfo(void *obj, all_nodes_t *n,int structsize,csdFile_csb *csb)
{
   pfMatrix m;
   pfDCS *dcs_ptr;

#ifdef DEBUG_NODE
printf("csdLoadTransformInfo \n" );
#endif

   // Load base class info
   csdLoadGroupInfo( obj, n, structsize,csb);

   // Load csTransform info

   if (csb->version >= CSB_VERSION_TRANSFORM)
   {
	// M = -C * -SO * S * SO * R * C * T

	m.makeTrans(	n->transform.translation[0], 
			n->transform.translation[1], 
			n->transform.translation[2]);

	m.preTrans(	n->transform.center[0],
			n->transform.center[1], 
			n->transform.center[2], 
			m);

	if (n->transform.rotation[3] != 0.0f)
	    m.preRot(	PF_RAD2DEG(n->transform.rotation[3]),
			n->transform.rotation[0], 
			n->transform.rotation[1],
			n->transform.rotation[2], 
			m);

	if (n->transform.scaleOrientation[3] != 0.0f)
	    m.preRot(	PF_RAD2DEG(n->transform.scaleOrientation[3]),
			n->transform.scaleOrientation[0], 
			n->transform.scaleOrientation[1],
			n->transform.scaleOrientation[2], 
			m);

	m.preScale(	n->transform.scale[0],
			n->transform.scale[1], 
			n->transform.scale[2],
			m);

	if (n->transform.scaleOrientation[3] != 0.0f)
	    m.preRot(	PF_RAD2DEG(-n->transform.scaleOrientation[3]),
			n->transform.scaleOrientation[0], 
			n->transform.scaleOrientation[1],
			n->transform.scaleOrientation[2], 
			m);

	m.preTrans(	-n->transform.center[0],
			-n->transform.center[1], 
			-n->transform.center[2],
			m);
   }
   else
   {
	bcopy( n->transform_894.mat, m.mat, sizeof(m.mat));
   }

    dcs_ptr = (pfDCS *)obj;
    dcs_ptr->setMat(m);
   
}   // end csdLoadTransformInfo()

static int csTransform_load(void *obj, csdFile_csb *csb)
{
    all_nodes_t n;
    
    if (csb->version >= CSB_VERSION_TRANSFORM)
        csdLoadTransformInfo(obj,&n,sizeof(transform_t),csb);
    else
        csdLoadTransformInfo(obj,&n,sizeof(transform_894_t),csb);

    load_node_name((pfNode *) obj, n.node.name_size, csb);

    return 0;
    
}   // end csTransform_load()

// ******** Registering csSwitch 
static pfSwitch *pfSwitch_new(void)
{
   return new pfSwitch();
   
}   // end pfSwitch_new()

static void csdLoadSwitchInfo(void *node, all_nodes_t *n, int structsize,csdFile_csb *csb)
{
#ifdef DEBUG_NODE
printf("load switch info \n");
#endif
   
   // Load base class 
   csdLoadGroupInfo( (pfGroup *)node, n, structsize, csb);

   // Load csSwitch Info 
   if (n->swtch.which_child == CSB_SWITCH_NO_CHILDREN)
   {   
      ((pfSwitch *)node)->setVal(PFSWITCH_OFF);
   }
   else if (n->swtch.which_child == CSB_SWITCH_ALL_CHILDREN)
   {
      ((pfSwitch *)node)->setVal(PFSWITCH_ON);
   }
   else
   {
       if( n->swtch.which_child > n->group.num_children )
       {
          ((pfSwitch *)node)->setVal(PFSWITCH_ON);
       }
       else
       {
          ((pfSwitch *)node)->setVal(n->swtch.which_child);
       }
     
   }   // end else if which child 
   
}   // end csdLoadSwitchInfo()


static int csSwitch_load(void *obj, csdFile_csb *csb)
{
    all_nodes_t n;

    csdLoadSwitchInfo( obj, &n, sizeof(switch_t), csb);
    load_node_name((pfNode *)obj, n.node.name_size, csb);

    return 0;
    
}   // end csSwitch_load()

// ******** Registering csLOD 
static pfLOD *pfLOD_new(void)
{
    return new pfLOD();

}   // end pfLOD_new()

static void csdLoadLODInfo(void *node, all_nodes_t *n, int structsize,csdFile_csb *csb)
{
    // Load base class 
    csdLoadGroupInfo((pfGroup *)node,n,structsize,csb);

#ifdef DEBUG_NODE 
printf("load lod info \n");
print_lod( &n.cslod );
#endif
   
    // Load csLOD info
    pfVec3 center;
    int j;
    pfLOD *lod_ptr;
    
    lod_ptr = (pfLOD *)node;
   
    center[0] = n->cslod.center[0];
    center[1] = n->cslod.center[1];
    center[2] = n->cslod.center[2];

    lod_ptr->setCenter(center);

    for (j=0; j < n->cslod.num_children+1; j++)
    {
	//printf("setting LOD range %d = %f \n",  j, n->cslod.ranges[j] ); 
	lod_ptr->setRange(j, n->cslod.ranges[j]);
    }

}   // csdLoadLODInfo() 

static int csLOD_load(void *obj, csdFile_csb *csb)
{
    all_nodes_t n;

    csdLoadLODInfo(obj, &n, sizeof(cslod_t), csb);
    load_node_name((pfNode *) obj, n.node.name_size, csb);

    return 0;
}

// ***** Register csShape
static pfGeode *pfGeode_new(void)
{
    return new pfGeode;

}   // end pfGeode_new()

static void csdLoadShapeInfo(void *node, all_nodes_t *n, int structsize,csdFile_csb *csb)
{
    int32 *geoms;
    int i;
    int j;
    int count;
    int mode;
    int num_geometry;
    int color_bind;
    pfGeoSet *gset_ptr;
    pfGeoState *gstate_ptr;

#ifdef DEBUG_NODE
printf("load shape info \n" );
#endif
  
    pfGeode *shape = (pfGeode *)node;

    CSB_FREAD(n, structsize, 1, csb->fp);
    num_geometry = n->shape.num_geometry; 

#ifdef DEBUG_NODE
printf("geode has %d children \n", num_geometry );
#endif

    geoms = GET_BUF(num_geometry);
    CSB_FREAD(geoms, sizeof(int32), num_geometry, csb->fp);
    
    // In cosmo shape is assigned an apperance
    // which would mean in perf that geodes have geostates
    // but if we share GeoSets then we have to copy the 
    // GeoSets and then assign them GeoStates
    
    for (i = 0; i < num_geometry; i++)
    {
	if ( (geoms[i] != -1) &&
	      (csb->rl_list[L_GEOM][geoms[i]] != NULL) )
	{
	    if( n->shape.appearance != -1 )
	    {
	         if( csb->geo_ref[geoms[i]] == 0 )
		 {
		    // first use of this GeoSet don't need to clone
		    csb->geo_ref[geoms[i]] += 1;
#ifdef DEBUG_NODE		 
printf("geoset %d using geostate %d \n", geoms[i],  n.shape.appearance );
#endif
		    gset_ptr = (pfGeoSet *)csb->rl_list[L_GEOM][geoms[i]];
		    gstate_ptr = (pfGeoState *)csb->rl_list[L_APP][n->shape.appearance];
		   
		    mode = gstate_ptr->getMode( PFSTATE_SHADEMODEL);
		    if( mode == PFSM_FLAT )
		    {
			convert_to_flat_shade( gset_ptr, csb );
		    }
		    
		    shape->addGSet( gset_ptr );
		    gset_ptr->setGState( gstate_ptr );
		}
		else
		{
		     // geo set has been used before clone it
		     // so geo states don't collide
		     csb->geo_ref[geoms[i]] += 1;
#ifdef DEBUG_NODE		    
printf("REgeoset %d using geostate %d \n", geoms[i],  n.shape.appearance );
#endif
		     pfGeoSet *c_gset = new pfGeoSet();
	             gset_ptr = (pfGeoSet *)csb->rl_list[L_GEOM][geoms[i]];
		     gstate_ptr = (pfGeoState *)csb->rl_list[L_APP][n->shape.appearance];
	             pfCopy( c_gset, gset_ptr );
		     shape->addGSet( c_gset );
	             c_gset->setGState( gstate_ptr);

		     gset_ptr = c_gset;

		}   //end if ref_count == 0

		color_bind = gset_ptr->getAttrBind(PFGS_COLOR4);

    		if (csb->version < CSB_VERSION_ADDED_MATERIAL_MODE)
		{
		    if (color_bind == PFGS_OFF)
		    {
			pfMaterial *mtl = (pfMaterial *)gstate_ptr->getAttr(PFSTATE_FRONTMTL);
			if (mtl != NULL)
			    mtl->setColorMode(PFMTL_FRONT,  PFMTL_CMODE_OFF );
		    }
		}
		else  // csb->version >= CSB_VERSION_ADDED_MATERIAL_MODE
		{
		    // Remove color bindings if the appearance's 
		    // materialModeEnabled was NO_COLOR_MATERIAL.
		    // In cosmo, in this case the colorsets are ignored.

		    if (color_bind != PFGS_OFF &&
			csb->gstate_mtl_mode[n->shape.appearance] ==
			NO_COLOR_MATERIAL)
		    {
			gset_ptr->setAttr(PFGS_COLOR4, PFGS_OFF, NULL, NULL);
		    }

                    if ( csb->gstate_mtl_mode[n->shape.appearance] == NO_COLOR_MATERIAL )
                    {
                        pfMaterial *mtl = (pfMaterial *)gstate_ptr->getAttr(PFSTATE_FRONTMTL);
                        if (mtl != NULL)
                            mtl->setColorMode(PFMTL_FRONT,  PFMTL_CMODE_OFF );
                    }

		}

            }
	    else   // shape has no apperance
	    {
	       gset_ptr = (pfGeoSet *) csb->rl_list[L_GEOM][geoms[i]];
	       shape->addGSet( gset_ptr);
	       
	       if (csb->version >= CSB_VERSION_ADDED_MATERIAL_MODE)
	       {
		    // Remove color bindings since the default 
		    // materialModeEnabled for cosmo is NO_COLOR_MATERIAL.
		    // In cosmo, in this case the colorsets are ignored.

		    color_bind = gset_ptr->getAttrBind(PFGS_COLOR4);
		    if (color_bind != PFGS_OFF)
		    {
			gset_ptr->setAttr(PFGS_COLOR4, PFGS_OFF, NULL, NULL);
		    }
	       } 

	    }   // end if shape has apperance
	    
	 }   // end if index is valid
	 
    }   // end for num_geometry
    
}   // end csShape_load()

int csShape_load( void *obj, csdFile_csb *csb )
{
    all_nodes_t n;

    csdLoadShapeInfo(obj, &n, sizeof(shape_t), csb);
    load_node_name((pfNode *) obj, n.node.name_size, csb);

    return 0;
}


// ***** Register csBillboard
static void *pfBillboard_new(void)
{
    return new pfGroup();
}

static void csdLoadBillboardInfo(void *obj, all_nodes_t *n,int structsize,csdFile_csb *csb)
{
    float32 *pos;
    // pfBillboard *billboard = (pfBillboard *) obj;

    // Load base class info
    csdLoadGroupInfo(obj,n,structsize,csb);

    // billboard->position()->setCount(n->billboard.num_position);
    // pos = (float32*) billboard->position()->edit();
    pos = new float32[n->billboard.num_position * 3];
    CSB_FREAD(pos, sizeof(float32), n->billboard.num_position * 3, csb->fp);
    delete [] pos;
    // billboard->position()->editDone();

    // billboard->setAxis(n->billboard.axis[0], n->billboard.axis[1],
    //                    n->billboard.axis[2]);
    // billboard->setMode((csBillboard::ModeEnum)bbm_table[n->billboard.mode]);

    // if (csb->version >= CSB_VERSION_BILLBOARD_FORWARD_UP)
    // {
    //    billboard->setForward((csBillboard::VectorEnum)bbv_table[n->billboard.forward]);
    //    billboard->setUp((csBillboard::VectorEnum)bbv_table[n->billboard.up]);
    // }
}

static int csBillboard_load(void *obj, csdFile_csb *csb)
{
    all_nodes_t n;

    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csBillboard");

    if (csb->version >= CSB_VERSION_BILLBOARD_FORWARD_UP)
        csdLoadBillboardInfo(obj,&n,sizeof(billboard_t),csb);
    else
        csdLoadBillboardInfo(obj,&n,sizeof(billboard_t_893),csb);

    load_node_name((pfNode *)obj, n.node.name_size, csb);

    return 0;
}

// ***** Common logic for loading lights
static void csdLoadLightInfo(void */*obj*/, all_nodes_t *n, int structsize, csdFile_csb *csb)
{
    // csLight *light = (csLight*) obj;

    CSB_FREAD(n, structsize, 1, csb->fp);

    // light->setOn(n->light.on);
    // light->setIntensity(n->light.intensity);
    // light->setAmbientIntensity(n->light.ambientIntensity);
    // light->setColor(n->light.color[0], n->light.color[1], n->light.color[2]);
}

// ***** Register csDirectionalLight
static void *csDirectionalLight_new(void)
{
    // return new csDirectionalLight;
    return NULL;
}

static void csdLoadDirectionalLightInfo(void *obj, all_nodes_t *n, int structsize, csdFile_csb *csb)
{
    // csDirectionalLight *light = (csDirectionalLight*) obj;

    // Load base class info
    csdLoadLightInfo(obj,n,structsize,csb);

    // light->setDirection(n->directional_light.direction[0],
    //                    n->directional_light.direction[1],
    //                    n->directional_light.direction[2]);
}


static int csDirectionalLight_load(void *obj, csdFile_csb *csb)
{
    all_nodes_t n;

    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csDirectionalLight");

    csdLoadDirectionalLightInfo(obj, &n, sizeof(directional_light_t), csb);
    // XXX use load_node_name if directional light is derived from pfNode
    load_name(obj, n.node.name_size, csb);
    // load_node_name((pfNode *)obj, n.node.name_size, csb);

    return 0;
}

// ***** Register csPointLight
static void *csPointLight_new(void)
{
   // return new csPointLight;
   return NULL;
}


static void csdLoadPointLightInfo(void *obj, all_nodes_t *n, int structsize, csdFile_csb *csb)
{
    // csPointLight *light = (csPointLight*) obj;

    // Load base class info
    csdLoadLightInfo(obj,n,structsize,csb);

    // light->setRadius(n->point_light.radius);

    // light->setLocation(n->point_light.location[0],
    //                    n->point_light.location[1],
    //                    n->point_light.location[2]);

    // light->setAttenuation(n->point_light.attenuation[0],
    //                       n->point_light.attenuation[1],
    //                       n->point_light.attenuation[2]);
}

static int csPointLight_load(void *obj, csdFile_csb *csb)
{
    all_nodes_t n;

    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csPointLight");

    csdLoadPointLightInfo(obj, &n, sizeof(point_light_t), csb);
    // XXX use load_node_name if point light is derived from pfNode
    load_name(obj, n.node.name_size, csb);
    // load_node_name((pfNode *)obj, n.node.name_size, csb);

    return 0;
}

// ***** Register csSpotLight
static void *csSpotLight_new(void)
{
    // return new csSpotLight;
    return NULL;
}


static void csdLoadSpotLightInfo(void *obj, all_nodes_t *n, int structsize,
csdFile_csb *csb)
{
    // csSpotLight *light = (csSpotLight*) obj;

    // Load base class info
    csdLoadPointLightInfo(obj,n,structsize,csb);

    // light->setExponent(n->spot_light.exponent);
    // light->setCutOffAngle(n->spot_light.cutOffAngle);

    // light->setDirection(n->spot_light.direction[0],
    //                     n->spot_light.direction[1],
    //                     n->spot_light.direction[2]);
}


static int csSpotLight_load(void *obj, csdFile_csb *csb)
{
    all_nodes_t n;

    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csSpotLight");

    csdLoadSpotLightInfo(obj, &n, sizeof(spot_light_t), csb);
    // XXX use load_node_name if spot light is derived from pfNode
    load_name(obj, n.node.name_size, csb);
    // load_node_name((pfNode *)obj, n.node.name_size, csb);

    return 0;
}


// ***** Register csFog
static void *csFog_new(void)
{
   // return new csFog;
   return NULL;
}

static void csdLoadFogInfo(void */*obj*/, all_nodes_t *n, int structsize, csdFile_csb *csb)
{
    // csFog *fog = (csFog*) obj;

    CSB_FREAD(n, structsize, 1, csb->fp);

    // fog->setOn(n->fog.on);
    // fog->setMode((csFog::FogModeEnum)fgm_table[n->fog.mode]);
    // fog->setDensity(n->fog.density);
    // fog->setStart(n->fog.start);
    // fog->setEnd(n->fog.end);
    // fog->setIndex(n->fog.index);
    // fog->setColor(n->fog.color[0], n->fog.color[1],
    //               n->fog.color[2], n->fog.color[3]);
}


static int csFog_load(void *obj, csdFile_csb *csb)
{
    all_nodes_t n;

    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csFog");

    csdLoadFogInfo(obj, &n, sizeof(fog_t), csb);
    // XXX use load_node_name if directional light is derived from pfNode
    load_name(obj, n.node.name_size, csb);
    // load_node_name((pfNode *)obj, n.node.name_size, csb);

    return 0;
}
// ***** Register csEnvironment
static void *csEnvironment_new(void)
{
    return new pfGroup();
}

static void csdLoadEnvironmentInfo(void *obj, all_nodes_t *n, int structsize, csdFile_csb *csb)
{
    int *light, i;
    // csEnvironment *environment = (csEnvironment*) obj;

    // Load base class info
    csdLoadGroupInfo(obj,n,structsize,csb);

    // Set fog
    // if ((n->environment.fog >= 0) && (n->environment.fog <= csb->curListLength))        
    // 	environment->setFog((csFog*)csb->rl_list[L_NODE][n->environment.fog]);

    // Get buffer for lights and read in light info
    light = GET_BUF(n->environment.num_light);

    CSB_FREAD(light, sizeof(int), n->environment.num_light, csb->fp);
    // for (i = 0; i < n->environment.num_light; i++)
    // {
    //     if ((light[i] >= 0) && (light[i] <= csb->curListLength))
    //         environment->light()->set(i,
    //                     (csLight *)csb->rl_list[L_NODE][light[i]]);
    // }
}


static int csEnvironment_load(void *obj, csdFile_csb *csb)
{
    all_nodes_t n;

    pfNotify(PFNFY_WARN, PFNFY_PRINT,"Unsupported node: csEnvironment");

    csdLoadEnvironmentInfo(obj, &n, sizeof(environment_t), csb);
    load_node_name((pfNode *)obj, n.node.name_size, csb);

    return 0;
}



static void csRegisterTypes()
{
   pfType *type;

   // Register csGroup 
   type = pfGroup::getClassType();
   csdAddCustomObj_csb( type, type->getName(),
                        NULL,
                        NULL,
                        (csdNewCustomObjFuncType_csb)&pfGroup_new,
                        &pfGroup_load);

   // Register csTransform 
   type = pfDCS::getClassType();
   csdAddCustomObj_csb( type, type->getName(),
                        NULL,
                        NULL,
                        (csdNewCustomObjFuncType_csb)&pfDCS_new,
                        &csTransform_load);
   // Register csSwitch 
   type = pfSwitch::getClassType();
   csdAddCustomObj_csb( type, type->getName(),
                        NULL,
                        NULL,
                        (csdNewCustomObjFuncType_csb)&pfSwitch_new,
                        &csSwitch_load);

   // Register csLOD
   type = pfLOD::getClassType();
   csdAddCustomObj_csb( type, type->getName(),
                        NULL,
                        NULL,
                        (csdNewCustomObjFuncType_csb)&pfLOD_new,
                        &csLOD_load);

   // Register csShape
   type = pfGeode::getClassType();
   csdAddCustomObj_csb( type, type->getName(),
                        NULL,
                        NULL,
                        (csdNewCustomObjFuncType_csb)&pfGeode_new,
                        &csShape_load);

/* XXX load methods are called directly until a performer class is associated
       with these Cosmo3D types...

   // Register csBillboard
   type = pfBillboard::getClassType();
   csdAddCustomObj_csb( type, type->getName(),
			NULL,
			NULL,
                        (csdNewCustomObjFuncType_csb)&pfBillboard_new,
                        &csBillboard_load);

   // Register csEnvironment
   type = csEnvironment::getClassType();
   csdAddCustomObj_csb( type, type->getName(),
                        NULL,
                        NULL,
                        (csdNewCustomObjFuncType_csb)&csEnvironment_new,
                        &csEnvironment_load);

   // Register csSpotLight
   type = csSpotLight::getClassType();
   csdAddCustomObj_csb( type, type->getName(),
			NULL,
			NULL,
                        (csdNewCustomObjFuncType_csb)&csSpotLight_new,
                        &csSpotLight_load);

   // Register csDirectionalLight
   type = csDirectionalLight::getClassType();
   csdAddCustomObj_csb( type, type->getName(),
			NULL,
			NULL,
                        (csdNewCustomObjFuncType_csb)&csDirectionalLight_new,
                        &csDirectionalLight_load);

   // Register csPointLight
   type = csPointLight::getClassType();
   csdAddCustomObj_csb( type, type->getName(),
			NULL,
			NULL,
                        (csdNewCustomObjFuncType_csb)&csPointLight_new,
                        &csPointLight_load);

   // Register csFog
   type = csFog::getClassType();
   csdAddCustomObj_csb( type, type->getName(),
			NULL,
			NULL,
                        (csdNewCustomObjFuncType_csb)&csFog_new,
                        &csFog_load);
*/

}   // end csRegisterTypes()

