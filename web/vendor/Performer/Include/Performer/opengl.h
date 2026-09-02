/*
 *
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
 *
 * file: opengl.h	  Include file for gl-specific stuff for performer
 * ----------
 *
 * $Revision: 1.63 $ $Date: 2002/04/11 23:44:51 $
 *
 */


#ifndef _PF_OPENGL_H_
#define _PF_OPENGL_H_	 /* guard */


#ifndef IRISGL /* OPENGL */

/* user enabled typedefs for easier irisgl->opengl */
#ifdef  IRISGL_TYPES
typedef GLboolean Boolean;
typedef GLuint Object;
typedef GLfloat Matrix[16];
typedef GLushort Linestyle;
typedef GLint Screencoord;
#endif

#define GLOBJECT GLuint

/*--------------------------- Handy GL-neutral macros ----------------------*/
/* XXX we don't want to use to many of these as they will make removing
 * all of the ifdefs harder!!! Use these only to avoid extreme code
 * ugliness.
 */
#define GL_VERTEX3(_v) glVertex3fv(_v)
#define GL_COLOR3(_clr)	glColor3fv(_clr)
#define GL_COLOR4(_clr)	glColor4fv(_clr)
/*------------------------------------------------------------------------*/


/*--------------------------- Performer GL-based Constants -------------------------*/

/* pfShadeModel() */
#define PFSM_FLAT	    	GL_FLAT
#define PFSM_GOURAUD	    	GL_SMOOTH


/* pfAlphaFunc() */
#define PFAF_OFF        	GL_ALWAYS        
#define PFAF_NEVER	    	GL_NEVER
#define PFAF_LESS		GL_LESS
#define PFAF_EQUAL		GL_EQUAL
#define PFAF_LEQUAL		GL_LEQUAL
#define PFAF_GREATER		GL_GREATER
#define PFAF_NOTEQUAL      	GL_NOTEQUAL
#define PFAF_GEQUAL        	GL_GEQUAL        
#define PFAF_ALWAYS        	GL_ALWAYS        

/* pfCullFace() */
#define PFCF_OFF	    	0
#define PFCF_BACK	    	GL_BACK
#define PFCF_FRONT	    	GL_FRONT
#define PFCF_BOTH	    	GL_FRONT_AND_BACK

/*---------------------------- pfTexture -------------------------*/

#define TX_WRAP			0x300
#define TX_EXTERNAL_FORMAT      0x700
#define TX_INTERNAL_FORMAT      0x600
#define TX_BICUBIC              0x230
#define TX_SHARPEN              0x240
#define TX_MIPMAP_QUADLINEAR    0x280
#define TX_TILE			0x400
#define TX_SELECT		0x303

#define PFTEX_WRAP		TX_WRAP
#define PFTEX_WRAP_S		GL_TEXTURE_WRAP_S
#define PFTEX_WRAP_T		GL_TEXTURE_WRAP_T
#ifndef EXT_texture3D /* EXT_texture3D */
#define GL_TEXTURE_WRAP_R_EXT	0x8072
#endif
#define PFTEX_WRAP_R		GL_TEXTURE_WRAP_R_EXT

#define PFTEX_REPEAT		GL_REPEAT
#define PFTEX_CLAMP		GL_CLAMP


/* The linux build (Mesa) is currently based on OpenGL 1.2 so many
   of the tokens are there, without the _EXT or _SGIS */
#if defined(__linux__) || defined(WIN32)
#ifndef GL_POLYGON_OFFSET_EXT
#define GL_POLYGON_OFFSET_EXT GL_POLYGON_OFFSET_FILL
#endif
#ifndef GL_EXT_texture_object
#define GL_EXT_texture_object
#endif
#ifndef GL_UNSIGNED_INT_10_10_10_2
#define GL_UNSIGNED_INT_10_10_10_2 0x8036
#endif
#ifdef  GL_UNSIGNED_INT_10_10_10_2
#ifndef GL_UNSIGNED_INT_10_10_10_2_EXT
#define GL_UNSIGNED_INT_10_10_10_2_EXT GL_UNSIGNED_INT_10_10_10_2
#endif
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8
#define GL_UNSIGNED_INT_8_8_8_8 0x8035
#endif
#ifdef GL_UNSIGNED_INT_8_8_8_8
#ifndef GL_UNSIGNED_INT_8_8_8_8_EXT
#define GL_UNSIGNED_INT_8_8_8_8_EXT GL_UNSIGNED_INT_8_8_8_8
#endif
#endif
#ifndef GL_UNSIGNED_SHORT_4_4_4_4
#define GL_UNSIGNED_SHORT_4_4_4_4  0x8033
#endif
#ifdef  GL_UNSIGNED_SHORT_4_4_4_4
#ifndef GL_UNSIGNED_SHORT_4_4_4_4_EXT
#define GL_UNSIGNED_SHORT_4_4_4_4_EXT GL_UNSIGNED_SHORT_4_4_4_4
#endif
#endif
#ifndef GL_UNSIGNED_BYTE_3_3_2
#define GL_UNSIGNED_BYTE_3_3_2 0x8032
#endif
#ifdef  GL_UNSIGNED_BYTE_3_3_2
#ifndef GL_UNSIGNED_BYTE_3_3_2_EXT
#define GL_UNSIGNED_BYTE_3_3_2_EXT GL_UNSIGNED_BYTE_3_3_2
#endif
#endif
#ifndef GL_TEXTURE_MAX_LOD_SGIS
#define GL_TEXTURE_MAX_LOD_SGIS GL_TEXTURE_MAX_LOD
#endif
#ifndef GL_TEXTURE_MIN_LOD_SGIS
#define GL_TEXTURE_MIN_LOD_SGIS GL_TEXTURE_MIN_LOD
#endif
#ifndef GL_TEXTURE_MAX_LEVEL_SGIS
#define GL_TEXTURE_MAX_LEVEL_SGIS GL_TEXTURE_MAX_LEVEL
#endif
#ifndef GL_DETAIL_TEXTURE_2D_SGIS
#define GL_DETAIL_TEXTURE_2D_SGIS 0xdeadbeef
#endif
#ifndef GL_TEXTURE_3D_EXT
#define GL_TEXTURE_3D_EXT 0xbeefdead
#endif
#endif

#define PFTEX_TILE		TX_TILE
#define PFTEX_SELECT		TX_SELECT

#ifndef GL_EXT_texture /* EXT_texture */
#define GL_ALPHA4_EXT                       0x803B
#define GL_ALPHA8_EXT                       0x803C
#define GL_ALPHA12_EXT                      0x803D
#define GL_ALPHA16_EXT                      0x803E
#define GL_LUMINANCE4_EXT                   0x803F
#define GL_LUMINANCE8_EXT                   0x8040
#define GL_LUMINANCE12_EXT                  0x8041
#define GL_LUMINANCE16_EXT                  0x8042
#define GL_LUMINANCE4_ALPHA4_EXT            0x8043
#define GL_LUMINANCE6_ALPHA2_EXT            0x8044
#define GL_LUMINANCE8_ALPHA8_EXT            0x8045
#define GL_LUMINANCE12_ALPHA4_EXT           0x8046
#define GL_LUMINANCE12_ALPHA12_EXT          0x8047
#define GL_LUMINANCE16_ALPHA16_EXT          0x8048
#define GL_INTENSITY_EXT                    0x8049
#define GL_INTENSITY4_EXT                   0x804A
#define GL_INTENSITY8_EXT                   0x804B
#define GL_INTENSITY12_EXT                  0x804C
#define GL_INTENSITY16_EXT                  0x804D
#define GL_RGB2_EXT                         0x804E
#define GL_RGB4_EXT                         0x804F
#define GL_RGB5_EXT                         0x8050
#define GL_RGB8_EXT                         0x8051
#define GL_RGB10_EXT                        0x8052
#define GL_RGB12_EXT                        0x8053
#define GL_RGB16_EXT                        0x8054
#define GL_RGBA2_EXT                        0x8055
#define GL_RGBA4_EXT                        0x8056
#define GL_RGB5_A1_EXT                      0x8057
#define GL_RGBA8_EXT                        0x8058
#define GL_RGB10_A2_EXT                     0x8059
#define GL_RGBA12_EXT                       0x805A
#define GL_RGBA16_EXT                       0x805B
#endif /* ext_texture */

#ifndef GL_UNSIGNED_SHORT_5_5_5_1_EXT
#define GL_UNSIGNED_SHORT_5_5_5_1_EXT       0x8034
#endif

#ifndef GL_LINEAR_CLIPMAP_LINEAR_SGIX
#define GL_LINEAR_CLIPMAP_LINEAR_SGIX	    0x8170
#endif 

#ifndef GL_TEXTURE_CLIPMAP_CENTER_SGIX
#define GL_TEXTURE_CLIPMAP_CENTER_SGIX 	    0x8171
#endif

#ifndef GL_TEXTURE_CLIPMAP_FRAME_SGIX
#define GL_TEXTURE_CLIPMAP_FRAME_SGIX	    0x8172
#endif

#ifndef GL_TEXTURE_CLIPMAP_OFFSET_SGIX
#define GL_TEXTURE_CLIPMAP_OFFSET_SGIX	    0x8173
#endif

#ifndef GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX	
#define GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX	0x8174
#endif

#ifndef GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX
#define GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX	0x8175
#endif

#ifndef GL_TEXTURE_CLIPMAP_DEPTH_SGIX
#define GL_TEXTURE_CLIPMAP_DEPTH_SGIX		0x8176
#endif

#ifndef GL_MAX_CLIPMAP_DEPTH_SGIX
#define GL_MAX_CLIPMAP_DEPTH_SGIX		0x8177
#endif

#ifndef GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX
#define GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX	0x8178
#endif

#ifndef GL_POST_TEXTURE_FILTER_BIAS_SGIX
#define	GL_POST_TEXTURE_FILTER_BIAS_SGIX	0x8179
#endif

#ifndef GL_POST_TEXTURE_FILTER_SCALE_SGIX
#define GL_POST_TEXTURE_FILTER_SCALE_SGIX	0x817A
#endif

#ifndef GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX
#define GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX	0x817B
#endif

#ifndef GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX
#define	GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX	0x817C
#endif

#ifndef GL_REFERENCE_PLANE_SGIX
#define GL_REFERENCE_PLANE_SGIX			0x817D
#endif

#ifndef GL_REFERENCE_PLANE_EQUATION_SGIX
#define GL_REFERENCE_PLANE_EQUATION_SGIX	0x817E
#endif

#ifndef GL_IR_INSTRUMENT1_SGIX
#define GL_IR_INSTRUMENT1_SGIX			0x817F
#endif

#ifndef GL_INSTRUMENT_BUFFER_POINTER_SGIX
#define GL_INSTRUMENT_BUFFER_POINTER_SGIX	0x8180
#endif

#ifndef GL_INSTRUMENT_MEASUREMENTS_SGIX
#define GL_INSTRUMENT_MEASUREMENTS_SGIX		0x8181
#endif

#ifndef GL_LIST_PRIORITY_SGIX
#define GL_LIST_PRIORITY_SGIX			0x8182
#endif

#ifndef GL_CALLIGRAPHIC_FRAGMENT_SGIX
#define GL_CALLIGRAPHIC_FRAGMENT_SGIX		0x8183
#endif

#ifndef GL_PIXEL_TEX_GEN_Q_CEILING_SGIX
#define GL_PIXEL_TEX_GEN_Q_CEILING_SGIX		0x8184
#endif

#ifndef GL_PIXEL_TEX_GEN_Q_ROUND_SGIX
#define GL_PIXEL_TEX_GEN_Q_ROUND_SGIX		0x8185
#endif

#ifndef GL_PIXEL_TEX_GEN_Q_FLOOR_SGIX
#define GL_PIXEL_TEX_GEN_Q_FLOOR_SGIX		0x8186
#endif

#ifndef GL_PIXEL_TEX_GEN_ALPHA_REPLACE_SGIX
#define GL_PIXEL_TEX_GEN_ALPHA_REPLACE_SGIX	0x8187
#endif

#ifndef GL_PIXEL_TEX_GEN_ALPHA_NO_REPLACE_SGIX
#define GL_PIXEL_TEX_GEN_ALPHA_NO_REPLACE_SGIX	0x8188
#endif

#ifndef GL_PIXEL_TEX_GEN_ALPHA_LS_SGIX
#define GL_PIXEL_TEX_GEN_ALPHA_LS_SGIX		0x8189
#endif

#ifndef GL_PIXEL_TEX_GEN_ALPHA_MS_SGIX
#define GL_PIXEL_TEX_GEN_ALPHA_MS_SGIX		0x818A
#endif

#ifndef GL_FRAMEZOOM_SGIX
#define GL_FRAMEZOOM_SGIX			0x818B
#endif

#ifndef GL_FRAMEZOOM_FACTOR_SGIX
#define GL_FRAMEZOOM_FACTOR_SGIX		0x818C
#endif

#ifndef GL_MAX_FRAMEZOOM_FACTOR_SGIX
#define GL_MAX_FRAMEZOOM_FACTOR_SGIX		0x818D
#endif

#ifndef GL_TEXTURE_LOD_BIAS_S_SGIX
#define	GL_TEXTURE_LOD_BIAS_S_SGIX		0x818E
#endif

#ifndef GL_TEXTURE_LOD_BIAS_T_SGIX
#define GL_TEXTURE_LOD_BIAS_T_SGIX		0x818F
#endif

#ifndef GL_TEXTURE_LOD_BIAS_R_SGIX
#define GL_TEXTURE_LOD_BIAS_R_SGIX		0x8190
#endif

#ifndef GL_GENERATE_MIPMAP_SGIS
#define GL_GENERATE_MIPMAP_SGIS			0x8191
#endif

#ifndef GL_GENERATE_MIPMAP_HINT_SGIS
#define	GL_GENERATE_MIPMAP_HINT_SGIS		0x8192
#endif

#ifndef GL_GEOMETRY_DEFORMATION_SGIX
#define GL_GEOMETRY_DEFORMATION_SGIX		0x8194
#endif

#ifndef GL_TEXTURE_DEFORMATION_SGIX
#define GL_TEXTURE_DEFORMATION_SGIX		0x8195
#endif

#ifndef GL_DEFORMATIONS_MASK_SGIX
#define GL_DEFORMATIONS_MASK_SGIX		0x8196
#endif

#ifndef GL_MAX_DEFORMATION_ORDER_SGIX
#define GL_MAX_DEFORMATION_ORDER_SGIX		0x8197
#endif

#ifndef GL_FOG_OFFSET_SGIX
#define GL_FOG_OFFSET_SGIX			0x8198
#endif

#ifndef GL_FOG_OFFSET_VALUE_SGIX
#define GL_FOG_OFFSET_VALUE_SGIX		0x8199
#endif


#ifndef GL_TEXTURE_COMPARE_SGIX
#define GL_TEXTURE_COMPARE_SGIX			0x819A
#endif

#ifndef GL_TEXTURE_COMPARE_OPERATOR_SGIX
#define GL_TEXTURE_COMPARE_OPERATOR_SGIX	0x819B
#endif

#ifndef GL_TEXTURE_LEQUAL_R_SGIX
#define GL_TEXTURE_LEQUAL_R_SGIX		0x819C
#endif

#ifndef GL_TEXTURE_GEQUAL_R_SGIX
#define GL_TEXTURE_GEQUAL_R_SGIX		0x819D
#endif

#ifndef GL_DEPTH_COMPONENT16_SGIX
#define GL_DEPTH_COMPONENT16_SGIX		0x81A5
#endif

#ifndef GL_DEPTH_COMPONENT24_SGIX
#define GL_DEPTH_COMPONENT24_SGIX		0x81A6
#endif

#ifndef GL_DEPTH_COMPONENT32_SGIX
#define GL_DEPTH_COMPONENT32_SGIX		0x81A7
#endif


#define PFTEX_BYTE		GL_BYTE
#define PFTEX_UNSIGNED_BYTE	GL_UNSIGNED_BYTE
#define PFTEX_SHORT		GL_SHORT
#define PFTEX_UNSIGNED_SHORT	GL_UNSIGNED_SHORT
#define PFTEX_INT		GL_INT
#define PFTEX_UNSIGNED_INT	GL_UNSIGNED_INT
#define PFTEX_FLOAT		GL_FLOAT
#define PFTEX_UNSIGNED_BYTE_3_3_2	GL_UNSIGNED_BYTE_3_3_2_EXT
#define PFTEX_UNSIGNED_SHORT_4_4_4_4	GL_UNSIGNED_SHORT_4_4_4_4_EXT
#define PFTEX_UNSIGNED_SHORT_5_5_5_1	GL_UNSIGNED_SHORT_5_5_5_1_EXT
#define PFTEX_UNSIGNED_INT_8_8_8_8	GL_UNSIGNED_INT_8_8_8_8_EXT
#define PFTEX_UNSIGNED_INT_10_10_10_2	GL_UNSIGNED_INT_10_10_10_2_EXT

#define PFTEX_PACK_8		GL_UNSIGNED_BYTE
#define PFTEX_PACK_16		GL_UNSIGNED_SHORT
#define PFTEX_LUMINANCE		GL_LUMINANCE
#define PFTEX_LUMINANCE_ALPHA	GL_LUMINANCE_ALPHA
#define PFTEX_RGB		GL_RGB
#define PFTEX_RGBA		GL_RGBA

#ifndef GL_RGB5_A1_EXT
#define GL_RGB5_A1_EXT 		0x8057
#endif
#define PFTEX_RGB5_A1		GL_RGB5_A1_EXT
#define PFTEX_RGB_5A_1		GL_RGB5_A1_EXT

#define PFTEX_I_8		GL_LUMINANCE8_EXT
#define PFTEX_I_12A_4		GL_LUMINANCE12_ALPHA4_EXT
#define PFTEX_IA_8		GL_LUMINANCE8_ALPHA8_EXT
#define PFTEX_RGB_5		GL_RGB5_EXT
#define PFTEX_RGB_4		GL_RGB4_EXT
#define PFTEX_RGB_8             GL_RGB8_EXT
#define PFTEX_RGBA_4		GL_RGBA4_EXT
#define PFTEX_IA_12 		GL_LUMINANCE12_ALPHA12_EXT
#define PFTEX_RGBA_8		GL_RGBA8_EXT
#define PFTEX_RGB_12		GL_RGB12_EXT
#define PFTEX_RGBA_12 		GL_RGBA12_EXT
#define PFTEX_I_16		GL_INTENSITY16_EXT


/*-------------------------- pfTexEnv ----------------------------*/

#ifndef TV_SHADOW
/* IRIS GL texture environment definitions */
#define TV_MODULATE		0x101
#define TV_BLEND		0x102
#define TV_DECAL		0x103
#define TV_COLOR		0x200
#define TV_SHADOW               0x104
#define TV_ALPHA                0x105
#define TV_COMPONENT_SELECT     0x300
#define TV_I_GETS_R             0x310
#define TV_I_GETS_G             0x320
#define TV_I_GETS_B             0x330
#define TV_I_GETS_A             0x340
#define TV_IA_GETS_RG           0x350
#define TV_IA_GETS_BA           0x360
#define TV_I_GETS_I             0x370
#define TV_NULL			0x000
#endif

#define PFTE_MODULATE		GL_MODULATE
#define PFTE_BLEND		GL_BLEND
#define PFTE_DECAL		GL_DECAL
#define PFTE_REPLACE		GL_REPLACE
#define PFTE_ALPHA		TV_ALPHA	    /* XXX not in OpenGL */
#define PFTE_ADD		GL_ADD	    
#define PFTE_BIAS		GL_TEXTURE_ENV_BIAS_SGIX	    

#define PFTE_COMP_I_GETS_R	GL_PIXEL_MAP_I_TO_R
#define PFTE_COMP_I_GETS_G	GL_PIXEL_MAP_I_TO_G
#define PFTE_COMP_I_GETS_B	GL_PIXEL_MAP_I_TO_B
#define PFTE_COMP_I_GETS_A	GL_PIXEL_MAP_I_TO_A
#define PFTE_COMP_I_GETS_I	GL_PIXEL_MAP_I_TO_I
#define PFTE_COMP_IA_GETS_RG	TV_IA_GETS_RG
#define PFTE_COMP_IA_GETS_BA	TV_IA_GETS_BA

#define PFTE_COLOR		GL_TEXTURE_ENV_COLOR
#define PFTE_SHADOW		TV_SHADOW    /* XXX not in OpenGL */
#define PFTE_COMPONENT_SELECT	TV_COMPONENT_SELECT
#define PFTE_I_GETS_R		GL_PIXEL_MAP_I_TO_R
#define PFTE_I_GETS_G		GL_PIXEL_MAP_I_TO_G
#define PFTE_I_GETS_B		GL_PIXEL_MAP_I_TO_B
#define PFTE_I_GETS_A		GL_PIXEL_MAP_I_TO_A
#define PFTE_IA_GETS_RG		TV_IA_GETS_RG	    /* XXX not in OpenGL */
#define PFTE_IA_GETS_BA		TV_IA_GETS_BA	    /* XXX not in OpenGL */
#define PFTE_I_GETS_I		GL_PIXEL_MAP_I_TO_I


/*-------------------------- pfTexGen ----------------------------*/

#ifndef GL_OBJECT_DISTANCE_TO_LINE_SGIS
#define GL_OBJECT_DISTANCE_TO_LINE_SGIS 0x000081f3
#define GL_OBJECT_LINE_SGIS		0x000081f7
#endif
#ifndef GL_EYE_DISTANCE_TO_LINE_SGIS
#define GL_EYE_DISTANCE_TO_LINE_SGIS    0x000081f2
#define GL_EYE_LINE_SGIS		0x000081f6
#endif

/* pfTGenMode() */
#define PFTG_OFF			0
#define PFTG_EYE_LINEAR			GL_EYE_LINEAR
#define PFTG_EYE_LINEAR_IDENT		0x23ff
#define PFTG_OBJECT_LINEAR		GL_OBJECT_LINEAR
#define PFTG_SPHERE_MAP			GL_SPHERE_MAP
#define PFTG_OBJECT_DISTANCE_TO_LINE	GL_OBJECT_DISTANCE_TO_LINE_SGIS
#define PFTG_OBJECT_LINE		GL_OBJECT_LINE_SGIS
#define PFTG_EYE_DISTANCE_TO_LINE	GL_EYE_DISTANCE_TO_LINE_SGIS
#define PFTG_EYE_DISTANCE_TO_LINE_IDENT 0x23fe
#define PFTG_EYE_LINE			GL_EYE_LINE_SGIS

/*--------------------------- pfLight ---------------------------*/

/* these are hardwired in an array dimension which also initializes with */
/* the light id tokens in pfLight.C so it's a little rigid right now */

#define PF_MAX_LIGHTS 8 /* XXX need to query opengl for max lights */

/*------------------------- pfMaterial --------------------------*/

#define PFMTL_EMISSION	    	GL_EMISSION
#define PFMTL_AMBIENT	    	GL_AMBIENT
#define PFMTL_DIFFUSE	    	GL_DIFFUSE
#define PFMTL_SPECULAR	    	GL_SPECULAR

#define PFMTL_CMODE_EMISSION	GL_EMISSION
#define PFMTL_CMODE_SPECULAR	GL_SPECULAR
#define PFMTL_CMODE_AMBIENT	GL_AMBIENT
#define PFMTL_CMODE_DIFFUSE	GL_DIFFUSE
#define PFMTL_CMODE_AMBIENT_AND_DIFFUSE	GL_AMBIENT_AND_DIFFUSE
#define PFMTL_CMODE_OFF		0

/* IrisGL compatability defines */
#define PFMTL_CMODE_COLOR	0	/* NA in OpenGL */
#define PFMTL_CMODE_NULL	0
#define PFMTL_CMODE_AD		PFMTL_CMODE_AMBIENT_AND_DIFFUSE

#define PFLT_AMBIENT	    	GL_AMBIENT
#define PFLT_DIFFUSE	    	GL_DIFFUSE
#define PFLT_SPECULAR	    	GL_SPECULAR

/*----------------------------- pfFog --------------------------------*/

#define PFFOG_ON		PF_ON
#define PFFOG_OFF		PF_OFF

#ifndef GL_FOG_FUNC_SGIS
#define GL_FOG_FUNC_SGIS	0x812A
#endif

/* XXX no per-vertex fog in opengl 
 * - we should nix the VTX and PIX and have PFFOG_LIN, etc.
 */
#define PFFOG_VTX_LIN	    	GL_LINEAR
#define PFFOG_PIX_LIN	    	GL_LINEAR
#define PFFOG_VTX_EXP	    	GL_EXP
#define PFFOG_PIX_EXP	    	GL_EXP
#define PFFOG_VTX_EXP2	    	GL_EXP2
#define PFFOG_PIX_EXP2	    	GL_EXP2
#define PFFOG_PIX_SPLINE	GL_FOG_FUNC_SGIS /* FG_PIX_SPLINE */


#endif /* GL type */

#endif	/* _PF_OPENGL_H_ guard */





