/*
 * Copyright 1995, 1996 Silicon Graphics, Inc.
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
 * page.c: simple database paging example with region of interest
 *
 * $Revision: 1.19 $ $Date: 2002/11/09 21:44:04 $ 
 *
 */

#include <math.h>
#include <stdlib.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>

/* size of the region, in tiles, that we page in each time */
#define REGW 2
#define REGH 2
#define NUMREQ (REGW*REGH)

#define DONE 0
#define REQUEST 1

#define APPOWN 0
#define DBPOWN 1

typedef struct {
    int	state;
    int filenum;
    pfNode *node;
} Request;
Request *llist;	/* list of load requests */
Request *dlist; /* list of delete requests */

typedef struct {
    pfGroup *group;
    pfDCS *dcs;
} Tile;
Tile *tlist;

typedef struct {
    int	fcnt;
    char **fnames;
    pfScene *scene;
    pfGroup *group;
    int done;
    int gridw, gridh;
} SharedData;
SharedData *shared;

/* ARGSUSED */
void
dbfunc(void *data)
{
    pfNode *node=0;
    static pfBuffer *buf=0;
    static int i;

#ifdef WIN32
    // Since in win32 there is no separate dbase process we cannot change
    // buffer and leave it changed. We have to get the current buffer and 
    // restore it at the end.
    pfBuffer *currBuf = pfGetCurBuffer();
#endif
    if (!buf)
    {
	buf = pfNewBuffer();
	pfSelectBuffer(buf);

	/* Open a window and context so loaders won't need to create */
	/* their own contexts each time. */
	pfOpenNewNoPortWin("Query", -1);
    }
#ifdef WIN32
    else
        // we have to set the buf every time we call dbfunc
        pfSelectBuffer(buf);
#endif

    /*
     * NOTE: instead of using a real semaphore, we just use an integer
     * (shared->done) to keep track of ownership, to keep this example
     * simple.  The integer works because writes and reads are atomic
     * and there is no race here.  But a real application should use
     * a semaphore.
     */
    if (shared->done == DBPOWN) {
	for (i=0; i < NUMREQ; i++) {
	    if (llist[i].state == REQUEST) {
		node = pfdLoadFile(shared->fnames[llist[i].filenum]);
		if (!node) {
		    pfNotify(PFNFY_NOTICE, PFNFY_PRINT,
			     "cannot load file %s", shared->fnames[i]);
		    exit(1);
		}
		llist[i].node = node;
		llist[i].state = DONE;
	    } else {
		printf("dbfunc: llist %d is %d should be request\n",
		       i, llist[i].state);
	    } 
	}
	pfMergeBuffer();
	shared->done = APPOWN;
    }
    pfDBase();

#ifdef WIN32
    // restore the original buffer so that the buffer 'buf' does not confuse
    // app, cull, and draw
    pfSelectBuffer(currBuf);
#endif
}

float tsize = 40;
int regx, regy;

int
main (int argc, char *argv[])
{
    void	*arena;
    pfPipe      *pipe;
    pfPipeWindow *pwin;
    pfChannel   *chan;
    pfGeoState  *gstate;
    pfSphere	bsphere;
    pfGeoSet	*gset;
    pfGeode	*geode;
    pfMaterial	*material;
    float       t = 0.0;
    float	*verts, *norms;
    int		*plens;
    int		i, j, n, w, h;
    int		fnum, firstarg = 1;
    int		doscale = 0;
    int         numFilesFound = 0;

    if (argc < 2)
    {
	printf("usage: %s [-s][-t tilesize] file ... \n", argv[0]);
	exit(1);
    }
    for (i=1; i < argc; i++) {
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {
	      case 's':
		doscale = 1;
		firstarg += 1;
		break;
	      case 't':
		tsize = atoi(argv[i+1]);
		printf("using tile size %f\n", tsize);
		firstarg += 2;
		break;
	    }
	}
    }
    
    /* Initialize Performer */
    pfInit();	
    
    /* Get shared memory arena */
    arena = pfGetSharedArena();
    shared = (SharedData *) pfCalloc(1, sizeof(SharedData), arena);
    llist = (Request *) pfCalloc(NUMREQ, sizeof(Request), arena);
    dlist = (Request *) pfCalloc(NUMREQ, sizeof(Request), arena);

    /* Use a square, i.e. 3x3, 4x4, etc. tile layout */
    fnum = argc - firstarg;
    w = ceil(sqrt(fnum));
    if (w < REGW) w = REGW;

    /* If there are not enough models to fill the entire square, reuse some. */
    shared->gridw = shared->gridh = h = w;
    shared->fcnt = w * h;
    shared->fnames = (char **) pfCalloc(shared->fcnt,sizeof(char*), arena);

    pfFilePath(".:/usr/share/Performer/data:../../../../data:../pftexture");
    for (i=0; i < shared->fcnt; i++) {
        char path[PF_MAXSTRING];

	shared->fnames[i] = argv[(i % fnum) + firstarg];
        if (pfFindFile(shared->fnames[i], path, R_OK))
	    pfdInitConverter(shared->fnames[i]), numFilesFound++;
    }
    
    /* Exit if no files were found */
    if (!numFilesFound) 
      pfNotify(PFNFY_FATAL, PFNFY_PRINT, "Unable to find any of the "
	"files specified on command line. Exiting.");

    tlist = (Tile *) pfCalloc(shared->fcnt, sizeof(Tile), arena);

    pfMultiprocess(PFMP_APPCULLDRAW|PFMP_FORK_DBASE);
    pfConfig();			
    pfDBaseFunc(dbfunc);

    shared->scene = pfNewScene();
    shared->group = pfNewGroup();
    pfAddChild(shared->scene, shared->group);

    for (j=0; j < h; j++) {
	for (i=0; i < w; i++) {
	    n = j * w + i;
	    tlist[n].dcs = pfNewDCS();
	    pfAddChild(shared->group, tlist[n].dcs);
	    tlist[n].group = pfNewGroup();
	    pfAddChild(tlist[n].dcs, tlist[n].group);

	    /* translate each tile so that scene is centered at 0,0 */
	    pfDCSTrans(tlist[n].dcs, (i - (w-1.0)/2) * tsize,
		       (j - (h-1.0)/2) * tsize, 0);
	}
    }

    /* Create a scene pfGeoState with lighting enabled */
    gstate = pfNewGState(arena);
    pfGStateMode(gstate, PFSTATE_ENLIGHTING, PF_ON);
    pfSceneGState(shared->scene, gstate);
    pfAddChild(shared->scene, pfNewLSource());

    /* Create and configure a pfChannel. */
    pipe = pfGetPipe(0);
    pwin = pfNewPWin(pipe);
    pfPWinOriginSize(pwin, 0, 0, 400, 400);
    pfOpenPWin(pwin);
    chan = pfNewChan(pipe);
    pfChanScene(chan, shared->scene);
    pfChanFOV(chan, 60.0f, 0.0f);

    /* Make a floor for the scene. */
    verts = (float *) pfMalloc(4 * 3 * sizeof(float), arena);
    norms = (float *) pfMalloc(3 * sizeof(float), arena);
    plens = (int *) pfMalloc(sizeof(int), arena);
    for (j=0; j < 2; j++) {
	for (i=0; i < 2; i++) {
	    verts[(j*2+i)*3 + 0] = (i-0.5)*w*tsize;
	    verts[(j*2+i)*3 + 1] = (j-0.5)*h*tsize; 
	    verts[(j*2+i)*3 + 2] = 0;
	}
    }
    norms[0] = norms[1] = 0; norms[2] = 1;
    plens[0] = 4;
    gset = pfNewGSet(arena);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, verts, NULL);
    pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OVERALL, norms, NULL);
    pfGSetPrimType(gset, PFGS_TRISTRIPS);
    pfGSetPrimLengths(gset, plens);
    pfGSetNumPrims(gset, 1);

    /* Set up lighting for the floor */
    gstate = pfNewGState(arena);
    material = pfNewMtl(arena);
    pfMtlColor(material, PFMTL_DIFFUSE, 0.4, 0.4, 0.4);
    pfGStateAttr(gstate, PFSTATE_FRONTMTL, material);
    pfGStateMode(gstate, PFSTATE_ENLIGHTING, PF_ON);
    pfGStateMode(gstate, PFSTATE_ENTEXTURE, PF_OFF);
    pfGSetGState(gset, gstate);
    geode = pfNewGeode();
    pfAddGSet(geode, gset);
    pfAddChild(shared->group, geode);

    while (t < 600)
    {
	pfCoord	view;
	pfNode *new;
	static int firsttime=1;

	pfSync();		
	t = pfGetTime();

	/* determine extent of scene's geometry */
	pfChanNearFar(chan, 1, 3*h*tsize);
	pfSetVec3(view.hpr, 0, -30, 0);
	pfSetVec3(view.xyz, 0, -h*tsize, h*tsize/2);
	pfChanView(chan, view.xyz, view.hpr);

	pfFrame();
	if (firsttime || shared->done==APPOWN) {
	    for (j=0; j < REGH; j++) {
		for (i=0; i < REGW; i++) {
		    n = j*REGW + i;
		    if (llist[n].state != DONE) continue;

		    new = llist[n].node;
		    if (new) {
			pfAddChild(tlist[llist[n].filenum].group, new);
			if (doscale) {
			    /* very slow, but at least it lets you see */
			    pfGetNodeBSphere(new, &bsphere);
			    pfDCSScale(tlist[llist[n].filenum].dcs,
				       tsize*0.5/bsphere.radius);
			}
		    }
		    if (dlist[n].node) {
			pfRemoveChild(tlist[dlist[n].filenum].group,
				      dlist[n].node);
			pfAsyncDelete(dlist[n].node);
		    }
		    dlist[n].node = llist[n].node;
		    dlist[n].filenum = llist[n].filenum;
		    llist[n].filenum = (regy+j)*w + (regx+i);
		    llist[n].state = REQUEST;
		}
	    }
	    shared->done = DBPOWN;
	    
	    /* move region right, and maybe up */
	    regx++;
	    if (regx >= w-1) {
		regx = 0;
		regy++;
		if (regy >= h-1) {
		    regy = 0;
		}
	    }
	    firsttime = 0;
	}
	pfDrawChanStats(chan);
    }
    pfExit();

    return 0;
}
