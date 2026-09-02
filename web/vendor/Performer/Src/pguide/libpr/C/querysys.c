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
 * $Revision: 1.16 $
 *
 */

#include <Performer/pr.h>

static char *glstr[] = {"IRISGL",  "OPENGL"};

int main (int argc, char **argv)
{
    int ret;
    /* Initialize Performer */
    pfInit();

    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"Mach String: %s", pfGetMachString());
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"Release String: %s", pfGetRelString());

    pfQuerySys(PFQSYS_MAJOR_VERSION, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"Major Version: %d", ret);
    pfQuerySys(PFQSYS_MINOR_VERSION, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"Minor Version: %d", ret);
    pfQuerySys(PFQSYS_MAINT_VERSION, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"Maint Version: %d", ret);

    pfQuerySys(PFQSYS_GL, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"GL TYPE: %s", glstr[ret]);
    
    pfNotify(PFNFY_NOTICE,PFNFY_PRINT,"Machine Configuration:");
    
    pfQuerySys(PFQSYS_NUM_CPUS, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tNUM_CPUS: %d", ret);
    
    pfQuerySys(PFQSYS_NUM_SCREENS, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tNUM_SCREENS: %d", ret);
    
    pfQuerySys(PFQSYS_SIZE_PIX_X, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tSIZE_PIX_X: %d", ret);
    
    pfQuerySys(PFQSYS_SIZE_PIX_Y, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tSIZE_PIX_Y: %d", ret);
    
    pfQuerySys(PFQSYS_MAX_SNG_RGB_BITS, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_SNG_RGB_BITS: %d", ret);
    pfQuerySys(PFQSYS_MAX_SNG_ALPHA_BITS, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_SNG_ALPHA_BITS: %d", ret);
    pfQuerySys(PFQSYS_MAX_SNG_ACCUM_BITS, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_SNG_ACCUM_BITS: %d", ret);
    
    pfQuerySys(PFQSYS_MAX_DBL_RGB_BITS, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_DBL_RGB_BITS: %d", ret);
    pfQuerySys(PFQSYS_MAX_DBL_ALPHA_BITS, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_DBL_ALPHA_BITS: %d", ret);
    pfQuerySys(PFQSYS_MAX_DBL_ACCUM_BITS, &ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_DBL_ACCUM_BITS: %d", ret);
    
    pfQuerySys(PFQSYS_MAX_DEPTH_BITS,&ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_DEPTH_BITS: %d", ret);

    pfQuerySys(PFQSYS_MAX_DEPTH_VAL,&ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_DEPTH_VAL: %d", ret);
    pfQuerySys(PFQSYS_MIN_DEPTH_VAL,&ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMIN_DEPTH_VAL: %d", ret);
    
    pfQuerySys(PFQSYS_MAX_STENCIL_BITS,&ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_STENCIL_BITS: %d", ret);
    
    pfQuerySys(PFQSYS_MAX_MS_SAMPLES,&ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_MS_SAMPLES: %d", ret);
    
    pfQuerySys(PFQSYS_MAX_MS_DEPTH_BITS,&ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_MS_DEPTH_BITS: %d", ret);
    
    pfQuerySys(PFQSYS_MAX_MS_STENCIL_BITS,&ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_MS_STENCIL_BITS: %d", ret);

    pfQuerySys(PFQSYS_TEXTURE_MEMORY_BYTES,&ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tTEXTURE_MEMORY_BYTES: %d", ret);

    pfQuerySys(PFQSYS_MAX_TEXTURE_SIZE,&ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_TEXTURE_SIZE: %d", ret);

    pfQuerySys(PFQSYS_MAX_CLIPTEXTURE_SIZE,&ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tMAX_CLIPTEXTURE_SIZE: %d", ret);

    pfQuerySys(PFQSYS_CLIPTEXTURE_CENTER_ALIGNMENT,&ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tCLIPTEXTURE_CENTER_ALIGNMENT: %d", ret);

    pfQuerySys(PFQSYS_TEX_SUBLOAD_ALIGNMENT_S,&ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tTEX_SUBLOAD_ALIGNMENT_S: %d", ret);
    pfQuerySys(PFQSYS_TEX_SUBLOAD_ALIGNMENT_T,&ret);
    pfNotify(PFNFY_NOTICE,PFNFY_MORE,"\tTEX_SUBLOAD_ALIGNMENT_T: %d", ret);

    return 0;
}













