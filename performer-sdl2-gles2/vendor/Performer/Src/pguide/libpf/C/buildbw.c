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
 */

#include <stdio.h>
#include <math.h>
#ifndef WIN32
#include <sys/resource.h>
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/image.h>

#define MAXX 2402
#define MAXY 2402

static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: buildbw file.bw tile_pre_name config_filename pagelookahead_filename\n");
    exit(1);
}

void
setlimit(void)
{
#ifndef WIN32
    struct rlimit   rlp;
    rlp.rlim_cur = RLIM_INFINITY;
    rlp.rlim_max = RLIM_INFINITY;
    setrlimit (RLIMIT_VMEM, &rlp);


    getrlimit (RLIMIT_DATA, &rlp);
    printf ("DATA cur, max: %d %d\n", rlp.rlim_cur, rlp.rlim_max);

    getrlimit (RLIMIT_FSIZE, &rlp);
    printf ("FSIZE cur, max: %d %d\n", rlp.rlim_cur, rlp.rlim_max);

    getrlimit (RLIMIT_VMEM, &rlp);
    printf ("VMEM cur, max: %d %d\n", rlp.rlim_cur, rlp.rlim_max);

    getrlimit (RLIMIT_RSS, &rlp);
    printf ("RSS cur, max: %d %d\n", rlp.rlim_cur, rlp.rlim_max);
#endif

    pfNotify(PFNFY_WARN, PFNFY_PRINT,
        "shared Arena size %d\n", pfGetSharedArenaSize());
}

void
main(int argc, char *argv[])
{
    IMAGE *data;
    int ncols, nrows, r, c;
    int xcols, yrows, yr, xc;
    int xcut, ycut;
    int i, j;
    float xllcorner, yllcorner, cellsize;
    int nodata;
    float xval, yval, zval;
    pfVec3 *tData;
    pfASD *node;
    char filePath[512];
    char outfname[100];
    unsigned short *buff;
    int lookahead[2];

    /* check argument */
    if (argc < 5)
        Usage();

    pfInit();

    setlimit();

    /* find file in OpenGL Performer directory-search path */
    if (!pfFindFile(argv[1], filePath, R_OK))
    {
        pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
            "openFile: Could not find file \"%s\"", argv[1]);
        return;
    }

    if ((data = iopen(filePath, "r")) == NULL)
        return;

    ncols = data->xsize;
    nrows = data->ysize;
    xllcorner = 0.0;
    yllcorner = 0.0;
    cellsize = 1.0;
    
    xcut = ncols/MAXX;
    if(MAXX*xcut != ncols) xcut++;
    ycut = nrows/MAXY;
    if(MAXY*ycut != nrows) ycut++;
    pfNotify(PFNFY_INFO, PFNFY_PRINT,
    	"read in %d col, %d rows\n", ncols, nrows);
    buff = (unsigned short *)pfMalloc(sizeof(ushort)*ncols, pfGetSharedArena());

    if(ncols > MAXX) xcols = MAXX; 
    else xcols = ncols;
    if(nrows > MAXY) yrows = MAXY;
    else yrows = nrows;

    tData = (pfVec3 *) pfMalloc(sizeof(pfVec3)*xcols*yrows, pfGetSharedArena());

    pfNotify(PFNFY_INFO, PFNFY_PRINT,
    	"Allocated %d Kbytes\n", sizeof(pfVec3)*xcols*yrows/(1<<10));
    for (r=0, yr=0, yval=yllcorner+(nrows-1)*cellsize;r<nrows;r+=ycut,yr++,yval-=(ycut*cellsize))
    {
	getrow(data, buff, r, 0);
	pfNotify(PFNFY_INFO, PFNFY_PRINT,
		"read in row %d\n", r);
        for (c=0, xc=0, xval=xllcorner; c<ncols; c+=xcut, xc++, xval+=(xcut*cellsize))
        {
                PFSET_VEC3(tData[(yrows-yr-1)*xcols + xc], xval, yval, buff[c]);
        }
    }

    pfFree(buff);
    iclose(data);

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "buildbw: %s", argv[1]);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "Dimensions: %d x %d vertices",
        ncols, nrows);

    lookahead[0] = lookahead[1] = 6;
    pfdBuildASD(xcols, yrows, tData, 0, 10, 1, argv[2], argv[3], argv[4], lookahead);
    pfFree(tData);

    /* print statistics */

}

