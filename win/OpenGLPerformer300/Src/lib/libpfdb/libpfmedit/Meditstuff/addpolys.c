/*--------------------------Message start-------------------------------

This software is copyright (C) 1994-1995 by Medit Productions


This software is provided under the following terms, if you do not
agree to all of these terms, delete this software now. Any use of
this software indicates the acceptance of these terms by the person
making said usage ("The User").

1) Medit Productions provides this software "as is" and without
warranty of any kind. All consequences of the use of this software
fall completely on The User.

2) This software may be copied without restriction, provided that an
unaltered and clearly visible copy of this message is placed in any
copies of, or derivatives of, this software.


---------------------------Message end--------------------------------*/




/************************************************************************
The Lod converter creates these two lists for us to work on...
************************************************************************/
static PolyListPtr NormalPolys, PolysWithSubfaces;

/************************************************************************
This routine breaks up lists of polygons by material/texture attribute
and converts them into gsets
************************************************************************/
static void AddPolygons(PolyListPtr list, pfGeode *geode)
{
    PolyListPtr work;

    reg MPolygonPtr p;
    reg MTexturePtr t;
    reg PolyListPtr pl;
    reg MMaterialPtr mat;
    reg int pflags, done, texturing = DoTextures;

    done = (list IS NULL);
    until (done) {
	pl = list;		/* See if we can find an unprocessed polygon */
	while (pl) {
	    p = pl->Polygon;
	    if (!p->Flag) {
		p->Flag = TRUE;		/* yup */
		if (texturing) {
		    t = p->Texture;
		    mat = p->TextureMaterial;
		}	
		else {
		    mat = p->NormalMaterial;
		}
		pflags = p->Flags;

		work = NULL;		/* make a list of similar polygons */
		AddToPolyList(&work,p);
		while (pl = pl->Next) {
		    p = pl->Polygon;
		    if (!p->Flag) {
			if (texturing) {
			    if ((p->Texture IS t)
			    AND (p->TextureMaterial IS mat)
			    AND (p->Flags IS pflags)) {
				p->Flag = TRUE;
				AddToPolyList(&work,p);
			    }
			}
			else {
			    if ((p->NormalMaterial IS mat)
			    AND (p->Flags IS pflags)) {
				p->Flag = TRUE;
				AddToPolyList(&work,p);
			    }
			}
		    }
		}
		MeshPolygons(geode,work);
		FreePolyList(work);
	    }
	    else if (!(pl = pl->Next)) {
		done = TRUE;
	    }
	}
    }
}
/************************************************************************
Spatially partition large lists of polygons
Strategy: Generates an octree of polygons, with a fixed limit of polygons
	  in each octant.
	  nb. We use the first vertex of each polygon for the calculation.
	  This is not strictly true, but we think it is good enough (and
	  certainly the fastest).
************************************************************************/
#define MaxPolygonsInGeoset 5000	    /* this is the maximum */
static void PartitionAndAdd(pfGroup *parent, reg const PolyListPtr list)
{
    pfGroup *newp;
    pfGeode *geode;
    reg MPolygonPtr p;
    reg PolyListPtr pl;
    reg MPolygonBeadPtr pb;
    PolyListPtr	octStore[8];
    reg PolyListPtr next, *oct;
    reg int i, count, o, OnlyDoQuad, PolysInQuad;
    reg flag partitioned, inx, iny, inz, texturing;
    reg float left, right, top, bottom, front, back, max, xc, yc, zc;

    oct = octStore;

    if (pl = list) {
	count = 0;
	texturing = DoTextures;
	while (pl) {
	    p = pl->Polygon;
	    p->Flag = 0;    /* Needed above, might as well do it here... */
	    count++;
	    pl = pl->Next;
	}
	partitioned = FALSE;
	if (count > MaxPolygonsInGeoset) {
	    pl = list;
	    left = bottom = front = 1.0e38;
	    right = top = back = -left;
	    while (pl) {
		pb = pl->Polygon->FirstBead;
		xc = pb->Coordinate[X];
		yc = pb->Coordinate[Y];
		zc = pb->Coordinate[Z];
		if (xc < left) left = xc;	if (xc > right) right = xc;
		if (yc < front) front = yc;	if (yc > back) back = yc;
		if (zc < bottom) bottom = zc;   if (zc > top) top = zc;
		pl = pl->Next;
	    }
	    /* Decide which axes are worth partitioning */
	    max = xc = right-left;
	    if ((yc = back-front) > max) max = yc;
	    if ((zc = top-bottom) > max) max = zc;
	    if (max ISNT 0.0) {
		max /= 4.0;
		inx = (xc > max);	    /* Partition in these axes only */
		iny = (yc > max);
		inz = (zc > max);

		xc = left + (xc/2.0);	    /* The centre of the octree */
		yc = front + (yc/2.0);
		zc = bottom + (zc/2.0);
		for (i=0; i<8; i++) {	    /* Initialise octree */
		    oct[i] = NULL;
		}
		pl = list;
		while (pl) {
		    o = 0;
		    next = pl->Next;
		    pb = pl->Polygon->FirstBead;
		    if (inx AND (pb->Coordinate[X] > xc)) {
			o += 1;
		    }
		    if (iny AND (pb->Coordinate[Y] > yc)) {
			o += 2;
		    }
		    if (inz AND (pb->Coordinate[Z] > zc)) {
			o += 4;
		    }
		    pl->Next = oct[o];
		    oct[o] = pl;
		    pl = next;
		}
		newp = pfNewGroup();
		pfAddChild(parent, newp);
		for (i=0; i<8; i++) {
		    PartitionAndAdd(newp, oct[i]);	/* Partition recursively */
		}
		partitioned = TRUE;
	    }
	}
	if (!partitioned) {
	    geode = pfNewGeode();
	    pfAddChild(parent, geode);
	    AddPolygons(list, geode);
	    FreePolyList(list);
	}
    }
}
/************************************************************************
Break up a list of polygons by attributes and add it to a geode
Strategy: This routine is passed a list of polygons, we need to break
	  them into smaller lists of polygons with similar lighting
	  and texturing, then pass them along to the partitioner.
	  nb. This routine may only add *ONE* node to the parent
************************************************************************/
static void BreakUpAndAdd(reg pfGroup *parent, reg PolyListPtr pl)
{
    reg int types;
    reg pfGroup *newp;
    reg MPolygonPtr p;
    reg PolyListPtr next;
    reg flag texturing = LoadingTextures;
    reg PolyListPtr FlatPolys = NULL, FlatTexturedPolys = NULL;
    reg PolyListPtr UnlitPolys = NULL, UnlitTexturedPolys = NULL;
    reg PolyListPtr SmoothPolys = NULL, SmoothTexturedPolys = NULL;

    /* Generate sublists of polygons, according to type */
    while (pl) {
	next = pl->Next;
	p = pl->Polygon;
	if (!p->Texture OR !texturing) {
	    switch (p->Lighting) {
		case Unlit:	pl->Next = UnlitPolys;
				UnlitPolys = pl;
				break;
		case Lit:	pl->Next = FlatPolys;
				FlatPolys = pl;
				break;
		case Smooth:	pl->Next = SmoothPolys;
				SmoothPolys = pl;
				break;
	    }
	}
	else {
	    switch (p->Lighting) {
		case Unlit:	pl->Next = UnlitTexturedPolys;
				UnlitTexturedPolys = pl;
				break;
		case Lit:	pl->Next = FlatTexturedPolys;
				FlatTexturedPolys = pl;
				break;
		case Smooth:	pl->Next = SmoothTexturedPolys;
				SmoothTexturedPolys = pl;
				break;
	    }
	}
	pl = next;
    }
    types = 0;
    if (UnlitPolys) types++;	if (UnlitTexturedPolys) types++;
    if (FlatPolys) types++;	if (FlatTexturedPolys) types++;
    if (SmoothPolys) types++;	if (SmoothTexturedPolys) types++;

    if (types > 0) {
	if (types > 1) {
	    pfGroup *newp = pfNewGroup();
	    pfAddChild(parent, newp);
	    parent = newp;
	}
	DoTextures = FALSE;
	ShadingType = Unlit;	PartitionAndAdd(parent, UnlitPolys);
	ShadingType = Lit;	PartitionAndAdd(parent, FlatPolys);
	ShadingType = Smooth;	PartitionAndAdd(parent, SmoothPolys);
    
	DoTextures = TRUE;
	ShadingType = Unlit;	PartitionAndAdd(parent, UnlitTexturedPolys);
	ShadingType = Lit;	PartitionAndAdd(parent, FlatTexturedPolys);
	ShadingType = Smooth;	PartitionAndAdd(parent, SmoothTexturedPolys);
    }
    else {
	fprintf(stderr, "-----empty object\n");
    }
}
/************************************************************************
Add a list of polygons to the Performer data structure. We need to create
layers as necessary.
nb. This routine may only add *ONE* node to "parent"
************************************************************************/
typedef struct {
int		MaxDepth;
int		CurrentDepth;
PolyListPtr	*List;
} SubPolyVars;
typedef SubPolyVars *SubPolyVarPtr;
static void FindDeepestLayer(reg MPolygonPtr p, reg SubPolyVarPtr v)
{
    while (p) {
	if (p->Child) {
	    v->CurrentDepth++;
	    if (v->CurrentDepth > v->MaxDepth) {
		v->MaxDepth = v->CurrentDepth;
	    }
	    FindDeepestLayer(p->Child,v);
	    v->CurrentDepth--;
	}
	p = p->Next;
    }
}
static void FindLayerPolygons(reg MPolygonPtr p, reg SubPolyVarPtr v)
{
    reg PolyListPtr *l = v->List+v->CurrentDepth;
    while (p) {
	if (p->Child) {
	    v->CurrentDepth++;
	    FindLayerPolygons(p->Child,v);
	    v->CurrentDepth--;
	}
	AddToPolyList(l,p);
	p = p->Next;
    }
}
static void AddPolyList(pfGroup *parent)
{
    pfLayer *layer;
    SubPolyVars vars;
    pfGroup *layerparent;

    reg PolyListPtr pl;
    reg int i, maxdepth;
    reg SubPolyVarPtr v = &vars;

    /* If we have both types of polygon, generate an extra group to hold them */
    if (NormalPolys AND PolysWithSubfaces) {
	pfGroup *g = pfNewGroup();
	pfAddChild(parent, g);
	parent = g;
    }

    /* Add polygons without subfaces... */
    BreakUpAndAdd(parent, NormalPolys);

    /* Then polygons with subfaces (one at a time...) */
    if (pl = PolysWithSubfaces) {
	v->CurrentDepth = v->MaxDepth = 1;		/* Work out depth of layering needed	*/
	while (pl) {
	    FindDeepestLayer(pl->Polygon->Child,v);
	    pl = pl->Next;
	}
	maxdepth = v->MaxDepth;
	v->List = malloc((maxdepth+1) * sizeof(PolyListPtr*));
	for (i=0; i<=maxdepth; i++) {
	    v->List[i] = NULL;
	}
	pl = PolysWithSubfaces;
	while (pl) {
	    AddToPolyList(v->List,pl->Polygon);		/* Top level polygon	*/
	    v->CurrentDepth = 1;
	    FindLayerPolygons(pl->Polygon->Child,v);	/* Subpolygons		*/

	    layer = pfNewLayer();
	    pfAddChild(parent,layer);
	    for (i=0; i<=maxdepth; i++) {		/* Add all the layers	*/
		BreakUpAndAdd((pfGroup*)layer, v->List[i]);
		v->List[i] = NULL;
	    }
	    pl = pl->Next;
	}
	free(v->List);
    }
}
