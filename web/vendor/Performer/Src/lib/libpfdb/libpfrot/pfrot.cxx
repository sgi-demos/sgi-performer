/*
 * Copyright 1996, Silicon Graphics, Inc.
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
 * pfrot.c - ".rot" pseudo-loader.
 *
 * To load a file and rotate it, say:
 *	perfly <filename>.<h>,<p>,<r>.rot
 * also works with the ".scale" and ".trans" pseudo-loaders.
 *
 * Don't worry, the numbers can have decimal points (but don't
 * use exponential notation).
 */

#include <stdio.h>
#include <ctype.h>

#ifdef WIN32
#include <windows.h>

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pfdu.h>
#include <Performer/pf/pfSCS.h>
#ifdef __linux__
#include <limits.h>
#endif

char *strrnchr(char *s, int n, int c)
{
    char *p = NULL;
    int i;
    for (i = 0; i < n && s[i] != '\0'; ++i)
	if (s[i] == c)
	    p = s+i;
    return p;
}

extern "C" PFPFB_DLLEXPORT pfNode *
pfdLoadFile_rot(char *fileName)
{
    float h, p, r;
    char *lastdot, *lastcomma, *firstcomma, *firstdot;
    if ((lastdot = strrchr(fileName, '.')) == NULL
     || (lastcomma = strrnchr(fileName, lastdot-fileName, ',')) == NULL
     || (firstcomma = strrnchr(fileName, lastcomma-fileName, ',')) == NULL
     || (firstdot = strrnchr(fileName, firstcomma-fileName, '.')) == NULL
     || firstdot-fileName > 0 &&
        (firstdot[-1] == '.' || isdigit(firstdot[-1])) &&
        (firstdot = strrnchr(fileName, firstdot-fileName, '.')) == NULL
     || sscanf(firstdot+1, "%f,%f,%f", &h, &p, &r) != 3)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
		"pfdLoadFile_rot: bad file name %s", fileName);
	return NULL;
    }

    char realName[PF_MAXSTRING];
    strncpy(realName, fileName, firstdot-fileName);
    realName[firstdot-fileName] = '\0';

    pfNode *child = pfdLoadFile(realName);
    if (child == NULL)
	return NULL; /* error message printed already */

    pfNotify(PFNFY_INFO, PFNFY_PRINT, "Rotating scene by %g,%g,%g\n", h,p,r);

    pfCoord coord;
    coord.xyz.set(0,0,0);
    coord.hpr.set(h,p,r);
    pfMatrix mat;
    mat.makeCoord(&coord);
    pfSCS *scs = new pfSCS(mat);
    PFASSERTALWAYS(scs != NULL);
    scs->addChild(child);

    return scs;
}
