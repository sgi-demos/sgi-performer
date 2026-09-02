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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <math.h>
#include <malloc.h>
#include <assert.h>

#include <Performer/pf.h>
#include <Performer/pfdu.h>

typedef struct
{
    int 		nofTriangles;
    int 		nofAllocatedTriangles;
    float       	*v;
    float       	*t;
    float       	*c;
    float       	*n;
} pfdTriangleList;

static void pfdInitTriangleList(pfdTriangleList *tri);
static void pfdZeroTriangleList(pfdTriangleList *tri);
static void pfdAddTriangle
		(pfdTriangleList *tri, float *v, float *t, float *c, float *n);
static void	DFSExtractGraphTriangles (pfNode *node, pfdTriangleList *tri);

/*===========================================================================*/
static void pfdInitTriangleList(pfdTriangleList *tri)
/*===========================================================================*/
{
    tri -> nofAllocatedTriangles = 0;
    tri -> nofTriangles = 0;
    tri -> v = NULL;
    tri -> t = NULL;
    tri -> c = NULL;
    tri -> n = NULL;
}

/*===========================================================================*/
static void pfdZeroTriangleList(pfdTriangleList *tri)
/*===========================================================================*/
{
    tri -> nofTriangles = 0;
}

/*===========================================================================*/
static void pfdAddTriangle
		(pfdTriangleList *tri, float *v, float *t, float *c, float *n)
/*===========================================================================*/
{
    int		new;
    int		i;

    if (tri -> nofTriangles >= tri -> nofAllocatedTriangles)
    {
        new = (1 + tri -> nofAllocatedTriangles) * 5 / 3;
        if (! tri -> v)
	{
            tri -> v = (float *) pfCalloc (new, 3 * 3 * sizeof (float),
                                            pfGetSharedArena());
            tri -> t = (float *) pfCalloc (new, 3 * 2 * sizeof (float),
                                            pfGetSharedArena());
            tri -> c = (float *) pfCalloc (new, 3 * 4 * sizeof (float),
                                            pfGetSharedArena());
            tri -> n = (float *) pfCalloc (new, 3 * 3 * sizeof (float),
                                            pfGetSharedArena());
	}
        else
        {
            tri -> v = (float *) pfRealloc (tri -> v,
                                        new * 3 * 3 * sizeof (float));
            tri -> t = (float *) pfRealloc (tri -> t,
                                        new * 3 * 2 * sizeof (float));
            tri -> c = (float *) pfRealloc (tri -> c,
                                        new * 3 * 4 * sizeof (float));
            tri -> n = (float *) pfRealloc (tri -> n,
                                        new * 3 * 3 * sizeof (float));
        }

    tri -> nofAllocatedTriangles = new;
    }

    for (i = 0 ; i < 3 ; i ++)
    {
	if (v)
	{
	    tri -> v[tri -> nofTriangles * 9 + i * 3    ] = v[i * 3    ];
	    tri -> v[tri -> nofTriangles * 9 + i * 3 + 1] = v[i * 3 + 1];
	    tri -> v[tri -> nofTriangles * 9 + i * 3 + 2] = v[i * 3 + 2];
	}
	else
	{
	    tri -> v[tri -> nofTriangles * 9 + i * 3    ] = 0.0;
	    tri -> v[tri -> nofTriangles * 9 + i * 3 + 1] = 0.0;
	    tri -> v[tri -> nofTriangles * 9 + i * 3 + 2] = 0.0;
	}

	if (t)
	{
	    tri -> t[tri -> nofTriangles * 6 + i * 2    ] = t[i * 2    ];
	    tri -> t[tri -> nofTriangles * 6 + i * 2 + 1] = t[i * 2 + 1];
	}
	else
	{
	    tri -> t[tri -> nofTriangles * 6 + i * 2    ] = 0.0;
	    tri -> t[tri -> nofTriangles * 6 + i * 2 + 1] = 0.0;
	}

	if (c)
	{
	    tri -> c[tri -> nofTriangles * 12 + i * 4    ] = c[i * 4    ];
	    tri -> c[tri -> nofTriangles * 12 + i * 4 + 1] = c[i * 4 + 1];
	    tri -> c[tri -> nofTriangles * 12 + i * 4 + 2] = c[i * 4 + 2];
	    tri -> c[tri -> nofTriangles * 12 + i * 4 + 3] = c[i * 4 + 3];
	}
	else
	{
	    tri -> c[tri -> nofTriangles * 12 + i * 4    ] = 0.0;
	    tri -> c[tri -> nofTriangles * 12 + i * 4 + 1] = 0.0;
	    tri -> c[tri -> nofTriangles * 12 + i * 4 + 2] = 0.0;
	    tri -> c[tri -> nofTriangles * 12 + i * 4 + 3] = 0.0;
	}

	if (n)
	{
	    tri -> n[tri -> nofTriangles * 9 + i * 3    ] = n[i * 4    ];
	    tri -> n[tri -> nofTriangles * 9 + i * 3 + 1] = n[i * 4 + 1];
	    tri -> n[tri -> nofTriangles * 9 + i * 3 + 2] = n[i * 4 + 2];
	}
	else
	{
	    tri -> n[tri -> nofTriangles * 9 + i * 3    ] = 0.0;
	    tri -> n[tri -> nofTriangles * 9 + i * 3 + 1] = 0.0;
	    tri -> n[tri -> nofTriangles * 9 + i * 3 + 2] = 0.0;
	}
    }

    tri -> nofTriangles ++;
}

/*===========================================================================*/
static void	DFSExtractGraphTriangles (pfNode *node, pfdTriangleList *tri)
/*===========================================================================*/
{
    int				i, j, k;
    pfGeoSet			*gset;
    float                       *v, *t, *c, *n;
    ushort                      *foo;
    float			vv[9], tt[6], cc[12], nn[9];
    int				nofPrims;
    int				primType;
    int				v_base, t_base, c_base, n_base, base;
    int				*primLength;

    if (pfIsExactType (node,pfGetGeodeClassType()))
    {
	/*
 	 *	==========================================
	 *	Geode ? 
	 *	    Process its GeoSets.
 	 *	==========================================
	 */

	for (i = 0 ; i < pfGetNumGSets ((pfGeode *) node) ; i ++)
	{
	    gset = pfGetGSet ((pfGeode *) node, i);
            primType = pfGetGSetPrimType (gset);
	    nofPrims = pfGetGSetNumPrims (gset);
	    pfGetGSetAttrLists (gset, PFGS_COORD3, (void **) &v, &foo);
	    pfGetGSetAttrLists (gset, PFGS_TEXCOORD2, (void **) &t, &foo);
	    pfGetGSetAttrLists (gset, PFGS_COLOR4, (void **) &c, &foo);
	    pfGetGSetAttrLists (gset, PFGS_NORMAL3, (void **) &n, &foo);

	    switch (primType)
		{
		case PFGS_TRIS:

		    for (j = 0 ; j < nofPrims ; j ++)
			pfdAddTriangle (tri,v ? & v[9  * j] : NULL, 
					t ? & t[6  * j] : NULL,
					c ? & c[12 * j] : NULL,
					n ? & n[9  * j] : NULL);

		    break;

		case PFGS_QUADS:

		    for (j = 0 ; j < nofPrims ; j ++)
		    {
			/*
			 * 	Make two triangles out of each quad.
			 */

			pfdAddTriangle (tri,v ? & v[4 * 3 * j] : NULL, 
					    t ? & t[4 * 2 * j] : NULL,
					    c ? & c[4 * 4 * j] : NULL,
					    n ? & n[4 * 3 * j] : NULL);

			if (v)
			{
			    for (k = 0 ; k < 3 ; k ++)
				vv[k] = v[4 * 3 * j + k];
			    for (k = 0 ; k < 6 ; k ++)
				vv[3 + k] = v[4 * 3 * j + 6 + k];
			}

			if (t)
			{
			    for (k = 0 ; k < 2 ; k ++)
				tt[k] = t[4 * 2 * j + k];
			    for (k = 0 ; k < 4 ; k ++)
				tt[2 + k] = t[4 * 2 * j + 4 + k];
			}

			if (c)
			{
			    for (k = 0 ; k < 4 ; k ++)
				cc[k] = c[4 * 4 * j + k];
			    for (k = 0 ; k < 8 ; k ++)
				nn[4 + k] = n[4 * 4 * j + 8 + k];
			}

			if (n)
			{
			    for (k = 0 ; k < 3 ; k ++)
				nn[k] = n[4 * 3 * j + k];
			    for (k = 0 ; k < 6 ; k ++)
				nn[3 + k] = n[4 * 3 * j + 6 + k];
			}

			pfdAddTriangle (tri, v ? vv : NULL, 
					     t ? tt : NULL,
					     c ? cc : NULL,
					     n ? nn : NULL);
		    }
		    break;

		case PFGS_TRISTRIPS:

		    primLength = pfGetGSetPrimLengths(gset);

		    base = 0;
		    for (j = 0 ; j < nofPrims ; j ++)
		    {
			v_base = base * 3;
			t_base = base * 2;
			c_base = base * 4;
			n_base = base * 3;

			for (k = 0 ; k < primLength[j] - 2 ; k ++)
			{
			    pfdAddTriangle (
					tri,
					v ? & v[v_base + 3 * k] : NULL, 
					t ? & t[t_base + 2 * k] : NULL,
					c ? & c[c_base + 4 * k] : NULL,
					n ? & n[n_base + 3 * k] : NULL);
			}
		
			base += primLength[j];
		    }

		    break;

		default:
		    pfNotify(PFNFY_INFO, PFNFY_PRINT,
			"pfdExtractGraphTriangles: GSetPrimType = "
			"%d not supported.\n", primType);
		}
	}
    }
    else
    if (pfIsOfType (node,pfGetGroupClassType()))
    {
	/*
	 *	==========================================
	 *	Not Geode ?
	 *	    Process its Children.
	 *	==========================================
	 */

	for (i = 0 ; i < pfGetNumChildren ((pfGroup *) node) ; i ++)
	    DFSExtractGraphTriangles (pfGetChild ((pfGroup *) node, i), tri);
    }
}

/*===========================================================================*/
PFDUDLLEXPORT void	pfdExtractGraphTriangles 
			(pfNode *node, pfGeoSet *gset, unsigned long flags)
/*===========================================================================*/
{
    static pfdTriangleList		tri;
    static int				virgin = 1;
    float				*v, *t, *c, *n;
    int					i;
    int					nofTriangles;

    if (virgin)
    {
	virgin = 0;
	pfdInitTriangleList(&tri);
    }

    pfdZeroTriangleList(&tri);

    DFSExtractGraphTriangles (node, &tri);

    /*
     * 	Make Geoset attributes from the results..
     */
    nofTriangles = tri . nofTriangles;

    if (flags & PR_QUERY_TRI_COORD)
	v = (float *) pfMalloc (3 * nofTriangles * 3 * sizeof (float), 
					pfGetSharedArena());
    
    if (flags & PR_QUERY_TRI_TEXTURE)
	t = (float *) pfMalloc (3 * nofTriangles * 2 * sizeof (float), 
					pfGetSharedArena());

    if (flags & PR_QUERY_TRI_COLOR)
	c = (float *) pfMalloc (3 * nofTriangles * 4 * sizeof (float), 
					pfGetSharedArena());

    if (flags & PR_QUERY_TRI_NORMAL)
	n = (float *) pfMalloc (3 * nofTriangles * 3 * sizeof (float), 
					pfGetSharedArena());

    for (i = 0 ; i < nofTriangles * 3 ; i ++)
    {
	if (flags & PR_QUERY_TRI_COORD)
	{
	    v[i*3 + 0] = tri . v[i*3 + 0];
	    v[i*3 + 1] = tri . v[i*3 + 1];
	    v[i*3 + 2] = tri . v[i*3 + 2];
	}

	if (flags & PR_QUERY_TRI_TEXTURE)
	{
	    t[i*2 + 0] = tri . t[i*2 + 0];
	    t[i*2 + 1] = tri . t[i*2 + 1];
	}

	if (flags & PR_QUERY_TRI_COLOR)
	{
	    c[i*4 + 0] = tri . c[i*4 + 0];
	    c[i*4 + 1] = tri . c[i*4 + 1];
	    c[i*4 + 2] = tri . c[i*4 + 2];
	    c[i*4 + 3] = tri . c[i*4 + 3];
	}

	if (flags & PR_QUERY_TRI_NORMAL)
	{
	    n[i*3 + 0] = tri . n[i*3 + 0];
	    n[i*3 + 1] = tri . n[i*3 + 1];
	    n[i*3 + 2] = tri . n[i*3 + 2];
	}
    }

    if (flags & PR_QUERY_TRI_COORD)
	pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, (void *) v, NULL);
    else
	pfGSetAttr(gset, PFGS_COORD3, PFGS_OFF, NULL, NULL);

    if (flags & PR_QUERY_TRI_TEXTURE)
	pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, (void *) t, NULL);
    else
	pfGSetAttr(gset, PFGS_TEXCOORD2, PFGS_OFF, NULL, NULL);

    if (flags & PR_QUERY_TRI_COLOR)
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_PER_VERTEX, (void *) c, NULL);
    else
	pfGSetAttr(gset, PFGS_COLOR4, PFGS_OFF, NULL, NULL);

    if (flags & PR_QUERY_TRI_NORMAL)
	pfGSetAttr(gset, PFGS_NORMAL3, PFGS_PER_VERTEX, (void *) n, NULL);
    else
	pfGSetAttr(gset, PFGS_NORMAL3, PFGS_OFF, NULL, NULL);

    pfGSetNumPrims(gset, nofTriangles);

    pfGSetPrimType (gset, PFGS_TRIS);
}

