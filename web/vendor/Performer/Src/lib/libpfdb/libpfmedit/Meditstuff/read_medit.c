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

#ifdef __linux__
#include <asm/byteorder.h>
#endif

#ifndef P_32_SWAP
#define	P_32_SWAP(a) {							\
	uint _tmp = *(uint *)a;				\
	((char *)a)[0] = ((char *)&_tmp)[3];				\
	((char *)a)[1] = ((char *)&_tmp)[2];				\
	((char *)a)[2] = ((char *)&_tmp)[1];				\
	((char *)a)[3] = ((char *)&_tmp)[0];				\
}
#endif  /* P_32_SWAP */




/************************************************************************
Read a Medit file
************************************************************************/

static flag InformationLost;	/* Flag - tells us if unrecognised data was discarded */
static void FlagInformationLoss(void)
{
    InformationLost = TRUE;
}
/************************************************************************
Get bytes...
************************************************************************/
static boolean GetByte(register FILE *f, register int *Result)
{
    if (!FileError) {
	if ((*Result = getc(f)) IS EOF) {
	    SetError(REACHED_END);
	    return FALSE;
	}
	return TRUE;	/* Ok */
    }
    else {
	return FALSE;
    }
}
/************************************************************************
Get n shorts...
************************************************************************/
static boolean GetShort(register FILE *f, register int *Result)
{
    if (!FileError) {
	int b1,b2;
	if (!GetByte(f,&b1) OR !GetByte(f,&b2)) {
	    SetError(BAD_FILE);
	    return FALSE;
	}
	else {
	    *Result = (b1<<8) + b2;
	    return TRUE;	/* Ok */
	}
    }
    else {
	return FALSE;
    }
}
/************************************************************************
Get n integers...
************************************************************************/
static boolean GetInts(register FILE *f, register int *Result, register int no)
{
    if (!FileError) {
	if (fread(Result,sizeof(int),no,f) ISNT no) {
	    SetError(BAD_FILE);
	    return FALSE;
	}
	else {
#ifdef __LITTLE_ENDIAN /* swap bytes */
		int i;
		int *p = Result;
		for(i=0; i<no; i++, p++)
		{
			P_32_SWAP(p);
		}
#endif
	    return TRUE;	/* Ok */
	}
    }
    else {
	return FALSE;
    }
}
/************************************************************************
Put n floats...
************************************************************************/
static boolean GetFloats(register FILE *f, register float *Result, register int no)
{
    if (!FileError) {
	if (fread(Result,sizeof(float),no,f) ISNT no) {
	    SetError(BAD_FILE);
	    return FALSE;
	}
	else {
#ifdef __LITTLE_ENDIAN /* swap bytes */
		int i;
		int *p = Result;
		for(i=0; i<no; i++, p++)
		{
			P_32_SWAP(p);
		}
#endif
	    return TRUE;	/* Ok */
	}
    }
    else {
	return FALSE;
    }
}
/************************************************************************
Get n reals...
************************************************************************/
#ifdef MEDIT
static boolean GetReals(register FILE *f, register real *Result, register int no)
{
    if (!FileError) {
	if (no > MaxRealBlock) {
	    ErrorMessage("Tried to read too many reals...");
	    KillProgram(ProgramError);
	}
	if (GetFloats(f,FloatWork,no)) {
	    register int i;
	    register real *rp = Result;
	    register float *fp = FloatWork;
	    for (i=0; i<no; i++) {
		*rp++ = (real)*fp++;
	    }
	    return TRUE;	/* Ok */
	}
	else {
	    return FALSE;
	}
    }
    else {
	return FALSE;
    }
}
#else
#define GetReals(f,r,n) GetFloats(f,r,n)
#endif
/************************************************************************
Put strings...
************************************************************************/
static boolean GetString(FILE *f, register char *Result, register int MaxChars)
{
    if (!FileError) {
	if (!fgets(Result,MaxChars,f)) {
	    SetError(BAD_STRING);
	    return FALSE;
	}
	Result += strlen(Result);
	while (*(Result-1) IS '\n') {
	    *--Result = '\0';
	}
	return TRUE;			/* Ok */
    }
    else {
	return FALSE;
    }
}
/************************************************************************
Skip a bit of data
************************************************************************/
static boolean SkipData(FILE *f, int bytes)
{
    int i,trash;
    for (i=0; i<bytes; i++) {
	ifbad(GetByte(f,&trash)) return FALSE;
    }
    return TRUE;
}
static boolean GetSize(FILE *f, int *n)
{
    ifbad(GetByte(f,n)) return FALSE;
    if (*n IS 0) {
	ifbad(GetInts(f,n,1)) return FALSE;
    }
    return TRUE;
}
static boolean SkipFuture(FILE *f)
{
    int n;
    ifbad(GetSize(f,&n)) return FALSE;	/* Suss size of data... */
    ifbad(SkipData(f,n)) return FALSE;	/* Skip that much */
    return TRUE;
}
/************************************************************************
Change the coordinate system of old files
************************************************************************/
static void TwistReal(register real *n)
{
    if (Compatibility < 301) {
	register real temp = n[Y];
	n[Y] = -n[Z];
	n[Z] = temp;
    }
}
static void TwistFloat(register float *n)
{
    if (Compatibility < 301) {
	register float temp = n[Y];
	n[Y] = -n[Z];
	n[Z] = temp;
    }
}
static boolean GetMatrix(FILE *f, MatrixPtr m)
{
    register int i;
    register real temp;

    if (!GetReals(f,m[X],4)) return FALSE;
    if (!GetReals(f,m[Y],4)) return FALSE;
    if (!GetReals(f,m[Z],4)) return FALSE;
    if (!GetReals(f,m[W],4)) return FALSE;

    if (Compatibility < 301) {
	TwistReal(m[X]);
	TwistReal(m[Y]);
	TwistReal(m[Z]);
	TwistReal(m[W]);
	for (i=0; i<XYZ; i++) {
	    temp = m[Y][i];
	    m[Y][i] = -m[Z][i];
	    m[Z][i] = temp;
	}
    }
    return TRUE;
}
/************************************************************************
Materials
************************************************************************/
static int NoFileMaterials = 0;
static MaterialPtr *FileMaterial;
static void ReadMaterial(FILE *f, MaterialPtr m)
{
    int token;
    char name[MaxName];
    while (GetByte(f,&token)) {
	switch (token) {
	    case BlockEnd:	return;
	    case FMName:	GetStuff(GetString(f,name,MaxName));
				RenameMaterial(m,name);			break;
	    case FMAmbient:	GetStuff(GetFloats(f,m->Ambient,3));	break;
	    case FMDiffuse:	GetStuff(GetFloats(f,m->Diffuse,3));	break;
	    case FMSpecular:	GetStuff(GetFloats(f,m->Specular,3));	break;
	    case FMEmissive:	GetStuff(GetFloats(f,m->Emissive,3));	break;
	    case FMShine:	GetStuff(GetFloats(f,&m->Shine,1));	break;
	    case FMAlpha:	GetStuff(GetFloats(f,&m->Alpha,1));	break;
	    default:		if (!SkipFuture(f)) {
				    SetError(BAD_MATERIAL);
				}
				else {
				    FlagInformationLoss();
				}
	}
    }
}
#define CopyMaterial(dest,source) memcpy(dest,source,sizeof(Material))
static void ReadMaterials(FILE *f)
{
    MaterialPtr m;
    int i,index,token;
    for (i=0; i<NoFileMaterials; i++) {	/* Initialise list of materials */
	FileMaterial[i] = NULL;
    }
    while (GetByte(f,&token)) {
	switch (token) {
	    case BlockEnd:	return;

	    case FMat:		if (Compatibility < 302) {
				    GetStuff(GetByte(f,&index));
				}
				else {
				    GetStuff(GetInts(f,&index,1));
				}
				m = NewMaterial();
				ReadMaterial(f,m);
				if (index >= NoFileMaterials) {
				    int newsize = index+100;
				    if (NoFileMaterials IS 0) {
					FileMaterial = malloc(newsize*sizeof(MaterialPtr));
				    }
				    else {
					FileMaterial = realloc(FileMaterial,
						newsize*sizeof(MaterialPtr));
				    }
				    for (i=NoFileMaterials; i<newsize; i++) {
					FileMaterial[i] = NULL;
				    }
				    NoFileMaterials = newsize;
				}
				FileMaterial[index] = m;
				m->Used = FALSE;
				m->Pack  = ((lcolour)(m->Diffuse[0]*255)) << 16;
				m->Pack |= ((lcolour)(m->Diffuse[1]*255)) << 8;
				m->Pack |= ((lcolour)(m->Diffuse[2]*255));
				m->Pack |= ((lcolour)(m->Alpha     *255)) << 24;
				break;

	    default:		if (!SkipFuture(f)) {
				    SetError(BAD_MATERIAL);
				}
				else {
				    FlagInformationLoss();
				}
	}
    }
}
/************************************************************************
After a file read, add the used materials to the list in memory
************************************************************************/
static void AddUsedMaterials(void)
{
    register int i;
    register MaterialPtr m;
    for (i=0; i<NoFileMaterials; i++) {
	if (m = FileMaterial[i]) {
	    if (m->Used OR (Compatibility >= 300)) {
		AddMaterial(m);		/* Material used, add it to palette */
	    }
	    else {
		FreeMaterial(m);	/* Not used, throw it away */
	    }
	}
    }
}
/************************************************************************
Subroutines for texture readers
************************************************************************/
#ifdef MEDIT
static flag MissingTextures;
void ShowTextureInfo(flag readit)
{
    if (readit) {
	OpenManual("R_FindingTextures");
    }
}
#else
static char DataFileName[MaxFileName];
static void SplitFileName(char *s, char **path, char **name)
{
    char *search,*find;
    static char workspace[MaxFileName],nullpath[] = "\0";
    strcpy(workspace,s);

    search = workspace;
    while (find = strchr(search,'/')) {
	search = ++find;
    }
    if (search IS workspace) {	/* None found... */
	*path = nullpath;
	*name = search;
    }
    else {
	*path = workspace;
	*name = search--;
	*search = '\0';		/* Split into path/name... */
    }
}
#endif

static boolean TryTexture(char *fname)
{
    if (IsAnImageFile(fname)) {
#ifdef MEDIT
	char temp[MaxFileName], *p, *n;
	strcpy(temp, fname);
	SplitFileName(temp, &p, &n);
	printf("Texture: %s\n",n);
#endif
	return TRUE;
    }
    else {
	return FALSE;
    }
}
static boolean FindImageFile(char *tname, char *fname)
{
    char *p,*n;
    register size_t i,l;
    char name[MaxFileName];
    SplitFileName(tname,&p,&n); strcpy(name,n);

    while (!strncmp(tname,"./",2)) {			/* Strip any ./ from the start of tname */
	l = strlen(tname);
	for (i=0; i<l-1; i++) {
	    tname[i] = tname[i+2];
	}
    }

    sprintf(fname,"%s/textures/%s",FilePath,name);	/* Look in the directory ./textures	*/
    if (TryTexture(fname)) {
	return TRUE;
    }

    strcpy(fname,name);					/* Look in the same directory as the model */
    if (TryTexture(fname)) {
	return TRUE;
    }

    sprintf(fname,"%s/%s",TextureDir,name);		/* Look in texture library directory	*/
    if (TryTexture(fname)) {
	return TRUE;
    }

    sprintf(fname,"%s/%s",FilePath,name);		/* Look relative to the model (../textures/ is common) */
    if (TryTexture(fname)) {
	return TRUE;
    }

    strcpy(fname,tname);				/* Try absolute file name */
    if (TryTexture(fname)) {
	return TRUE;
    }

    sprintf(fname,"%s/%s",TextureDir,tname);		/* Look relative to library directory */
    if (TryTexture(fname)) {
	return TRUE;
    }

#ifndef MEDIT
    sprintf(fname,"%s/%s",OriginalTextureDir,tname);	/* Look in library used when file was saved */
    if (TryTexture(fname)) {
	return TRUE;
    }
    sprintf(fname,"%s/%s",Medit_FilePath,name);		/* Look relative to dir used when file was last saved */
    if (TryTexture(fname)) {
	return TRUE;
    }
    sprintf(fname,"%s/textures/%s",Medit_FilePath,name);
    if (TryTexture(fname)) {
	return TRUE;
    }
    SplitFileName(DataFileName,&p,&n);			/* Look relative to model file name */
    sprintf(fname,"%s/textures/%s",p,name);
    if (TryTexture(fname)) {
	return TRUE;
    }
#endif

    fprintf(stderr, "Warning: texture %s not found\n",name);
    return FALSE;
}

/* Read in and remember texture path used when creating this file */
static void ReadTextureDir(FILE *f)
{
#ifdef MEDIT
    char junk[MaxFileName];						/* Chuck it away in Medit */
    GetString(f,junk,MaxFileName);
#else
    GetString(f,OriginalTextureDir,MaxFileName);	/* Remember it when using the library */
#endif
}
/************************************************************************
Texture reference (new style with attributes)
* We haven't done any attributes yet, this is just forward planning	*
************************************************************************/
static void ReadTextureRef(FILE *f, flag ReadAttributes)
{
    TexturePtr t;
    char tname[MaxFileName],fname[MaxFileName];
    float minfilter,magfilter;
    int id,token,clampx,clampy,fast,size;

    GetStuff(GetInts(f,&id,1));
    GetStuff(GetString(f,tname,MaxFileName));

    if (FindImageFile(tname,fname)) {
	if (id < 0) {
	    SetError(BAD_TEXTURE);
	}
	else {
	    if (t = AddTexture(fname)) {
		t->Id = id;
	    }
#ifdef MEDIT
	    else {				/* Some sort of error in the image file... */
		MissingTextures = TRUE;
	    }
#endif
	}
    }
    else {					/* Failed to find it... */
#ifdef MEDIT
	MissingTextures = TRUE;
#endif
	t = NULL;
    }

    minfilter = MT_NULL;			/* Default values for texture attributes */
    magfilter = MT_NULL;
    clampx = FALSE;
    clampy = FALSE;
    fast = FALSE;

    while (ReadAttributes AND GetByte(f,&token)) {
	switch (token) {
	    case BlockEnd:	ReadAttributes = FALSE;
				break;

	    case FMinFilter:	if (Compatibility < 300) {
				    GetStuff(GetSize(f,&size));
				}
				GetStuff(GetFloats(f,&minfilter,1));
				break;

	    case FMagFilter:	if (Compatibility < 300) {
				    GetStuff(GetSize(f,&size));
				}
				GetStuff(GetFloats(f,&magfilter,1));
				break;

	    case FFastTex:	if (Compatibility < 300) {
				    GetStuff(GetSize(f,&size));
				}
				GetStuff(GetInts(f,&fast,1));
				break;

	    case FOldClamp:	GetStuff(GetSize(f,&size));
				GetStuff(GetInts(f,&clampx,1));
				clampy = clampx;
				break;

	    case FClampTexX:	GetStuff(GetInts(f,&clampx,1));
				clampy = clampx;
				break;

	    case FClampTexY:	GetStuff(GetInts(f,&clampy,1));
				clampy = clampx;
				break;

	    default:		if (!SkipFuture(f)) {
				    SetError(BAD_TEXTURE);
				}
				else {
				    FlagInformationLoss();
				}
	}
    }
    if (t) {
	t->MinFilter = minfilter;
	t->MagFilter = magfilter;
	t->ClampX = clampx;
	t->ClampY = clampy;
	t->Fast = fast;
    }
}
/************************************************************************
Vertices...
************************************************************************/
static int VListSize;
#ifdef MEDIT
static VertexPtr *VListBase;
static void ReadVertices(FILE *f)
{
    int i,count;
    float p[XYZ];
    GetStuff(GetInts(f,&count,1));
    if (count > VListSize) {
	if (!VListSize) {
	    VListBase = Allocate(count*sizeof(VertexPtr));
	}
	else {
	    VListBase = Reallocate(VListBase,count*sizeof(VertexPtr));
	}
	VListSize = count;
    }
    for (i=0; i<count; i++) {
	GetFloats(f,p,XYZ);
	TwistFloat(p);
	VListBase[i] = AddVertex(p[X],p[Y],p[Z]);
    }
}
#else
static real (*VListBase)[XYZ];
static void ReadVertices(FILE *f)
{
    int i,count;
    GetStuff(GetInts(f,&count,1));
    if (count > VListSize) {
	if (!VListSize) {
	    VListBase = Allocate(count*XYZ*sizeof(real));
	}
	else {
	    VListBase = Reallocate(VListBase,count*XYZ*sizeof(real));
	}
	VListSize = count;
    }
    for (i=0; i<count; i++) {
	GetFloats(f,VListBase[i],XYZ);
	TwistFloat(VListBase[i]);
    }
}
#endif
/************************************************************************
Vertices...
************************************************************************/
#ifndef MEDIT
static int FoundVertexNormal;
#endif
static void ReadVertex(FILE *f, PolygonPtr p)
{
#ifdef MEDIT
    VertexPtr v;
#endif
    real c[XYZ];
    int token,index,colour;
    flag HadCoord = FALSE;
    PolygonBeadPtr lastpb = NULL;

    while (GetByte(f,&token)) {
	switch (token) {
	    case BlockEnd:	return;

	    case FVCoord:   if (HadCoord) {
				SetError(BAD_VERTEX);
			    }
			    else {
				HadCoord = TRUE;
				GetStuff(GetReals(f,c,XYZ));
				TwistReal(c);
#ifdef MEDIT
				v = AddVertex(c[X],c[Y],c[Z]);
				lastpb = AddPolygonBead(&(p->FirstBead),v);
				lastpb->Owner = p;
#else
				lastpb = AddPolygonBead(&(p->FirstBead),c);
#endif
			    }
			    break;

	    case FVIndex:   if (HadCoord) {
				SetError(BAD_VERTEX);
			    }
			    else {
				if (!VListBase) {
				    SetError(BAD_VERTEX);
				}
				HadCoord = TRUE;
				GetStuff(GetInts(f,&index,1));
				if ((index < 0) OR (index >= VListSize)) {
				    SetError(BAD_VERTEX_INDEX);
				}
				else {
#ifdef MEDIT
				    v = VListBase[index];
				    lastpb = AddPolygonBead(&(p->FirstBead),v);
				    lastpb->Owner = p;
#else
				lastpb = AddPolygonBead(&(p->FirstBead),VListBase[index]);
#endif
				}
			    }
			    break;

	    case FVTexture: if (lastpb) {
				GetStuff(GetFloats(f,lastpb->TextureCoord,XY));
			    }
			    else {
				SetError(BAD_VERTEX);
			    }
			    break;

	    case FVNormal:  if (lastpb) {
				GetStuff(GetFloats(f,lastpb->Normal,XYZ));
				TwistFloat(lastpb->Normal);
			    }
			    else {
				SetError(BAD_VERTEX);
			    }
#ifndef MEDIT
			    FoundVertexNormal = TRUE;
#endif
			    break;

	    case FVColour:  GetStuff(GetInts(f,&colour,1));
			    break;

	    default:	    if (!SkipFuture(f)) {
				SetError(BAD_VERTEX);
			    }
			    else {
				FlagInformationLoss();
			    }
	}
    }
}
/************************************************************************
Polygons...
************************************************************************/
static void ReadPolygon(FILE *f, PolygonPtr parent)
{
    int token,data;
#ifndef MEDIT
    register flag HadNormal = FALSE;
#endif
    register PolygonPtr p = AddPolygon(NULL,parent);
    p->Flag = -1;	/* Flag is used as texture id (-1 = no texture) */

    while (GetByte(f,&token)) {
	switch (token) {
#ifdef MEDIT
	    case BlockEnd:	return;
#else
	    case BlockEnd:	switch (p->Lighting) {
				    case Lit:	    if (!HadNormal) {
							MeditFileHadNormals = FALSE;
						    }
						    break;
				    case Smooth:    if (!FoundVertexNormal) {
							MeditFileHadNormals = FALSE;
						    }
						    break;
				}
				return;
#endif

	    case FPolygon:	ReadPolygon(f,p);
				break;

	    case FPMaterial:	if (Compatibility < 300) {
				    GetStuff(GetByte(f,&data));
				}
				else {
				    GetStuff(GetShort(f,&data));
				}
				p->NormalMaterial = FileMaterial[data];
				FileMaterial[data]->Used = TRUE;
#ifndef MEDIT
				p->Colour = FileMaterial[data]->Pack;
#endif
				break;

	    case FPLighting:	GetStuff(GetByte(f,&data));
				p->Lighting = data;
				if (p->Lighting > Smooth) {
				    SetError(BAD_LIGHTING);
				}
				break;

	    case FPNormal:	GetStuff(GetFloats(f,p->Normal,XYZ));
				TwistFloat(p->Normal);
#ifndef MEDIT
				HadNormal = TRUE;
#endif
				break;

	    case FPTexture:	GetStuff(GetInts(f,&(p->Flag),1));
				break;

	    case FPTextureMat:	if (Compatibility < 300) {
				    GetStuff(GetByte(f,&data));
				}
				else {
				    GetStuff(GetShort(f,&data));
				}
				p->TextureMaterial = FileMaterial[data];
				FileMaterial[data]->Used = TRUE;
				break;

	    case FPLightGroup:	GetStuff(GetInts(f,&(p->LightGroup),1));
				break;

	    case FPFlags:	GetStuff(GetByte(f,&data));
				p->Flags = data;
				break;

	    case FVertex:	ReadVertex(f,p);
				break;

	    default:		if (!SkipFuture(f)) {
				    SetError(BAD_POLYGON);
				}
				else {
				    FlagInformationLoss();
				}
	}
    }
}
/************************************************************************
Sub objects...
************************************************************************/
static void ReadSubObject(FILE *f)
{
    int token;
    char name[MaxName];

    GetString(f,name,MaxName);
    AddSubObject(name);

    while (GetByte(f,&token)) {
	switch (token) {
	    case BlockEnd:	return;

	    case FSubObjectId:	GetInts(f,&(*SelectedSubObject())->Id,1);
				break;

	    case FVList:	ReadVertices(f);
				break;

	    case FPolygon:	ReadPolygon(f,NULL);
				break;

	    default:		if (!SkipFuture(f)) {
				    SetError(BAD_SUB_OBJECT);
				}
				else {
				    FlagInformationLoss();
				}
	}
    }
}

/************************************************************************
Levels of detail
************************************************************************/
static void ReadLod(FILE *f)
{
    flag found;
    float switchout;
    char name[MaxName];
    int token,id, size, value;

    LodBeadPtr lb;
    SubObjectPtr s;

    GetString(f,name,MaxName);
    AddLod(name);
    lb = NULL;

    while (GetByte(f,&token)) {
	switch (token) {
	    case BlockEnd:		return;

	    case FSwitchOut:	GetFloats(f,&switchout,1);
				switchout = fabs(switchout);
				(*SelectedLod())->SwitchOut = switchout;
				break;

	    /* For backwards compatibility only */
	    case FSubObject:	found = FALSE;
				GetString(f,name,MaxName);
				LoopThroughAllSubObjects(s) {
				    if (!strcmp(s->Name,name)) {
					lb = AddLodBead(s);
					found = TRUE;
					break;
				    }
				}
				if (!found) {
				    SetError(SUB_OBJECT_NOT_FOUND);
				}
				break;

	    case FBeadVisible:	GetStuff(GetInts(f, &value, 1));
#ifdef MEDIT
				if (lb) {
				    lb->Visible = value;
				}
#endif
				break;

	    case FSubObjectId:	GetStuff(GetInts(f,&id,1));
				LoopThroughAllSubObjects(s) {
				    if (s->Id IS id) {
					lb = AddLodBead(s);
					found = TRUE;
					break;
				    }
				}
				if (!found) {
				    SetError(SUB_OBJECT_NOT_FOUND);
				}
				break;
    
	    default:		if (!SkipFuture(f)) {
				    SetError(BAD_LOD);
				}
				else {
				    FlagInformationLoss();
				}
	}
    }
}
/************************************************************************
Work pointers to polygon textures...
************************************************************************/
static void FindMaxTexture(PolygonPtr p, void *max)
{
    if (p->Flag > *(int*)max) {
	*(int*)max = p->Flag;
    }
}
static void LookUpTexture(PolygonPtr p, void *lookup)
{
    p->Texture = *((TexturePtr*)lookup + p->Flag);
}
void SussTextures(void)
{
    int i,max;
    PolygonPtr p;
    SubObjectPtr s;
    TexturePtr t,*lookup;

    max = 0;				/* Find out the max texture id */
    LoopThroughAllSubObjects(s) {
	LoopThroughSubObjectsPolygons(s,p) {
	    ScanSubPolygons(p,FindMaxTexture,&max);
	    if (p->Flag > max) {
		max = p->Flag;
	    }
	}
    }
    LoopThroughAllTextures(t) {
	if (t->Id > max) {
	    max = t->Id;
	}
    }
    max += 2;

    lookup = Allocate(max * sizeof(TexturePtr));	/* Build an Id to Texture lookup table */
    for (i=0; i<max; i++) {
	lookup[i] = NULL;
    }
    lookup++;			/* -1 = No texture... */
    LoopThroughAllTextures(t) {
	lookup[t->Id] = t;
    }

    LoopThroughAllSubObjects(s) {
	LoopThroughSubObjectsPolygons(s,p) {
	    ScanSubPolygons(p,LookUpTexture,lookup);
	    p->Texture = lookup[p->Flag];
	}
    }
    Free(--lookup);
}
/************************************************************************
Read a view structure
************************************************************************/
#ifdef MEDIT
static void ReadView(FILE *f)
{
    View v;
    int id,token;

    GetInts(f,&id,1);
    v.HasCentreOfGravity = FALSE;
    
    while (GetByte(f,&token)) {
	if (token IS BlockEnd) {
	    break;
	}
	else switch (token) {
	    case FViewThreeDee:	    GetStuff(GetByte(f,&v.ThreeDee));
				    break;
	    case FViewAxis:	    GetStuff(GetByte(f,&v.Axis));
				    break;
	    case FViewViewMatrix:   GetStuff(GetMatrix(f,v.ViewMatrix));
				    break;
	    case FViewProjectMatrix:GetStuff(GetMatrix(f,v.ProjectionMatrix));
				    break;
	    case FViewGridMatrix:   GetStuff(GetMatrix(f,v.GridMatrix));
				    break;
	    case FViewMagnification:GetStuff(GetReals(f,&v.Magnification,1));
				    break;
	    case FViewCOG:	    v.HasCentreOfGravity = TRUE;
				    GetStuff(GetReals(f,v.CentreOfGravity,3));
				    break;
	    default:		    if (!SkipFuture(f)) {
					SetError(BAD_VIEW);
				    }
				    else {
					FlagInformationLoss();
				    }
	}
    }
    v.Valid = v.Reproject = TRUE;
    if ((id >= 0) AND (id < NoViews)) {			/* Object views */
	CopyView(DefaultObject2D(id),&v);
    }
    else if (id IS 0x3d) {
	CopyView(DefaultObject3D(),&v);
    }
    else if ((id >= 0x100) AND (id < (0x100+MaxViewports))) {
	CopyView(ObjectView(id-0x100),&v);
    }

    if (!Mergeing) {					/* Scene views */
	if ((id >= 0x200) AND (id < (0x200+NoViews))) {
	    CopyView(DefaultScene2D+(id-0x200),&v);
	}
	else if (id IS 0x23d) {
	    CopyView(&DefaultScene3D,&v);
	}
	else if ((id >= 0x300) AND (id < (0x300+MaxViewports))) {
	    CopyView(SceneView+(id-0x300),&v);
	}
    }
}
#else
static void ReadView(FILE *f)
{
    int token;
    GetInts(f,&token,1);
    while (GetByte(f,&token)) {
	if (token IS BlockEnd) {
	    break;
	}
	else switch (token) {
	    case FViewThreeDee:		GetStuff(SkipData(f,1));		break;
	    case FViewAxis:		GetStuff(SkipData(f,1));		break;
	    case FViewViewMatrix:	GetStuff(SkipData(f,16*sizeof(float)));	break;
	    case FViewProjectMatrix:	GetStuff(SkipData(f,16*sizeof(float)));	break;
	    case FViewGridMatrix:	GetStuff(SkipData(f,16*sizeof(float)));	break;
	    case FViewMagnification:	GetStuff(SkipData(f,sizeof(float)));	break;
	    case FViewCOG:		GetStuff(SkipData(f,3*sizeof(float)));	break;
	    default:			if (!SkipFuture(f)) {
					    SetError(BAD_VIEW);
					}
					else {
					    FlagInformationLoss();
					}
	}
    }
}
#endif
/************************************************************************
Read objects...
************************************************************************/
static void ReadObject(FILE *f)
{
    int token;
#ifndef MEDIT
    int discard;
#endif
    char name[MaxFileName];
    flag GotSubObject,GotLod;

    GetString(f,name,MaxFileName);
#ifdef MEDIT
    AddObject(name,FALSE, FALSE);
#else
    AddObject(name);
#endif

    GotSubObject = GotLod = FALSE;
    while (GetByte(f,&token)) {
	switch (token) {
	case BlockEnd:	    if ((Compatibility < 4.0) AND (!GotSubObject OR !GotLod)) {
				SetError(BAD_FILE);
			    }
#ifdef MEDIT
			    else {
				printf("Object: %s\n",name);
				SussTextures();			/* MUST be first... */
				DeleteDuplicateVertices();
				SortObject();
				*SelectedLod() = *FirstLod();
				*ConnectionsOK() = FALSE;
				MaterialsNeedDefining = TRUE;
				CurrentLighting = Lit;
			    }
#else
			    else {
				SussTextures();
			    }
#endif
			    return;

	case FObjectId:	    GetStuff(GetInts(f,ObjectId(),1));
			    break;

	case FVList:	    ReadVertices(f);
			    break;

	case FSubObject:    ReadSubObject(f);
			    GotSubObject = TRUE;
			    break;

	case FLod:	    ReadLod(f);
			    GotLod = TRUE;
			    break;

	case FView:	    ReadView(f);
			    break;
#ifdef MEDIT
	case FGridIndex:    GetStuff(GetInts(f,GridIndex(),1));
			    break;

	case FGridSteps:    GetStuff(GetInts(f,GridSteps(),1));
			    break;
#else
	case FGridIndex:    GetStuff(GetInts(f,&discard,1));
			    break;

	case FGridSteps:    GetStuff(GetInts(f,&discard,1));
			    break;
#endif
	case FObjectAttributes:	GetStuff(GetInts(f,ObjectAttributes(),1));
			    break;

	default:	    if (!SkipFuture(f)) {
				SetError(BAD_FILE);
			    }
			    else {
				FlagInformationLoss();
			    }
	}
    }
}
/************************************************************************
Read the scene tree...
************************************************************************/
static void ReadTree(FILE *f, BranchPtr b)
{
    MeditObjectPtr o;
    int token, object, n;
    char name[MaxFileName];

    GetString(f,name,MaxFileName);
    b = AddBranch(b,ACROSS);
    BranchName(b) = CopyOfString(name);

    while (GetByte(f,&token)) {
	switch (token) {
	case FScenePop:	    return;

	case FScenePush:    ReadTree(f,b);
			    break;

	case FSceneBranch:  if (GetString(f,name,MaxFileName)) {
				b = AddBranch(b,DOWN);
				BranchName(b) = CopyOfString(name);
			    }
			    else {
				SetError(BAD_TREE);
			    }
			    break;

	case FSceneObject:  GetStuff(GetInts(f,&object,1));
			    LoopThroughAllObjects(o) {
				if (o->_Id IS object) {
				    BranchObject(b) = o;
				    break;
				}
			    }
			    if (!o) {
				SetError(BAD_OBJECT);
			    }
			    break;

	case FSceneMatrix:  GetStuff(GetMatrix(f,BranchMatrix(b)));
			    break;

	case FSceneVisible: GetStuff(GetInts(f, &n, 1));
#ifdef MEDIT
			    b->Visible = n;
#endif
			    break;

	case FSceneFolded:  GetStuff(GetInts(f, &n, 1));
#ifdef MEDIT
			    b->Folded = n;
#endif
			    break;

	default:	    if (!SkipFuture(f)) {
				SetError(BAD_TREE);
			    }
			    else {
				FlagInformationLoss();
			    }
	}
    }
}
/************************************************************************
Read data items...
************************************************************************/
#ifdef MEDIT
static int FileObjectId;
#endif
static void ReadDataItem(FILE *f)
{
    int item,size,cid;

    GetStuff(GetByte(f,&item));
    GetStuff(GetSize(f,&size));

#ifdef MEDIT
    if (Mergeing) {
	SkipData(f,size);
    }
    else switch (item) {
	int n;
	case DCurrentObject:	GetStuff(GetInts(f,&FileObjectId,1));	break;
	case DSceneGridIndex:	GetStuff(GetInts(f,&SceneGridIndex,1));	break;
	case DSceneGridSteps:	GetStuff(GetInts(f,&SceneGridSteps,1));	break;

	case DNoViewports:	GetStuff(GetInts(f,&n,1));
				if (n ISNT NoViewports) {
				    ChangeNoViewports();
				}					break;
	case DHideSubObjects:	GetStuff(GetInts(f,&HideSubObjects,1));	break;
	case DHideScene:	GetStuff(GetInts(f,&HideScene,1));	break;

	case DObjectIsolated:	GetStuff(GetInts(f,&ObjectIsolated,1));	break;

	case DFileCreateId:	GetStuff(GetInts(f,&cid,1));
				FileCreateId = cid;			break;
	case DFilePublicDomain:	GetStuff(GetInts(f,&FilePublicDomain,1));break;

	case DFileTitle:	GetString(f,FileTitle,CommentSize);	break;
	case DFileAuthor:	GetString(f,FileAuthor,CommentSize);	break;
	case DFileCompany:	GetString(f,FileCompany,CommentSize);	break;
	case DFileContact:	GetString(f,FileContact,CommentSize);	break;
	case DFileComment1:	GetString(f,FileComment1,CommentSize);	break;
	case DFileComment2:	GetString(f,FileComment2,CommentSize);	break;
	case DFileComment3:	GetString(f,FileComment3,CommentSize);	break;
	case DFileComment4:	GetString(f,FileComment4,CommentSize);	break;

	case DOriginalFilePath:	SkipData(f,size);			break;

	/* Unknown in this revision, throw it away */
	default:		SkipData(f,size);
    }
#else
    switch (item) {
	case DOriginalFilePath:	GetString(f,Medit_FilePath,MaxFileName);break;
	default:		SkipData(f,size);
    }
#endif
}
/************************************************************************
Read a medit object file...
************************************************************************/
static void MeditRead(FILE *f)
{
    int token;
#ifdef MEDIT
    MeditObjectPtr o;

    ResetStorage();
    FileObjectId = -1;
#endif
    VListSize = 0;
    while (GetByte(f,&token)) {
	switch (token) {
	    case BlockEnd:	AddUsedMaterials();
#ifdef MEDIT
				if (!Mergeing) {
					*SelectedBranch() = NULL;
					LoopThroughAllObjects(o) {
					    if (ObjectsId(o) IS FileObjectId) {
						*CurrentObject() = o;
						break;
					    }
					}
					if (!o) {	/* Not found... */
					    *CurrentObject() = *FirstObject();
					}
					*SelectedLod() = *FirstLod();
					*CurrentTexture() = *FirstTexture();
					DefineAllTextures();
				}
				SortObjectList();
#endif
				return;
	    case FMaterials:	ReadMaterials(f);
				break;
	    case FTextureDir:	ReadTextureDir(f);
				break;
	    case FTexture:	ReadTextureRef(f,FALSE);	/* Backwards compatibility */
				break;
	    case FTextureRef:	ReadTextureRef(f,TRUE);		/* Modern version of the above */
				break;
	    case FObject:	ReadObject(f);
				break;
	    case FScenePush:	ReadTree(f,*Scene());
				break;
	    case FData:		ReadDataItem(f);
				break;
	    case FView:		ReadView(f);
				break;

	    default:		if (!SkipFuture(f)) {
				    SetError(BAD_THING);
				}
				else {
				    FlagInformationLoss();
				}
	}
    }
}
#define HeaderMax 200
boolean ReadMedit(char *fname)
{
    FILE *f;
    int type;
    float compat;
    char Header[HeaderMax];
    char *path,*name,n[MaxFileName];

    InformationLost = FALSE;
#ifdef MEDIT
    MissingTextures = FALSE;
#else
    strcpy(n,fname);
    SplitFileName(n,&path,&name);
    strcpy(FilePath, path);
    strcpy(DataFileName, fname);
#endif
    if (!(f = GetReadyForRead(fname))) {
	return FALSE;
    }

    if (fgets(Header,HeaderMax,f)) {
	if (sscanf(Header,"Compatibility: %f",&compat) IS 1) {
	    Compatibility = compat*100;
#ifdef MEDIT
	    if (Compatibility IS 100) {	/* Internal use only. There will never */
		ReadMedit100(f);	/*  be any 1.00 files in the real world */
	    }
	    else
#endif
	    if ((Compatibility >= 200) AND (Compatibility <= 305)) {
		if (GetByte(f,&type) AND (type IS BinaryFile)) {
		    MeditRead(f);
		    if (VListSize) {
			Free(VListBase);
		    }
		}
	    }
	    else {
		SetError(BAD_REVISION);
	    }
	}
	else {
	    SetError(UNKNOWN_FILE);
	}
    }
    FinishReading(f);
#ifdef MEDIT
    if (!FileError AND InformationLost) {
	fprintf(stderr,"Medit warning: Information lost!\007\n");
	fprintf(stderr,"This file was created with a more recent version\n");
	fprintf(stderr,"of the Medit modelling software, information\n");
	fprintf(stderr,"was discarded when the file was loaded...\n");
    }
    if (MissingTextures) {
	DoOkCancel("Some textures could not be found.\n\n"
	    "Click on Ok to read the manual on how to correct this problem.",ShowTextureInfo);
    }
#endif
    return !FileError;
}

