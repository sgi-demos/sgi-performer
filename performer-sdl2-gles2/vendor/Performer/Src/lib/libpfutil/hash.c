/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
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
 * file: hash.c 
 * ------------
 * hash-table management
 *
 * $Revision: 1.37 $ 
 * $Date: 2002/05/15 00:31:09 $
 */

#include <stdio.h>
#ifndef WIN32
#include <alloca.h>
#endif
#include <stdlib.h>
#ifdef mips
#include <bstring.h>
#endif 
#include <math.h>
#include <Performer/pfutil.h>

/* #define	STATS */

#define ELTS_PER_BUCKET		1

#define SIZEOF_ELT(ht)	     					\
        ((int)sizeof(pfuHashElt) + (ht)->eltSize)

#define INDEX_ELT(ht,b,i)    					\
        ((pfuHashElt*)((char*) (b)->elts + (i)*SIZEOF_ELT(ht)))

#define COPY_ELT(ht,dst,e)	{				 \
	(dst)->listIndex = (e)->listIndex; (dst)->key = (e)->key;\
        (dst)->id = (e)->id; (dst)->userData = (e)->userData; \
    	bcopy((char*)(e)->data, (char*)(dst)->data, (int)((ht)->eltSize)); \
}

static pfuHashBucket* 	newBucket(pfuHashTable *ht);
static void		delBucket(pfuHashBucket *b);
static unsigned int	getKey(pfuHashTable *ht, pfuHashElt *elt);

static float		CreaseTol = .2f; 	/* acos(~78 degrees) */


	/*------------------------------------------------------*/

PFUDLLEXPORT pfuHashTable*
pfuNewHTable(int numb, int eltsize, void *arena)
{
    int		realeltsize, i, j, bucketSize;
    pfuHashBucket 	*b, *bucket;
    pfuHashTable	*ht;

    /* eltSize must be multiple of sizeof(b) bytes */
    realeltsize = eltsize;
    if (eltsize & (sizeof(b)-1))
    {
        eltsize += sizeof(b) - eltsize&(sizeof(b)-1);
    }

    numb = pfuCalcHashSize(numb);

    ht = (pfuHashTable*) pfCalloc(1, (unsigned int)sizeof(pfuHashTable), arena);

    ht->numBuckets = numb;
    ht->buckets = (pfuHashBucket**)
	pfMalloc((unsigned int)(sizeof(pfuHashBucket*)*numb), arena);
    ht->eltSize = eltsize;
    ht->realeltSize = realeltsize;
    ht->arena = arena;
	
    /* 
     * Malloc buckets in one big chunk for faster mallocing and better
     * memory localization.
     */
    bucketSize = (int)sizeof(pfuHashBucket) + ELTS_PER_BUCKET * SIZEOF_ELT(ht);
    b = (pfuHashBucket*)pfCalloc((unsigned int)numb, (unsigned int)bucketSize, ht->arena);

    for(i=0; i<numb; i++)
    {
	bucket = (pfuHashBucket*) ((char*)b + i*bucketSize);
	bucket->elts = (pfuHashElt*) ((char*)bucket + sizeof(pfuHashBucket));

    	for(j=0; j<ELTS_PER_BUCKET; j++)
	    INDEX_ELT(ht, bucket, j)->data = 
		(void*) ((char*)INDEX_ELT(ht, bucket, j) + sizeof(pfuHashElt));
					 	
	ht->buckets[i] = bucket;
    }

    ht->list = (pfuHashElt**) pfCalloc((unsigned int)numb, sizeof(pfuHashElt*), arena);
    ht->listCount = 0;
    ht->listAvail = numb;

    return ht;
}

PFUDLLEXPORT void
pfuDelHTable(pfuHashTable* ht)
{
    int	i;

#ifdef STATS
    unsigned int minOccupancy = ~0;
    unsigned int maxOccupancy =  0;
    unsigned int totOccupancy =  0;
#endif

    for(i=0; i<ht->numBuckets; i++)
    {
#ifdef STATS
	pfuHashBucket	*b;
	unsigned int minOcc = ~0;
	unsigned int maxOcc =  0;

	b = ht->buckets[i];
	if (b)
	{
	    int chainSize = 0;
	    while (b)
	    {
		chainSize += b->nelts;
		b = b->next;
	    }

	    if (minOccupancy > chainSize)
		minOccupancy = chainSize;
	    if (maxOccupancy < chainSize)
		maxOccupancy = chainSize;
	    totOccupancy += chainSize;
	}
#endif

	if (ht->buckets[i]->next)
	    delBucket(ht->buckets[i]->next);
    }
#ifdef STATS
    pfNotify(PFNFY_ALWAYS, PFNFY_RESOURCE, 
	"pfuDelHTable: tot %8d, size %5d, min %3d, max %4d, load %6.4f", 
	    totOccupancy,
	    ht->numBuckets, 
	    minOccupancy,
	    maxOccupancy,
	    (double)totOccupancy/(double)ht->numBuckets);
#endif

    pfFree(ht->buckets[0]);
    pfFree(ht->list);
    pfFree(ht->buckets);
    pfFree(ht);
}

static void
delBucket(pfuHashBucket *b)
{
    pfuHashBucket 	*next;

    while(b)
    {
	next = b->next;
	pfFree(b);
	b = next;
    }
}

static pfuHashBucket*
newBucket(pfuHashTable *ht)
{
    pfuHashBucket* 	b;
    int		i;

    b = (pfuHashBucket*)pfCalloc(1, (unsigned int)(sizeof(pfuHashBucket) + ELTS_PER_BUCKET * 
					SIZEOF_ELT(ht)), ht->arena);

    b->elts = (pfuHashElt*) ((char*)b + sizeof(pfuHashBucket));

    for(i=0; i<ELTS_PER_BUCKET; i++)
	INDEX_ELT(ht, b, i)->data = (void*) ((char*)INDEX_ELT(ht, b, i) 
					 	+ sizeof(pfuHashElt));
					
    return b;
}

PFUDLLEXPORT void
pfuResetHTable(pfuHashTable *ht)
{
    int 		i;
    pfuHashBucket	*b;

    for (i=0; i<ht->numBuckets; i++)
    {
	b = ht->buckets[i];
	while (b)
	{
	    b->nelts = 0;
	    b = b->next;
	}
    }
    ht->listCount = 0;
}

PFUDLLEXPORT int
pfuFindHash(pfuHashTable *ht, pfuHashElt *elt)
{
    int 		i, nlongs, *ld0;
    pfuHashElt		*dst;

    nlongs = ht->realeltSize >> 2;
    ld0 = (int*)elt->data;

    for (i=0; i<ht->listCount; i++)
    {
	int	j, *ld1;

	dst = ht->list[i];

	if(elt->key != dst->key)
	    continue;

    	ld1 = (int*)dst->data;

	for(j=0; j<nlongs; j++)
	{
	    if(ld0[j] != ld1[j])
	         break;
	}
	    
	/* Found */
	if(j == nlongs)
	    return 1;
    }
    return 0;
}

PFUDLLEXPORT int
pfuRemoveHash(pfuHashTable *ht, pfuHashElt *elt)
{
    unsigned int 		index;
    pfuHashBucket	*b;
    pfuHashElt		*search;

    elt->key = getKey(ht, elt);
    index = elt->key % ht->numBuckets;

    b = ht->buckets[index];

    if(b->nelts == 0)
	return 0;
    else
    {
	int	i, j, n, nlongs, *ld0, *ld1;

	nlongs = ht->realeltSize >> 2;
    	ld0 = (int*)elt->data;

        while(b != NULL)
        {
	    n = b->nelts; 

	    for(i=0; i<n; i++)
	    {
	        search = INDEX_ELT(ht, b, i);

	        if(elt->key != search->key)
	    	    continue;

    	        ld1 = (int*)search->data;

	        for(j=0; j<nlongs; j++)
	        {
	    	    if(ld0[j] != ld1[j])
	    	        break;
	        }
	    
	        /* Found */
	        if(j == nlongs)
		{
		    while (1)
		    {
                        int        k;
                        pfuHashElt   *src, *dst;

                        /* Shift down elements in bucket */
    		    	for(k=i; k<b->nelts-1; k++)
			{
                            dst = INDEX_ELT(ht, b, k); 
                            src = INDEX_ELT(ht, b, k+1);
			    COPY_ELT(ht, dst, src);
                                         
			}
			if (b->next)
			{
                            dst = INDEX_ELT(ht, b, ELTS_PER_BUCKET-1);
                            src = INDEX_ELT(ht, b->next, 0);
			    COPY_ELT(ht, dst, src);
	                                 
                            b = b->next;
                            i = 0;
			}
                        else
                        {
			    /* Put end of list into removed element's spot */
			    ht->list[--ht->listCount]->listIndex = 
							search->listIndex;
			    ht->list[search->listIndex] = 
						ht->list[ht->listCount];

                            b->nelts--;
                            return 1;
                        }
                    }
		}
	    }
	
	    if(n < ELTS_PER_BUCKET)
		break;
            b = b->next;
	}

        /* Element not in bucket */
	return 0;
    }
}
    


PFUDLLEXPORT pfuHashElt*
pfuEnterHash(pfuHashTable *ht, pfuHashElt *elt)
{
    unsigned int 		index;
    pfuHashBucket	*b, *tmp;
    pfuHashElt		*dst;

    elt->key = getKey(ht, elt);
    index = elt->key % ht->numBuckets;

    b = ht->buckets[index];

    if(b->nelts == 0)
    {
	if(ht->listCount >= ht->listAvail)
	{
	    ht->listAvail <<= 1;
	    ht->list = (pfuHashElt**) pfRealloc(ht->list, 
					(unsigned int)(ht->listAvail * sizeof(pfuHashElt*)));
	}

	dst = INDEX_ELT(ht, b, 0);
	elt->listIndex = ht->listCount;
	ht->list[ht->listCount++] = dst;
	COPY_ELT(ht, dst, elt);

    	b->nelts++;
    	return NULL;
    }
    else 
    {
	int	i, j, n, nlongs, *ld0, *ld1;

	nlongs = ht->realeltSize >> 2;
    	ld0 = (int*)elt->data;

        while(b != NULL)
        {
	    n = b->nelts; 

	    for(i=0; i<n; i++)
	    {
	        dst = INDEX_ELT(ht, b, i);

	        if(elt->key != dst->key)
	    	    continue;

    	        ld1 = (int*)dst->data;

	        for(j=0; j<nlongs; j++)
	        {
	    	    if(ld0[j] != ld1[j])
	    	        break;
	        }
	    
	        /* Found */
	        if(j == nlongs)
	    	    return dst;
	    }
	
	    if(n < ELTS_PER_BUCKET)
		break;
	    tmp = b;
            b = b->next;
        }
        
        /* Element not in bucket */

        /* Link new bucket if necessary */
        if(b == NULL)
        {
            b = newBucket(ht);
            tmp->next = b;
        }

        /* Copy element into bucket */
	dst = INDEX_ELT(ht, b, b->nelts);
	COPY_ELT(ht, dst, elt);

        /* Add element to flat list */
	if(ht->listCount >= ht->listAvail)
	{
	    ht->listAvail <<= 1;
	    ht->list = (pfuHashElt**) pfRealloc(ht->list, 
					(unsigned int)(ht->listAvail * sizeof(pfuHashElt*)));
	}
	dst->listIndex = ht->listCount;
	ht->list[ht->listCount++] = dst;

	b->nelts++;
	return NULL;
    }
}

/*
 *  Obscure, simple, fast, and TESTED hash function
 */
static unsigned int
getKey(pfuHashTable *ht, pfuHashElt *elt)
{
    unsigned int   size = ht->realeltSize;
    unsigned char *data = (unsigned char *)elt->data;
    unsigned int   hash = 0;
    unsigned int   wrap = 0;

    while (size--)
    {
	hash = (hash << 4) + *data++;
	wrap =  hash & 0xf0000000;
	if (wrap != 0)
	    hash ^= wrap >> 24;
    }
    return hash;
}

/*
 *  Division method of table indexing needs prime table size
 */
PFUDLLEXPORT int
pfuCalcHashSize(int size)
{
    /* prime[i] is largest prime less than 2^(i+2) */
    static const int prime[] =
    {
             3, 
	     7, 
	    13, 
	    31,
            61, 
	   127, 
	   251, 
	   509,
          1021, 
	  2039, 
	  4093, 
	  8191,
         16381, 
	 32749, 
	 65521, 
	131071,
        262139, 
	524287, 
       1048573, 
       2097143,
       4194301, 
       8388593, 
      16777213, 
      33554393,
      67108859, 
     134217689, 
     268435399, 
     536870909,
    1073741789, 
    2147483647
    };
    static const int primeCount = sizeof(prime)/sizeof(prime[0]);

    int	i		= 0;
    int	hashSize	= 0;

    /* find prime near next power of two larger than size */
    while (i < primeCount)
	if ((hashSize = prime[i++]) >= size)
	    break;

    /* return chosen size */
    return hashSize;
}

static int
hashGSetVerts(pfGeoSet *gset, int computeNorms)
{
    int            i, j, n, nprims, vcount, vicount;
    pfVec3         *coords, *newCoords = NULL, *normals, *newNormals = NULL;
    pfVec2         *texCoords[PF_MAX_TEXTURES], 
		   *newTexCoords[PF_MAX_TEXTURES];
    pfVec4         *colors,  *newColors = NULL;
    unsigned short         *vindex, *tindex[PF_MAX_TEXTURES], *nindex,  *cindex;
    int            cbind, nbind, tbind[PF_MAX_TEXTURES], coff, noff, 
		   toff[PF_MAX_TEXTURES + 1];
    void	    *arena = pfGetArena(gset);
    pfuHashElt	    elt, *nelt;
    pfuHashTable    *ht;
    float           vert[3 + 2 + 3 + 4];
    pfList	    *triList;
    pfVec3	    *triNorms;
    int		    vertsPerPrim, primType;
    int		    t, numTextures;
    int		    texture_coord_size;

    for (t = 0 ; t < PF_MAX_TEXTURES ; t ++)
	newTexCoords[t] = NULL;

    numTextures = pfGetGSetNumTextures(gset);

    primType = pfGetGSetPrimType(gset);
    switch (primType) {
    case PFGS_QUADS:
	vertsPerPrim = 4;
	break;
    case PFGS_TRIS:
	vertsPerPrim = 3;
	break;
    case PFGS_LINES:
	vertsPerPrim = 2;
	break;
    case PFGS_POINTS:
	vertsPerPrim = 1;
	break;
    default:
	return 0;
    }

    nprims = pfGetGSetNumPrims(gset);
    pfGetGSetAttrLists(gset, PFGS_COORD3, (void **) &coords, &vindex);
    pfGetGSetAttrLists(gset, PFGS_COLOR4, (void **) &colors, &cindex);
    for (t = 0 ; t < numTextures ; t ++)
        pfGetGSetMultiAttrLists(gset, PFGS_TEXCOORD2, t,
                                (void**)&texCoords[t], &tindex[t]);
    pfGetGSetAttrLists(gset, PFGS_NORMAL3, (void **) &normals, &nindex);

    cbind = pfGetGSetAttrBind(gset, PFGS_COLOR4);
    nbind = pfGetGSetAttrBind(gset, PFGS_NORMAL3);
    for (t = 0 ; t < numTextures ; t ++)
        tbind[t] = pfGetGSetMultiAttrBind(gset, PFGS_TEXCOORD2, t);


    /* "Convert" nonindexed geoset into indexed */
    if (vindex == NULL)
    {
	if (nprims * vertsPerPrim > 0xffff)
	{
	    pfNotify(PFNFY_WARN, PFNFY_RESOURCE, "pfdHashGSetVerts() "
		     "Cannot hash verts. pfGeoSet has too many vertices (%d)"
		     "to fit into ushort index list.", nprims * vertsPerPrim);
	    return 0;
	}

	vindex = (ushort *)pfMalloc((uint)(sizeof(ushort) * 
					   nprims * vertsPerPrim), arena);
					    
	for (i=0; i<vertsPerPrim*nprims; i++)
	    vindex[i] = (ushort)i;

	if (nbind == PFGS_PER_VERTEX)
	    nindex = vindex;
	else if (nbind == PFGS_PER_PRIM)
	{
	    nindex = (ushort *)pfMalloc((uint)(sizeof(ushort) * nprims), arena);
	    for (i=0; i<nprims; i++)
		nindex[i] = (ushort)i;
	}
	else if (nbind == PFGS_OVERALL)
	{
	    nindex = (ushort *)pfMalloc((uint)sizeof(ushort), arena);
	    nindex[0] = 0;
	}
	
	if (cbind == PFGS_PER_VERTEX)
	    cindex = vindex;
	else if (cbind == PFGS_PER_PRIM)
	{
	    cindex = (ushort *)pfMalloc((uint)(sizeof(ushort) * nprims), arena);
	    for (i=0; i<nprims; i++)
		cindex[i] = (ushort)i;
	}
	else if (cbind == PFGS_OVERALL)
	{
	    cindex = (ushort *)pfMalloc((uint)sizeof(ushort), arena);
	    cindex[0] = 0;
	}

	for (t = 0 ; t < numTextures ; t ++)
	    if (tbind[t] == PFGS_PER_VERTEX)
		tindex[t] = vindex;

	pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, coords, vindex);
        pfGSetAttr(gset, PFGS_NORMAL3, nbind, normals, nindex);
	for (t = 0 ; t < numTextures ; t ++)
	    pfGSetMultiAttr(gset, PFGS_TEXCOORD2, t, tbind[t], 
				  texCoords[t], tindex[t]);
        pfGSetAttr(gset, PFGS_COLOR4, cbind, colors, cindex);
    }
    /* 
     * Return success if all PER_VERTEX attributes already share the
     * same index list.
    */
    else 
    {
	int	got_unique_texture;

	got_unique_texture = 0;
	for (t = 0 ; t < numTextures ; t ++)
	    if ((tbind[t] == PFGS_PER_VERTEX) && (vindex != tindex[t]))
		got_unique_texture = 1;

	if (!((cbind == PFGS_PER_VERTEX && cindex != vindex) ||
	       (got_unique_texture) ||
	       (nbind == PFGS_PER_VERTEX && nindex != vindex)))
	return 1;
    }

    texture_coord_size = 0;

    for (t = 0 ; t < numTextures ; t ++)
	if (tbind[t] == PFGS_PER_VERTEX)
	    texture_coord_size += 2;

    ht = pfuNewHTable(nprims,  (3 + 
			   texture_coord_size + 
			   (nbind == PFGS_PER_VERTEX)*3 + 
			   (cbind == PFGS_PER_VERTEX)*4)
		  * (int)sizeof(float), NULL);

    /* 
     *	Compute offsets for all available texture units.
     *
     *  Offset of texture-unit[numTextures] is the offset of the element
     *  following textures (noff).
     */
    toff[0] = 3;
    for (t = 1 ; t <= numTextures ; t ++)
	toff[t] = toff[t - 1] + (tbind[t - 1] == PFGS_PER_VERTEX) * 2;

    noff = toff[numTextures];
    coff = noff + (nbind == PFGS_PER_VERTEX) * 3;

    elt.data = (void *) vert;

    if (computeNorms && primType == PFGS_TRIS)
	triList = pfNewList(sizeof(void*), 4, NULL);

    /* Enter all vertices into hash table */
    vcount = vicount = 0;
    for (i = 0; i < nprims; i++)
    {
	for (j = 0; j < vertsPerPrim; j++)
	{
	    int	v = vicount + j;

	    PFCOPY_VEC3(vert, coords[vindex[v]]);
	    for (t = 0 ; t < numTextures ; t ++)
		if (tbind[t] == PFGS_PER_VERTEX)
		    PFCOPY_VEC2(vert + toff[t], texCoords[t][tindex[t][v]]);
	    if (nbind == PFGS_PER_VERTEX)
		PFCOPY_VEC3(vert + noff, normals[nindex[v]]);
	    if (cbind == PFGS_PER_VERTEX)
		PFCOPY_VEC4(vert + coff, colors[cindex[v]]);

	    elt.id = vcount;
	    elt.userData = triList;
	    nelt = pfuEnterHash(ht, &elt);

	    if (nelt != NULL)	/* Found vertex match */
	    {
		if (computeNorms && primType == PFGS_TRIS)
		    pfAdd((pfList*) nelt->userData, (void*)i);
		vindex[vicount+j] = (ushort)nelt->id;
	    }
	    else
	    {
		vindex[vicount+j] = (ushort)vcount++;
		if (computeNorms && primType == PFGS_TRIS)
		{
		    pfAdd(triList, (void*)i);
		    triList = pfNewList(sizeof(void*), 4, NULL);
		}
	    }
	}
	vicount += vertsPerPrim;

#ifdef	STATS
	if ((i % 128) == 0)
	{
	    pfNotify(PFNFY_INTERNAL_DEBUG, PFNFY_RESOURCE, "pfuHashGSetVerts: %.2f%% hash", 
		(float)i / (float)nprims);
	}
#endif
    }

    n = ht->listCount;

    if (n >= (1<<16))
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, "pfuHashGSetVerts: Too many vertices.");

    newCoords = (pfVec3 *) pfMalloc((uint)(sizeof(pfVec3) * n), 
				    arena);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, newCoords, vindex);
    pfDelete(coords);

    for (t = 0 ; t < numTextures ; t ++)
    {
	if (tbind[t] == PFGS_PER_VERTEX)
	{
	    newTexCoords[t] = (pfVec2 *) pfMalloc((uint)(sizeof(pfVec2) * n),
					       arena);
	    pfGSetMultiAttr(gset, PFGS_TEXCOORD2, t, PFGS_PER_VERTEX,
		       newTexCoords[t], vindex);
	    pfDelete(texCoords[t]);
	    if (tindex[t] != vindex)
		pfDelete(tindex[t]);
	}
    }
    if (nbind == PFGS_PER_VERTEX || (computeNorms && primType == PFGS_TRIS))
    {
	newNormals = (pfVec3 *) pfMalloc((uint)(sizeof(pfVec3) * n), 
					 arena);
	pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX,
		   newNormals, vindex);
	if (normals)
	    pfDelete(normals);
	if (nindex && nindex != vindex)
	    pfDelete(nindex);
    }
    if (cbind == PFGS_PER_VERTEX)
    {
	newColors = (pfVec4 *) pfMalloc((uint)(sizeof(pfVec4) * n), 
					arena);
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX,
		   newColors, vindex);
	pfDelete(colors);
	if (cindex != vindex)
	    pfDelete(cindex);
    }
    /* Fill in new per-vertex attribute arrays shared attribute values */
    for (i = 0; i < n; i++)
    {
	float           *f =(float *) ht->list[i]->data;
	int		id = ht->list[i]->id;

	pfCopyVec3(newCoords[id], f);

	for (t = 0 ; t < numTextures ; t ++)
	    if (tbind[t] == PFGS_PER_VERTEX)
		pfCopyVec2(newTexCoords[t][id], f + toff[t]);

	if (nbind == PFGS_PER_VERTEX)
	    pfCopyVec3(newNormals[id], f + noff);

	if (cbind == PFGS_PER_VERTEX)
	    pfCopyVec4(newColors[id], f + coff);
    }

    if (computeNorms && primType == PFGS_TRIS)
    {
	int	extraCount = 0, finalCount;
	int	*extraCoords;
	pfVec3	*extraNormals, *finalNormals, *finalCoords;

	triNorms = (pfVec3*) alloca((int)sizeof(pfVec3) * nprims);
	extraCoords = (int*) alloca((int)sizeof(int) * vertsPerPrim);
	extraNormals = (pfVec3*) alloca((int)sizeof(pfVec3) * vertsPerPrim);

	/* Compute triangle face normals */
	for (i=0; i<nprims; i++)
	{
	    pfVec3	v0, v1;

	    PFSUB_VEC3(v0, newCoords[vindex[3*i+1]], newCoords[vindex[3*i+0]]);
	    PFSUB_VEC3(v1, newCoords[vindex[3*i+2]], newCoords[vindex[3*i+0]]);
	    pfCrossVec3(triNorms[i], v0, v1);
	    pfNormalizeVec3(triNorms[i]);
	}

	/* For each unique coordinate */
	for (i=0; i<ht->listCount; i++)
	{
	    pfList	*tlist;
	    int		j, nt, nnorms=1, split, vertId, newVertId;
	    float	s;
	    pfVec3	nsum;

	    /* Grab list of triangles sharing this vertex */
	    tlist = (pfList*) ht->list[i]->userData;
	    nt = pfGetNum(tlist);

	    vertId = ht->list[i]->id;
	    split = 0;

	    /* For each triangle sharing this vertex */
	    for (j=0; j<nt; j++)
	    {
		int	t, k;

		/* Grab first tri on list */
		t = (int)pfGet(tlist, j);

		/* Remove tri from vertex list */
		pfRemoveIndex(tlist, j);
		j--;
		nt--;

		if (split)
		{
		    int		tt=t*3;

		    /* Must create new vertex */
		    extraCoords[extraCount] = vertId;
		    newVertId = vcount + extraCount++;

		    if (vindex[tt] == vertId)
			vindex[tt] = newVertId;
		    else if (vindex[tt+1] == vertId)
			vindex[tt+1] = newVertId;
		    else if (vindex[tt+2] == vertId)
			vindex[tt+2] = newVertId;
		}

		PFCOPY_VEC3(nsum, triNorms[t]);

		/* For every other triangle sharing this vertex */
		for (k=0; k<nt; k++)
		{
		    int	othert;

		    /* Get other triangle */
		    othert = (int)pfGet(tlist, k);
		    
		    /* Don't accumulate normal if too different */
		    if (PFDOT_VEC3(triNorms[t], triNorms[othert]) > CreaseTol)
		    {
			PFADD_VEC3(nsum, nsum, triNorms[t]);
			nnorms++;

			/* Remove combined tri from list */
			pfRemoveIndex(tlist, k);
			k--;
			nt--;

			if (split)
			{
			    int		tt=othert*3;

			    if (vindex[tt] == vertId)
				vindex[tt] = newVertId;
			    else if (vindex[tt+1] == vertId)
				vindex[tt+1] = newVertId;
			    else if (vindex[tt+2] == vertId)
				vindex[tt+2] = newVertId;
			}
		    }
		}
		s = 1.0f / (float)nnorms;
		
		if (split++)
		{
		    pfScaleVec3(extraNormals[newVertId - vcount], s, nsum);
		    pfNormalizeVec3(extraNormals[newVertId - vcount]);
		}
		else
		{
		    pfScaleVec3(newNormals[vertId], s, nsum);
		    pfNormalizeVec3(newNormals[vertId]);
		}
	    }
	    
	    pfDelete(tlist);
 	}
	
	finalCount = vcount + extraCount;
	finalNormals = (pfVec3 *) pfMalloc((uint)
			   (sizeof(pfVec3) * finalCount), arena);
	finalCoords = (pfVec3 *) pfMalloc((uint)
			   (sizeof(pfVec3) * finalCount), arena);

	for (i=0; i<vcount; i++)
	{
	    PFCOPY_VEC3(finalCoords[i], newCoords[i]);
	    PFCOPY_VEC3(finalNormals[i], newNormals[i]);
	}

	for (j=0, i=vcount; i<finalCount; i++, j++)
	{
	    PFCOPY_VEC3(finalCoords[i], newCoords[extraCoords[j]]);
	    PFCOPY_VEC3(finalNormals[i], extraNormals[j]);
	}

	pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX,
		   finalNormals, vindex);

	pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX,
		   finalCoords, vindex);

	pfDelete(triList);
	pfDelete(newNormals);
	pfDelete(newCoords);;
    }

    pfuDelHTable(ht);

    return 1;
}

PFUDLLEXPORT int
pfuHashGSetVerts(pfGeoSet *gset)
{
    return hashGSetVerts(gset, 0);
}

PFUDLLEXPORT int
pfuComputeGSetNorms(pfGeoSet *gset)
{
    return hashGSetVerts(gset, 1);
}
