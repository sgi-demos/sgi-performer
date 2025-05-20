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
#include <stdlib.h>
#include <sys/types.h>
#include <malloc.h>

#ifdef __linux__
#include <asm/byteorder.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <limits.h>

typedef unsigned int uint;
typedef unsigned short ushort;

#define PFPFB_DLLEXPORT __declspec(dllexport)
#else
#define PFPFB_DLLEXPORT

#endif /* WIN32 */

#ifndef P_32_SWAP
#define	P_32_SWAP(a) {							\
	uint _tmp = *(uint *)a;				\
	((char *)a)[0] = ((char *)&_tmp)[3];				\
	((char *)a)[1] = ((char *)&_tmp)[2];				\
	((char *)a)[2] = ((char *)&_tmp)[1];				\
	((char *)a)[3] = ((char *)&_tmp)[0];				\
}
#endif  /* P_32_SWAP */

#ifndef P_16_SWAP
#define	P_16_SWAP(a) {							\
	ushort _tmp = *(ushort *)a;				\
	((char *)a)[0] = ((char *)&_tmp)[1];				\
	((char *)a)[1] = ((char *)&_tmp)[0];				\
}
#endif  /* P_16_SWAP */


size_t swapped_fread32(void *ptr, size_t size, size_t nitems, FILE *stream)
{
        unsigned char *p = ptr;
        int i, n;

		if((size != 4) && (nitems > 1))
		{
			n = (size * nitems)/4;
		}
		else if((size != 4) && nitems == 1)
		{
			n = size/4;
		}
		else
		{
			n = nitems;
		}

        /* do 32bit swap */
        nitems = fread(ptr, 4, n, stream);
        for(i=0; i<n; ++i)
        {
            P_32_SWAP(p);
            p += 4;
        }
        return nitems;
}

size_t swapped_fread16(void *ptr, size_t size, size_t nitems, FILE *stream)
{
        unsigned char *p = ptr;
        int i;

        /* do 16bit swap */
        nitems = fread(ptr, 2, nitems, stream);
        for(i=0; i<nitems; ++i)
        {
            P_16_SWAP(p);
            p += 2;
        }
        return nitems;
}

size_t pfb_fread(void *ptr, size_t size, size_t nitems, FILE *stream)
{
#ifdef __LITTLE_ENDIAN
		if(size == 1)
		{
			/* normal fread */
			nitems = fread(ptr, size, nitems, stream);
			return nitems;
		}
		else if(size == 2)
		{
			/* swapped_fread16 */
			nitems = swapped_fread16(ptr, size, nitems, stream);
			return nitems;
		}
		else if((size == 4) || (size%4 == 0))
		{
			/* swapped_fread32 */
			nitems = swapped_fread32(ptr, size, nitems, stream);
			return nitems;
		}
#else
			/* normal fread */
			nitems = fread(ptr, size, nitems, stream);
			return nitems;
#endif
}



#include <Performer/pf.h>
#include <Performer/pfdu.h>

typedef struct {
    int level;
    int tsid;
    int vert[3];
    int norm[3];
    int refvert[3];
    int snorm[3];
    int child[4];
} evtFace;

int cnt;

#ifndef WIN32
void printMallInfo(struct mallinfo *mall)
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Heap - total bytes used: %d",
        mall->usmblks + mall->uordblks);
}

void printAMallInfo(struct mallinfo *mall)
{
    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "Arena - total bytes used: %d",
        mall->usmblks + mall->uordblks);
}
#endif

void *
copyLeaf(pfVec3 *V, int fid, pfASDFace *faces, pfASDVert *verts)
{
    int i, j;
	int isleaf = 0;
	for(j = 0; j < 4; j++)
	    if(faces[fid].child[j] != PFASD_NIL_ID)
	    {
		copyLeaf(V, faces[fid].child[j], faces, verts);
		isleaf = 1;
	    }
	if(isleaf == 0)
	{
	PFCOPY_VEC3(V[cnt], verts[faces[fid].vert[0]].v0);
	cnt++;
	PFCOPY_VEC3(V[cnt], verts[faces[fid].vert[1]].v0);
	cnt++;
	PFCOPY_VEC3(V[cnt], verts[faces[fid].vert[2]].v0);
	cnt++;
	}
}

PFPFB_DLLEXPORT extern pfNode *
pfdLoadFile_evt(char *fileName)
{
    FILE *evtFile;
    int numlods;
    pfASDLODRange *lods;
    int numverts;
    pfVec3 *v0, *vd;
    int numnormals;
    pfVec3 *n0, *nd;
    int numfaces, numfaces0;
    evtFace *evtfaces;
    pfASDFace *faces;
    pfASDVert *verts;
    float *attrs, *ptr;
    int stride;
    int numattrs, attrmode;
    pfASD *node;
    pfGeoState *gstate, **gsa;
    pfMaterial *material;
    int xx;

    char fname[150];

    int i, j;

    if ((evtFile = pfdOpenFile(fileName)) == NULL)
    {
        fprintf(stderr,"can't open inputfile %s\n", fileName);
        return(NULL);
    }

    /* LOD Ranges */

    pfb_fread(&numlods, sizeof(int), 1, evtFile);
    lods = (pfASDLODRange *) pfMalloc(sizeof(pfASDLODRange) * (numlods+3),
                                    pfGetSharedArena());
    pfb_fread(lods, sizeof(pfASDLODRange), numlods, evtFile);
    if(lods[0].switchin < 0.05)
    {
	lods[0].switchin = pow( 2, 18.0 );
        lods[0].morph = 0;
	for(i = 1; i < numlods; i++)
	{
	    lods[i].switchin = lods[i-1].switchin/2;
	    lods[i].morph = lods[i].switchin * 0.6;
   	}
    }
    /* Vertices */
    pfb_fread(&numverts, sizeof(int), 1, evtFile);
    printf("numverts %d\n", numverts);
    v0 = (pfVec3 *) pfMalloc(sizeof(pfVec3) * numverts,
                             pfGetSharedArena());
    vd = (pfVec3 *) pfMalloc(sizeof(pfVec3) * numverts,
                             pfGetSharedArena());
    verts = (pfASDVert *)pfMalloc(sizeof(pfASDVert) * numverts,
                             pfGetSharedArena());
    pfb_fread(v0, sizeof(pfVec3), numverts, evtFile);
    pfb_fread(vd, sizeof(pfVec3), numverts, evtFile);

    /* Normals */

    pfb_fread(&numnormals, sizeof(int), 1, evtFile);
    n0 = (pfVec3 *) pfMalloc(sizeof(pfVec3) * numnormals,
                             pfGetSharedArena());
    nd = (pfVec3 *) pfMalloc(sizeof(pfVec3) * numnormals,
                             pfGetSharedArena());
    pfb_fread(n0, sizeof(pfVec3), numnormals, evtFile);
    pfb_fread(nd, sizeof(pfVec3), numnormals, evtFile);

    /* for asdfly, we are not dealing with per vertex normals */

    /* Faces */

    pfb_fread(&numfaces, sizeof(int), 1, evtFile);

    pfb_fread(&numfaces0, sizeof(int), 1, evtFile);
    printf("numfaces %d numfaces0 %d\n", numfaces, numfaces0);
    evtfaces = (evtFace *)pfMalloc(sizeof(evtFace) * numfaces,
                                pfGetSharedArena());
    faces = (pfASDFace *) pfMalloc(sizeof(pfASDFace) * numfaces,
                                 pfGetSharedArena());
    pfb_fread(evtfaces, sizeof(evtFace), numfaces, evtFile);


    fclose(evtFile);

    for(i = 0; i < numverts; i++)
    {
        PFCOPY_VEC3(verts[i].v0, v0[i]);
        PFCOPY_VEC3(verts[i].vd, vd[i]);
    }

#define COLORRING
#ifdef COLORRING
/* assing a color per LOD */
    stride = 4;
    attrs = (float *)pfMalloc(sizeof(float)*2*stride*20,
                        pfGetSharedArena());
    ptr = attrs;
        PFSET_VEC4(ptr, 199.9/255.0, 53.0/255.0, 156.0/255.0, 1.0);
    ptr += stride;
        PFSET_VEC4(ptr, 0, 0, 0, 0);
    ptr += stride;
        PFSET_VEC4(ptr, 211.0/255.0, 1.0, 87.0/255.0, 1.0);
    ptr += stride;
        PFSET_VEC4(ptr, 0, 0, 0, 0);
    ptr += stride;
        PFSET_VEC4(ptr, 1.0, 1.0, 121.0/255.0, 1.0);
    ptr += stride;
        PFSET_VEC4(ptr, 0, 0, 0, 0);
    ptr += stride;
        PFSET_VEC4(ptr, 1.0, 0.0, 186.0/255.0, 1.0);
    ptr += stride;
        PFSET_VEC4(ptr, 0, 0, 0, 0);
    ptr += stride;
        PFSET_VEC4(ptr, 1.0, 104.0/255.0, 108.0/255.0, 1.0);
    ptr += stride;
        PFSET_VEC4(ptr, 0, 0, 0, 0);
    ptr += stride;
        PFSET_VEC4(ptr, 139.0/255.0, 1.0, 108.0/255.0, 1.0);
    ptr += stride;
        PFSET_VEC4(ptr, 0, 0, 0, 0);
    ptr += stride;
        PFSET_VEC4(ptr, 139.0/255.0, 0.0, 108.0/255.0, 1.0);
    ptr += stride;
        PFSET_VEC4(ptr, 0, 0, 0, 0);
    ptr += stride;
        PFSET_VEC4(ptr, 1.0, 119.0/255.0, 204.0/255.0, 1.0);
    ptr += stride;
        PFSET_VEC4(ptr, 0, 0, 0, 0);
    ptr += stride;
        PFSET_VEC4(ptr, 139.0/255.0, 182.0/255.0, 1.0, 1.0);
    ptr += stride;
        PFSET_VEC4(ptr, 0, 0, 0, 0);
    ptr += stride;
        PFSET_VEC4(ptr, 1.0, 34.0/255.0, 0.0, 1.0);
    ptr += stride;
        PFSET_VEC4(ptr, 0, 0, 0, 0);
    ptr += stride;
        PFSET_VEC4(ptr, 1.0, 0.0, 1.0, 1.0);
    ptr += stride;
        PFSET_VEC4(ptr, 0, 0, 0, 0);
    ptr += stride;
        PFSET_VEC4(ptr, 1.0, 0.0, 0.5, 1.0);
    ptr += stride;
        PFSET_VEC4(ptr, 0, 0, 0, 0);
    ptr += stride;
        PFSET_VEC4(ptr, 1.0, 1.0, 0.5, 1.0);
    ptr += stride;
        PFSET_VEC4(ptr, 0, 0, 0, 0);
    ptr += stride;

    numattrs = (ptr - attrs)/(2*stride);
    attrmode = PFASD_COLORS;

    for(i = 0; i < numfaces; i++)
    {
        faces[i].level = evtfaces[i].level;
        faces[i].tsid = evtfaces[i].tsid;
        for(j = 0; j < 3; j++)
        {
            faces[i].vert[j] =  evtfaces[i].vert[j];
            faces[i].attr[j] = faces[i].level;
            faces[i].refvert[j] = evtfaces[i].refvert[j];
            faces[i].sattr[j] = faces[i].level+1;
            faces[i].child[j] = evtfaces[i].child[j];
        }

        faces[i].child[3] = evtfaces[i].child[3];
	faces[i].gstateid = 0;
	faces[i].mask = 0;
    }
#endif /* COLORRING */

#ifdef NORMAL
    attrs = (float *)pfMalloc(sizeof(float) * 2 * 3 * numnormals,
                             pfGetSharedArena());
    stride = 3; /* we only have normal */
    for(i = 0; i < numnormals; i++)
    {
        attrs[2*i*stride+0] = n0[i][0];
        attrs[2*i*stride+1] = n0[i][1];
        attrs[2*i*stride+2] = n0[i][2];
        attrs[(2*i+1)*stride+0] = nd[i][0];
        attrs[(2*i+1)*stride+1] = nd[i][1];
        attrs[(2*i+1)*stride+2] = nd[i][2];
    }
    numattrs = numnormals;
    attrmode = PFASD_NORMALS;

    for(i = 0; i < numfaces; i++)
    {
	faces[i].level = evtfaces[i].level;
        faces[i].tsid = evtfaces[i].tsid;
        for(j = 0; j < 3; j++)
        {
            faces[i].vert[j] =  evtfaces[i].vert[j];
	    faces[i].attr[j] =  evtfaces[i].norm[j];
	    faces[i].refvert[j] = evtfaces[i].refvert[j];
	    faces[i].sattr[j] = evtfaces[i].snorm[j];
	    faces[i].child[j] = evtfaces[i].child[j];
        }

        faces[i].child[3] = evtfaces[i].child[3];
    }
#endif

#ifdef BUILDPAGING
        {
            int lookahead[2];
            char prename[300], confname[300], pagename[300];
            lookahead[0] = lookahead[1] = 6;

            sprintf(prename, "tile");
            sprintf(confname, "conf");
            sprintf(pagename, "page");

        pfdBreakTiles(numlods, (pfASDLODRange *)lods, numverts, verts, numfaces, faces, numfaces0, prename, confname);
        pfdPagingLookahead(confname, pagename, lookahead);
	node = NULL;
        }
#else

    node = pfNewASD();

    pfASDAttr(node, PFASD_LODS, 0, numlods, lods);
    pfASDAttr(node, PFASD_COORDS, 0, numverts, verts);
    if(numattrs > 0)
    {
        pfASDAttr(node, PFASD_PER_VERTEX_ATTR, attrmode, numattrs, attrs);
	/* the morphing of color is controled by applications */
        pfASDMorphAttrs(node, PFASD_COLORS);
    }

    pfASDAttr(node, PFASD_FACES, 0, numfaces, faces);
    pfASDNumBaseFaces(node, numfaces0);
    /*
     *  Make a default GeoState for the newly loaded ASD node.
     *  This just uses the default material; note that the
     *  pfMtlColorMode must be enabled to allow pfGeoSet or other
     *  RGB color commands to override this default color setting.
     */

/*
    gstate = pfNewGState(pfGetSharedArena());
    gsa = (pfGeoState **) pfMalloc(sizeof(pfGeoState*), pfGetSharedArena());
    *gsa = gstate;
    material = pfNewMtl(pfGetSharedArena());
    pfMtlColorMode(material, PFMTL_FRONT, PFMTL_CMODE_OFF);
    pfGStateAttr(gstate, PFSTATE_FRONTMTL, material);
    pfGStateMode(gstate, PFSTATE_ENLIGHTING, PF_ON);
    pfASDGStates(node, gsa, 1);
    pfFree(gsa);
    pfRef(gstate);
*/

    pfASDConfig(node);
    {
        char *e = getenv("_PFASD_CLIPRINGS");
        int _PFASD_CLIPRINGS = (e ? *e ? atoi(e) : -1 : 0);

        if(_PFASD_CLIPRINGS)
            pfdASDClipring(node, _PFASD_CLIPRINGS);
    }

#endif

    pfFree(v0);
    pfFree(vd);
    pfFree(n0);
    pfFree(nd);
    pfFree(evtfaces);

    return (pfNode *) node;
}

