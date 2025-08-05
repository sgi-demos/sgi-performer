
// This file contains the public interface from the Performer
// header file Performer/opengl.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <GL/gl.h>
#include <GL/glu.h>
#include <Performer/opengl.h>
%}


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

