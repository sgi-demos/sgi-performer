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
 *
 * file: irisgl.h	  Include file for gl-specific stuff for performer
 * ---------------
 *
 * $Revision: 1.40 $
 * $Date: 1999/06/16 19:52:03 $
 *
 */

#ifndef _PF_IRISGL_H_
#define _PF_IRISGL_H_	 /* guard */

#ifdef IRISGL

/* 
 * defines for supporting move to opengl 
 */

/* lighting defines */
#define PF_MAX_LIGHTS 			    MAXLIGHTS


#define GLOBJECT Object

/* typedefs for irisgl->opengl */
#ifndef _TYPES_SET_UP_
typedef float GLfloat;
#endif

/*--------------------------- Handy GL-neutral macros ----------------------*/

#define GL_VERTEX3(_clr) v3f(_clr)
#define GL_COLOR3(_clr)	c3f(_clr)
#define GL_COLOR4(_clr)	c4f(_clr)

/*--------------------- pre-RE RE defines  and declarations ----------------*/

#ifndef MSA_MASK
/* defines for msalpha */
#define MSA_MASK		0x0
#define MSA_MASK_ONE		0x1
#define MSA_ALPHA		0x2
#endif

#ifndef MSS_POINT
/* defines for mssample */
#define MSS_POINT		0x0
#define MSS_AREA		0x1
#define MSS_CENTER_POINT	0x2
#endif

#ifndef MSP_DEFAULT
/* defines for mspattern */
#define MSP_DEFAULT		0x0
#define MSP_2PASS_0		0x1
#define MSP_2PASS_1		0x2
#define MSP_4PASS_0		0x3
#define MSP_4PASS_1		0x4
#define MSP_4PASS_2		0x5
#define MSP_4PASS_3		0x6
#endif

#ifndef BF_CA
/* defines for blendfunction */
#define BF_CA			9
#define BF_MCA			10
#define BF_CC			11
#define BF_MCC			12
#endif

#ifndef TX_DETAIL
/* defines for texture targets */
#define TX_TEXTURE_DETAIL       1 /* This binds a texture to the DETAIL texture resource */
#define TX_TEXTURE_IDLE         2
/* defines for texture filters and texdef param tokens */
#define TX_WRAP_R		0x330
#define TX_DETAIL               0xe00
#define TX_MAGFILTER_COLOR      0xa00
#define TX_MAGFILTER_ALPHA      0xb00
#define TX_BICUBIC              0x230
#define TX_SHARPEN              0x240
#define TX_MODULATE_DETAIL      0x250
#define TX_ADD_DETAIL           0x260
#define TX_TRILINEAR            0x270
#define TX_MIPMAP_QUADLINEAR    0x280
#define TX_BICUBIC_GEQUAL       0x290
#define TX_BICUBIC_LEQUAL       0x2a0
#define TX_BILINEAR_GEQUAL      0x2b0
#define TX_BILINEAR_LEQUAL      0x2c0
/* choices for TX_INTERNAL_FORMAT */
#define TX_EXTERNAL_FORMAT      0x700
#define TX_PACK_8               0x710   /* just like STAPUFT 4.0 */
#define TX_PACK_16              0x720   /* */
#define TX_INTERNAL_FORMAT      0x600
/* choices for TX_INTERNAL_FORMAT */
#define TX_I_12A_4              0x610   /* 1-comp 2-comp               full    speed */
#define TX_I_8			0x620   /* 1-comp                      full    speed */
#define TX_IA_8                 0x620   /*        2-comp               full    speed */
#define TX_RGB_5                0x630   /*               3-comp        full    speed */
#define TX_RGBA_4               0x640   /*                      4-comp full    speed */
#define TX_IA_12                0x650   /*        2-comp               half    speed */
#define TX_RGBA_8               0x660   /*                      4-comp half    speed */
#define TX_RGB_12               0x680   /*               3-comp 4-comp quarter speed */
#define TX_RGBA_12              0x670   /*               3-comp 4-comp third   speed */
#define TX_I_16                 0x690   /* should only be used in SHADOW mode */
#define TX_MIPMAP_FILTER_KERNEL 0x900   /* seperable 8x8x8 filter kernel */
#define TX_FRAMEBUFFER_SRC                      0x1000
#define TX_SUBTEXLOAD                           0x2000
#define TX_CONTROL_POINT        0xc00
#define TX_CONTROL_CLAMP        0xd00
#endif

#ifndef GD_MULTISAMPLE
#define GD_MULTISAMPLE		84
#endif



/*--------------------------- Performer GL-based Constants -------------------------*/

/* pfShadeModel() */
#define PFSM_FLAT	    	FLAT
#define PFSM_GOURAUD	    	GOURAUD

/* pfCullFace() */
#define PFCF_OFF	    	0
#define PFCF_BACK	    	1
#define PFCF_FRONT	    	2
#define PFCF_BOTH	    	3


/* pfAlphaFunc() */
#define PFAF_OFF        	AF_ALWAYS        
#define PFAF_NEVER	    	AF_NEVER
#define PFAF_LESS		AF_LESS
#define PFAF_EQUAL		AF_EQUAL
#define PFAF_LEQUAL		AF_LEQUAL
#define PFAF_GREATER		AF_GREATER
#define PFAF_NOTEQUAL      	AF_NOTEQUAL
#define PFAF_GEQUAL        	AF_GEQUAL        
#define PFAF_ALWAYS        	AF_ALWAYS        


/*---------------------------- pfTexture -------------------------*/

#define PFTEX_WRAP		TX_WRAP
#define PFTEX_WRAP_R		TX_WRAP_R
#define PFTEX_WRAP_S		TX_WRAP_S
#define PFTEX_WRAP_T		TX_WRAP_T
#define PFTEX_TILE		TX_TILE
#define PFTEX_REPEAT		TX_REPEAT
#define PFTEX_CLAMP		TX_CLAMP
#define PFTEX_SELECT		TX_SELECT

#define PFTEX_INT		TX_PACK_16
#define PFTEX_UNSIGNED_INT	TX_PACK_16
#define PFTEX_BYTE		TX_PACK_8
#define PFTEX_UNSIGNED_BYTE	TX_PACK_8
#define PFTEX_SHORT		TX_PACK_16
#define PFTEX_UNSIGNED_SHORT	TX_PACK_16
#define PFTEX_FLOAT		TX_PACK_16
#define PFTEX_PACK_8		TX_PACK_8
#define PFTEX_PACK_16		TX_PACK_16
#define PFTEX_UNSIGNED_BYTE_3_3_2	TX_PACK_8
#define PFTEX_UNSIGNED_SHORT_4_4_4_4	TX_PACK_16
#define PFTEX_UNSIGNED_SHORT_5_5_5_1	TX_PACK_16
#define PFTEX_UNSIGNED_INT_8_8_8_8	TX_PACK_16
#define PFTEX_UNSIGNED_INT_10_10_10_2	TX_PACK_16

#define PFTEX_I_12A_4		TX_I_12A_4
#define PFTEX_I_8		TX_I_8
#define PFTEX_IA_8   		TX_IA_8
#define PFTEX_RGB_5		TX_RGB_5
#define PFTEX_RGB_4		PFTEX_RGB_5
#define PFTEX_RGB_8             TX_RGB_8
#define PFTEX_RGBA_4		TX_RGBA_4
#define PFTEX_IA_12 		TX_IA_12
#define PFTEX_RGBA_8		TX_RGBA_8
#define PFTEX_RGB_12		TX_RGB_12
#define PFTEX_RGBA_12 		TX_RGBA_12
#define PFTEX_I_16		TX_I_16

#ifndef GL_RGB5_A1_EXT
/* XXX should these be OpenGL, or IRIS GL PM_ values ?? */
#define GL_RGB5_A1_EXT 		0x8057
#define GL_LUMINANCE		0x1909
#define GL_LUMINANCE_ALPHA	0x190A
#define GL_RGB			0x1907
#define GL_RGBA			0x1908
#endif
#define PFTEX_LUMINANCE		GL_LUMINANCE
#define PFTEX_LUMINANCE_ALPHA	GL_LUMINANCE_ALPHA
#define PFTEX_RGB		GL_RGB
#define PFTEX_RGBA		GL_RGBA
#define PFTEX_RGB5_A1		GL_RGB5_A1_EXT
#define PFTEX_RGB_5A_1		GL_RGB5_A1_EXT

/*-------------------------- pfTexEnv ----------------------------*/

#define PFTE_MODULATE		TV_MODULATE
#define PFTE_BLEND		TV_BLEND
#define PFTE_DECAL		TV_DECAL
#define PFTE_REPLACE		TV_DECAL
#define PFTE_ALPHA		TV_ALPHA
/* only supported in OpenGL */
#define PFTE_ADD		0x0104
#define PFTE_BIAS		0x80BE

#define PFTE_COMP_I_GETS_R	TV_I_GETS_R
#define PFTE_COMP_I_GETS_G	TV_I_GETS_G
#define PFTE_COMP_I_GETS_B	TV_I_GETS_B
#define PFTE_COMP_I_GETS_A	TV_I_GETS_A
#define PFTE_COMP_I_GETS_I	TV_I_GETS_I
#define PFTE_COMP_IA_GETS_RG	TV_IA_GETS_RG
#define PFTE_COMP_IA_GETS_BA	TV_IA_GETS_BA

#define PFTE_COLOR		TV_COLOR
#define PFTE_SHADOW		TV_SHADOW
#define PFTE_COMPONENT_SELECT	TV_COMPONENT_SELECT
#define PFTE_I_GETS_R		TV_I_GETS_R
#define PFTE_I_GETS_G		TV_I_GETS_G
#define PFTE_I_GETS_B		TV_I_GETS_B
#define PFTE_I_GETS_A		TV_I_GETS_A
#define PFTE_IA_GETS_RG		TV_IA_GETS_RG
#define PFTE_IA_GETS_BA		TV_IA_GETS_BA
#define PFTE_I_GETS_I		TV_I_GETS_I

/*-------------------------- pfTexGen ----------------------------*/

/* only in OpenGL */
#ifndef GL_OBJECT_DISTANCE_TO_LINE_SGIS
#define GL_OBJECT_DISTANCE_TO_LINE_SGIS 0x000081f3
#define GL_OBJECT_LINE_SGIS             0x000081f7
#endif
#ifndef GL_EYE_DISTANCE_TO_LINE_SGIS
#define GL_EYE_DISTANCE_TO_LINE_SGIS    0x000081f2
#define GL_EYE_LINE_SGIS                0x000081f6
#endif

/* pfTGenMode() */
#define PFTG_OFF			0
#define PFTG_EYE_LINEAR			TG_CONTOUR
#define PFTG_EYE_LINEAR_IDENT		8
#define PFTG_OBJECT_LINEAR		TG_LINEAR
#define PFTG_SPHERE_MAP			TG_SPHEREMAP
#define PFTG_OBJECT_DISTANCE_TO_LINE    GL_OBJECT_DISTANCE_TO_LINE_SGIS
#define PFTG_OBJECT_LINE                GL_OBJECT_LINE_SGIS
#define PFTG_EYE_DISTANCE_TO_LINE       GL_EYE_DISTANCE_TO_LINE_SGIS
#define PFTG_EYE_DISTANCE_TO_LINE_IDENT 0x23fe
#define PFTG_EYE_LINE                   GL_EYE_LINE_SGIS

/*------------------------- pfMaterial --------------------------*/

#define PFMTL_EMISSION	    	EMISSION
#define PFMTL_AMBIENT	    	AMBIENT
#define PFMTL_DIFFUSE	    	DIFFUSE
#define PFMTL_SPECULAR	    	SPECULAR

#define PFMTL_CMODE_EMISSION		LMC_EMISSION
#define PFMTL_CMODE_SPECULAR		LMC_SPECULAR
#define PFMTL_CMODE_AMBIENT		LMC_AMBIENT
#define PFMTL_CMODE_DIFFUSE		LMC_DIFFUSE
#define PFMTL_CMODE_AMBIENT_AND_DIFFUSE	LMC_AD
#define PFMTL_CMODE_OFF			LMC_NULL

/* IrisGL compatability defines */
#define PFMTL_CMODE_COLOR		LMC_COLOR	/* NA in OpenGL */
#define PFMTL_CMODE_NULL		PFMTL_CMODE_OFF
#define PFMTL_CMODE_AD			PFMTL_CMODE_AMBIENT_AND_DIFFUSE

#define PFLT_AMBIENT	    	AMBIENT
#define PFLT_DIFFUSE	    	DIFFUSE
#define PFLT_SPECULAR	    	SPECULAR


/*----------------------------- pfFog --------------------------------*/

#define PFFOG_ON		FG_ON
#define PFFOG_OFF		FG_OFF
#define PFFOG_VTX_EXP	    	FG_VTX_EXP
#define PFFOG_VTX_LIN	    	FG_VTX_LIN
#define PFFOG_PIX_EXP	    	FG_PIX_EXP
#define PFFOG_PIX_LIN	    	FG_PIX_LIN
#define PFFOG_VTX_EXP2	    	FG_VTX_EXP2
#define PFFOG_PIX_EXP2	    	FG_PIX_EXP2
#define PFFOG_PIX_SPLINE	1000	/* FG_PIX_SPLINE */

#define PFFOG_UNIFORM           1
#define PFFOG_LAYERED           2

#endif /* IRISGL */

#endif	/* _PF_IRISGL_H_ guard */
