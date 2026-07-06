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
 * inherit.c   - transform inheritance example
 * $Revision: 1.21 $
 * $Date: 2002/09/15 03:51:02 $
 */

#include <math.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>
#ifdef WIN32
#define sleep(a) Sleep((a) * 1000)
#endif

int
main(int argc, char *argv[])
{
    pfPipe         *pipe;
    pfPipeWindow   *pw;
    pfScene        *scene;
    pfChannel      *chan;
    pfCoord         view;
    float           z, s, c;
    pfNode	   *model1, *model2;
    pfDCS	   *node1, *node2;
    pfDCS	   *dcs1, *dcs2, *dcs3, *dcs4;
    pfSphere        sphere;
    char	   *file1, *file2;

    /* choose default objects of none specified */
    file1 = (argc > 1) ? argv[1] : "blob.nff";
    file2 = (argc > 1) ? argv[1] : "torus.nff";

    /* Initialize Performer */
    pfInit();

    pfFilePath(
	"."
	":./data"
	":../data"
	":../../data"
	":../../../data"
	":../../../../data"
	":/usr/share/Performer/data");

    /* Single thread for simplicity */
    pfMultiprocess(PFMP_DEFAULT);

    /* Load all loader DSO's before pfConfig() forks */
    pfdInitConverter(file1);
    pfdInitConverter(file2);

    /* Configure */
    pfConfig();

    /* Load the files */
    if ((model1 = pfdLoadFile(file1)) == NULL)
    {
	pfExit();
	exit(-1);
    }
    if ((model2 = pfdLoadFile(file2)) == NULL)
    {
	pfExit();
	exit(-1);
    }

    /* scale models to unit size */
    node1 = pfNewDCS();
    pfAddChild(node1, model1);
    pfGetNodeBSphere(model1, &sphere);
    if (sphere.radius > 0.0f)
	pfDCSScale(node1, 1.0f/sphere.radius);

    node2 = pfNewDCS();
    pfAddChild(node2, model2);
    pfGetNodeBSphere(model2, &sphere);
    if (sphere.radius > 0.0f)
	pfDCSScale(node2, 1.0f/sphere.radius);

    /* Create the hierarchy */
    dcs4 = pfNewDCS();
    pfAddChild(dcs4, node1);
    pfDCSScale(dcs4, 0.5f);

    dcs3 = pfNewDCS();
    pfAddChild(dcs3, node1);
    pfAddChild(dcs3, dcs4);

    dcs1 = pfNewDCS();
    pfAddChild(dcs1, node2);

    dcs2 = pfNewDCS();
    pfAddChild(dcs2, node2);
    pfDCSScale(dcs2, 0.5f);
    pfAddChild(dcs1, dcs2);

    scene = pfNewScene();
    pfAddChild(scene, dcs1);
    pfAddChild(scene, dcs3);
    pfAddChild(scene, pfNewLSource());

    /* Configure and open GL window */
    pipe = pfGetPipe(0);
    pw = pfNewPWin(pipe);
    pfPWinType(pw, PFPWIN_TYPE_X);
    pfPWinName(pw, "OpenGL Performer");
    pfPWinOriginSize(pw, 0, 0, 500, 500);
    pfOpenPWin(pw);

    chan = pfNewChan(pipe);
    pfChanScene(chan, scene);

    pfSetVec3(view.xyz, 0.0f, 0.0f, 15.0f);
    pfSetVec3(view.hpr, 0.0f, -90.0f, 0.0f);
    pfChanView(chan, view.xyz, view.hpr);

    /* Loop through various transformations of the DCS's */
    for (z = 0.0f; z < 1084; z += 4.0f)
    {
	pfDCSRot(dcs1,
	   (z < 360) ? (int) z % 360 : 0.0f,
	   (z > 360 && z < 720) ? (int) z % 360 : 0.0f,
	   (z > 720) ? (int) z % 360 : 0.0f);

	pfSinCos(z, &s, &c);
	pfDCSTrans(dcs2, 1.0f * c, 1.0f * s, 0.0f);

	pfDCSRot(dcs3, z, 0, 0);
	pfDCSTrans(dcs3, 4.0f * c, 4.0f * s, 4.0f * s);
	pfDCSRot(dcs4, 0, 0, z);
	pfDCSTrans(dcs4, 1.0f * c, 1.0f * s, 0.0f);

	pfFrame();
    }

    /* show objects static for three seconds */
    sleep(3);

    pfExit();
    exit(0);
}
