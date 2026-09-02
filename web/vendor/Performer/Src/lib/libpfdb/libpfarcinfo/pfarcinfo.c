/*
 * Copyright 2000, Silicon Graphics, Inc.
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

#include <stdio.h>
#include <math.h>

#ifdef WIN32
#include <windows.h>
#include "wintypes.h"

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pf.h>
#include <Performer/pfdu.h>


PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_arcinfo(char *fileName)
{
    FILE *arcinfoFile;
    unsigned int ncols, nrows, r, c;
    float xllcorner, yllcorner, cellsize;
    int nodata;
    float xval, yval, zval;
    pfVec3 *tData;
    pfASD *node;


    if ((arcinfoFile = pfdOpenFile(fileName)) == NULL)
        return NULL;

    if (fscanf(arcinfoFile, "%*s%u%*s%u%*s%f%*s%f%*s%f%*s%d", &ncols, &nrows,
               &xllcorner, &yllcorner, &cellsize, &nodata) != 6)
           pfNotify(PFNFY_WARN, PFNFY_PRINT,
                    "pfdLoadFile_arcinfo: Error reading file header");

    tData = (pfVec3 *) pfMalloc(sizeof(pfVec3) * ncols * nrows, NULL);

    for (r=0, yval=yllcorner+(nrows-1)*cellsize;r<nrows;++r,yval-=cellsize)
        for (c=0, xval=xllcorner; c<ncols; ++c, xval+=cellsize)
        {
            if (fscanf(arcinfoFile, "%f", &zval) != 1)
                pfNotify(PFNFY_WARN, PFNFY_PRINT,
                   "pfdLoadFile_arcinfo: Error reading data file");
#ifdef WIN32
                if (fabs((float) nodata - zval) < 0.0001)
#else
                if (fabsf((float) nodata - zval) < 0.0001)
#endif
                    zval = 0.0f;
                PFSET_VEC3(tData[(nrows-r-1)*ncols + c], xval, yval, zval);
        }

    fclose(arcinfoFile);

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfdLoadFile_arcinfo: %s", fileName);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "Dimensions: %d x %d vertices",
        ncols, nrows);
    node = pfdBuildASD(ncols, nrows, tData, 0, 8, 0, NULL, NULL, NULL, NULL);
#if 0
    {
	char outFile[200];
        sprintf(outFile, "%s.pfb", fileName);
	pfNotify(PFNFY_INFO, PFNFY_MORE, "store node in file %s\n", outFile);
        pfdStoreFile((pfNode *)node, outFile);
    }
#endif
    pfFree(tData);

    /* print statistics */

    {
        int num;
  	int bind;
        void *dummy;
        pfGetASDAttr(node, PFASD_LODS, &bind, &num, &dummy);
        pfNotify(PFNFY_INFO,   PFNFY_MORE,  "Levels of Detail: %d",
        num);
    }

    return (pfNode *) node;
}

