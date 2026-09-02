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
Storage routines for the Performer loader...
************************************************************************/
#define PL struct poly_list_struct
#define VL struct vl_struct
#define TRI struct triangle_struct

typedef TRI tristruct;		/* Resolve circular definitions */

static void CopyXY(reg float dest[], reg float source[])
{
    dest[X] = source[X];
    dest[Y] = source[Y];
}
static void CopyXYZ(reg float dest[], reg float source[])
{
    dest[X] = source[X];
    dest[Y] = source[Y];
    dest[Z] = source[Z];
}
enum { Red=0, Green, Blue };

/************************************************************************
Lists of polygons...
************************************************************************/
typedef PL {
PL		*Next;
MPolygonPtr	Polygon;
} PolyList;
typedef PolyList *PolyListPtr;

static PolyListPtr SparePolyLists = NULL;	/* To cache list beads */
static void AddToPolyList(reg PolyListPtr *list, reg MPolygonPtr p)
{
    reg PolyListPtr new;
    if (SparePolyLists) {
	new = SparePolyLists;
	SparePolyLists = SparePolyLists->Next;
    }
    else {
	new = malloc(sizeof(PolyList));
    }
    new->Next = *list;
    *list = new;
    new->Polygon = p;
}
static void FreePolyList(reg PolyListPtr list)
{
    reg PolyListPtr l, next;
    if (l = list) {
	while (next = l->Next) {
	    l = next;
	}
	l->Next = SparePolyLists;
	SparePolyLists = list;
    }
}

/************************************************************************
Vertex lists
************************************************************************/
typedef VL {
tristruct	*Owner;
float		*Coordinate;
int		Index;
} VList;
typedef VList *VListPtr;

/************************************************************************
Triangle storage lists
************************************************************************/
typedef TRI {
TRI		*Next;			/* Link to next triangle		*/
TRI		*Chain;			/* Link to next triangle		*/
TRI		*Neighbour[3];		/* Pointer to neighbours		*/
unsigned char	Index[3];		/* Index no of neighbours		*/
unsigned char	Done;			/* Flag - triangle has been processed	*/
float		Coordinate[3][3];	/* Vertices				*/
float		Normal[3][3];		/* Normals				*/
float		TCoord[3][2];		/* Texture coordinates			*/
int		LightGroup;		/* Lighting group of this triangle	*/
int		Size;			/* Chain size...			*/
int		meshgroup;		/* Temp for mesh generation		*/
} Triangle;
typedef Triangle *TrianglePtr;

static TrianglePtr SpareTris = NULL;	/* To cache triangles */
static TrianglePtr NewTri(void)
{
    reg TrianglePtr new;
    if (SpareTris) {
	new = SpareTris;
	SpareTris = SpareTris->Next;
    }
    else {
	new = malloc(sizeof(Triangle));
    }
    new->Next = NULL;
    new->Neighbour[0] = NULL;
    new->Neighbour[1] = NULL;
    new->Neighbour[2] = NULL;
    new->meshgroup	  = 0;
    return new;
}
static TrianglePtr AddTriangle(TrianglePtr *base)
{
    reg TrianglePtr new = NewTri();
    new->Next = *base;
    *base = new;
    return new;
}
static void FreeTriList(TrianglePtr *base)
{
    if (*base) {
	reg TrianglePtr t = *base;
	while (t->Next) {
	    t = t->Next;
	}
	t->Next = SpareTris;
	SpareTris = *base;
	*base = NULL;
    }
}
/************************************************************************
undefs
************************************************************************/
#undef PL
#undef VL
#undef TRI




/************************************************************************
Repeated objects in the scene are pfCloned. The list generated here
is used to remember objects which have been converted/can be cloned
************************************************************************/
static pfGroup **ObjectList;
static void InitialiseObjectList(void)
{
    int i, NoObjects;
    MeditObjectPtr o;

    NoObjects = 0;
    o = CurrentMeditFile->FirstObject;
    while (o) {
	o->Id = NoObjects++;	/* o->Id is an index into the list */
	o = o->Next;
    }
    ObjectList = malloc(NoObjects * sizeof(pfGroup*));
    for (i=0; i<NoObjects; i++) {
	ObjectList[i] = NULL;
    }
}

static void FreeObjectList(void)
{
    free(ObjectList);
}


