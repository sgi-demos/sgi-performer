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
 * imerge.C: $Revision: 1.2 $
 */

#if defined(O32) || defined(N32)
#define CAN_USE_IL
#include <ifl/iflSize.h>
#include <il/ilImage.h>
#include <il/ilFileImg.h>
#include <il/ilMergeImg.h>
#include <il/ilConfigure.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <Performer/pf.h>
#include <Performer/pfdu.h>

#include "3dsftk.h"

extern "C" {
extern int mergeTextures(char *rgbFileName, char *alphaFileName,
			 char *newFileName);
}


int
mergeTextures(char *rgbFileName, char *alphaFileName, char *newFileName)
{
#ifdef CAN_USE_IL
    ilFileImg	  *rgbImage	= NULL;
    ilFileImg     *alphaImage	= NULL;
    ilFileImg     *newImage	= NULL;
    ilMergeImg    *mergeImage	= NULL;
    iflSize         size;
#endif
    char 	  *dot;

    if (newFileName == NULL)
	return 1;
    newFileName[0] = '\0';

    /*
     *  is file already of type ".rgb" or ".rgba" ?
     */
    dot = strrchr(rgbFileName, '.');
    if (dot != NULL && (strcmp(dot, ".rgb") == 0 || strcmp(dot, ".rgba") == 0))
    {
	strcpy(newFileName, rgbFileName);
	return 0;
    }

    pfNotify(PFNFY_INFO, PFNFY_PRINT,
	     "  Converting and merging texture images");

#ifdef CAN_USE_IL

    /*
     *  read RGB image
     */
    if (rgbFileName != NULL && *rgbFileName != '\0')
    {
	rgbImage = new ilFileImg(rgbFileName, O_RDONLY);
	if (rgbImage != NULL && rgbImage->getStatus() != ilOKAY)
	{
	    delete rgbImage;
	    rgbImage = NULL;
	}
    }
    if (rgbImage == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "Unable to read:\"%s\"", rgbFileName);
	return 1;
    }
    pfNotify(PFNFY_DEBUG, PFNFY_MORE, "Reading RGB image :\"%s\"", rgbFileName);
    rgbImage->setColorModel(iflBGR);
    rgbImage->getSize(size);

    /*
     *  read Alpha (transparency/opacity) image
     */
    if (alphaFileName != NULL && *alphaFileName != '\0')
    {
	alphaImage = new ilFileImg(alphaFileName, O_RDONLY);
	if (alphaImage != NULL && alphaImage->getStatus() != ilOKAY)
	{
	    delete alphaImage;
	    alphaImage = NULL;
	}
    }
    if (alphaImage != NULL)
	pfNotify(PFNFY_DEBUG, PFNFY_MORE, "Reading Alpha image: \"%s\"",
	    alphaFileName);

    if (alphaImage != NULL)
    {
	/* create merged RGBA image */
        mergeImage = new ilMergeImg(rgbImage,
				    alphaImage, NULL, 
				    rgbImage->getOrder(),
				    rgbImage->getDataType());
	mergeImage->setColorModel(iflRGBA);
	mergeImage->getSize(size);

	/* give new file name the ".rgba" extension */
	strcpy(newFileName, rgbFileName);
	dot = strrchr(newFileName, '.');
	strcpy(dot, ".rgba");

        pfNotify(PFNFY_DEBUG, PFNFY_MORE, "Writing merged RGBA image: \"%s\"",
		 newFileName);
	iflFileConfig fc(&size, mergeImage->getDataType(),
			 mergeImage->getOrder());
	newImage = new ilFileImg(newFileName, NULL, &fc, NULL);

        newImage->copyTile(0, 0, size.x, size.y, mergeImage,
			   0, 0, NULL);

	delete newImage;
    }
    else
    {
	/* give new file name the ".rgb" extension */
	strcpy(newFileName, rgbFileName);
	dot = strrchr(newFileName, '.');
	strcpy(dot, ".rgb");

        pfNotify(PFNFY_DEBUG, PFNFY_MORE, "Writing RGB image: \"%s\"",
		 newFileName);
	iflFileConfig fc(&size, rgbImage->getDataType(),
			 rgbImage->getOrder());
	newImage = new ilFileImg(newFileName, NULL, &fc, NULL);

        newImage->copyTile(0, 0, size.x, size.y, rgbImage,
			   0, 0, NULL);

	delete newImage;
    }

    return 0;

#else	/* can not use il */

    pfNotify(PFNFY_WARN, PFNFY_PRINT, "Can not use il to convert image");
    return 1;

#endif
}
