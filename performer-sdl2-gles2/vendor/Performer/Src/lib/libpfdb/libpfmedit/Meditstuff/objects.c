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
This bit tries to find duplicate (cloned/instanced) geometry
************************************************************************/

typedef struct rstruct {
struct rstruct	*Next;
MeditObjectPtr	obj;
GeometryPtr	Geom;
int		NoTris;
int		NoQuads;
Matrix		CompMat;
} Remember;
typedef Remember *RememberPtr;
static RememberPtr RememberedObjects = NULL;

/************************************************************************
Count the number of tris/quads in some geometry
************************************************************************/
static void CountPolys(GeometryPtr geom, int *tris, int *quads)
{
    reg PolygonPtr p;
    reg int t=0, q=0;
    while (geom) {
	p = geom->FirstPolygon;
	while (p) {
	    if (p->NoVerts IS 3) {
		t++;
	    }
	    else if (p->NoVerts IS 4) {
		q++;
	    }
	    p = p->Next;
	}
	geom = geom->Next;
    }
    *tris = t;
    *quads = q;
}
/************************************************************************
Generate a matrix to orient geometry
************************************************************************/
static void GenerateCompareMatrix(GeometryPtr geom, MatrixPtr mat)
{
    int i;
    Vector v;
    real angle;
    PolygonPtr p;
    Coordinate c;
    float xc, yc, zc, length;
    
    xc = yc = zc = 0.0;
    p = geom->FirstPolygon;
    if (p) {
	CalcPolyNormal(p, v);
	RotateToXAxis(v, mat);
	PreTranslate(mat, -p->Pos[0][X], -p->Pos[0][Y], -p->Pos[0][Z]);
	CopyXYZ(c, p->Pos[1]);
	TransformCoordinate(c, mat, c);
	angle = deg(atan2(c[Z], c[Y]));
	PostPitch(mat, -angle);
	CopyXYZ(c, p->Pos[1]);
	TransformCoordinate(c, mat, v);
	length = LengthOfVector(v);
	if (length ISNT 0.0) {
	    length = 1.0/length;
	    PostScale(mat, length, length, length);
	}
    }
    else {
	Identify(mat);
    }
}
/************************************************************************
See if we can find an object in the list of objects we already have
************************************************************************/
static void RememberObject(MeditObjectPtr o, GeometryPtr geom)
{
    int i;
    PolygonPtr p;
    RememberPtr r;
    GeometryPtr g;

    r = malloc(sizeof(Remember));
    r->Next = RememberedObjects;
    RememberedObjects = r;
    r->obj = o;
    r->Geom = geom;
    CountPolys(geom, &(r->NoTris), &(r->NoQuads));
    GenerateCompareMatrix(geom, r->CompMat);
    g = geom;
    while (g) {
	p = g->FirstPolygon;
	while (p) {
	    for (i=0; i<p->NoVerts; i++) {
		TransformCoordinatef(p->Pos[i], r->CompMat, p->Pos[i]);
	    }
	    p = p->Next;
	}
	g = g->Next;
    }
}
/************************************************************************
Decide if two numbers are nearly equal
************************************************************************/
#define Tolerence 0.01
static boolean NearlyEqual(float n1, float n2)
{
    if (fabsf(n1) < Tolerence) {
	if (n1 > 0) {
	    return (n2 < Tolerence);
	}
	else {
	    return (n2 > -Tolerence);
	}
    }
    else {
	return (fabsf((n1/n2)-1.0) < Tolerence);
    }
}
/************************************************************************
See if we can find an object in the list of objects we already have
nb. This one uses a "goto"!!!    (so watcha gonna do 'bout it)
************************************************************************/
#define FailIf(condition) if (condition) goto next;
static MeditObjectPtr FindObject(GeometryPtr geom, float (*trans)[4])
{
    int i;
    float c[3];
    RememberPtr r;
    Matrix m, comp;
    int tris, quads;
    reg PolygonPtr p, rp;
    reg GeometryPtr g, rg;

    r = RememberedObjects;
    CountPolys(geom, &tris, &quads);
    GenerateCompareMatrix(geom, comp);
    while (r) {
	if ((r->NoTris IS tris) AND (r->NoQuads IS quads)) {
	    g = geom;
	    rg = r->Geom;
	    while (g) {
		FailIf(!rg);
		FailIf(g->Billboard ISNT rg->Billboard);
		FailIf(g->HasSwitch ISNT rg->HasSwitch);
		if (g->HasSwitch) {
		    FailIf(g->SwitchIn ISNT rg->SwitchIn);
		    FailIf(g->SwitchOut ISNT rg->SwitchOut);
		}
		p = g->FirstPolygon;
		rp = rg->FirstPolygon;
		while (p) {
		    FailIf(!rp);
		    FailIf(p->mat ISNT rp->mat);
		    FailIf(p->tex ISNT rp->tex);
		    FailIf(p->NoVerts ISNT rp->NoVerts);
		    FailIf(p->lighting ISNT rp->lighting);
		    for (i=0; i<p->NoVerts; i++) {
			TransformCoordinatef(p->Pos[i], comp, c);
			FailIf(!NearlyEqual(c[X], rp->Pos[i][X]));
			FailIf(!NearlyEqual(c[Y], rp->Pos[i][Y]));
			FailIf(!NearlyEqual(c[Z], rp->Pos[i][Z]));
			if (p->tex) {
			    FailIf(p->Tex[i][X] ISNT rp->Tex[i][X]);
			    FailIf(p->Tex[i][Y] ISNT rp->Tex[i][Y]);
			}
		    }
		    p = p->Next;
		    rp = rp->Next;
		}
		FailIf(rp);
		g = g->Next;
		rg = rg->Next;
	    }
	    FailIf(rg);
	    MultMatrix(r->CompMat, MatrixInverse(comp), m);
	    RealMatrixToFloat(m, trans);
	    return r->obj;
	}
	next:	r = r->Next;
    }
    return NULL;
}
