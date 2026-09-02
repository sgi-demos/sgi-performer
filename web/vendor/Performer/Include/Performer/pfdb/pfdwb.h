
/*=======================================================================
 *
 * MODULE	: pfdwb.h							
 *
 * DESCRIPTION  : Header file for the Designer's Workbench Performer loader.
 *		  for Performer 2.0
 *
 * AUTHOR	: David Cooper					
 *
 *------------------------------------------------------------------------
 * 
 * COPYRIGHT NOTICE: 							
 *
 * Copyright (C) 1989-1992 Coryphaeus Software,985 University Ave.,Suite 31,
 * Los Gatos, CA  95030.
 *						
 *------------------------------------------------------------------------
 *
 * REVISIONS:$Id: pfdwb.h,v 1.16 2002/11/17 01:40:32 rmech Exp $	
 *						
 *========================================================================*/


#ifndef __PFDWB_H__
#define __PFDWB_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux__
#include <asm/byteorder.h>
#endif

/* 
 * Texture environment 
 */

#define DWB_TV_MODULATE             0x101
#define DWB_TV_BLEND                0x102
#define DWB_TV_DECAL                0x103
#define DWB_TV_COLOR                0x200
#define DWB_TV_SHADOW               0x104
#define DWB_TV_ALPHA                0x105
#define DWB_TV_COMPONENT_SELECT     0x300
#define DWB_TV_I_GETS_R             0x310
#define DWB_TV_I_GETS_G             0x320
#define DWB_TV_I_GETS_B             0x330
#define DWB_TV_I_GETS_A             0x340
#define DWB_TV_IA_GETS_RG           0x350
#define DWB_TV_IA_GETS_BA           0x360
#define DWB_TV_I_GETS_I             0x370
#define DWB_TV_NULL                 0x000

/*
 * Texture filters
 */

#define DWB_TX_POINT                0x110
#define DWB_TX_BILINEAR             0x220
#define DWB_TX_MIPMAP               0x120
#define DWB_TX_MIPMAP_POINT         0x121
#define DWB_TX_MIPMAP_LINEAR        0x122
#define DWB_TX_MIPMAP_BILINEAR      0x123
#define DWB_TX_MIPMAP_TRILINEAR     0x124
#define DWB_TX_BICUBIC                      0x230
#define DWB_TX_SHARPEN                      0x240
#define DWB_TX_MODULATE_DETAIL              0x250
#define DWB_TX_ADD_DETAIL                   0x260
#define DWB_TX_TRILINEAR                    0x270
#define DWB_TX_MIPMAP_QUADLINEAR            0x280
#define DWB_TX_BICUBIC_GEQUAL               0x290
#define DWB_TX_BICUBIC_LEQUAL               0x2a0
#define DWB_TX_BILINEAR_GEQUAL              0x2b0
#define DWB_TX_BILINEAR_LEQUAL              0x2c0

/* 
 * Texture repeat modes
 */

#define DWB_TX_REPEAT               0x301
#define DWB_TX_CLAMP                0x302
#define DWB_TX_SELECT               0x303

/*
 * Texture internal format
 */

#define DWB_TX_I_12A_4              0x610
#define DWB_TX_IA_8                 0x620
#define DWB_TX_RGB_5                0x630
#define DWB_TX_RGBA_4               0x640
#define DWB_TX_IA_12                0x650
#define DWB_TX_RGBA_8               0x660
#define DWB_TX_RGBA_12              0x670
#define DWB_TX_RGB_12               0x680
#define DWB_TX_I_16                 0x690

/*
 * Texture external format
 */
#define DWB_TX_PACK_8               0x710
#define DWB_TX_PACK_16              0x720
#define DWB_TX_PIXMODE              0x3000

/*
 * Decal modes
 */

#define DWB_DECAL_BASE_STENCIL            1
#define DWB_DECAL_BASE_DISPLACE           2
#define DWB_DECAL_BASE_HIGH_QUALITY       3
#define DWB_DECAL_BASE_FAST               4

/*
 * Sequence modes
 */

#define DWB_SEQ_CYCLE                     0
#define DWB_SEQ_SWING                     1

/*========================================================================
 *========================================================================
 *
 * 	                   PUBLIC functions
 *
 *========================================================================
 *========================================================================*/


#ifdef WIN32
#ifdef __BUILD_LIBPFDB__
#define PFDWB_DLLEXPORT __declspec(dllexport)
#else
#define PFDWB_DLLEXPORT __declspec(dllimport)
#endif /* __BUILD_LIBPFDB__ */
#else
#define PFDWB_DLLEXPORT
#endif

#ifdef __ESCENE__
extern pfNode	*pfdLoad_dwb ( char *_file );
#else
extern PFDWB_DLLEXPORT pfNode	*pfdLoadFile_dwb ( char *_file );
#endif



/*========================================================================
 *========================================================================
 *
 * 		          constants & macros...
 *
 *========================================================================
 *========================================================================*/

#define CLIP_RECALCULATE        01  /* recalculate the clip region every frame*/
#define CLIP_INITIAL            02  /* only calculate a clip region once */

/*------------------------------------------------------*/
/* Designer's Workbench Format Version 2.005 opcodes 	*/
/*------------------------------------------------------*/
#define DB_HEADER			6001	/* top level record	*/
#define DB_GROUP			6002	/* container node	*/
#define DB_SWITCH			6003	/* LOD			*/
#define DB_FACE				6005	/* poly/line/points/mesh*/
		
#define DB_VERTEX			6010	/* lowest level node	*/	
#define DB_CONSTRUCTION			6011	/* list of const. pts	*/
#define DB_PACKED_VERTEX		6012	/* packed colr-not index*/

#define DB_BSPLINE			6020
#define DB_BSPLINE_KNOTS		6021
#define DB_BSPLINE_CNTRL_PNTS		6022

#define DB_LIGHT_PT			6023	/* face light point     */

#define DB_ARC                          6024    /* arcs                 */

#define DB_COMMENT    			6031	/* user comments	*/
#define DB_COLOR_TABLE			6032	/* file color table	*/
#define DB_VIEW_PARAMS			6033	/* viewing parameters	*/

#define DB_NAME_STR			6040	/* dynamic length name	*/
#define DB_FONT_STR			6041	/* name of string font  */
#define DB_STRING_STR			6042	/* string string 	*/
#define DB_FILE_STR			6043	/* external file name	*/
#define DB_TEX_FILE_STR			6044	/* texture file name	*/
#define DB_SOUND_FILE_STR		6045    /* sound file name      */

#define DB_MATRIX			6049	/* static coord. system */

#define DB_REPLICATE			6060	/* not implemented yet	*/
#define DB_INSTANCE_REF			6061	/* define node as instanance */
#define DB_INSTANCE_DEF			6062	/* define node as instanable */
#define DB_EXTERNAL			6063	/* external             */
#define DB_EOF_EXTERNAL			6064	/* make end of external */

#define DB_STRING			6080	/* dynamic string node 	*/
#define DB_PAGE				6081	/* perspective page node*/
#define DB_IMAGE			6082	/* 2D bitmap image node */

#define DB_PUSH				6090	/* push record		*/
#define DB_POP				6091	/* pop record		*/

#define DB_LINESTYLE_TABLE		6120	/* linestyle palette	*/
#define DB_MATERIAL_TABLE  		6122	/* material palette	*/
#define DB_TEXTURE_TABLE	  	6123	/* texture palette	*/
#define DB_LSOURCE	  		6124	/* lightsource defn.	*/	
#define DB_LMODEL  			6125	/* lightmodel defn.	*/	
#define DB_TEXMAP  			6126	/* polygon Tex map def - dwb specific  */
#define DB_SOUND_PALETTE		6127	/* Sound Palette record */
#define DB_LASTREC  			6127	/*value of greatest opcode rec*/


/*------------------------------------------------------*/
/* file unit types - all coords stored as floating point*/
/* values.						*/
/*------------------------------------------------------*/
#define UT_METERS			0	/* most as in old-dwb 	*/
#define UT_KILOMETERS			1	/* format 		*/
#define UT_YARDS			2	
#define UT_MILES			3
#define UT_FEET				4
#define UT_INCHES			5
#define UT_NAUTICAL_MILES		8
#define UT_CENTIMETERS			9


/*------------------------------------------------------*/
/* unit conversion values				*/
/*------------------------------------------------------*/
#define NM_TO_METERS			1852.0	/* Assuming INTERNATIONAL naut. mile */
#define MILES_TO_METERS			1609.344/* Assuming US STATUTE mile */
#define YARDS_TO_METERS			0.9144
#define	FEET_TO_METERS			0.3048
#define INCHES_TO_METERS		0.0254  
#define	KM_TO_METERS			1000.0
#define	CM_TO_METERS			0.01
#define	MM_TO_METERS			0.001



/*------------------------------------------------------*/
/* drawstyle.. how to globally draw the	model If 	*/
/* DS_SELECTIVE then group determines.	Some of these   */
/* values are used in the drawstyle/movestyle variable	*/
/* in the HEADER & the drawstyle field in the GROUP.	*/
/* Others refer to the the drawType of a FACE node.	*/
/*------------------------------------------------------*/

/*----------------------------------------------*/
/* drawstyle					*/
/*----------------------------------------------*/
#define DS_SOLID_BOTH_SIDES		0
#define DS_SOLID			1
#define DS_CLOSED_WIRE			2
#define DS_OPEN_WIRE			3
#define DS_POINTS			4
#define DS_WIRE				5
#define DS_WIRE_ON_MOVE			6
#define DS_POINT_ON_MOVE		7
#define DS_WIRE_OVER_SOLID		8
#define DS_SELECTIVE			9
#define DB_OMNIDIRECTIONAL 1
#define DB_UNIDIRECTIONAL 2
#define DB_BIDIRECTIONAL 3

/*------------------------------------------------------*/
/* switch philosophies					*/
/*------------------------------------------------------*/
#define ST_DISTANCE			0
#define ST_THRESHOLD			1


/*------------------------------------------------------*/
/* shading models available				*/
/*------------------------------------------------------*/
#define SM_FLAT_NON_LIT			0
#define SM_GOURAUD_NON_LIT		1
#define SM_ILLUMINATED			2 /* material binding decides whether flat/gouraud */
#define SM_SELECTIVE			3

/*------------------------------------------------------*/
/* poly types.Poly could be a collection of points, open*/
/* line or closed line primitive OR it could be a meshed*/
/* primitive.						*/
/*------------------------------------------------------*/
#define PT_POLY				0
#define PT_TMESH			1
#define PT_QMESH			2

/*------------------------------------------------------*/
/* zbuffer/backface/aa toggle defines			*/
/*------------------------------------------------------*/
#define TS_GLOBAL_OFF			0
#define TS_GLOBAL_ON			1
#define TS_SELECTIVE			2

/*------------------------------------------------------*/
/* background fill types.				*/
/*------------------------------------------------------*/
#define BF_FLAT				0
#define BF_INVERSE			1
#define BF_SHADED			2

/*------------------------------------------------------*/
/* string stuff 					*/
/*------------------------------------------------------*/
#define ST_VECTOR			0
#define ST_RASTER			1
#define ST_POLYGON			2

#define LEFT_TO_RIGHT			0
#define RIGHT_TO_LEFT			1	
#define TOP_TO_BOTTOM			2	
#define BOTTOM_TO_TOP			3

#define H_ALIGN_LEFT			0
#define H_ALIGN_RIGHT			1
#define H_ALIGN_CENTER			2

#define V_ALIGN_BOTTOM			0
#define V_ALIGN_TOP			1
#define V_ALIGN_CENTER			2

/*------------------------------------------------------*/
/* Material binding types		 		*/
/*------------------------------------------------------*/
#define MB_LIGHTING_OFF			0
#define MB_PER_GROUP			1
#define MB_PER_FACE			2
#define MB_PER_VERTEX			3
#define MB_SELECTIVE			4

/*------------------------------------------------------*/
/* vertex sort types		 			*/
/*------------------------------------------------------*/
#define TEXTURED_VERT			8 /* offset for texture variant */
#define VT_FLAT_VERTEX			1	
#define VT_SHADED_VERTEX		2
#define VT_ILLUM_VERTEX			3
#define VT_SHADED_ILLUM_VERTEX		4
#define VT_TEXTURED_FLAT_VERTEX		VT_FLAT_VERTEX + TEXTURED_VERT
#define VT_TEXTURED_SHADED_VERTEX 	VT_SHADED_VERTEX + TEXTURED_VERT
#define VT_TEXTURED_ILLUM_VERTEX	VT_ILLUM_VERTEX + TEXTURED_VERT	
#define VT_TEXTURED_SHADED_ILLUM_VERTEX	VT_SHADED_ILLUM_VERTEX + TEXTURED_VERT	

/*------------------------------------------------------*/
/* up vector			 			*/
/*------------------------------------------------------*/
#define X_UP				1
#define Y_UP				2
#define Z_UP				3

/*------------------------------------------------------*/
/* Misc...						*/
/*------------------------------------------------------*/
#define DWB_MAX_MATERIALS               120
#define DWB_MAX_TEXTURES		1024
#define DWB_VLISTSIZE			2048	/* Chunk size of vertex lists */
#define PACKED_COLOR_INDEX		3840	/* *special* index for cpack colored vertices */
#define TEXTURE_NOT_FOUND		-3



/*========================================================================
 *========================================================================
 *
 * 	        Designer's Workbench Version 2 Disk Records
 *
 *========================================================================
 *========================================================================*/




/*----------------------------------------------*/
/* dwbOpcodeRec					*/
/* store the record opcode & size of the follow */
/* ing record. Size doesn't include the 4 byte  */
/* dwb_OPCODE_REC.				*/
/*----------------------------------------------*/

typedef struct _dwbOpcodeRec
{
    short opcode;
    short size;
}dwbOpcodeRec;



/*----------------------------------------------*/
/* dwbUserRec					*/
/* convenience struct to hold  user data     	*/
/* fields that can be stored in most of the     */
/* node types.					*/
/*----------------------------------------------*/

typedef struct _dwbUserRec
{
    int    ifields[2];		/* couple of ints - 			*/
    float  ffields[2];		/* couple of floats for user's use.Saved*/
}dwbUserRec;			/* and restored by dwb.			*/



/*----------------------------------------------*/
/* dwbHeader					*/
/* top-level database struct for any model 	*/
/* loaded 					*/
/*----------------------------------------------*/

typedef struct _dwbHeader
{      
    short units;		/* different unit types 		*/
    short reserved;		/* spare				*/

    float version;		/* file format revision			*/
    char lastrev[32];		/* time/date written to disk.		*/

    short next_node_ids[6];	/* Ids to auto generate a new node name */
    				/* order is group,switch,face,string,	*/
    				/* image & page.			*/

    float bbox[6];		/* bbox of entire model... order is bx	*/
    				/* by, bz, lx, ly, lz			*/

    short drawstyle;		/* how to (globally) draw this model 	*/
    short movestyle;		/* How to (globally) draw the model when*/
    				/* moving				*/
    short shademodel;		/* How to (globally) shade the model 	*/
    short zbuffer;		/* global zbuffer state 		*/

    short backface;		/* global backface removal state 	*/
    short concave;		/* global concave state 		*/

    struct 
    {	
        unsigned invalid_view_hint: 1; /* tell dwb to recalc the view */
	unsigned sparebits: 31;	/* reserved for future use.		*/
    } flags;			/* 4 bytes bit-flags 			*/
    
    dwbUserRec udata;  		/* user fields 				*/
    float spare[4];		/* reserved for future use.		*/
    short spare2; 		/* reserved for future use.		*/
    
    short color;		/* backround color index		*/

    short material_binding;	/* how to globally use materials 	*/
    short transparency;		/* global transparency on/off 		*/
    
    short texture;		/* global texture on/off 		*/
    short up_axis;		/* y is up or z is up in the database	*/
    
    int  spare4[46];

}dwbHeader;





/*----------------------------------------------*/
/* dwbGridInfo					*/
/* holds the info on a grid location etc	*/
/*----------------------------------------------*/

typedef struct _dwbGridInfo
{ 
    short grid_on;		/* display it or not			*/
    short grid_zbuffer;		/* zbuffer the grid			*/
    short grid_draw_after;	/* draw it after the model geometry	*/
    short grid_snap;		/* snap to grid is on.			*/
    short grid_orientation;	/* XY,FACE etc				*/
    float grid_xmajor;		/* distance between major divisions in x*/
    float grid_ymajor;		/* distance between major divisions in y*/
    short grid_xminor;		/* num minor divs between major divs - x*/
    short grid_yminor;		/* num minor divs between major divs - y*/
    float grid_width;		/* real-world width of the entire grid  */
    float grid_height;		/* real-world height of the entire grid */
    float grid_pos[3];		/* position of the grid center in 3D	*/
    float grid_normal[3];	/* normal to plane of the grid.		*/
    float grid_mat[4][4];	/* orientation (rotation) matrix	*/
    float grid_rot;		/* not used				*/
    float grid_edge_pt1[3];	/* not used				*/
    float grid_edge_pt2[3];	/* not used				*/
    float grid_offset;		/* offset from "correct plane" along    */
    				/* normal to plane.			*/
}dwbGridInfo;	






/*----------------------------------------------*/
/* dwbView					*/
/* structure to store the view params etc so 	*/
/* that the user can go back to them later.	*/
/*----------------------------------------------*/

typedef struct _dwbView
{
    float eye_p;		/* pitch angle 				*/
    float eye_r;		/* roll angle 				*/
    float eye_h;		/* yaw angle 				*/
    float eye_x;		/* eyepoint XYZ 			*/ 
    float eye_y;
    float eye_z;			     
    float fov;			/* field of view 			*/
    float _near_;		/* near clipping plane location 	*/
    float _far_;		/* far clipping plane location 		*/
    
    float model_x;		/* model position normally at origin 	*/
    float model_y;
    float model_z;
  
    float mat[4][4];		/* quarernion transformation matrix 	*/
    float quat[4];		/* quaternion direction 		*/
        
    float model_p;		/* components of matrix 		*/
    float model_r;
    float model_h;
    
    float cen_bx,cen_by,cen_bz;	/* display centroid region 		*/
    float cen_lx,cen_ly,cen_lz;
					

    dwbGridInfo gridinfo;	/* grid parameters 			*/

    short auto_clipplanes;	
    short background_clear_type;	/* 0 = BF_FLAT
    					   1 = BF_INVERSE
    					   2 = BF_SHADED */
    					   
    short spare;    					   		
    float spare2[10];			/* future expansion */		
    
} dwbView;






/*----------------------------------------------*/
/* dwbColorTable				*/
/* This is the same as the old dwb color table  */
/*----------------------------------------------*/

typedef struct _dwbColorTable
{						
    short shaded [31][3];	/* brightest colors of 128 intensities	*/	
    short fixed [64][3];	/* fixed RGB values.			*/
    short overlay[16][3];	/* geometry with these colors use ovlays*/
    short underlay[16][3];	/* use underlays			*/
    short spare [32][3];	
    short spare2;		
} dwbColorTable;






/*----------------------------------------------*/
/* dwbTexInfo					*/
/* texture info ref FOR THIS FILE.		*/
/*----------------------------------------------*/

typedef struct _dwbTexInfo
{  
    short xsize;		/* x size of texture in texels 		*/
    short ysize;		/* y size of texture in texels 		*/
    short zsize;		/* number of texture components 	*/
    short type;			/* RLE or varbatim 			*/

    float minfilter;  		/* TX_POINT, TX_BILINEAR, etc etc 	*/
    float magfilter;  		/* TX_POINT, TX_BILINEAR  etc etc 	*/
    
    float wrap;       		/* TX_REPEAT, TX_CLAMP, TX_SELECT 	*/
    float wraps;      		/* S only 				*/
    float wrapt;      		/* T only 				*/
    float envtype;    		/* texture environment 			*/
    
    int  spare [ 16 ];  

}dwbTexInfo;


/*----------------------------------------------*/
/* dwbExtTexInfo			        */
/* extended texture info FOR THIS FILE.         */
/* Supported only from V3.0 of the DWB format   */
/*----------------------------------------------*/

typedef struct _dwbExtTexInfo
{
    /*----------------------------------------------------------*/
    /* initial part of record is exactly the same as the old    */
    /* record. The opcode will be the same. Based on record size*/
    /* the user can read into the appropriate record.           */
    /*----------------------------------------------------------*/

    /*------------------*/
    /* BASIC TEX INFO   */
    /*------------------*/

    short xsize;                    /* x size of texture        */
    short ysize;                    /* y size of texture        */
    short zsize;                    /* num of texture components*/
    short type;                     /* RLE or varbatim          */

    float minfilter;                /* TX_POINT,TX_BILINEAR, etc*/
    float magfilter;                /* TX_POINT,TX_BILINEAR  etc*/

    float wrap;                     /* TX_REPEAT, TX_CLAMP, etc */
    float wraps;                    /* S only                   */
    float wrapt;                    /* T only                   */
    float envtype;                  /* texture environment      */

    /*------------------*/
    /*  PROPERTY USAGE  */
    /*------------------*/

    float version;                  /* DWB format version       */
    struct
    {
	unsigned enabled: 1;        /* This texture is on       */
	unsigned magfilter_split: 1;/* individual mag alpha/colr*/
	unsigned wrap_split: 1;     /* individual S & T wrap    */
	unsigned internal: 1;       /* internal image storage   */
	unsigned external: 1;       /* external image storage   */
	unsigned mipmap_filter: 1;  /* kernal filters           */
	unsigned bicubic_filter: 1;
	unsigned control_points: 1; /* for sharp/detail texture */
	unsigned tile: 1;           /* tile into larger texture */

	unsigned detail: 1;         /* Texture is a detail tex  */
	unsigned fast_define: 1;    /* GL will use user's array */
	unsigned component_select: 1;

	unsigned sparebits: 20;     /* future info              */
    }flags;


    /*------------------*/
    /*  NEW TEVDEF INFO */
    /*------------------*/
    float component_select;         /* use of 1,2 tex comp.     */
    float envcolor[4];              /* TEV color                */


    /*------------------*/
    /*  NEW TEXDEF INFO */
    /*------------------*/

    float magfilter_alpha;
    float magfilter_color;

    float bicubic_filter[2];        /* affects BICUBIC min/mag  */
    float mipmap_filter[8];         /* generate mipmap levels   */

    float internal;                 /* internal storage hint    */
    float external;                 /* external image storage   */

    float control_points[4][2];     /* detail/sharpen controls  */
    float control_clamp;

    float detail[5];                /* J,K,M,N,Scramble         */

    float tile[4];                  /* tile into the texture    */

    float spare [73];
}dwbExtTexInfo;



typedef struct _dwbTexture
{
    char *name;
    dwbExtTexInfo tex;
}dwbTexture;



typedef struct _dwbMaterial
{
    float diffuse[3];
    float ambient[3];
    float shininess;
    float specular[3];
    float emissive[3];
    float alpha;
    float spare[2];
}dwbMaterial;



/*----------------------------------------------*/
/* dwbMatInfo					*/
/* material info ref FOR THIS FILE.		*/
/*----------------------------------------------*/

typedef struct _dwbMatInfo
{     
    struct 			/* bit-flag structures.			*/
    {	
	unsigned sparebits: 32;	
    }flags;

    dwbMaterial mat;
    
    int  spare5 [ 23 ];  

}dwbMatInfo;


/*----------------------------------------------*/
/* dwbLSourceInfo				*/
/* light-source info ref FOR THIS FILE.		*/
/*----------------------------------------------*/

typedef struct _dwbLSourceInfo
{     
    struct 
    {	
	unsigned sparebits: 32;	
    }flags;			/* bit-flag structure.			*/
    
    float defn[15];		/* light-source defn.			*/
    
    int  spare5 [ 12 ];  

}dwbLSourceInfo;





/*----------------------------------------------*/
/* dwbLModelInfo				*/
/* light-model info ref FOR THIS FILE.		*/
/*----------------------------------------------*/

typedef struct _dwbLModelInfo
{     
    struct 
    {	
	unsigned sparebits: 32;	
    }flags;			/* bit-flag structures.			*/
    
    float defn[8];		/* light-model defn.			*/
    
    int  spare5 [ 11 ];  

}dwbLModelInfo;




/*----------------------------------------------*/
/* dwbSoundInfo					*/
/* sound palette info ref FOR THIS FILE.	*/
/*----------------------------------------------*/

typedef struct _dwbSoundInfo
{     
    float playback_volume;
    short playback_mode;

    short spare1;
    short spare2;
    float spare3[20];
}dwbSoundInfo;





/*----------------------------------------------*/
/* dwbLineStyle 				*/
/* texture info ref FOR THIS FILE.		*/
/*----------------------------------------------*/

typedef struct _dwbLineStyle
{  
    unsigned short lstyle;	/* linestyle index			*/
    short repeat;		/* lsrepeat factor.			*/
    
    int  spare[2];

}dwbLineStyle;





/*----------------------------------------------*/
/* dwbGroup					*/
/* General container group - can parent anything*/
/* except a HEADER. Special case of a group is  */
/* when it defines a perspective viewport or a  */
/* clip region.					*/
/*                                              */
/* This has changed since the 2.1 version.      */
/* fields that were unused before have been     */
/* renamed and are used and new fields have     */
/* introduced, which are:                       */
/* short surfaceType;       [old: short spare ] */
/* unsigned road_object: 1; [old: didn't exist] */
/* short decal_type;        [old: short spare2] */
/* short seq_random;        [old: short spare3] */
/*----------------------------------------------*/

typedef struct {
#ifdef __LITTLE_ENDIAN
	unsigned sequence: 1;	/* sequence geometry group */
	unsigned billboard: 1;	/* billboard geometry group */
	unsigned decal: 1;	/* coplanar geometry group */
	unsigned renderGrp: 1;	/* grp is parent of at least 1 drawable */
	unsigned pages: 1;	/* has pages - not necessarily persp	*/
	unsigned clip_region: 1;/* mask region type 			*/
	unsigned region_def:1;	/* viewport defined 			*/
	unsigned perspective:1;	/* perspective group in page-group-ovl 	*/	
#else
	unsigned perspective:1;	/* perspective group in page-group-ovl 	*/	
	unsigned region_def:1;	/* viewport defined 			*/
	unsigned clip_region: 1;/* mask region type 			*/
	unsigned pages: 1;	/* has pages - not necessarily persp	*/
	unsigned renderGrp: 1;	/* grp is parent of at least 1 drawable */
	unsigned decal: 1;	/* coplanar geometry group */
	unsigned billboard: 1;	/* billboard geometry group */
	unsigned sequence: 1;	/* sequence geometry group */
#endif
	unsigned sparebits: 24;	
    } Grp_flags;



typedef struct _dwbGroup
{	 			
    short layer; 		/* relative priority			*/				
    short color;		/* color index				*/			
    
    short drawstyle;		/* draw this group (lines,solid etc) 	*/
    short shademodel;		/* flat,gouraud etc 			*/

    short linestyle;		/* linestyle for this group.		*/
    short linewidth;		/* linewidth for this group.		*/
    
    short fill_pattern;		/* not used.				*/
    short texture;		/* not used - texture defined by face	*/

    float transparency;		/* 0.0 -> 1.0, 1.o is totally opaque.	*/
    
    short material;		/* material palette index.		*/
    short use_group_color;	/* use group color rather than primitive*/
    short use_group_dstyle;	/* use group dstyle or inherit global.	*/

    short surfaceType;
    

    float bbox[6];		/* bbox of entire model... order is lx	*/
    				/* bx, ly, by, lz, bz			*/
    
    Grp_flags			grp_flags;

    struct 			/* following flags take effect when 	*/
    {				/* approp' graphics option is SELECTIVE */
	unsigned zbuffer:1;	/* This group drawn with zbuffer TRUE	*/		
	unsigned backface:1;	/* This group drawn with backface TRUE	*/	
	unsigned texture: 1;	/* This group drawn with texture enabled*/		
	unsigned aa_lines: 1;	/* switch on line aa for this group	*/
	unsigned aa_polys: 1;	/* not implemented			*/
	unsigned concave: 1;	/* This group drawn with concave TRUE	*/	 	
	unsigned displist: 1;	/* not implemented 			*/		
	unsigned sparebits2: 25;	
    }gfx_flags;
    
    struct 			/* following flags have no visual effect*/
    {				/* for real-time use only.		*/
	unsigned day_object:1;		
	unsigned night_object:1;		
	unsigned dusk_object:1;		
	unsigned shadow_object: 1;		
	unsigned terrain_object: 1;		
	unsigned road_object: 1;
	unsigned sparebits3: 26;	
    }ig_flags;	

    float ll[3];		/* lower-left viewport/clip-region pt.	*/
    float ur[3];		/* upper-right viewport/clip-region pt.	*/
    float center[3];		/* not used.				*/
   
    dwbUserRec udata;  	/* user fields 				*/

    short material_binding;	/* how to bind materials for this group */
    				/* if mat-binding is SELECTIVE		*/
    short decal_type;           /* displace poly or stencil */
    
    unsigned int  packed_color;	/* use this color if the color index is	*/
   				/* 3840 - signifies direct color map	*/

    short collapsed;		/* structure chart indicator */
    short seq_mode;		/* 0 = cycle, 1 = swing */
    float seq_time;		/* frame time */
    float seq_speed;		/* sequence time */
    short seq_nreps;		/* number of repeats */
    short seq_bgn;		/* beging index */
    short seq_end;		/* end index */
    short seq_random;           /* randomly draw children */

    short sound_index;		/* Sound index */
    short spare5;

    float spare6[3];		/* spare fields 4 bytes each */
    
}dwbGroup;







/*----------------------------------------------*/
/* dwbPage					*/
/* A 1 of many switch. Based on active-page ID	*/
/* Page is typically used to setup a perspective*/
/* viewport for MFD (multi-function display)	*/
/*----------------------------------------------*/

typedef struct _dwbPage
{    		
    short layer; 				
    short color;				

    short pageid;
    short spare;
    
    struct 
    {
	unsigned perspective:1;	/* TRUE if viewparms used 		*/
	unsigned active:1;	/* TRUE if displayed 			*/
	unsigned sparebits: 30;
    } flags;
    					
    float fov;				
    float aspect;			
				/* model orientation in the viewport 	*/
    float pitch;			
    float roll;				
    float heading;			
    float scale;			
    float eyept[3];	
    
    float pgcen[3];		/* center of page geometry bbox		*/
    
    dwbUserRec udata;		/* user HEADER-stored fields 		*/

    float azim;			/* lookat parameters 			*/
    float elev;
    float twist;
    
    unsigned int  packed_color;		/* use this color if the color index is	*/
   					/* 3840 - signifies direct color map	*/
    float spare2[6];
    
}dwbPage;

		





/*----------------------------------------------*/
/* dwbSwitch					*/
/* Either an additive LOD or switchable 	*/
/* depending on distance to the eye.		*/
/*----------------------------------------------*/

typedef struct _dwbSwitch
{	
    short layer; 		/* relative priority - not used		*/
    short spare;
    			
    short switchtype;		/* either distance or threshold 	*/    
    short threshold;		/* switch in threshold value 		*/

    float switchin;		/* switchin (distance) value 		*/
    float switchout;		/* switchout(distance) value 		*/

    float center[3];		/* center of LOD block			*/		
    
    struct 
    {	
	unsigned sparebits: 32;	
    }flags;
    
    dwbUserRec udata;  		/* user  fields 			*/
   
    int  spare2[12];
     
}dwbSwitch;







/*----------------------------------------------*/
/* dwbFace					*/
/* Dwb Polygon primitive.A poy could be a 	*/
/* colection of points, an open/closed wire, a  */
/* poly with n verts or a mesh primitive.	*/
/*----------------------------------------------*/

typedef struct _dwbFace
{
    short layer; 		/* relative priority - not used		*/     
    short color;		/* poly color index.			*/	

    short numverts;		/* number of vertices in this poly	*/
    short ptype;	        /* poly,tmesh or qmesh 			*/
    
    short linewidth;		/* for wire-drawn styles.		*/	
    short drawstyle;		
    
    short linestyle;		/* line palatte index			*/
    short back_color;		/* colour of backfaced poly */

    short texture;		/* texture palette index 		*/
    short material;		/* not used - group defines.		*/

    float ir;			/* not used.				*/
    
    struct 
    {			
        unsigned vertColors: 1;	/* verts have different colors than poly*/
	unsigned sparebits: 31;
    } flags;
    
    dwbUserRec udata;  		/* user HEADER-stored fields 		*/

    unsigned int  packed_color;	/* use this color if the color index is	*/
   				/* 3840 - signifies direct color map	*/
    float spare2[7];
    
}dwbFace;


/*-----------------------------------------------*/
/* dwbNewFace - from version 3.0, has additional */
/* fields to take care detail texturing, light   */
/* point size and so on...                       */
/* size of the record is the same as the old one */
/*-----------------------------------------------*/


typedef struct _dwbNewFace {
    short layer;                /* relative priority - not used         */
    short color;                /* poly color index.                    */

    short numverts;             /* number of vertices in this poly      */
    short ptype;                /* poly, tmesh, qmesh, bspline, light_pt*/

    short linewidth;            /* for wire-drawn styles.               */
    short drawstyle;

    short linestyle;            /* line palatte index                   */
    short back_color;           /* colour of backfaced poly */

    short texture;              /* texture palette index                */
    short pntsize;              /* if ptype == LIGHTPOINT               */

    /* This is *VERY* important. This field (pntsize) is not used in    */
    /* the older versions though it's present in the name of material.  */
    /* To keep the loader simple, this new structure is used even while */
    /* loading older versions. The newer versions might use this field  */
    /* when the face type is LIGHTPOINT and they will address this field*/
    /* as given above.                                                  */


    float ir;                   /* infra-red, not used                  */

    struct {
      unsigned vertColors: 1;    
      unsigned intersection: 1; /* poly is part of a road intersection  */
      unsigned sparebits: 30;
    } flags;

    dwbUserRec udata;           /* user HEADER-stored fields            */

    unsigned int packed_color;  /* use this color if the color index is */
				/* 3840 - signifies direct color map    */

    struct {
      unsigned road: 1;
      unsigned intersection: 1;
      unsigned sparebits: 30;
    } ig_flags;

    short colapsed;
    short detail_tex;           /* detail texture index                 */

    short surface_material_code;/* used by DFAD                         */
    short feature_id;           /* used by DFAD                         */

    short alt_texture;          /* ignored                              */
    short material;             /* ignored                              */

    float spare2[3];
  } dwbNewFace;



/*----------------------------------------------*/
/* dwbString					*/
/* Dynamic string primitive - could be raster or*/
/* vector.string value & font name follow dwb_	*/
/* STRING record because both are dynamic 	*/
/* length.					*/
/*----------------------------------------------*/

typedef struct _dwbString
{   
    short layer; 		/* relative priority - not used		*/ 
    short type;			/* vector, raster, polygon 		*/
    short color;	
    
    struct 			
    {		
	unsigned boldface:1;		
	unsigned slanted:1;		
	unsigned underline:1;		
	unsigned strikeout:1;		
	unsigned text_entry:1;	
	unsigned sparebits:27;	
    } flags;
    short linewidth;		/* vector pixel width 			*/
    
    float    vfont_width;	/* width multiplier 			*/
    float    vfont_height;	/* height multiplier 			*/
    float    vfont_exp;		/* expansion factor  			*/
    unsigned vfont_display;	/* justification,alignment etc 		*/

    short direction;		/* Left-to-right, right-to-left etc	*/
    short horiz_alignment;	/* left, right,center 			*/
    short vert_alignment;	/* baseline, top, center 		*/

 
    float matrix[4][4];		/* vector orientation matrix 		*/
    float coords[3];		/* string origin in 3D database coords  */

    dwbUserRec udata;  		/* user HEADER-stored fields 		*/
 
    unsigned int  packed_color;	/* use this color if the color index is	*/
   				/* 3840 - signifies direct color map	*/
    float spare[9];
    
}dwbString;








/*----------------------------------------------*/
/* dwbImage					*/
/* render a 2D bit-mapped image - old fashioned */
/* (or non-texture machine) way of doing moving */
/* map displays.image filename follows dwb_IMAGE*/
/* structure (dynamic length)			*/
/*----------------------------------------------*/

typedef struct _dwbImage
{ 
    short layer; 		/* relative priority - not used		*/ 
    short spare;
    			
    struct 
    {		
	unsigned sparebits:32;
    } flags;
     				
    float xzoom;		/* rectzoom factors			*/			
    float yzoom;
    int xsize;			/* size in pixels 			*/				
    int ysize;	
    float coords[3];		/* 3D database coordinate 		*/	

    dwbUserRec udata;  		/* user HEADER-stored fields 		*/

    int  spare2[14];
    
}dwbImage;





/*----------------------------------------------*/
/* dwbVertex					*/
/* all vertices in dwb have color - normal info */
/*----------------------------------------------*/

typedef struct _dwbVertex
{
    short spare;		/* not used				*/		
    short color;		/* index into color_table 		*/
    float coords[3];		/* xyz position values 			*/
    float normal[3];
    float tex_coords[4];	/* texture coordinates 			*/
}dwbVertex;



/*----------------------------------------------*/
/* dwbPackedVertex				*/
/* As above but contains packed color instead of*/
/* an index into the color table.		*/
/*----------------------------------------------*/

typedef struct _dwbPackedVertex
{
    unsigned int  packed_color;	/* in cpack format			*/
    float coords[3];		/* xyz position values 			*/
    float normal[3];
    float tex_coords[4];	/* texture coordinates 			*/
}dwbPackedVertex;



typedef struct _dwbInstanceDef
{
  short val;
  short spare;

} dwbInstanceDef;

typedef struct _dwbInstanceRef
{
  short val;
  short spare;
  float mat[4][4];
} dwbInstanceRef;


typedef struct _instRefList
{
  pfNode *node;
  dwbInstanceDef inst;
  struct _instRefList *next;
} instRefList;

typedef struct _refBeforeDefList
{
  pfNode *parent;
  dwbInstanceRef inst;
  struct _refBeforeDefList *next;
} refBeforeDefList;


typedef struct _treeStack
{
  pfNode *parent;
  pfNode *current;
  float	 balance[2]; /*quadword align */
} treeStack;

#define STACKSIZE 25


typedef struct _groupValues
{
    int shading;
    int binding;
    int backface;
    int drawstyle;
    int zbuffer;
    int material;
    float alpha;
    int texture;

} groupValues;

typedef struct  _myLines
{
    int    numVerts;
    int	   cbind;
    pfVec3 *coords;
    pfVec3 *norms;
    pfVec4 *colors;
    struct _myLines *next;
} myLines;

/*----------------------------------------------*/
/* dwbGeoState					*/
/* some of the more widely changed render state */
/* info. This could also include fog, light 	*/
/* model etc 					*/
/*----------------------------------------------*/

typedef struct _dwbGeoState 
{
    groupValues values;
    pfGeoState	*pfgs;
    pfdGeoBuilder *builder;
    myLines	*lineList;
    struct _dwbGeoState *next;
} dwbGeoState;			



/* structure that will contain the file to be loaded in memory */
/* and the pointers needed to access that data */
typedef struct MemPtr {
                            char *ptr;
                            int pos; /* offset from file */
                            int length; /* length of the file */
} memPtr;



/*----------------------------------------------*/
/* dwbFile					*/
/* runtime info and Performer sub-tree as the 	*/
/* file is being loaded.			*/
/*----------------------------------------------*/

typedef struct _dwbFile
{
    char 	 fname[PF_MAXSTRING];
    memPtr       file;   /* copy of file in memory buffer */
    pfGroup 	*root;
    dwbHeader 	 header;    
    float        colorTable[4224][3];	/* all shades available in RGB 	*/
    dwbMatInfo  *materialTable;
    pfMaterial  **materials;
    int 	 materialsRead;
    dwbTexture  *textureTable;
    int 	 texturesRead;

    instRefList *instList;        /* contains list of instance definitions */
    refBeforeDefList *refList;    /* contains list of instance references  */
			          /* before they have been defined         */

    dwbGeoState *gsList;   /*global list of ALL dwbGeoStates in this model */
    int          treeDepth;
    treeStack    *stack;
    int          stackDepth;
    float 	 unitChange;
    groupValues  currentGroup;       /* contains the values of the */   
				     /* most recently read group */
    float	 spare[3];	     /* quadword align */
    float	 lodScale;		     /*lod scale for externals */
    int 	 *pfTexIndex;        /* local mapping of tex indice into */
                                     /* global texture table */

    int          containsInstancing; /* set when an instance or link is set */
    int          containsLinks;      /* database has links */
    int          containsDCS;        /* database has DCS's */
    int          refBeforeDef;   /* instances are referred before defined */
    int 	 insideExternal; /* set to indicate that an external is being */
				 /* processed */ 
    int          renderGroup;    /* set when the currentGroup is a */
                                 /* render group                   */
} dwbFile;

typedef struct _dwbMatrix
{
  float mat[4][4];
} dwbMatrix;


/*----------------------------------------------*/
/* dwbBSpline                                   */
/*----------------------------------------------*/

typedef struct _dwbBSpline
{
    int order;          
    int numKnots;        
    int drawCntrlPnts;
    short spare[2];
    float spare3[6];

}dwbBSpline;



/*----------------------------------------------*/
/* dwbArc                                        */
/*----------------------------------------------*/

typedef struct _dwbArc 
{
  float pfOrigin[3]; 
  float fStartAngle;
  float fEndAngle;
  float fRadius;
  float pfNormal[3];
  float matrix[4][4];

  short drawstyle;
  short layer;
  short color;

  short linewidth;
  short linestyle;

  struct {
    unsigned sparebits: 32;
  } flags;

  dwbUserRec udata;

  unsigned int packed_color;

  float spare2[8];
} dwbArc;


/*
 * Light point structures
*/

/*----------------------------------------------------------------------*/
/* dwbLightPtRec                                                        */
/*----------------------------------------------------------------------*/
/* This is a special node of "additional" info. associated with the prev*/
/* read dwbfaceRec Node. That face was flagged as a bspline face. There */
/* will be 2 variable-length additional records following this one. one */
/* is the knots array and one is the control points array               */
/*                                                                      */
/*----------------------------------------------------------------------*/

typedef struct _dwbLightPt
{
    short ltype;                        /* light type as follows...     */
                                        /* 1  = RUNWAY                  */
                                        /* 2  = HIGH_INT_RUNWAY         */
                                        /* 3  = MED_INT_RUNWAY          */
                                        /* 4  = THRESHOLD_STROBE        */
                                        /* 5  = TOUCHDOWN               */
                                        /* 6  = CENTERLINE              */
                                        /* 7  = TAXIWAY_TURNOFF         */
                                        /* 8  = ALS                     */
                                        /* 9  = HIGH_INT_ALS            */
                                        /* 10 = MED_INT_ALS             */
                                        /* 11 = SEQUENCED_FLASHING      */
                                        /* 12 = ALSF_I                  */
                                        /* 13 = ALSF_II                 */
                                        /* 14 = SHORT_ALSF              */
                                        /* 15 = MED_ALSF                */
                                        /* 16 = RUNWAY_ALIGNMENT        */
                                        /* 17 = SHORT_ALS               */
                                        /* 18 = MED_ALS                 */
                                        /* 19 = LDIN                    */
                                        /* 20 = ODALS                   */
                                        /* 21 = VASI                    */
                                        /* 22 = VASI_3BAR               */
                                        /* 23 = TVASI                   */
                                        /* 24 = PAPI                    */
                                        /* 25 = THRESHOLD               */
                                        /* 26 = TAXIWAY                 */
                                        /* 27 = BEACON                  */
                                        /* 28 = OBSTRUCTION             */
                                        /* 29 = DISPLACED_THRESHOLD     */
                                        /* 30 = VEHICLE_FRONT           */
                                        /* 31 = VEHICLE_REAR            */
                                        /* 32 = CITY                    */

    short directionality;               /* 1 = omni directional         */
                                        /* 2 = uni directional          */
                                        /* 3 = bi directional           */

    short shape;                        /* 1 = single point             */
                                        /* 2 = straight string          */
                                        /* 3 = curved string            */
                                        /* 4 = random                   */
    short suppress_last_light;          /* dont draw last light (eg last*/
                                        /* light is first light in      */
                                        /* another string)              */

    float dir[3];                       /* normalized direction vector  */
    float lwidth;                       /* angle around vector          */
    float lheight;                      /* angle around vector          */
    float diam;                         /* diameter of the bulb         */

    double intensity;                   /* real world intensity -       */
                                        /* candelas ?                   */
    double intensity_variance;          /* ie along a string or with    */
                                        /* time                         */

    float calligraphic_priority;        /* 1.0 is please make this light*/
                                        /* pt node calligraphic         */

    float spare2[13];

}dwbLightPt;

typedef struct _dwbLightString
{

  float position[3];
  struct _dwbLightString *next;

  double qwa;

} dwbLightString;


/* The following macros have been implemented to have exactly the same */
/* functionality as fread & fseek but operate on a memory buffer instead */
/* of a file. This speeds up processing considerably at the expense of */
/* error checking on the number of bytes read. They should be used by any */
/* user defined callbacks to access the file structure */

#define  memRead(toPtr,size, nitems, filePtr) \
{ \
  memcpy((toPtr),((filePtr)->ptr + (filePtr)->pos), (size) * (nitems)); \
  (filePtr)->pos += (size) * (nitems); \
}

#define memSeek(filePtr, offset, pos2) \
{\
  switch (pos2) { \
    case  SEEK_SET:   (filePtr)->pos = offset; break; \
    case  SEEK_CUR:   (filePtr)->pos += offset; \
                      break; \
  } \
}


int setUserFuncs(int type, int (func)(int type, void *rec,
                 pfNode *current, pfNode *parent, memPtr *ifp));

#ifdef __cplusplus
}
#endif

#endif /* __PFDWB_H__ */







