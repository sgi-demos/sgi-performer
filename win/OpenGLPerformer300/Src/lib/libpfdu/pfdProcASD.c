/*
 * Copyright 2000, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that (i) the above copyright notices and this
 * permission notice appear in all copies of the software and related
 * documentation, and (ii) the name of Silicon Graphics may not be
 * used in any advertising or publicity relating to the software
 * without the specific, prior written permission of Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR ANY SPECIAL,
 * INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY
 * THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <Performer/pfdu.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef mips
#include <bstring.h>
#else
#include <string.h>
#endif
#include <math.h>

#ifndef WIN32
#include <sys/resource.h>
#endif

#ifdef WIN32
#define READ_MODE "rb"
#define WRITE_MODE "wb"
#else
#define READ_MODE "r"
#define WRITE_MODE "w"
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))

typedef struct _Tile_Face
{
    int faceid;
    pfASDFace face;
} Tile_Face;

typedef struct _Tile_Vert
{
    int vertid;
    pfASDVert vert;
} Tile_Vert;

typedef struct _tileindex
{
    int id;
    short tileid[2];
} tileindex;

void writeProcessedTile(char *fname, int lod, short c0, short c1, int numf, int numv, pfASDFace *f, pfASDVert *v, pfBox *facebounds, pfASDTileNum *tilenum);
void copyvert(Tile_Vert *verts, pfASDVert *v, int vertid, int vertidinmem);
void copyface(Tile_Face *faces, pfASDFace *f, int faceid, tileindex *ftindex, tileindex *vtindex, pfASDTileNum *tilenum);

PFDUDLLEXPORT void 
pfdProcessASDTiles(char *fname, char *pagename)
{
    	FILE *confp, *fp, *pfp;
	int numlods;
	pfASDLODRange *lods;
	Tile_Face *faces;
	Tile_Vert *verts;
	pfASDFace *f;
	pfASDVert *v;	
	pfBox *facebounds;
	pfASDTileNum *tilenum;
        int *maxv, *maxf;
	int  mv, mf, numf, numv;
        int ci, cj;
        pfBox bbox;
        pfVec3 min;
 	pfVec3 *origin;
        int **totaltiles, **page, **cachesize;
        float **tilesize;
 	pfASD *asd;
 	int numfaces0;
	char prename[150];

   	tileindex *ftindex, *vtindex;
	int ***fmem, ***vmem;
	int findex, vindex;
   	short cache[2];

	int i, j, k, fi, start;
	int numtiles[2];

        if((confp = (FILE *)fopen(fname, READ_MODE))==NULL)
	{
	    pfNotify(PFNFY_WARN, PFNFY_ASSERT,
		"can't open file %s\n", fname);
	    return;
 	}

	if((pfp = (FILE *)fopen(pagename, READ_MODE))==NULL)
        {
            pfNotify(PFNFY_WARN, PFNFY_ASSERT,
                "can't open file %s\n", pagename);
  	}

	fscanf(confp, "%s", prename);
        fscanf(confp, "%d", &numfaces0);
        fscanf(confp, "%d", &numlods);
        page = (int **)pfMalloc(numlods*sizeof(int*), NULL);
        cachesize = (int **)pfMalloc(numlods*sizeof(int*), NULL);
        totaltiles = (int **)pfMalloc(numlods*sizeof(int*), NULL);
        tilesize = (float **)pfMalloc(numlods*sizeof(float *), NULL);
  	maxf = (int *)pfMalloc(numlods*sizeof(int), NULL);
  	maxv = (int *)pfMalloc(numlods*sizeof(int), NULL);
        for(ci = 0; ci < numlods; ci++)
        {
            page[ci] = (int *)pfMalloc(2*sizeof(int), NULL);
	    cachesize[ci] = (int *)pfMalloc(2*sizeof(int), NULL);
            totaltiles[ci] = (int *)pfMalloc(2*sizeof(int), NULL);
            tilesize[ci] = (float *)pfMalloc(2*sizeof(float), NULL);
        }

        lods = (pfASDLODRange *) pfMalloc(sizeof(pfASDLODRange) * numlods,
                                    pfGetSharedArena());
        for(ci = 0; ci < numlods; ci++)
            fscanf(confp, "%f %f", &(lods[ci].switchin), &(lods[ci].morph));
        fscanf(confp, "%f %f %f", &min[0], &min[1], &min[2]);
        PFCOPY_VEC3(bbox.min, min);
        fscanf(confp, "%f %f %f", &bbox.max[0], &bbox.max[1], &bbox.max[2]);
	origin = (pfVec3*)pfMalloc(sizeof(pfVec3)*numlods, NULL);
    	for(i = 0; i< numlods; i++)
            PFCOPY_VEC3(origin[i], min);
	numtiles[0] = numtiles[1] = 0;
        for(ci = 0; ci < numlods; ci++)
	{
            fscanf(confp, "%d %d", &totaltiles[ci][0], &totaltiles[ci][1]);
	    numtiles[0] += totaltiles[ci][0];
	    numtiles[1] += totaltiles[ci][1];
	}

        for(ci = 0; ci < numlods; ci++)
        {
            fscanf(pfp, "%d %d", &page[ci][0], &page[ci][1]);
 	    if(page[ci][0] == 0) page[ci][0]+=1;
	    if(page[ci][1] == 0) page[ci][1]+=1;
	    cachesize[ci][0] = MIN(2*page[ci][0], totaltiles[ci][0]);
	    cachesize[ci][1] = MIN(2*page[ci][1], totaltiles[ci][1]);
        }
        for(ci = 0; ci < numlods; ci++)
            fscanf(confp, "%f %f", &tilesize[ci][0], &tilesize[ci][1]);
 	mf = mv = -1;
     	for(ci = 0; ci < numlods; ci++)
	{
            fscanf(confp, "%d %d", &maxf[ci], &maxv[ci]);
	    if(maxf[ci] > mf) mf = maxf[ci];
	    if(maxv[ci] > mv) mv = maxv[ci];
	}

/* process tiles in 2 passes. */

/* pass 1: assign tile ids to each face and vertex entry */
    fmem = (int ***)pfMalloc(sizeof(int **)*numlods, pfGetSharedArena());
    vmem = (int ***)pfMalloc(sizeof(int **)*numlods, pfGetSharedArena());
    findex = vindex = 0;
    for(i = 0; i < numlods; i++)
    {
        fmem[i] = (int **)pfMalloc(sizeof(int *)*cachesize[i][0], pfGetSharedArena());
        vmem[i] = (int **)pfMalloc(sizeof(int *)*cachesize[i][0], pfGetSharedArena());

        for(j=0; j<cachesize[i][0]; j++)
        {
            fmem[i][j] = (int *)pfMalloc(sizeof(int)*cachesize[i][1], pfGetSharedArena());
            vmem[i][j] = (int *)pfMalloc(sizeof(int)*cachesize[i][1], pfGetSharedArena());
            for(k=0; k<cachesize[i][1]; k++)
            {
                fmem[i][j][k] = findex;
/*
printf("fmem[%d][%d][%d] = %d\n", i, j, k, findex);
*/
                findex += mf;
                vmem[i][j][k] = vindex;
                vindex += mv;
            }
        }
    }

    faces = (Tile_Face *)pfMalloc(sizeof(Tile_Face)*mf, pfGetSharedArena());
    facebounds = (pfBox *)pfMalloc(sizeof(pfBox)*mf, pfGetSharedArena());
    verts = (Tile_Vert *)pfMalloc(sizeof(Tile_Vert)*mv, pfGetSharedArena());
    ftindex = (tileindex *)pfMalloc(sizeof(tileindex)*mf*numtiles[0]*numtiles[1], pfGetSharedArena());
    vtindex = (tileindex *)pfMalloc(sizeof(tileindex)*mv*numtiles[0]*numtiles[1], pfGetSharedArena());

    f = (pfASDFace *)pfMalloc(sizeof(pfASDFace)*mf, pfGetSharedArena());
    v = (pfASDVert *)pfMalloc(sizeof(pfASDVert)*mv, pfGetSharedArena());
    tilenum = (pfASDTileNum *)pfMalloc(sizeof(pfASDTileNum)*mf, pfGetSharedArena());

    for(i = 0; i < numlods; i++)
	for(j = 0; j < totaltiles[i][0]; j++)
	    for(k = 0; k < totaltiles[i][1]; k++)
    	    {
		sprintf(fname, "%s%02d%03d%03d", prename, i, j, k);
		printf("processing file %s\n", fname);
		if(!(fp = (FILE *)fopen(fname, READ_MODE)))
		{
		    printf("can't open file %s\n", fname);
		    continue;
		}
		fread(&numf, sizeof(int), 1, fp);
		fread(&numv, sizeof(int), 1, fp);	
		fread(faces, sizeof(Tile_Face), numf, fp);
		fread(facebounds, sizeof(pfBox), numf, fp);
		fread(verts, sizeof(Tile_Vert), numv, fp);
		fclose(fp);
		cache[0] = j%(cachesize[i][0]);
		cache[1] = k%(cachesize[i][1]);
		start = fmem[i][cache[0]][cache[1]];
		for(fi = 0; fi < numf; fi++)
		{
		    ftindex[faces[fi].faceid].id = start+fi;
		    ftindex[faces[fi].faceid].tileid[0] = j;
		    ftindex[faces[fi].faceid].tileid[1] = k;
		}
		start = vmem[i][cache[0]][cache[1]];
		for(fi = 0; fi < numv; fi++)
		{
		    vtindex[verts[fi].vertid].id = start+fi;
/*
printf("vtindex[verts[%d].vertid = %d].id = %d %d+%d\n", fi, verts[fi].vertid, vtindex[verts[fi].vertid].id, start, fi);
*/
		    vtindex[verts[fi].vertid].tileid[0] = j;
		    vtindex[verts[fi].vertid].tileid[1] = k;
		}
	    }

/* pass 2, match up the indices in face structures */
    for(i = 0; i < numlods; i++)
        for(j = 0; j < totaltiles[i][0]; j++)
            for(k = 0; k < totaltiles[i][1]; k++)
            {
                sprintf(fname, "%s%02d%03d%03d", prename, i, j, k);
                printf("2nd pass to file %s\n", fname);
                if(!(fp = (FILE *)fopen(fname, READ_MODE)))
                {
                    printf("can't open file %s\n", fname);
                    continue;
                }
		cache[0] = j%(cachesize[i][0]);
                cache[1] = k%(cachesize[i][1]);
                start = vmem[i][cache[0]][cache[1]];
                fread(&numf, sizeof(int), 1, fp);
                fread(&numv, sizeof(int), 1, fp);
                fread(faces, sizeof(Tile_Face), numf, fp);
                fread(facebounds, sizeof(pfBox), numf, fp);
                fread(verts, sizeof(Tile_Vert), numv, fp);
                fclose(fp);
		for(fi = 0; fi < numf; fi++)
		    copyface(faces, f, fi, ftindex, vtindex, tilenum);
		for(fi = 0; fi < numv; fi++)
		    copyvert(verts, v, fi, start+fi);
		writeProcessedTile(fname, i, j, k, numf, numv, f, v, facebounds, tilenum);
	    }

}

void
copyface(Tile_Face *faces, pfASDFace *f, int faceid, tileindex *ftindex, tileindex *vtindex, pfASDTileNum *tilenum)
{
    int i;
    if(f == NULL) return;
    if(faces == NULL) return;

    memcpy(&f[faceid], &(faces[faceid].face), sizeof(pfASDFace));
    for(i = 0; i < 3; i++)
    {
/*
printf("before matching f[%d].vert[%d] = %d\n", faceid, i, f[faceid].vert[i]);
*/
	f[faceid].vert[i] = vtindex[f[faceid].vert[i]].id;
/*
printf("after matching f[%d].vert[%d] = %d\n", faceid, i, f[faceid].vert[i]);
*/

	if(f[faceid].refvert[i] != PFASD_NIL_ID)
	{
	    tilenum[faceid].s[i][0] = vtindex[f[faceid].refvert[i]].tileid[0];
	    tilenum[faceid].s[i][1] = vtindex[f[faceid].refvert[i]].tileid[1];
/*
printf("before matching f[%d].refvert[%d] = %d\n", faceid, i, f[faceid].refvert[i]);
*/
	    f[faceid].refvert[i] = vtindex[f[faceid].refvert[i]].id;
/*
printf("after matching f[%d].refvert[%d] = %d\n", faceid, i, f[faceid].refvert[i]);
*/
	}
    }
    for(i = 0; i < 4; i++)
	if(f[faceid].child[i] != PFASD_NIL_ID)
	{
	    tilenum[faceid].c[0] = ftindex[f[faceid].child[i]].tileid[0];
	    tilenum[faceid].c[1] = ftindex[f[faceid].child[i]].tileid[1];
	    f[faceid].child[i] = ftindex[f[faceid].child[i]].id;
	}
}

void
copyvert(Tile_Vert *verts, pfASDVert *v, int vertid, int vertidinmem)
{
    if(v == NULL) return;
    if(verts == NULL) return;

    memcpy(&v[vertid], &(verts[vertid].vert), sizeof(pfASDVert));
/*
    printf("vertid %d\n", vertidinmem);
    printf("v0 (%f, %f, %f), vd (%f, %f, %f)\n", v[vertid].v0[0],
	v[vertid].v0[1], v[vertid].v0[2], v[vertid].vd[0],
	v[vertid].vd[1], v[vertid].vd[2]);
*/
}

void
writeProcessedTile(char *fname, int lod, short c0, short c1, int numf, int numv, pfASDFace *f, pfASDVert *v, pfBox *facebounds, pfASDTileNum *tilenum)
{
    char newname[150];
    FILE *fp;

    sprintf(newname, "%s.asd", fname);
    printf("write file %s\n", newname);
    if(!(fp = (FILE *)fopen(newname, WRITE_MODE)))
    {
        printf("can't open file %s\n", newname);
        return;
    }

    fwrite(&numf, sizeof(int), 1, fp);
    fwrite(&numv, sizeof(int), 1, fp);
    fwrite(f, sizeof(pfASDFace), numf, fp);
    fwrite(facebounds, sizeof(pfBox), numf, fp);
    fwrite(v, sizeof(pfASDVert), numv, fp);
    fwrite(tilenum, sizeof(pfASDTileNum), numf, fp);
    fclose(fp);
}
