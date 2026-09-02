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
 * doubleDCS2.c : demonstrate a double-precision DCS node.
 *		  Show how single-precision pfDCS fails when scene is very
 *		  far from origin.
 *
 *		  The program rotates a camera around an object. Both the
 *		  camera and the object move away from the origin along the X 
 *		  axis. When using a double-precision DCS, the view is stable.
 *		  When using a single-precision DCS, the view vibrates as the 
 *		  camera grows farther from the origin.
 *
 *		  The -s parameter chooses a single-precision DCS. 
 *
 * $Revision: 1.4 $ $Date: 2002/11/14 00:02:43 $ 
 *
 */

#include <stdlib.h>
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

void loadIdentityChannelViewMatrix(pfChannel *chan);
void loadViewingMatrixOnDoubleDCS (pfDoubleDCS *ddcs, pfCoordd *coord);

/*
 *	Usage() -- print usage advice and exit. This
 *      procedure is executed in the application process.
 */
static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: doubleDCS2 [-s] file.ext ...\n");
    exit(1);
}


void
main (int argc, char *argv[])
{
    float	    t = 0.0f;
    pfScene	    *scene;
    pfNode	    *root;
    pfPipe	    *p;
    pfPipeWindow    *pw;
    pfChannel	    *chan;
    pfSphere	    bsphere;
    pfCoordd	    double_view;
    pfCoord	    view;
    pfDoubleDCS	    *view_double_dcs;
    pfDoubleDCS	    *object_double_dcs;
    pfDCS	    *object_dcs;
    int		    use_double_precision;
    char	    *filename;
    int		    i;

    if (argc < 2)
	Usage();

    use_double_precision = 1;
    for (i = 1 ; i < argc ; i ++)
    {
	if (strcmp (argv[i], "-s") == 0)
	    use_double_precision = 0;
	else
	    filename = argv[i];
    }

    /* Initialize Performer */
    pfInit();	

    /* Use default multiprocessing mode based on number of
     * processors.
     */
    pfMultiprocess( PFMP_DEFAULT );			

    /* Load all loader DSO's before pfConfig() forks */
    pfdInitConverter(filename);

    /* initiate multi-processing mode set in pfMultiprocess call 
     * FORKs for Performer processes,  CULL and DRAW, etc. happen here.
     */
    pfConfig();			

    /* Append to Performer search path, PFPATH, files in 
     *	    /usr/share/Performer/data */
    pfFilePath(".:/usr/share/Performer/data");

    /* Read a single file, of any known type. */
    if ((root = pfdLoadFile(filename)) == NULL) 
    {
	pfExit();
	exit(-1);
    }

    if (use_double_precision)
    {
	/* 
	 *	Make two double-precision DCS nodes 
	 *	One for translating the object and another for 
  	 *	expressing the channel camera position.
	 *
	 *	The channel viewing matrix is identity throughout the 
	 *	program.
	 */

	view_double_dcs = pfNewDoubleDCS();
	object_double_dcs = pfNewDoubleDCS();
    }
    else
    {
	/*
	 *	If single-precision, use a pfDCS to move the object, and the
	 *	pfChannel viewing matrix to move the camera.
	 */
	object_dcs = pfNewDCS();
    }

    /* Attach loaded file to a new pfScene. */
    scene = pfNewScene();

    if (use_double_precision)
    {
	pfMatrix	M;
	pfSCS		*scs;

	/*
	 *	Create a pfSCS node with identity matrix.
	 */
	pfMakeIdentMat(M);
	scs = pfNewSCS(M);
	/*
	 *	Double-precision mode - Build scene graph: 
         *
	 *	    scene 
	 *	      |
	 *      view_double_dcs
	 *	      |
	 *     object_double_dcs
	 *	      |
	 *	     scs
	 *	      |
	 *	    root
 	 *	
	 *	view_double_dcs contains the inverse of the viewing matrix.
	 *
	 *	object_double_dcs contains the translation of the object 
	 *	to world coordinates.
	 *
	 *	Together, view_double_dcs and object_double_dcs cancel the 
	 *	very large translation and can be expressed as a single-
	 *	precision matrix.
	 *
	 *	IMPORTANT:
	 *	The scs node forces the traversal of the root node into single
	 *	precision mode. This is important because culling is disabled 
	 *	when traversing nodes in double-precision mode. 
	 */
	pfAddChild(scene, view_double_dcs);
	pfAddChild(view_double_dcs, object_double_dcs);
	pfAddChild(object_double_dcs, scs);
	pfAddChild(scs, root);
    }
    else
    {
	/*
	 *	Single-precision - Build scene graph: 
	 *	    scene -> object_dcs -> root
	 */
	pfAddChild(scene, object_dcs);
	pfAddChild(object_dcs, root);
    }

    /* Create a pfLightSource and attach it to scene. */
    pfAddChild(scene, pfNewLSource());
    
    /* Configure and open GL window */
    p = pfGetPipe(0);
    pw = pfNewPWin(p);
    pfPWinType(pw, PFPWIN_TYPE_X);
    pfPWinName(pw, "Double-Precision DCS");
    pfPWinOriginSize(pw, 0, 0, 500, 500);
    /* Open and configure the GL window. */
    pfOpenPWin(pw);
    
    /* Create and configure a pfChannel. */
    chan = pfNewChan(p);	
    pfChanScene(chan, scene);
    pfChanFOV(chan, 45.0f, 0.0f);

    /* determine extent of scene's geometry */
    pfGetNodeBSphere (root, &bsphere);
    pfChanNearFar(chan, 1.0f, 10.0f * bsphere.radius);

    while (t < 200.0f)
    {
	float      	s, c;
	double		x_offset;

	/* Go to sleep until next frame time. */
	pfSync();		

	/* Initiate cull/draw for this frame. */
	pfFrame();
	
	/* Compute new view position. */
	t = pfGetTime();
	pfSinCos(45.0f*t, &s, &c);

	/*	
	 *	Translate the object and the camera by x_offset along 
	 *	the X axis. As x_offset grows, the single-precision 
	 *	version shows large position errors. In the double-precision 
	 *	version the carema looks stable all the time.
	 */
	x_offset = (t + 5.0) * 10000.0;

        if (use_double_precision)
	{
	    pfDoubleDCSTrans(object_double_dcs, x_offset, 0.0, 0.0);

	    pfSetVec3d(double_view.hpr, 45.0f*t, -10.0f, 0);
	    pfSetVec3d(double_view.xyz, 
			     2.0f * bsphere.radius * s + x_offset, 
			    -2.0f * bsphere.radius * c, 
			     0.5f * bsphere.radius);

	    /*
	     *    Instead of loading the new coordinates on the channel 
	     *    viewing matrix, as load identity on the channel and
	     *    load the double-dcs with the inverse of the desired viewing 
	     *    matrix.
	     */
	    loadIdentityChannelViewMatrix(chan);
	    loadViewingMatrixOnDoubleDCS (view_double_dcs, &double_view);
	}
	else
	{
	    pfDCSTrans(object_dcs, x_offset, 0.0, 0.0);

	    pfSetVec3(view.hpr, 45.0f*t, -10.0f, 0);
	    pfSetVec3(view.xyz, 
			     2.0f * bsphere.radius * s + x_offset, 
			    -2.0f * bsphere.radius * c, 
			     0.5f * bsphere.radius);

	    pfChanView(chan, view.xyz, view.hpr);
	}
    }

    /* Terminate parallel processes and exit. */
    pfExit();
}

void
loadIdentityChannelViewMatrix(pfChannel *chan)
{
#ifdef WIN32
    pfMatrix pfIdentMat = {0};
    pfIdentMat[0][0] = pfIdentMat[1][1] = pfIdentMat[2][2] = pfIdentMat[3][3] = 1;
#endif
    pfChanViewMat(chan, pfIdentMat);
}

void 
loadViewingMatrixOnDoubleDCS (pfDoubleDCS *ddcs, pfCoordd *coord)
{
    pfMatrix4d		mat, invMat;

    pfMakeCoorddMat4d (mat, coord);

    pfInvertOrthoNMat4d (invMat, mat);

    pfDoubleDCSMat (ddcs, invMat);
}
