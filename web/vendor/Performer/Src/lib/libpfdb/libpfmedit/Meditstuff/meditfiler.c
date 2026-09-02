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

#include <Performer/pf.h>
#include <Performer/pfdu.h>


/************************************************************************
Defines/renames
************************************************************************/
#define Allocate malloc
#define Reallocate realloc
#define Free free
#define CopyOfString strdup



#define Materials (CurrentMeditFile->MaterialList)
#define _Id Id
#define LoopThroughSubObjectsPolygons(s,p) 	for (p=s->FirstPolygon;	p;   p = p->Next)
#define LoopThroughPolygonsBeads(p,pb)		for (pb=p->FirstBead; 	pb; pb =pb->Next)
#define LoopThroughLodsBeads(l,lb)		for (lb=l->FirstBead; 	lb; lb =lb->Next)
#define	LoopThroughAllSubObjects(s)		for (s=FirstSubObject();s;   s = s->Next)
#define	LoopThroughAllLods(l)			for (l=FirstLod(); 		l;   l = l->Next)
#define	LoopThroughAllObjects(o)		for (o=FirstObject(); 	o;   o = o->Next)
#define LoopThroughAllTextures(t)		for (t=FirstTexture(); 	t;   t = t->Next)
#define BranchName(b)		b->Name
#define BranchMatrix(b)		b->Trans
#define BranchObject(b)		b->Obj
#define NextMaterial(m)		m->Next
#define MaterialId(m)		m->Id
#define TextureName(t)		t->File
#define TextureId(t)		t->Id
#define NextChild		Next

/* Rename functions to avoid clashes with user functions/data types */
#define MaxFileLine		100
#define AddObject		Medit_AddObject
#define AddSubObject		Medit_AddSubObject
#define AddLod			Medit_AddLod
#define AddLodBead		Medit_AddLodBead
#define AddPolygon		Medit_AddPolygon
#define AddBranch		Medit_AddBranch
#define AddMaterial		AddMaterialF
#define AddTexture		AddTextureF
#define CurrentLighting		Medit_Lighting
#define FileError		MeditError
#define FileErrorType		MeditErrorType
#define Ferr			MeditErrorList
#define TextureDir		Medit_TextureDir

#define Material		MMaterial
#define MaterialPtr		MMaterialPtr
#define TexturePtr		MTexturePtr
#define PolygonBeadPtr		MPolygonBeadPtr
#define PolygonPtr		MPolygonPtr
#define SubObjectPtr		MSubObjectPtr
#define LodBeadPtr		MLodBeadPtr
#define LodPtr			MLodPtr
#define BranchPtr		MBranchPtr

#define MaxName			M_MaxName

typedef float MatrixRow[4];
typedef MatrixRow MMatrix[4];
typedef MatrixRow *MatrixPtr;
#define Matrix MMatrix

#define LinkThing(base,size)	\
new = Allocate(size);		\
search = base;			\
if (search) {			\
    while (search->Next) {	\
	search = search->Next;	\
    }				\
    search->Next = new;		\
}				\
else {				\
    base = new;			\
}				\
new->Next = NULL;

#define AddThing(base,size,name)	\
LinkThing(base,size);			\
if (Name) {				\
    new->Name = CopyOfString(Name);	\
}					\
else {					\
    new->Name = CopyOfString(name);	\
}

/************************************************************************
File errors
************************************************************************/
static int Compatibility;
int FileError,FileErrorType;
#define ifbad(x) if (!(x))
#define GetStuff(x) ifbad(x) return
enum {
REACHED_END=0,			/* Types of error */
UNKNOWN_FILE,
BAD_FILE,
FILE_NOT_FOUND,
TOO_MANY_REALS,

BAD_STRING,
BAD_REVISION,
BAD_MATERIAL_INDEX,
BAD_MATERIAL,
BAD_VERTEX,

BAD_VERTEX_INDEX,
BAD_POLYGON,
BAD_LIGHTING,
BAD_MATERIAL_IN_POLY,
NOT_ENOUGH_VERTICES,

BAD_SUB_OBJECT,
SUB_OBJECT_NOT_FOUND,
BAD_LOD,
BAD_TREE,
BAD_OBJECT,

BAD_TEXTURE,
BAD_THING,
BAD_VIEW
};

char *Ferr[] = {
"Unexpected end of file",
"Unrecognised file",
"Bad file",
"File not found",
"Tried to read/write too many reals",

"Bad string in file",
"Bad file version, you need a newer version of libMedit!",
"Bad material index",
"Error in materials",
"Bad vertex definition",

"Bad vertex index",
"Bad polygon definition",
"Unknown lighting in polygon",
"Polygon has bad material",
"Polygon has less than 3 vertices",

"Rubbish in sub object",
"Undefined sub object in Lod",
"Rubbish in LOD definition",
"Bad tree definition",
"Unknown object in tree definition",

"Bad texture definition",
"Unknown data in file - object/scene expected",
"Bad view definition"
};

static void SetError(int type)
{
    FileError = TRUE;
    FileErrorType = type;
}
/************************************************************************
User errors
************************************************************************/
enum {
StartFileFirst=0,
AddObjectBeforeLod,
AddObjectFirst,
AddLodFirst,
AddSubObjectFirst,
NothingToWrite,
BadDirection
};
static char *MLibErrors[] = {
"call 'NewMeditFile()' before any other storage function",
"create an object with 'Medit_AddObject()' before you can create LODs",
"create an object with 'Medit_AddObject()' before you can create SubObjects",
"create an LOD with 'Medit_AddLod()' before you can create LodBeads",
"create a SubObject with 'Medit_AddSubObject()' before you can create Polygons",
"create some data before you call 'WriteMedit()'",
"bad direction in 'Medit_AddBranch()'"
};
static void MLibError(int message, char *reason)
{
    fprintf(stderr,"libMedit: User error.\nYou must %s\007\n",MLibErrors[message]);
    if (reason) {
	fprintf(stderr,"%s\n",reason);
    }
    exit(0);
}
/************************************************************************
Storage subroutines
************************************************************************/
typedef float real;
MeditFilePtr CurrentMeditFile = NULL;
char TextureDir[MaxFileName] = "";
static int Medit_Lighting = 0;
char FilePath[MaxFileName], Medit_FilePath[MaxFileName];
static char OriginalTextureDir[MaxFileName] = "";

static flag SubObjectsAllowed, PolygonsAllowed, LodsAllowed, LodBeadsAllowed;
int ObjectIsolated = TRUE;
int MeditFileHadNormals;
/************************************************************************
See if they have called NewMeditFile()
************************************************************************/
static void CheckNewFileStarted(char *err)
{
    if (!CurrentMeditFile) {
	char s[1000];
	sprintf(s,"Error in %s",err);
	MLibError(StartFileFirst,s);
    }
}
/************************************************************************
Create a new medit file...
************************************************************************/
static BranchPtr *Scene(void);
MeditFilePtr NewMeditFile(void)
{
    CurrentMeditFile = Allocate(sizeof(MeditFile));

    CurrentMeditFile->FirstMaterial	= NULL;
    CurrentMeditFile->FirstTexture	= NULL;
    CurrentMeditFile->FirstObject	= NULL;
    CurrentMeditFile->Scene		= NULL;
    LodsAllowed		=
    LodBeadsAllowed	=
    SubObjectsAllowed	=
    PolygonsAllowed	= FALSE;

    CurrentLighting	= Lit;

    (void)Scene();		/* Creates a default scene */
    return CurrentMeditFile;
}
/************************************************************************
Materials...
************************************************************************/
static MaterialPtr *FirstMaterial(void)
{
    return &(CurrentMeditFile->FirstMaterial);
}
static MaterialPtr NewMaterial(void)
{
    MaterialPtr new = Allocate(sizeof(Material));
    new->Name = NULL;
    new->Next = NULL;
    new->Ambient[0]  = new->Ambient[1]  = new->Ambient[2] = 0.3;
    new->Diffuse[0]  = new->Diffuse[1]  = new->Diffuse[2] = 0.5;
    new->Specular[0] = new->Specular[1] = new->Specular[2] = 1.0;
    new->Emissive[0] = new->Emissive[1] = new->Emissive[2] = 0.0;
    new->Shine = 64;
    new->Alpha = 1.0;
    return new;
}
static void AddMaterial(MaterialPtr m)
{
    register MaterialPtr next, search = *FirstMaterial();
    if (search) {
	while (next = NextMaterial(search)) {
	    search = next;
	}
	NextMaterial(search) = m;
    }
    else {
	*FirstMaterial() = m;
    }
    NextMaterial(m) = NULL;
}
static void RenameMaterial(MaterialPtr m, char *name)
{
    if (m->Name) {
	Free(m->Name);
    }
    if (m->Name = name) {
	m->Name = CopyOfString(name);
    }
}
static void FreeMaterial(MaterialPtr m)
{
    if (m->Name) {
	Free(m->Name);
    }
    Free(m);
}
MaterialPtr Medit_AddMaterial(char *Name)
{
    MaterialPtr new = NewMaterial();
    RenameMaterial(new,Name);
    AddMaterial(new);
    return new;
}
/************************************************************************
Textures
************************************************************************/
static TexturePtr FirstTexture(void)
{
    return CurrentMeditFile->FirstTexture;
}
static TexturePtr AddTextureF(char *filename)
{
    FILE *image;
    register TexturePtr search,new;

    CheckNewFileStarted("Medit_AddTexture");

#if 0
    if (image = fopen(filename,"r")) {
#else
    if (image = pfdOpenFile(filename)) {
#endif
	fclose(image);
	LinkThing((CurrentMeditFile->FirstTexture),sizeof(MTexture));
	new->File = CopyOfString(filename);
	new->MinFilter = MT_NULL;
	new->MagFilter = MT_NULL;
	new->ClampX = FALSE;
	new->ClampY = FALSE;

	return new;
    }
    else {
	return NULL;
    }
}
TexturePtr Medit_AddTexture(char *filename)
{
    register TexturePtr search,new;

    CheckNewFileStarted("Medit_AddTexture");
    LinkThing((CurrentMeditFile->FirstTexture),sizeof(MTexture));
    new->File = CopyOfString(filename);
    new->MinFilter = 0;
    new->MagFilter = 0;
    new->Fast = FALSE;
    new->ClampX = FALSE;
    new->ClampY = FALSE;
    return new;
}
#define IMAGIC 0732
static boolean IsAnImageFile(char *name)
{
    FILE *f;
    char header[2];

#if 0
    if (!(f = fopen(name,"rb"))) {
#else
    if (!(f = pfdOpenFile(name))) {
#endif
	return FALSE;
    }
    header[0] = fgetc(f);
    header[1] = fgetc(f);
    fclose(f);
    return ((((header[0] << 8) | header[1]) IS IMAGIC)
    OR      (((header[1] << 8) | header[0]) IS IMAGIC));
}
/************************************************************************
Object handlers
************************************************************************/
static MeditObjectPtr SelectedObject;
MeditObjectPtr AddObject(char *Name)
{
    register MeditObjectPtr search,new;

    CheckNewFileStarted("Medit_AddObject");
    AddThing(CurrentMeditFile->FirstObject,sizeof(MeditObject),"Unnamed");
    new->FirstLod	= NULL;
    new->FirstSubObject	= NULL;
    new->Attributes	= 0;
    SelectedObject	= new;

    SubObjectsAllowed = LodsAllowed = TRUE;

    return new;
}
static MeditObjectPtr FirstObject(void)
{
    return CurrentMeditFile->FirstObject;
}
static MeditObjectPtr *CurrentObject(void)
{
    return &SelectedObject;
}
static char **ObjectName(void)
{
    return &(SelectedObject->Name);
}
static int *ObjectId(void)
{
    return &(SelectedObject->Id);
}
static int *ObjectAttributes(void)
{
    return &(SelectedObject->Attributes);
}
/************************************************************************
SubObjects
************************************************************************/
static SubObjectPtr CurrentSubObject;
static SubObjectPtr FirstSubObject(void)
{
    return SelectedObject->FirstSubObject;
}
SubObjectPtr AddSubObject(char *Name)
{
    register SubObjectPtr search, new;

    CheckNewFileStarted("Medit_AddSubObject");
    if (SubObjectsAllowed) {
	AddThing(SelectedObject->FirstSubObject,sizeof(MSubObject),"Sub object");
	new->FirstPolygon	= NULL;
	CurrentSubObject	= new;
	PolygonsAllowed		= TRUE;
	return new;
    }
    else {
	MLibError(AddObjectFirst,NULL);
	return NULL;
    }
}
static SubObjectPtr *SelectedSubObject(void)
{
    return &CurrentSubObject;
}
void Medit_SetObject(MeditObjectPtr o, SubObjectPtr s)
{
    if (o) {
	SelectedObject = o;
	CurrentSubObject = o->FirstSubObject;
    }
    if (s) {
	CurrentSubObject = s;
    }
}
/************************************************************************
Lods
************************************************************************/
static LodPtr CurrentLod;
static LodPtr FirstLod(void)
{
    return SelectedObject->FirstLod;
}
LodPtr AddLod(char *Name)
{
    register LodPtr search, new;

    CheckNewFileStarted("Medit_AddLod");
    if (LodsAllowed) {
	AddThing(SelectedObject->FirstLod,sizeof(MLod),"Lod");
	new->FirstBead		= NULL;
	new->SwitchOut		= 100000.0;	/* Miles away... */
	CurrentLod		= new;
	LodBeadsAllowed		= TRUE;
	return new;
    }
    else {
	MLibError(AddObjectBeforeLod,NULL);
	return NULL;
    }
}
static LodPtr *SelectedLod(void)
{
    return &CurrentLod;
}

/* ----- Beads ----- */
LodBeadPtr AddLodBead(SubObjectPtr s)
{
    register LodBeadPtr search, new;

    CheckNewFileStarted("Medit_AddLodBead");
    if (LodBeadsAllowed) {
	LinkThing(CurrentLod->FirstBead,sizeof(MLodBead));
	new->SubObject = s;
	return new;
    }
    else {
	MLibError(AddLodFirst,NULL);
	return NULL;
    }
}
/************************************************************************
Vertices/polygons
************************************************************************/
static PolygonBeadPtr AddPolygonBead(PolygonBeadPtr *base, real *v)
{
    register PolygonBeadPtr pb, new = Allocate(sizeof(MPolygonBead));

    new->Coordinate[X] = v[X];
    new->Coordinate[Y] = v[Y];
    new->Coordinate[Z] = v[Z];
    new->TextureCoord[X] = new->TextureCoord[Y] = 0.0;

    if (pb = *base) {
	while (pb->Next) {
	    pb = pb->Next;
	}
	pb->Next = new;
    }
    else {
	*base = new;
    }
    new->Next = NULL;
    return new;
}
PolygonBeadPtr Medit_AddPolygonBead(PolygonBeadPtr *base, real *v, real *t)
{
    register PolygonBeadPtr new = AddPolygonBead(base,v);
    if (t) {
	new->TextureCoord[X] = t[X];
	new->TextureCoord[Y] = t[Y];
    }
    return new;
}
PolygonPtr AddPolygon(PolygonBeadPtr base, PolygonPtr parent)
{
    CheckNewFileStarted("Medit_AddPolygon");
    if (PolygonsAllowed) {
	register int count = 3;
	register MaterialPtr m;
	if (base) {
	    register PolygonBeadPtr pb = base;
	    count = 0;
	    while (pb) {
		count++;
		pb = pb->Next;
	    }
	}
	if (count >= 3) {
	    PolygonPtr new = Allocate(sizeof(MPolygon));
	    new->FirstBead = base;
	    if (parent) {
		new->NextChild = parent->Child;
		parent->Child = new;
	    }
	    else {
		new->Next = CurrentSubObject->FirstPolygon;
		CurrentSubObject->FirstPolygon = new;
	    }
	    if (!(m = *FirstMaterial())) {
		m = Medit_AddMaterial("default");
	    }
	    new->NormalMaterial	    = m;		/* Defaults... */
	    new->Lighting	    = Smooth;
	    new->Texture	    = NULL;
	    new->TextureMaterial    = NULL;
	    new->LightGroup	    = 0;
	    new->Flags		    = 0;
	    new->Child		    = NULL;
	    return new;
	}
	else {
	    return NULL;
	}
    }
    else {
	MLibError(AddSubObjectFirst,NULL);
	return NULL;
    }
}
static void ScanSubPolygons(PolygonPtr p, void (*Callback)(PolygonPtr,void*),void *vars)
{
    register PolygonPtr child = p->Child;
    if (child) {
	register void (*func)(PolygonPtr,void*) = Callback;
	while (child) {
	    func(child,vars);
	    if (child->Child) {
		ScanSubPolygons(child,func,vars);
	    }
	    child = child->NextChild;
	}
    }
}
/************************************************************************
Scene tree
************************************************************************/
static BranchPtr *Scene(void)
{
    CheckNewFileStarted("Medit_AddBranch");
    if (!CurrentMeditFile->Scene) {
	CurrentMeditFile->Scene = Allocate(sizeof(MBranch));
	CurrentMeditFile->Scene->Name = CopyOfString("Scene");
	CurrentMeditFile->Scene->Across = NULL;
	CurrentMeditFile->Scene->Down = NULL;
	CurrentMeditFile->Scene->Obj = NULL;
    }
    return &(CurrentMeditFile->Scene);
}
BranchPtr AddBranch(BranchPtr base, int Direction)
{
    BranchPtr new = Allocate(sizeof(MBranch));
    new->Across = new->Down = NULL;
    new->Name = NULL;
    new->Obj = NULL;
    switch (Direction) {
	case ACROSS:	if (base->Across) {
			    new->Down = base->Across;
			}
			base->Across = new;
			break;
	case DOWN:	if (base->Down) {
			    new->Down = base->Down;
			}
			base->Down = new;
			break;
	default:	MLibError(BadDirection,NULL);
    }
    return new;
}
void Medit_AttachObject(BranchPtr base, MeditObjectPtr o, float m[4][4])
{
    int i,j;
    BranchPtr b = AddBranch(base, ACROSS);
    b->Name = CopyOfString("Object");
    b->Obj = o;
    for (i=0; i<4; i++) {
	for (j=0; j<4; j++) {
	    if (m) {
		b->Trans[i][j] = m[i][j];
	    }
	    else {
		b->Trans[i][j] = (i IS j)?1.0:0.0;
	    }
	}
    }
}
void Medit_ScanTree(BranchPtr b, void (*Callback)(BranchPtr))
{
    while (b) {
	if (b->Across) {
	    Medit_ScanTree(b->Across,Callback);
	}
	Callback(b);
	b = b->Down;
    }
}
/************************************************************************
Initialisation/finalisation of file read...
************************************************************************/
static FILE *GetReadyForRead(char *name)
{
    FILE *f;

#if 0
    if ((f = fopen(name,"rb")) IS NULL) {
#else
    if ((f = pfdOpenFile(name)) IS NULL) {
#endif
	SetError(FILE_NOT_FOUND);
	return NULL;
    }
    FileError = FALSE;
    MeditFileHadNormals = TRUE;
    CurrentMeditFile = NewMeditFile();
    return f;
}
static void FinishReading(FILE *f)
{
    if (f) {
	fclose(f);
    }
    if (CurrentMeditFile){
	register int count;
        register MTexturePtr t = CurrentMeditFile->FirstTexture;
        register MMaterialPtr m = CurrentMeditFile->FirstMaterial;

	count = 0;
	while (m) {
	    m->Id = count++;
	    m = m->Next;
	}
	count = 0;
	while (t) {
	    t->Id = count++;
	    t = t->Next;
	}
    }
}
/************************************************************************
Convert between old/new coordinates
************************************************************************/
typedef void (*TwistFunction)(register float n[]);
static void Twist(register float n[])
{
    register float temp;
    temp = n[Y];
    n[Y] = -n[Z];
    n[Z] = temp;
}
static void Untwist(register float n[])
{
    register float temp;
    temp = n[Y];
    n[Y] = n[Z];
    n[Z] = -temp;
}
static void TwistPolygon(register MPolygonPtr p, register TwistFunction twist)
{
    register int l;
    register MPolygonBeadPtr pb;

    while (p) {
	if ((l = p->Lighting) IS Lit) {
	    twist(p->Normal);
	}
	pb = p->FirstBead;
	while (pb) {
	    twist(pb->Coordinate);
	    if (l IS Smooth) {
		twist(pb->Normal);
	    }
	    pb = pb->Next;
	}
	if (p->Child) {
	    TwistPolygon(p->Child,twist);
	}
	p = p->Next;
    }
}
static void TwistScene(MBranchPtr b, TwistFunction twist)
{
    register int i;
    float temp[XYZ];
    register MatrixPtr m;
    while (b) {
	if (b->Across) {
	    TwistScene(b->Across,twist);
	}
	if (b->Obj) {
	    m = b->Trans;
	    twist(m[X]);
	    twist(m[Y]);
	    twist(m[Z]);
	    twist(m[W]);
	    for (i=0; i<XYZ; i++) {
		temp[X] = m[X][i];
		temp[Y] = m[Y][i];
		temp[Z] = m[Z][i];
		twist(temp);
		m[X][i] = temp[X];
		m[Y][i] = temp[Y];
		m[Z][i] = temp[Z];
	    }
	}
	b = b->Down;
    }
}
static void ConvertCoordinates(TwistFunction t)
{
    register MLodPtr l;
    register MPolygonPtr p;
    register MSubObjectPtr s;
    register MeditObjectPtr o;

    o = CurrentMeditFile->FirstObject;
    while (o) {
	s = o->FirstSubObject;
	while (s) {
	    TwistPolygon(s->FirstPolygon, t);
	    s = s->Next;
	}
	l = o->FirstLod;
	while (l) {
	    l->SwitchOut = -l->SwitchOut;
	    l = l->Next;
	}
	o = o->Next;
    }
    TwistScene(CurrentMeditFile->Scene,t);
}
void Medit_ConvertToGLCoordinates(void)
{
    ConvertCoordinates(Untwist);
}
void Medit_ConvertFromGLCoordinates(void)
{
    ConvertCoordinates(Twist);
}
