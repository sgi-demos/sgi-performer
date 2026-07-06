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
 */

/*
 * pfdCallbacks.c
 *
 * $Revision: 1.27 $
 * $Date: 2002/05/15 00:31:09 $
 *
 * >>> Brief description of purpose. <<<
 *
 */

#include <stdlib.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <Performer/image.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>

static char CallBacks[PF_MAXSTRING] = "pfCallbacksDPool";

PFDUDLLEXPORT void 
pfdPrintTexgenParams(void)
{
    pfDataPool *dp;
    float *paramsX, *paramsY;

    dp = pfAttachDPool(CallBacks);
    if (dp == NULL)
	pfdTexgenParams(NULL, NULL);
    paramsX = (float *)pfDPoolFind(dp, 1);
    paramsY = (float *)pfDPoolFind(dp, 2);

    pfNotify(PFNFY_INFO, PFNFY_PRINT, "pfdPrintTexgenParams");
    pfNotify(PFNFY_INFO, PFNFY_PRINT, 
	"  X axis parameters: %f\t%f\t%f\t%f",
	paramsX[0], paramsX[1], paramsX[2], paramsX[3]);
    pfNotify(PFNFY_INFO, PFNFY_PRINT, 
	"  Y axis parameters: %f\t%f\t%f\t%f",
	paramsY[0], paramsY[1], paramsY[2], paramsY[3]);
    pfNotify(PFNFY_INFO, PFNFY_PRINT, NULL);
}

PFDUDLLEXPORT void 
pfdGetTexgenParams(float *newParamsX,float *newParamsY)
{
    pfDataPool *dp;
    float *paramsX, *paramsY;
    int i;

    dp = pfAttachDPool(CallBacks);
    if (dp == NULL)
	pfdTexgenParams(NULL, NULL);
    paramsX = (float *)pfDPoolFind(dp, 1);
    paramsY = (float *)pfDPoolFind(dp, 2);
    for (i=0;i<4;i++)
	newParamsX[i] = paramsX[i];    
    for (i=0;i<4;i++)
	newParamsY[i] = paramsY[i];
}

PFDUDLLEXPORT void 
pfdTexgenParams(const float *newParamsX, const  float *newParamsY)
{
    pfDataPool *dp;
    float *paramsX, *paramsY;
    
    dp = pfAttachDPool(CallBacks);
    if (dp == NULL)
    {
	dp = pfCreateDPool(sizeof(float) * 8, CallBacks);
    	paramsX = (float *)pfDPoolAlloc(dp, sizeof(float) * 4, 1);
    	paramsY = (float *)pfDPoolAlloc(dp, sizeof(float) * 4, 2);
    }
    else
    {
    	paramsX = (float *)pfDPoolFind(dp, 1);
    	paramsY = (float *)pfDPoolFind(dp, 2);
    }
    if (!newParamsX)
    {
	paramsX[0] = 1.0;
	paramsX[1] = 0.0;	
	paramsX[2] = 0.0;
	paramsX[3] = 0.5;
    }
    else
    {
	paramsX[0] = newParamsX[0];
	paramsX[1] = newParamsX[1];
	paramsX[2] = newParamsX[2];
	paramsX[3] = newParamsX[3];
    }
    if (!newParamsY)
    {
	paramsY[0] = 0.0;
	paramsY[1] = 0.0;	
	paramsY[2] = 1.0;
	paramsY[3] = 0.5;
    }
    else
    {
	paramsY[0] = newParamsX[0];
	paramsY[1] = newParamsX[1];
	paramsY[2] = newParamsX[2];
	paramsY[3] = newParamsX[3];
    }
}   

PFDUDLLEXPORT int pfdPreDrawTexgenExt(pfTraverser *trav, void *data)
{
    pfdExtensor *eng = data;
    switch(eng->mode)
    {
    case PFD_TEXGEN_LINEAR:
	pfdPreDrawLinearMap(trav,eng->data);
	break;
    case PFD_TEXGEN_CONTOUR:
	pfdPreDrawContourMap(trav,eng->data);
	break;
    case PFD_TEXGEN_REFLECTIVE:
	pfdPreDrawReflMap(trav,NULL);
	break;
    }
    return 0;
}

PFDUDLLEXPORT int pfdPostDrawTexgenExt(pfTraverser *trav, void *data)
{
    pfdExtensor *eng = data;
    switch(eng->mode)
    {
    case PFD_TEXGEN_LINEAR:
	pfdPostDrawLinearMap(trav,eng->data);
	break;
    case PFD_TEXGEN_CONTOUR:
	pfdPostDrawContourMap(trav,eng->data);
	break;
    case PFD_TEXGEN_REFLECTIVE:
	pfdPostDrawReflMap(trav,NULL);
	break;
    }
    return 0;
}

/*ARGSUSED*/
PFDUDLLEXPORT int 
pfdPreDrawContourMap(pfTraverser *trav, void *data)
{
    pfDataPool *dp;
    float *paramsX, *paramsY;
    
    if (data == NULL)
    {
        dp = pfAttachDPool(CallBacks);
    	if (dp == NULL)
	{
	    pfdTexgenParams(NULL, NULL);
            dp = pfAttachDPool(CallBacks);
   	}
    	paramsX = (float *)pfDPoolFind(dp, 1);
    	paramsY = (float *)pfDPoolFind(dp, 2);
    }
    else
    {
	paramsX = (float *)data;
	paramsY = (float *)(&(paramsX[4]));
    }
    
    glMatrixMode(GL_MODELVIEW);
    pfPushIdentMatrix();
    glTexGenfv(GL_S, GL_EYE_PLANE, paramsX);
    glTexGenfv(GL_T, GL_EYE_PLANE, paramsY);
    glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    pfPopMatrix();
    return 0;
}

/*ARGSUSED*/
PFDUDLLEXPORT int 
pfdPostDrawContourMap(pfTraverser *trav, void *data)
{
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    return 0;
}

/*ARGSUSED*/
PFDUDLLEXPORT int 
pfdPreDrawLinearMap(pfTraverser *trav, void *data)
{
    pfDataPool *dp;
    float *paramsX, *paramsY;
    
    if (data == NULL)
    {
        dp = pfAttachDPool(CallBacks);
    	if (dp == NULL)
	{
	    pfdTexgenParams(NULL, NULL);
            dp = pfAttachDPool(CallBacks);
   	}
    	paramsX = (float *)pfDPoolFind(dp, 1);
    	paramsY = (float *)pfDPoolFind(dp, 2);
    }
    else
    {
	paramsX = (float *)data;
	paramsY = (float *)(&(paramsX[4]));
    }

    glTexGenfv(GL_S, GL_OBJECT_PLANE, paramsX);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, paramsY);
    glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    return 0;
}

/*ARGSUSED*/
PFDUDLLEXPORT int 
pfdPostDrawLinearMap(pfTraverser *trav, void *data)
{
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    return NULL;
}

/*ARGSUSED*/
PFDUDLLEXPORT int 
pfdPreDrawReflMap(pfTraverser *trav, void *data)
{
    glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    return NULL;
}

/*ARGSUSED*/
PFDUDLLEXPORT int 
pfdPostDrawReflMap(pfTraverser *trav, void *data)
{
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    return NULL;
}
