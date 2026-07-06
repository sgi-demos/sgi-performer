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

#ifdef WIN32
#define MAXFLOAT FLT_MAX
#endif



/************************************************************************
Convert a geoset to polygons
************************************************************************/
PolygonPtr AddNewPoly(GeometryPtr g, MMaterialPtr mat, MTexturePtr tex, int lighting)
{
    PolygonPtr p = malloc(sizeof(myPolygon));
    p->Next = g->FirstPolygon;
    g->FirstPolygon = p;
    p->NoVerts = 0;
    p->mat = AddMaterial(mat);
    p->tex = tex;
    p->lighting = lighting;
    return p;
}
#define GetVerTex					\
v = vert[((indexed)? vindex[index]: index)];		\
if (tex) {						\
    t = texcoord[((indexed)? tindex[index]: index)];	\
}
#define NextMaterial MixMaterial(&mat, colour[((indexed)? cindex[col]: col)]);
#define GetMaterial(n)				\
add = NextMaterial;				\
if (colourbinding IS PFGS_PER_PRIM) {		\
    col++;					\
}						\
else if (colourbinding IS PFGS_PER_VERTEX) {	\
    col += n;					\
}

void AddToPoly(PolygonPtr p,  float *v,  float *t)
{
    int index = p->NoVerts;
    CopyXYZ(p->Pos[index], v);
    if (p->tex) {
	CopyXYZ(p->Tex[index], t);
    }
    p->NoVerts = ++index;
}
static void GrabGeoset(GeometryPtr g,  pfGeoSet *gset)
{
    pfTexture *pft;
    pfGeoState *state;

    PolygonPtr p;
    MMaterial mat;
    MTexturePtr tex;
    MMaterialPtr add;
    flag indexed, coloured, flipper;
    float *v, *v1, *v2, (*vert)[XYZ];
    float *c, *c1, *c2, (*colour)[XYZW];
    float *t, *t1, *t2, (*texcoord)[XY];
    int i, j, lighting, index, col, fudge;
    unsigned short *tindex, *vindex, *cindex;
    pflong noprims, primtype, *primlength, normalbinding, colourbinding, texbinding;

    state = pfGetGSetGState(gset);
    noprims = pfGetGSetNumPrims(gset);
    pfGetGSetAttrLists(gset, PFGS_COORD3, (void**)&vert, &vindex);
    indexed = (vindex ISNT NULL);

    tex = NULL;
    texbinding = pfGetGSetAttrBind(gset, PFGS_TEXCOORD2);
    if (texbinding ISNT PFGS_OFF) {
	if (state) {
	    if (pft = pfGetGStateAttr(state, PFSTATE_TEXTURE)) {
		tex = MakeTexture(pft);
	    }
	}
	pfGetGSetAttrLists(gset, PFGS_TEXCOORD2, (void**)&texcoord, &tindex);
    }

    normalbinding = pfGetGSetAttrBind(gset, PFGS_NORMAL3);
    colourbinding = pfGetGSetAttrBind(gset, PFGS_COLOR4);

    col = 0;
    if (colourbinding IS PFGS_OFF) {
	GenerateMaterial(&mat, pfGetGStateAttr(state, PFSTATE_FRONTMTL));
	add = &mat;
    }
    else {
	GenerateMaterial(&mat, NULL);
	pfGetGSetAttrLists(gset, PFGS_COLOR4, (void**)&colour, &cindex);
	if (colourbinding IS PFGS_OVERALL) {
	    add = NextMaterial;
	}
    }
    if ((colourbinding IS PFGS_PER_VERTEX) OR (normalbinding IS PFGS_PER_VERTEX)) {
	lighting = Smooth;
    }
    else {
	lighting = Lit;
    }


    t = NULL;
    index = 0;
    fudge = 0;
    DEBUG"indexed = %d\n", indexed);
    DEBUG"colour binding = %d\n", colourbinding);
    switch (primtype = pfGetGSetPrimType(gset)) {
    	case PFGS_TRIS:		DEBUG"%d triangles\n", noprims);
				for (i=0; i<noprims; i++) {
				    GetMaterial(3);
				    p = AddNewPoly(g, add, tex, lighting);
				    for (j=0; j<3; j++) {
					GetVerTex;
					AddToPoly(p, v, t);
					index++;
				    }
				}
				break;
	case PFGS_QUADS:	DEBUG"%d quads\n", noprims);
				for (i=0; i<noprims; i++) {
				    GetMaterial(4);
				    p = AddNewPoly(g, add, tex, lighting);
				    for (j=0; j<4; j++) {
					GetVerTex;
					AddToPoly(p, v, t);
					index++;
				    }
				}
				break;
				
	case PFGS_FLAT_TRISTRIPS:lighting = Lit;
				fudge = -2;
	case PFGS_TRISTRIPS:    DEBUG"%d tmeshes\n", noprims);
				primlength = pfGetGSetPrimLengths(gset);
				for (i=0; i<noprims; i++) {
				    GetMaterial(primlength[i]+fudge);
				    flipper = FALSE;
				    for (j=0; j<primlength[i]; j++) {
					GetVerTex;
					if (j >= 2) {
					    p = AddNewPoly(g, add, tex, lighting);
					    AddToPoly(p, v2, t2);
					    AddToPoly(p, v1, t1);
					    AddToPoly(p, v, t);
					}
					if (flipper) {
					    v1 = v;
					    t1 = t;
					}
					else {
					    v2 = v;
					    t2 = t;
					}
					flipper = !flipper;
					index++;
				    }
				}
				break;
    }
}
#undef MakePoly
#undef GetVerTex
/************************************************************************
Parse a geode and grab its geosets 
************************************************************************/
static void GeometryGrabber(pfNode *node, GListPtr vars, flag hasswitch, float in, float out)
{
    int i, type;
    GeometryPtr g;
    pfGeoSet *gset;
    flag billboard;

    billboard = ItsABillboard(node);
    if (ItsAGeode(node)) {
	for (i=0; i<pfGetNumGSets(node); i++) {
	    gset = pfGetGSet(node, i);
	    g = malloc(sizeof(Geometry));
	    if (g->Billboard = billboard) {
		g->Next = vars->FirstBill;
		vars->FirstBill = g;
		pfGetBboardPos((pfBillboard*)node, i, g->Centre);
	    }
	    else {
		g->Next = vars->FirstGeom;
		vars->FirstGeom = g;
	    }
	    g->FirstPolygon = NULL;
	    g->HasSwitch = hasswitch;
	    g->SwitchIn = in;
	    g->SwitchOut = out;
	    GrabGeoset(g, gset);
	}
    }
}
/************************************************************************
Parse a geode and grab its geosets 
************************************************************************/
static void GrabGeometry(pfNode *node, GListPtr vars, flag hasswitch, float in, float out)
{
    flag IsLod;
    pfNode *child;
    float nin, nout;
    int loop, type, children;

    if (IsLod = ItsALod(node)) {
	nin = 0.0;
    }
    LoopThroughChildren(node) {
	child = LoopChild;
	if (IsLod) {
	    if ((children IS 1) OR (loop ISNT (children-1))) {
		nout = pfGetLODRange((pfLOD*)node, loop+1);
		if (ItsAGeode(child)) {
		    GeometryGrabber(child, vars, TRUE, nin, nout);
		}
		else {
		    GrabGeometry(child, vars, TRUE, nin, nout);
		}
		nin = nout;
	    }
	}
	else {
	    if (ItsAGeode(child)) {
		GeometryGrabber(child, vars, hasswitch, in, out);
	    }
	    else {
		if (pfGetNumChildren(child) > 0) {
		    GrabGeometry(child, vars, hasswitch, in, out);
		}
	    }	
	}
    }
}




/************************************************************************
Convert geometry to a Medit object and attach it to the scene
************************************************************************/
static void ConvertGeometry(GeometryPtr g)
{
    int i;
    float *t;
    MPolygonPtr p;
    MTexturePtr tex;
    MPolygonBeadPtr pb;
    PolygonPtr l = g->FirstPolygon;

    while (l) {
	t = NULL;
	pb = NULL;
	tex = l->tex;
	for (i=0; i<l->NoVerts; i++) {
	    if (tex) {
		t = l->Tex[i];
	    }
	    Medit_AddPolygonBead(&pb, l->Pos[i], t);
	}
	p = Medit_AddPolygon(pb, NULL);
	p->Lighting = l->lighting;
	p->NormalMaterial = l->mat;
	p->TextureMaterial = l->mat;
	p->Texture = tex;	
	PolygonsCreated++;
	l = l->Next;
    }
}
static void AttachGeometry(MBranchPtr b, GListPtr data)
{
    MLodPtr l;
    int i, count;
    PolygonPtr p;
    char name[100];
    MSubObjectPtr s;
    MeditObjectPtr o;
    flag done, makenew;
    GeometryPtr g, nextg, search;
    float trans[4][4], vanish, distance, next;
    float xmin, xmax, xc, ymin, ymax, yc, zmin, zmax, zc;

    if (g = data->FirstGeom) {
	if (o = FindObject(g, trans)) {
	    Medit_AttachObject(b, o, trans);
	    ObjectsReplicated++;
	}
	else {
	    count = 0;
	    vanish = 0;			/* Find vanishing distance of object */
	    g = data->FirstGeom;
	    xmin = ymin = zmin =  MAXFLOAT;
	    xmax = ymax = zmax = -MAXFLOAT;
	    while (g) {
		if (g->HasSwitch AND (g->SwitchOut > vanish)) {
		    vanish = g->SwitchOut;
		}
		p = g->FirstPolygon;	/* Find centre of object at the same time */
		while (p) {
		    for (i=0; i<p->NoVerts; i++) {
			count++;
			if (p->Pos[i][X] > xmax) xmax = p->Pos[i][X];
			if (p->Pos[i][X] < xmin) xmin = p->Pos[i][X];
			if (p->Pos[i][Y] > ymax) ymax = p->Pos[i][Y];
			if (p->Pos[i][Y] < ymin) ymin = p->Pos[i][Y];
			if (p->Pos[i][Z] > zmax) zmax = p->Pos[i][Z];
			if (p->Pos[i][Z] < zmin) zmin = p->Pos[i][Z];
		    }
		    p = p->Next;
		}
		g = g->Next;
	    }
	    if (vanish IS 0) {		/* Default vanish point */
		vanish = 100000.0;
	    }
	    if (count) {
		xc = (xmin+xmax)/2.0;	/* Offset object by this much to centralise it */
		yc = (ymin+ymax)/2.0;
		zc = zmin;
	    }
	    else {
		xc = yc = zc = 0.0;
	    }

	    /* Apply switch range [0..vanish] to non switched stuff */
	    o = NULL;
	    count = 0;
	    g = data->FirstGeom;
	    while (g) {
		if (!o) {
		    o = Medit_AddObject(data->Name);
		}
		if (!g->HasSwitch) {
		    g->SwitchIn = 0;
		    g->SwitchOut = vanish;
		}
		p = g->FirstPolygon;
		while (p) {
		    for (i=0; i<p->NoVerts; i++) {
			p->Pos[i][X] -= xc;
			p->Pos[i][Y] -= yc;
			p->Pos[i][Z] -= zc;
		    }
		    p = p->Next;
		}
		g = g->Next;
	    }    
	
	    /* Build the lod structure */
	    if (o) {
		count = 0;
		distance = 0;
		until (distance IS vanish) {
		    next = vanish;		/* Find next switch distance */
		    g = data->FirstGeom;
		    while (g) {
			if ((g->SwitchIn > distance) AND (g->SwitchIn < next)) {
			    next = g->SwitchIn;
			}
			if ((g->SwitchOut > distance) AND (g->SwitchOut < next)) {
			    next = g->SwitchOut;
			}
			g = g->Next;
		    }
	
		    makenew = TRUE;		/* Add all subobjects in this switch range */
		    g = data->FirstGeom;
		    while (g) {
			if ((g->SwitchIn < next) AND (g->SwitchOut > distance)) {
			    if (makenew) {
				makenew = FALSE;
				sprintf(name, "Lod%d", count);
				l = Medit_AddLod(name);
				l->SwitchOut = next;
				sprintf(name, "SubObject%d", count++);
				Medit_AddLodBead(Medit_AddSubObject(name));
			    }
			    ConvertGeometry(g);
			}
			g = g->Next;
		    }
		    distance = next;
		}
		Identifyf(trans);
		trans[W][X] = xc;	    /* Apply translation due to object centre */
		trans[W][Y] = yc;
		trans[W][Z] = zc;
		Medit_AttachObject(b, o, trans);
		RememberObject(o, data->FirstGeom);
		ObjectsCreated++;
	    }
	}
    }

    if (g = data->FirstBill) {
	while (g) {
	    nextg = g->Next;
	    g->Next = NULL;
	    if (o = FindObject(g, trans)) {
		trans[W][X] += g->Centre[X];
		trans[W][Y] += g->Centre[Y];
		trans[W][Z] += g->Centre[Z];
		Medit_AttachObject(b, o, trans);
		ObjectsReplicated++;
	    }
	    else {
	    	o = Medit_AddObject(data->Name);
		Medit_AddLod("Lod");
		s = Medit_AddSubObject("bill");
		Medit_AddLodBead(s);
		ConvertGeometry(g);
		o->Attributes |= ObjBillboard;
		Identifyf(trans);
		CopyXYZ(trans[W], g->Centre);
		Medit_AttachObject(b, o, trans);
		RememberObject(o, g);
		ObjectsCreated++;
	    }
	    g = nextg;
	}
    }
}

