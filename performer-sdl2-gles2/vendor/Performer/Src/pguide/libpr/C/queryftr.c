/*
 * Copyright 1995, Silicon Graphics, Inc.
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
 *
 * queryconfig.c: query machine configuration of target DISPLAY
 *
 * $Revision: 1.22 $
 *
 */

#include <Performer/pr.h>

static char *glstr[] = {"IRISGL",  "OPENGL"};


int ftrToks[] = {
PFQFTR_VSYNC, 
PFQFTR_VSYNC_SET, 
PFQFTR_GANGDRAW, 
PFQFTR_HYPERPIPE, 
PFQFTR_STEREO_IN_WINDOW, 
PFQFTR_PBUFFER, 
PFQFTR_TAG_CLEAR, 
PFQFTR_XSGIVC,
PFQFTR_MULTISAMPLE, 
PFQFTR_MULTISAMPLE_TRANSP, 
PFQFTR_MULTISAMPLE_ROUND_POINTS, 
PFQFTR_MULTISAMPLE_STENCIL, 
PFQFTR_COLOR_ABGR, 
PFQFTR_DISPLACE_POLYGON, 
PFQFTR_DECAL_PLANE, 
PFQFTR_POLYMODE, 
PFQFTR_TRANSPARENCY, 
PFQFTR_AALINES, 
PFQFTR_AAPOINTS, 
PFQFTR_MTL_CMODE, 
PFQFTR_FOG_SPLINE, 
PFQFTR_ALPHA_FUNC, 
PFQFTR_ALPHA_FUNC_COMPARE_REF, 
PFQFTR_BLENDCOLOR, 
PFQFTR_BLEND_FUNC_SUBTRACT, 
PFQFTR_BLEND_FUNC_MINMAX, 
PFQFTR_FRAMEZOOM, 
PFQFTR_GLSPRITE, 
PFQFTR_LIGHTPOINT, 
PFQFTR_DVR, 
PFQFTR_LMODEL_ATTENUATION, 
PFQFTR_LIGHT_ATTENUATION, 
PFQFTR_LIGHT_CLR_SPECULAR, 
PFQFTR_TEXTURE, 
PFQFTR_TEXTURE_16BIT_IFMTS, 
PFQFTR_TEXTURE_SUBTEXTURE, 
PFQFTR_TEXTURE_TRILINEAR, 
PFQFTR_TEXTURE_DETAIL, 
PFQFTR_TEXTURE_SHARPEN, 
PFQFTR_TEXTURE_3D, 
PFQFTR_TEXTURE_PROJECTIVE, 
PFQFTR_TEXTURE_EDGE_CLAMP, 
PFQFTR_TEXTURE_CLIPMAP, 
PFQFTR_TEXTURE_5551,
PFQFTR_TEXTURE_LOD_RANGE,
PFQFTR_TEXTURE_LOD_BIAS,
PFQFTR_READ_MSDEPTH_BUFFER, 
PFQFTR_COPY_MSDEPTH_BUFFER, 
PFQFTR_READ_TEXTURE_MEMORY, 
PFQFTR_COPY_TEXTURE_MEMORY, 
PFQFTR_TEXTURE_SHADOW,
PFQFTR_SHADOW_AMBIENT, 
PFQFTR_CALLIGRAPHIC,
PFQFTR_PIPE_STATS,
0
};

char ftrStrs[][PF_MAXSTRING] = {
"VSYNC                         ", 
"VSYNC_SET                     ", 
"GANGDRAW                      ", 
"HYPERPIPE                     ", 
"STEREO_IN_WINDOW              ", 
"PBUFFER              	       ", 
"TAG_CLEAR                     ", 
"XSGIVC			       ",
"MULTISAMPLE                   ", 
"MULTISAMPLE_TRANSP            ", 
"MULTISAMPLE_ROUND_POINTS      ", 
"MULTISAMPLE_STENCIL           ", 
"COLOR_ABGR                    ", 
"DISPLACE_POLYGON              ", 
"DECAL_PLANE                   ", 
"POLYMODE                      ", 
"TRANSPARENCY                  ", 
"AALINES                       ",
"AAPOINTS                      ",
"MTL_CMODE                     ", 
"FOG_SPLINE                    ", 
"ALPHA_FUNC                    ", 
"ALPHA_FUNC_COMPARE_REF        ", 
"BLENDCOLOR                    ", 
"BLEND_FUNC_SUBTRACT           ", 
"BLEND_FUNC_MINMAX             ", 
"FRAMEZOOM                     ", 
"GLSPRITE                      ", 
"LIGHTPOINT                    ", 
"DVR                           ", 
"LMODEL_ATTENUATION            ", 
"LIGHT_ATTENUATION             ", 
"LIGHT_CLR_SPECULAR            ", 
"TEXTURE                       ", 
"TEXTURE_16BIT_IFMTS           ", 
"TEXTURE_SUBTEXTURE            ", 
"TEXTURE_TRILINEAR             ", 
"TEXTURE_DETAIL                ", 
"TEXTURE_SHARPEN               ", 
"TEXTURE_3D                    ", 
"TEXTURE_PROJECTIVE            ", 
"TEXTURE_EDGE_CLAMP            ", 
"TEXTURE_CLIPMAP               ", 
"TEXTURE_5551                  ",
"TEXTURE_LOD_RANGE             ",
"TEXTURE_LOD_BIAS              ",
"READ_MSDEPTH_BUFFER           ", 
"COPY_MSDEPTH_BUFFER           ", 
"READ_TEXTURE_MEMORY           ", 
"COPY_TEXTURE_MEMORY           ", 
"TEXTURE_SHADOW                ",
"SHADOW_AMBIENT                ",
"CALLIGRAPHIC                  ",
"PIPE_STATS                    ",

"MULTITEXTURE                  ",              
"FRAGMENT_LIGHTING             ",         
"FOG_OFFSET                    ",                
"FOG_SCALE                     ",                 
"FOG_LAYERED                   ", 
"TEXTURE_ANISOTROPIC           ",          
NULL
};

char ftrValStrs[][PF_MAXSTRING] = {
    "false", "true", "FAST", "FAST"
};

int main (int argc, char **argv)
{
    int i, ret;
    /* Initialize Performer */
    pfInit();

    pfQuerySys(PFQSYS_GL, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"GL TYPE: %s", glstr[ret]);
    
    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Available Graphics Features:");
    
    for (i=0; ftrToks[i] != 0; i++) {
      pfQueryFeature(ftrToks[i], &ret);
      
      /* Tokens which return values not equal to false, true, or FAST */
      if (ret == 0)
	pfNotify(PFNFY_NOTICE, PFNFY_MORE, "\t%s = %s", 
		 ftrStrs[i], ftrValStrs[ret]);
      else
	pfNotify(PFNFY_NOTICE, PFNFY_MORE, "\t%s = \t%s", 
		 ftrStrs[i], ftrValStrs[ret]);
    }
    
    return 0;
}
