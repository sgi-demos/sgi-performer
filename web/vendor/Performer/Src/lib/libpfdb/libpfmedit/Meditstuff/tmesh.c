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
Convert polygon lists into tmeshes/geosets
Strategy: Break all polygons into triangles. Work out which polygons
	  are connected to which others. Then find chains of polygons,
	  and add them as tmesh gsets
************************************************************************/



/************************************************************************
Global vars
************************************************************************/
static int BiggestPoly = 0;	/* Size of the list below */
static MPolygonBeadPtr *BeadList;

static int VListSize = 0;	/* Size of the two lists below */
static VListPtr VListStore, *VListIndices;

static int ChainSize = 0;	/* Size of the list below */
static TrianglePtr *ChainList;

static int inc1dec2[] = { 1, 2, 0 };	/* inc by 1 == dec by 2...*/
static int inc2dec1[] = { 2, 0, 1 };	/* inc by 2 == dec by 1...*/

/************************************************************************
Copy flat polygon normals into polygon beads for later use
************************************************************************/
static void SpreadVertexNormals(reg PolyListPtr base)
{
    reg float x,y,z;
    reg MPolygonPtr p;
    reg MPolygonBeadPtr pb;
    if (ShadingType IS Lit) {
	while (base) {
	    p = base->Polygon;
	    pb = p->FirstBead;
	    x = p->Normal[X];
	    y = p->Normal[Y];
	    z = p->Normal[Z];
	    while (pb) {
		pb->Normal[X] = x;
		pb->Normal[Y] = y;
		pb->Normal[Z] = z;
		pb = pb->Next;
	    }
	    base = base->Next;
	}
    }
}
/************************************************************************
Turn a list of polygons into triangles
************************************************************************/
#define CopyVertexInfo					\
t->LightGroup = p->LightGroup;				\
CopyXYZ(t->Coordinate[0],bl[head]->Coordinate);		\
CopyXYZ(t->Normal[0],	 bl[head]->Normal);		\
CopyXY( t->TCoord[0],	 bl[head]->TextureCoord);	\
\
CopyXYZ(t->Coordinate[1],bl[centre]->Coordinate);	\
CopyXYZ(t->Normal[1],	 bl[centre]->Normal);		\
CopyXY( t->TCoord[1],	 bl[centre]->TextureCoord);	\
\
CopyXYZ(t->Coordinate[2],bl[tail]->Coordinate);		\
CopyXYZ(t->Normal[2],	 bl[tail]->Normal);		\
CopyXY( t->TCoord[2],	 bl[tail]->TextureCoord);

static int ShatterPolygonList(PolyListPtr base, reg TrianglePtr *trilist)
{
    reg int bigp,head,tail,centre,tlistsize;
    reg MPolygonPtr p;
    reg PolyListPtr pl;
    reg TrianglePtr t,previous;
    reg MPolygonBeadPtr pb, *bl;

    bl = BeadList;		/* constants */
    bigp = BiggestPoly;

    tlistsize = 0;
    *trilist = NULL;

    pl = base;		/* Go through all the polygons and make triangles */
    while (pl) {
	p = pl->Polygon;

	tail = 0;	/* Make a list of the vertices in this polygon */
	pb = p->FirstBead;
	while (pb) {
	    if (tail IS bigp) {	/* Enough room to store it? */
		if (bigp) {	/* Nope, allocate some more...*/
		    BeadList = realloc(BeadList,
			(bigp+100)*sizeof(MPolygonBeadPtr));
		}
		else {
		    BeadList = malloc(100*sizeof(MPolygonBeadPtr));
		}
		bigp = BiggestPoly = bigp + 100;
		bl = BeadList;
	    }
	    bl[tail++] = pb;
	    pb = pb->Next;
	}

	head = 0;			/* Break the polygon into triangles */
	tail--;
	previous = NULL;
	until (head IS tail) {
	    centre = head+1;
	    if (centre IS tail) {		/* Hit the end, stop */
		break;
	    }
	    t = AddTriangle(trilist);
	    CopyVertexInfo;
	    if (previous) {
		t->Index[2] = 0;
		t->Neighbour[2] = previous;
		previous->Index[0] = 2;
		previous->Neighbour[0] = t;
	    }
	    previous = t;
	    head = centre;
	    tlistsize++;

	    centre = tail-1;
	    if (head IS centre) {		/* Hit the end, stop */
		break;
	    }
	    t = AddTriangle(trilist);
	    CopyVertexInfo;
	    t->Index[2] = 1;
	    t->Neighbour[2] = previous;
	    previous->Index[1] = 2;
	    previous->Neighbour[1] = t;
	    previous = t;
	    tail = centre;
	    tlistsize++;
	}
	pl = pl->Next;			/* Loop through all polygons */
    }
    return tlistsize;
}
/************************************************************************
Generate a vertex list, and pointers to it from a triangle list
************************************************************************/
static int MakeVertexList(TrianglePtr TriList, int NoTriangles, VListPtr *List, VListPtr **Result)
{
    reg int i;
    reg TrianglePtr t;
    reg VListPtr index, *indexindex;

    if ((NoTriangles*3) > VListSize) {
	int newsize = (NoTriangles*3) + 1000;
	if (VListSize IS 0) {
	    VListStore	= malloc(newsize*sizeof(VList));
	    VListIndices    = malloc(newsize*sizeof(VListPtr));
	}
	else {
	    VListStore	= realloc(VListStore,  newsize*sizeof(VList));
	    VListIndices	= realloc(VListIndices,newsize*sizeof(VListPtr));
	}
	VListSize = newsize;
    }
    t = TriList;
    index = VListStore;
    indexindex = VListIndices;
    while (t) {
	for (i=0; i<3; i++) {
	    index->Owner = t;
	    index->Index = i;
	    index->Coordinate = t->Coordinate[i];
	    *indexindex++ = index++;
	}
	t = t->Next;
    }
    *Result = VListIndices;
    return NoTriangles*3;
}
/************************************************************************
Find all the neighbours of a triangle...
************************************************************************/
static int VListCompare(const void *v1, const void *v2)
{
    reg float *a,*b;
    reg float aa,bb;

    a = (*(VListPtr*)v1)->Coordinate;
    b = (*(VListPtr*)v2)->Coordinate;

    aa = a[X]; bb = b[X];
    if (aa < bb) return -1;
    if (aa > bb) return  1;

    aa = a[Y]; bb = b[Y];
    if (aa < bb) return -1;
    if (aa > bb) return  1;

    aa = a[Z]; bb = b[Z];
    if (aa < bb) return -1;
    if (aa > bb) return  1;

    return 0;
}
static void ConnectTriangles(VListPtr *vl, int NoVertices)
{
    reg TrianglePtr t, ts;
    reg float *c, tx, ty, tz;
    reg int i, is, is2, index, two = 2;
    reg VListPtr *start, *end, *search;

    start = vl;
    end = start+NoVertices;
    until (start IS end) {
	t = (*start)->Owner;
	index = (*start)->Index;
	if (!t->Neighbour[index]) {
	    i = index+1; if (i > two) i = 0;
	    tx = t->Coordinate[i][X];
	    ty = t->Coordinate[i][Y];
	    tz = t->Coordinate[i][Z];
	    search = start+1;
	    until (search IS end) {
		ts = (*search)->Owner;
		is = (*search)->Index - 1; if (is < 0) is = two;
		if (!ts->Neighbour[is]) {
		    c = ts->Coordinate[is];
		    if ((c[X] IS tx) AND (c[Y] IS ty) AND (c[Z] IS tz)) {
			t->Neighbour[index] = ts;
			ts->Neighbour[is] = t;
			t->Index[index] = is;
			ts->Index[is] = index;
			break;
		    }
		}
		search++;
	    }
	}

	index = (*start)->Index-1; if (index < 0) index = two;
	if (!t->Neighbour[index]) {
	    i = index;
	    tx = t->Coordinate[i][X];
	    ty = t->Coordinate[i][Y];
	    tz = t->Coordinate[i][Z];
	    search = start+1;
	    until (search IS end) {
		ts = (*search)->Owner;
		is = (*search)->Index;
		is2 = is + 1; if (is2 > two) is2 = 0;
		if (!ts->Neighbour[is]) {
		    c = ts->Coordinate[is2];
		    if ((c[X] IS tx) AND (c[Y] IS ty) AND (c[Z] IS tz)) {
			t->Neighbour[i] = ts;
			ts->Neighbour[is] = t;
			t->Index[i] = is;
			ts->Index[is] = i;
			break;
		    }
		}
		search++;
	    }
	}
	start++;
    }
}
static void FindNeighbours(int NoVertices, VListPtr *List)
{
    reg int count;
    reg float *c,x,y,z;
    reg VListPtr *start, *end, *search;

    qsort(List,NoVertices,sizeof(VListPtr),VListCompare);

    start = List; end = start+NoVertices;
    until (start IS end) {
	c = (*start)->Coordinate;
	x = c[X]; y = c[Y]; z = c[Z];
	search = start+1;
	until (search IS end) {
	    c = (*search)->Coordinate;
	    if ((c[X] ISNT x) OR (c[Y] ISNT y) OR (c[Z] ISNT z)) {
		break;
	    }
	    search++;
	}
	count = (int)(search - start); 	/* assume range is not > range(int) */
					/* grantham@asd.sgi.com 9/6/95 */
	if (count ISNT 1) {
	    ConnectTriangles(start,count);
	}
	start = search;
    }
}
/************************************************************************
Disconnect triangle where normals/texture coordinates clash...
************************************************************************/
static void CheckClashes(TrianglePtr start, reg flag CheckTextures)
{
    reg int i,index;
    reg TrianglePtr t,n;
    reg int *inc1 = inc1dec2;
    reg float *norm,nx,ny,nz, *ntex,tx,ty;

    t = start;
    while (t) {
	for (i=0; i<3; i++) {
	    if (n = t->Neighbour[i]) {
		nx = t->Normal[i][X];
		ny = t->Normal[i][Y];
		nz = t->Normal[i][Z];
		index = t->Index[i];
		norm = n->Normal[inc1[index]];
		if ((norm[X] ISNT nx) OR (norm[Y] ISNT ny) OR (norm[Z] ISNT nz)) {
		    t->Neighbour[i] = NULL;
		    n->Neighbour[index] = NULL;
		}
	    }
	    if (CheckTextures AND (n = t->Neighbour[i])) {
		tx = t->TCoord[i][X];
		ty = t->TCoord[i][Y];
		index = t->Index[i];
		ntex = n->TCoord[inc1[index]];
		if ((ntex[X] ISNT tx) OR (ntex[Y] ISNT ty)) {
		    t->Neighbour[i] = NULL;
		    n->Neighbour[index] = NULL;
		}
	    }
	}
	t = t->Next;
    }
}
/************************************************************************
Suss out a tmesh length given a start/direction...
************************************************************************/
#define IdSwap temp = id; id = id1; id1 = temp
typedef void (*FPC)(reg TrianglePtr,reg TrianglePtr*,reg int);
#define SussLength(direction,incdec1,incdec2,flag)		\
    t = start;						\
    index = direction;					\
    id = incdec1; id1 = incdec2;				\
    while (next = t->Neighbour[index]) {			\
	if (next->Done OR (next->meshgroup IS mesh)) {	\
	    break;					\
	}						\
	index = id[t->Index[index]];			\
	IdSwap;						\
	t = next;					\
	t->meshgroup = mesh;				\
	count++;					\
	if (callback) {					\
	    callback(t,base,flag);			\
	}						\
    }

static int ScanMesh(const TrianglePtr start, const int direction,
		    const FPC Callback, reg TrianglePtr *base)
{
    static int meshno = 1;

    reg FPC callback;
    reg int index,count;
    reg TrianglePtr t,next;
    reg int *id,*id1,*id2 = inc2dec1,*temp;
    reg unsigned int mesh = meshno;

    count = 1;
    if (callback = Callback) {		/* Always at least 1 triangle... */
	*base = NULL;
	callback(start,base,0);
    }
    start->meshgroup = mesh;		/* Each triangle can only be used once! */
    if (direction < 3) {
	SussLength(direction,id2,inc1dec2,1);
	SussLength(id2[direction],inc1dec2,id2,0);
    }
    else {
	SussLength(id2[direction-3],inc1dec2,id2,0);
	SussLength(direction-3,id2,inc1dec2,1);
    }
    if (++meshno IS 0) {
	meshno = 1;
    }

    return count;
}
/************************************************************************
Callback to generate lists of triangles for tmeshes
************************************************************************/
static void GenerateChain(reg TrianglePtr new, reg TrianglePtr *base, reg int headtail)
{
    static TrianglePtr last;
    reg TrianglePtr c,t;
    if (!*base) {					/* Initialisation */
	*base = new;
	new->Size = 1;				/* Size of chain */
	new->Chain = NULL;
	last = NULL;
    }
    else {
	if (headtail) {				/* Add to end of list */
	    t = *base;
	    t->Size++;
	    if (last) {
		t = last;
	    }
	    else {
		while (c = t->Chain) {
		    t = c;
		}
	    }
	    t->Chain = last = new;
	    new->Chain = NULL;
	}
	else {					/* Add to start of list */
	    t = *base;
	    new->Size = t->Size + 1;
	    new->Chain = t;
	    *base = new;
	}
    }
    new->Done = TRUE;
}
/************************************************************************
Check for and kill left handed tmeshes
************************************************************************/
#define FindNext(t) ((t->Chain IS t->Neighbour[0])?0: ((t->Chain IS t->Neighbour[1])? 1: 2))
static void CheckForLefty(TrianglePtr *base)
{
    reg int size,link1,link2;
    reg TrianglePtr t,chain;

    t = *base;
    size = t->Size;
    if (size > 2) {				/* Leftys must be at least 3 triangles */
	chain = t->Chain;
	link1 = t->Index[FindNext(t)];
	link2 = FindNext(chain);
	if (link2 IS inc1dec2[link1]) {	/* Its a lefty... */
	    if (size & 1) {			/* Size is odd, reverse the chain */
		*base = NULL;
		while (t) {
		    chain = t->Chain;
		    t->Chain = *base;
		    *base = t;
		    t = chain;
		}
		(*base)->Size = size;
	    }
	    else {				/* Size is even, chop a triangle off */
		*base = t->Chain;
		t->Chain = NULL;
		t->Done = NULL;
		(*base)->Size = size-1;
	    }
	}
    }
}
/************************************************************************
We have all the triangles connected, look for chains...
************************************************************************/
static int FindPolygonChains(int NoTriangles, TrianglePtr trilist)
{
    reg TrianglePtr t, next;
    reg int direction, NoChains, size, biggest, bdirection;

    t = trilist;				/* Initialise everything, allocate workspace */
    while (t) {
	t->Done = FALSE;
	t = t->Next;
    }
    if (!ChainSize) {
	ChainSize	= NoTriangles+1000;
	ChainList	= malloc(ChainSize * sizeof(TrianglePtr));
    }
    else if (NoTriangles >= ChainSize) {
	ChainSize	= NoTriangles+1000;
	ChainList	= realloc(ChainList,  ChainSize * sizeof(TrianglePtr));
    }
    NoChains = 0;

    t = trilist;
    while (t) {
	next = t->Next;
	if (!t->Done) {
	    biggest = 0;
	    for (direction = 0; direction<6; direction++) {
		size = ScanMesh(t,direction,NULL,NULL);
		if (size > biggest) {
		    biggest = size;
		    bdirection = direction;
		}
	    }
	    ScanMesh(t,bdirection,GenerateChain,&ChainList[NoChains]);
	    CheckForLefty(&ChainList[NoChains]);
	    NoChains++;
	    next = trilist;
	}
	t = next;
    }
    return NoChains;
}
/************************************************************************
Turn polygon chains into geosets
************************************************************************/
#define AllocVec2(n)	pfMalloc((n)*sizeof(pfVec2),MeditArena)
#define AllocVec3(n)	pfMalloc((n)*sizeof(pfVec3),MeditArena)
#define AllocVec4(n)	pfMalloc((n)*sizeof(pfVec4),MeditArena)
#define AllocInt(n)	pfMalloc((n)*sizeof(int),   MeditArena)
static flag ShowMeshes = FALSE;	/* Show meshes for debugging */
static pfVec4 *SussColour(int n, MMaterialPtr m, int *attr)
{
    reg int i;
    reg pfVec4 *colour;
    if (ShowMeshes) {
	colour = AllocVec4(n);
	*attr = PFGS_PER_PRIM;
	for (i=0; i<n; i++) {
	    colour[i][0] = (float)rand()/(float)RAND_MAX;
	    colour[i][1] = (float)rand()/(float)RAND_MAX;
	    colour[i][2] = (float)rand()/(float)RAND_MAX;
	    colour[i][3] = 1.0;
	}
    }
    else {
	colour = AllocVec4(1);
	*attr = PFGS_OVERALL;
	colour[0][0] = m->Diffuse[0];
	colour[0][1] = m->Diffuse[1];
	colour[0][2] = m->Diffuse[2];
	colour[0][3] = m->Alpha;
    }
    return colour;
}
static void AddChains(int NoChains, pfGeode *geode, MMaterialPtr mat, MTexturePtr tex, flag EnvMap)
{
    pfGeoSet *geoset;
    reg TrianglePtr t;
    reg int i, j, entry, index, size, shading;
    reg int *i1d2 = inc1dec2, *i2d1 = inc2dec1;
    reg int NoVertices, NoTris, TriNormCount, StripNormCount, NoStrips, StripTris;

    NoVertices = NoTris = NoStrips = StripTris = 0;		/* Count tris/tmeshes */
    for (i=0; i<NoChains; i++) {
	size = ChainList[i]->Size;
	if (size IS 1) {
	    NoTris++;
	}
	else if (size > 1) {
	    NoStrips++;
	    StripTris += size;
	    NoVertices += size+2;
	}
    }

    shading = (ShowMeshes)? Unlit: ShadingType;

    if (NoStrips) {
	reg pfVec4 *Colour;
	reg pfVec2 *StripTex;
	int colourtype, *StripLength = AllocInt(NoStrips);
	reg pfVec3 *StripVert = AllocVec3(NoVertices), *StripNorm;
	if ((shading IS Unlit) OR FastLighting) {
	    Colour = SussColour(NoStrips,mat,&colourtype);
	}
	if (shading IS Lit) {
	    StripNorm = AllocVec3(StripTris);
	}
	if (shading IS Smooth) {
	    StripNorm = AllocVec3(NoVertices);
	}
	if (tex) {
	       StripTex	= AllocVec2(NoVertices);
	}

	NoVertices = NoStrips = StripNormCount = 0;
	for (i=0; i<NoChains; i++) {
	    t = ChainList[i];
	    if (t->Size > 1) {
		StripLength[NoStrips++] = t->Size+2;

		/* First triangle in the mesh*/
		index = i2d1[FindNext(t)];
		if (shading IS Lit) {
		    CopyXYZ(StripNorm[StripNormCount++],t->Normal[0]);
		}
		for (j=0; j<3; j++) {
		    if (shading IS Smooth) {
			CopyXYZ(StripNorm[StripNormCount++],t->Normal[index]);
		    }
		    if (tex) {
			CopyXY(StripTex[NoVertices],t->TCoord[index]);
		    }
		    CopyXYZ(StripVert[NoVertices++],t->Coordinate[index]);
		    index = i1d2[index];
		}

		/* Other triangles in the mesh */
		entry = t->Index[FindNext(t)];
		while (t = t->Chain) {
		    index = i2d1[entry];
		    if (shading ISNT Unlit) {
			CopyXYZ(StripNorm[StripNormCount++],t->Normal[index]);
		    }
		    if (tex) {
			CopyXY(StripTex[NoVertices],t->TCoord[index]);
		    }
		    CopyXYZ(StripVert[NoVertices++],t->Coordinate[index]);
		    entry = t->Index[FindNext(t)];
		}
	    }
	}
	geoset = pfNewGSet(MeditArena);
	pfGSetPrimType(geoset, (shading IS Smooth)? PFGS_TRISTRIPS:PFGS_FLAT_TRISTRIPS);
	pfGSetNumPrims(geoset, NoStrips);
	pfGSetPrimLengths(geoset, StripLength);
	pfGSetAttr(geoset, PFGS_COORD3, PFGS_PER_VERTEX, StripVert, NULL);
	if ((shading IS Unlit) OR FastLighting) {
	    pfGSetAttr(geoset, PFGS_COLOR4, colourtype, Colour, NULL);
	}
	if ((shading IS Lit) OR (shading IS Smooth)) {
	    pfGSetAttr(geoset, PFGS_NORMAL3, PFGS_PER_VERTEX, StripNorm, NULL);
	}
	if (tex) {
	    pfGSetAttr(geoset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, StripTex, NULL);
	}
	pfGSetGState(geoset, MakeGeostate(FALSE,mat,tex,EnvMap));
	pfAddGSet(geode, geoset);
    }

    if (NoTris) {
	int colourtype;
	reg pfVec4 *Colour;
	reg pfVec2 *TriTex;
	reg pfVec3 *TriVert = AllocVec3(3*NoTris), *TriNorm;
	if ((shading IS Unlit) OR FastLighting) {
	    Colour = SussColour(NoTris,mat,&colourtype);
	}
	if (shading IS Lit) {
	    TriNorm = AllocVec3(NoTris);
	}
	if (shading IS Smooth) {
	    TriNorm = AllocVec3(3*NoTris);
	}
	if (tex) {
	    TriTex = AllocVec2(3*NoTris);
	}

	NoTris = TriNormCount = 0;
	for (i=0; i<NoChains; i++) {
	    t = ChainList[i];
	    if (t->Size IS 1) {
		if (shading IS Lit) {
		    CopyXYZ(TriNorm[TriNormCount++],t->Normal[0]);
		}
		for (index=0; index<3; index++) {
		    if (shading IS Smooth) {
			CopyXYZ(TriNorm[TriNormCount++],t->Normal[index]);
		    }
		    if (tex) {
			CopyXY(TriTex[NoTris],t->TCoord[index]);
		    }
		    CopyXYZ(TriVert[NoTris++],t->Coordinate[index]);
		}
	    }
	}
	NoTris /= 3;
	geoset = pfNewGSet(MeditArena);
	pfGSetPrimType(geoset, PFGS_TRIS);
	pfGSetNumPrims(geoset, NoTris);
	pfGSetAttr(geoset, PFGS_COORD3, PFGS_PER_VERTEX, TriVert, NULL);
	if ((shading IS Unlit) OR FastLighting) {
	    pfGSetAttr(geoset, PFGS_COLOR4, colourtype, Colour, NULL);
	}
	if (shading IS Lit) {
	    pfGSetAttr(geoset, PFGS_NORMAL3, PFGS_PER_PRIM, TriNorm, NULL);
	}
	if (shading IS Smooth) {
	    pfGSetAttr(geoset, PFGS_NORMAL3, PFGS_PER_VERTEX, TriNorm, NULL);
	}
	if (tex) {
	    pfGSetAttr(geoset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, TriTex, NULL);
	}
	pfGSetGState(geoset, MakeGeostate(FALSE,mat,tex,EnvMap));
	pfAddGSet(geode, geoset);
    }
}
/************************************************************************
Turn a list of polygons into triangles then remesh them
************************************************************************/
static void MeshPolygons(pfGeode *geode, PolyListPtr base)
{
    MTexturePtr tex;
    MMaterialPtr mat;
    flag checktextures, envmap;
    int NoTriangles,NoVertices,NoChains;

    VListPtr *Vindex;		/* Index into the vertex list below */
    VListPtr vertexlist;	/* Pointer to vertex list */
    TrianglePtr trilist;	/* Pointer to list of triangles */

    
/* First break the polygons into separate triangles */

    SpreadVertexNormals(base);
    NoTriangles = ShatterPolygonList(base,&trilist);
    NoVertices = MakeVertexList(trilist,NoTriangles,&vertexlist,&Vindex);
    FindNeighbours(NoVertices,Vindex);
    checktextures = (DoTextures AND !(base->Polygon->Flags & MEnvironmentMap));
    CheckClashes(trilist,checktextures);


/* Then find chains of connected polygons */

    NoChains = FindPolygonChains(NoTriangles,trilist);



/* Then convert these chains into geosets */

    if (DoTextures) {
	tex = base->Polygon->Texture;
	mat = base->Polygon->TextureMaterial;
	envmap = ((base->Polygon->Flags & MEnvironmentMap) ISNT 0);
    }
    else {
	tex = NULL;
	mat = base->Polygon->NormalMaterial;
	envmap = FALSE;
    }
    AddChains(NoChains,geode, mat, tex, envmap);


    /* Free temporary storage */
    FreeTriList(&trilist);
}

