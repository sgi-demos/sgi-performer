/*
 * Copyright 1994, 1995, Silicon Graphics, Inc.
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
 * ilgettile.c $Revision: 1.1 $ $Date: 2000/11/21 21:39:34 $
 */

#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#define IL2_5_COMPAT
#include <il/ilGenericImgFile.h>
#include <il/ilConfigure.h>
#include <il/ilRotZoomImg.h>
#include <il/ilABGRImg.h>
#include <il/ilSubImg.h>
#include <il/ilMemoryImg.h>

#if IL_VERSION_MAJOR < 3
#define ilHwAccelOff 0
#else /* IL_VERSION_MAJOR >= 3 */
#endif /* IL_VERSION_MAJOR >= 3 */

#include <Performer/pf.h>

#define MAX_IMAGES 100

static char		    *ImageNames[MAX_IMAGES];
static ilRotZoomImg	    *Images[MAX_IMAGES];
static int		    NImages = 0;

extern "C" int
ildeffile(char *filename, char *name, int xs,  int ys)
{
    int			    xsize, ysize, i;
    ilFileImg		    *inimg;
    ilImage		    *tmpImg;
    char path[PF_MAXSTRING];

    if (NImages >= MAX_IMAGES -1)
    {
 	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
		 "ildeffile: Too many images \"%s\"", name);
	return 0;
    }
	
    for(i=0;i<NImages;i++)
	if(strcmp(ImageNames[i], filename) == 0)
	    return i;

    ilHwAccelerate(ilHwAccelOff);

    if (pfFindFile(filename, path, R_OK))
	inimg = ilOpenImgFile(path, "r");
    else
	inimg = ilOpenImgFile(filename, "r");
    
    if (inimg == NULL) 
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "pfdLoadFile_ptu: can't open %s", filename);
	exit(0);
    }
    
    if (name == NULL)
	ImageNames[NImages] = strdup(filename);
    else 
	ImageNames[NImages] = strdup(name);
    xsize = inimg->getXsize();
    ysize = inimg->getYsize();
    pfNotify(PFNFY_INFO, PFNFY_PRINT, 
	     "pfdLoadFile_ptu: defining image from %s: <%d %d> * <%.3f %.3f> => <%d %d>", 
	     filename, 
	     xsize, 
	     ysize, 
	     (float)xs/(float)xsize, 
	     (float)ys/(float)ysize, 
	     xs, 
	     ys);
    if (inimg->getCsize() > 2)
    {
	tmpImg = (ilImage*)inimg;
	inimg = (ilFileImg*) new ilABGRImg(inimg);
	((ilOpImg *)inimg)->hwAccelerate(0);
    }

    Images[NImages++] = new ilRotZoomImg(
			    inimg,  0.0f, 
			    (float)xs/(float)xsize, 
			    (float)ys/(float)ysize,
			    ilNoFlip, ilBiCubic);
	
    ilHwAccelerate(ilHwAccelOff);    
    if (inimg->getCsize() > 2)
    {
    	ImageNames[NImages] = strdup("blah");
    	Images[NImages++] = (ilRotZoomImg*)tmpImg;
	return (NImages-2);
    }

    return (NImages-1);
}

extern "C" void *
ilgettile(char *name, float fxo, float fyo, int xs, int ys, int zsx, int zsy)
{
    int			    i;
    ilRotZoomImg	    *inimg	= NULL;
    ilSubImg		    *subimg	= NULL;
    ilRotZoomImg	    *zoomimg	= NULL;
    ilMemoryImg		    *memimg	= NULL;

    float		    ratx	= (float)(zsx) / (float)(xs);
    float		    raty	= (float)(zsy) / (float)(ys);

    for(i=0; i<NImages; i++)
	if (strcmp(ImageNames[i], name) == 0)
	    inimg = Images[i];

    if (inimg == NULL)
	return NULL;

    ilHwAccelerate(ilHwAccelOff);

    zoomimg = new ilRotZoomImg(inimg,0.0,ratx,raty,ilNoFlip,ilBiCubic);
    subimg = new ilSubImg(zoomimg, fxo * ratx, fyo * raty, 
						zsx, zsy,(ilConfig*)NULL); 
    memimg = new ilMemoryImg(subimg);
    if (subimg->getCsize() < 3)
      	subimg->setDataType(ilUShort);

    return ( memimg->getDataPtr() );
}
extern "C" void
ilfreetiles(void)
{
    int i;
    for(i=0;i<NImages;i++)
	free(Images[i]);
    NImages = 0;
}

