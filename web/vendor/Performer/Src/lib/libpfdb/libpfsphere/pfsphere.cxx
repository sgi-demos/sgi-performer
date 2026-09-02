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
 * pfsphere.c
 *
 * Pseudo-loader that creates a scene graph consisting of a front-of-
 * sphere section with an approximately constant number of triangles,
 * that morphs to be good for the current
 * distance-to-ground (doesn't bother about back-facing stuff),
 * and doesn't have any triangles spanning the international date line.
 *
 * Usage:
 *	perfly <n>.sphere #tesselate starting with an n-gon (must be power of 2)
 *	perfly sphere	  #default 128
 *
 * With either of the above methods, the update can be made to
 * happen every n frames (so that once the geometry is made for a given view,
 * the scene may be rotated and examined for a period of time before it is
 * updated again).  For example, to update every 120 frames,
 *	setenv PFSPHERE_UPDATE_FREQUENCY 120
 */

#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#include <Performer/pfutil.h>
#include <Performer/pr/pfTexture.h>
#ifdef __linux__
#include <limits.h>
#endif

extern "C" PFPFB_DLLEXPORT pfNode *
pfdLoadFile_sphere(char *fileName)
{
    int n = 128; /* default */
    char texname[PF_MAXSTRING] = "";;

    char params[PF_MAXSTRING];
    char *lastdot = strrchr(fileName, '.');
    if (lastdot != NULL)
    {
	strncpy(params, fileName, lastdot-fileName);
	params[lastdot-fileName] = '\0';
	if (sscanf(params, "%s%d", texname, &n) < 1)
	{
	    pfNotify(PFNFY_WARN, PFNFY_USAGE,
		    "pfdLoadFile_sphere: bad file name %s", fileName);
	    return NULL;
	}
    }

    pfTexture *tex = new pfTexture();
    if (!tex->loadFile(texname))
    {
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
		"pfdLoadFile_sphere: couldn't load texture \"%s\"\n",
		texname);
	pfFree(tex);
	tex = NULL;
    }

    char *e = getenv("PFSPHEREFRONT_UPDATE_FREQUENCY");
    int update_frequency = (e ? *e ? atoi(e) : 1 : 1);

    e = getenv("_PFSPHERE_TOL"); // XXX make better interface
    float hightol = (e ? *e ? atof(e) : 1 : 1);
    float lowtol = hightol * .5;

    if (getenv("_PFSPHERE_NEW"))
	return pfuNewSphereFrontOld(n, tex, NULL, NULL, update_frequency);
    else
	return pfuNewSphereFront(hightol, lowtol, tex, NULL, NULL, update_frequency);
}
