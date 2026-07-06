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
Write a .medit file
************************************************************************/

/************************************************************************
Put bytes...
************************************************************************/
static boolean PutByte(register FILE *f, register int b)
{
    if (!FileError) {
	if (putc(b,f) IS EOF) {
	    SetError(-1);
	    return FALSE;
	}
	return TRUE;	/* Ok */
    }
    else {
	return FALSE;
    }
}
/************************************************************************
Put n integers...
************************************************************************/
static boolean PutInts(register FILE *f, register int *r, register int no)
{
    if (!FileError) {
	if (fwrite(r,sizeof(int),no,f) ISNT no) {
	    SetError(-1);
	    return FALSE;
	}
	else {
	    return TRUE;	/* Ok */
	}
    }
    else {
	return FALSE;
    }
}
/************************************************************************
Put n shorts...
************************************************************************/
static boolean PutShort(register FILE *f, register int n)
{
    if (!FileError) {
	if (!PutByte(f,n>>8) OR !PutByte(f,n)) {
	    SetError(-1);
	    return FALSE;
	}
	else {
	    return TRUE;	/* Ok */
	}
    }
    else {
	return FALSE;
    }
}
/************************************************************************
Get n floats...
************************************************************************/
static boolean PutFloats(register FILE *f, register float *r, register int no)
{
    if (!FileError) {
	if (fwrite(r,sizeof(float),no,f) ISNT no) {
	    SetError(-1);
	    return FALSE;
	}
	else {
	    return TRUE;	/* Ok */
	}
    }
    else {
	return FALSE;
    }
}
/************************************************************************
Put n reals...
************************************************************************/
#ifdef MEDIT
#define MaxRealBlock 16
static float FloatWork[MaxRealBlock];
static boolean PutReals(register FILE *f, register real *r, register int no)
{
    if (!FileError) {
	register int i;
	register real *rp = r;
	register float *fp = FloatWork;
	if (no > MaxRealBlock) {
	    ErrorMessage("Tried to write too many reals...");
	    KillProgram(ProgramError);
	}
	for (i=0; i<no; i++) {
	    *fp++ = (float)*rp++;
	}
	if (PutFloats(f,FloatWork,no)) {
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
#define PutReals(f,r,n) PutFloats(f,r,n)
#endif
/************************************************************************
Put strings...
************************************************************************/
static boolean PutString(FILE *f, char *s)
{
    if (!FileError) {
	while (*s ISNT '\0') {
	    ifbad (PutByte(f,*s++)) {
		return FALSE;
	    }
	}
	ifbad (PutByte(f,'\n')) {
	    return FALSE;
	}
	return TRUE;			/* Ok */
    }
    else {
	return FALSE;
    }
}
/************************************************************************
Write a matrix
************************************************************************/
static void PutMatrix(FILE *f, MatrixPtr m)
{
    if (!FileError) {
	PutReals(f,m[X],4);
	PutReals(f,m[Y],4);
	PutReals(f,m[Z],4);
	PutReals(f,m[W],4);
    }
}
/************************************************************************
Write out the size of something
************************************************************************/
static void PutSize(FILE *f, int size)
{
    if (size > 255) {
	PutByte(f,0);
	PutInts(f,&size,1);
    }
    else {
	PutByte(f,size);
    }
}
/************************************************************************
Materials
************************************************************************/
static void WriteMaterials(FILE *f)
{
    register int index = 0;
    register MaterialPtr m;

    PutByte(f,FMaterials);
    m = *FirstMaterial();
    while (m) {
	MaterialId(m) = index++;
	PutByte(f,FMat); PutInts(f,&MaterialId(m),1);
	    PutByte(f,FMName);	PutString(f,m->Name);
	    PutByte(f,FMAmbient);	PutFloats(f,m->Ambient,3);
	    PutByte(f,FMDiffuse);	PutFloats(f,m->Diffuse,3);
	    PutByte(f,FMSpecular);	PutFloats(f,m->Specular,3);
	    PutByte(f,FMEmissive);	PutFloats(f,m->Emissive,3);
	    PutByte(f,FMShine);	PutFloats(f,&m->Shine,1);
	    PutByte(f,FMAlpha);	PutFloats(f,&m->Alpha,1);
	PutByte(f,BlockEnd);
	m = NextMaterial(m);
    }
    PutByte(f,BlockEnd);
}
/************************************************************************
Textures
************************************************************************/
#ifdef MEDIT
static void WriteDataString(FILE *f, int token, char *string)
{
    PutByte(f,FData);
    PutByte(f,token);
    PutSize(f,strlen(string)+1);
    PutString(f,string);
}
#endif
void WriteTextures(FILE *f)
{
    size_t l;
    int id = 0;
    TexturePtr t;
    char tname[MaxFileName];

#ifdef MEDIT
    PutByte(f,FTextureDir); PutString(f,TextureDir);
    WriteDataString(f, DOriginalFilePath, FilePath);
#endif
    LoopThroughAllTextures(t) {
	TextureId(t) = id++;
	PutByte(f,FTextureRef);
	PutInts(f,&TextureId(t),1);
	if ((l=strlen(FilePath)) AND !strncmp(TextureName(t),FilePath,l)) {
	    sprintf(tname,".%s",TextureName(t)+strlen(FilePath));
	}
	else if ((l=strlen(TextureDir)) AND !strncmp(TextureName(t),TextureDir,l)) {
	    sprintf(tname,".%s",TextureName(t)+strlen(TextureDir));
	}
	else {
	    strcpy(tname,TextureName(t));
	}
#ifdef MEDIT
	DEBUG"Wrote texture as:%s\n",tname);
#endif
	PutString(f,tname);

	PutByte(f,FMinFilter);	PutFloats(f,&(t->MinFilter),1);
	PutByte(f,FMagFilter);	PutFloats(f,&(t->MagFilter),1);
	PutByte(f,FFastTex);	PutInts(f,&(t->Fast),1);
	PutByte(f,FClampTexX);	PutInts(f,&(t->ClampX),1);
	PutByte(f,FClampTexY);	PutInts(f,&(t->ClampY),1);

	PutByte(f,BlockEnd);
    }
}
/************************************************************************
Vertices...
************************************************************************/
#ifdef MEDIT
static void WriteVertices(FILE *f)
{
    VertexPtr v;
    int count = 0;
    LoopThroughAllVertices(v) {
	v->Flag = count++;
    }
    PutByte(f,FVList);
    PutInts(f,&count,1);
    LoopThroughAllVertices(v) {
	PutReals(f,v->Pos,XYZ);
    }
}
#endif
/************************************************************************
Polygons...
************************************************************************/
static void WritePolygon(FILE *f, PolygonPtr p)
{
    TexturePtr t;
    PolygonPtr sub;
    PolygonBeadPtr pb;

#ifdef MEDIT
    if (IsSubPolygon(p)) {
	p->Flags |=  PChild;
    }
    else {
	p->Flags &= ~PChild;
    }
#endif
    PutByte(f,FPolygon);
    PutByte(f,FPMaterial); PutShort(f,p->NormalMaterial->Id);
    PutByte(f,FPLighting); PutByte(f,p->Lighting);
#ifdef MEDIT
    if (p->Lighting IS Lit) {
	PutByte(f,FPNormal);		PutFloats(f,p->Normal,XYZ);
    }
#endif
    if (t = p->Texture) {
	PutByte(f,FPTexture);		PutInts(f,&(t->Id),1);
	PutByte(f,FPTextureMat);	PutShort(f,p->TextureMaterial->Id);
    }
    if (p->LightGroup ISNT 0) {
	PutByte(f,FPLightGroup);	PutInts(f,&(p->LightGroup),1);
    }
    if (p->Flags) {
	PutByte(f,FPFlags);		PutByte(f,p->Flags);
    }
    LoopThroughPolygonsBeads(p,pb) {
	PutByte(f,FVertex);
#ifdef MEDIT
	    PutByte(f,FVIndex);	PutInts(f,&pb->Vert->Flag,1);
#else
	    PutByte(f,FVCoord);	PutReals(f,pb->Coordinate,XYZ);
#endif
#ifdef MEDIT
	    if (p->Lighting IS Smooth) {
		PutByte(f,FVNormal);
		PutFloats(f,pb->Normal,XYZ);
	    }
#endif
	    if (t) {
		PutByte(f,FVTexture);
		PutFloats(f,pb->TextureCoord,XY);
	    }
	PutByte(f,BlockEnd);
    }
    if (sub = p->Child) {
	while (sub) {
	    WritePolygon(f,sub);
	    sub = sub->NextChild;
	}
    }
    PutByte(f,BlockEnd);
}
/************************************************************************
Sub objects...
************************************************************************/
static void WriteSubObject(FILE *f, SubObjectPtr s)
{
    PolygonPtr p;

    PutByte(f,FSubObject);
    PutString(f,s->Name);
    PutByte(f,FSubObjectId); PutInts(f,&s->Id,1);

    LoopThroughSubObjectsPolygons(s,p) {
	WritePolygon(f,p);
    }
    PutByte(f,BlockEnd);
}
/************************************************************************
Levels of detail
************************************************************************/
static void WriteLod(FILE *f, LodPtr l)
{
    LodBeadPtr lb;
    float s = l->SwitchOut;
    PutByte(f,FLod); 		PutString(f,l->Name);
    PutByte(f,FSwitchOut);	PutFloats(f,&s,1);
    LoopThroughLodsBeads(l,lb) {
	PutByte(f,FSubObjectId); PutInts(f,&lb->SubObject->Id,1);
#ifdef MEDIT
	PutByte(f,FBeadVisible); PutInts(f,&lb->Visible,1);
#endif
    }
    PutByte(f,BlockEnd);
}
/************************************************************************
Write out a view structure
************************************************************************/
#ifdef MEDIT
static void WriteView(FILE *f, int id, ViewPtr view)
{
    if (view->Valid) {
	PutByte(f,FView);			PutInts(f,&id,1);
	PutByte(f,FViewThreeDee);		PutByte(f,view->ThreeDee);
	PutByte(f,FViewAxis);			PutByte(f,view->Axis);
	PutByte(f,FViewViewMatrix);		PutMatrix(f,view->ViewMatrix);
	PutByte(f,FViewProjectMatrix);		PutMatrix(f,view->ProjectionMatrix);
	PutByte(f,FViewGridMatrix);		PutMatrix(f,view->GridMatrix);
	PutByte(f,FViewMagnification);		PutReals(f,&view->Magnification,1);
	if (view->HasCentreOfGravity) {
	    PutByte(f,FViewCOG);		PutReals(f,view->CentreOfGravity,3);
	}	
	PutByte(f,BlockEnd);
    }
}
#endif
/************************************************************************
Write objects...
************************************************************************/
static void WriteObject(FILE *f)
{
#ifdef MEDIT
    int i;
#endif
    LodPtr l;
    SubObjectPtr s;
    int SubObjectId = 0;

#ifdef MEDIT
    CleanUpObject();
#endif
    PutByte(f,FObject);
	PutString(f,*ObjectName());
	PutByte(f,FObjectId);		PutInts(f,ObjectId(),1);
	PutByte(f,FObjectAttributes);	PutInts(f,ObjectAttributes(),1);

#ifdef MEDIT
	WriteView(f,0x3d,DefaultObject3D());
	for (i=0; i<NoViews; i++) {
	    WriteView(f,i,DefaultObject2D(i));
	}
	for (i=0; i<MaxViewports; i++) {
	    WriteView(f,0x100+i,ObjectView(i));
	}
	PutByte(f,FGridIndex);		PutInts(f,GridIndex(),1);
	PutByte(f,FGridSteps);		PutInts(f,GridSteps(),1);

	WriteVertices(f);		/* Write out vertices	*/
#endif
	LoopThroughAllSubObjects(s) {	/* SubObjects		*/
	    s->Id = SubObjectId++;
	    WriteSubObject(f,s);
	}

	LoopThroughAllLods(l) {		/* Lods			*/
	    WriteLod(f,l);
	}

    PutByte(f,BlockEnd);
}
/************************************************************************
Write the scene tree...
************************************************************************/
static void WriteTree(FILE *f, BranchPtr b)
{
    while (b) {
	if (BranchName(b)) {
	    PutString(f,BranchName(b));
	}
	else {
	    PutString(f,"Unnamed");
	}
#ifdef MEDIT
	PutByte(f,FSceneVisible);	PutInts(f, &(b->Visible), 1);
	PutByte(f,FSceneFolded);	PutInts(f, &(b->Folded), 1);
#endif
	if (b->Across) {
	    PutByte(f,FScenePush);
		WriteTree(f,b->Across);
	    PutByte(f,FScenePop);
	}
	if (BranchObject(b)) {
	    PutByte(f,FSceneObject);
	    PutInts(f,&(BranchObject(b)->_Id),1);
	    PutByte(f,FSceneMatrix);
	    PutMatrix(f,BranchMatrix(b));
	}
	b = b->Down;
	if (b) {
	    PutByte(f,FSceneBranch);
	}
    }
}
/************************************************************************
Read/write medit object files...
************************************************************************/
static void WriteDataItems(FILE *f)
{
#ifdef MEDIT
    int cid = FileCreateId;
    /*				Item			Size		 Data		*/
    PutByte(f,FData); PutByte(f,DCurrentObject);	PutSize(f,4); PutInts(f,ObjectId(),1);
    PutByte(f,FData); PutByte(f,DSceneGridIndex);	PutSize(f,4); PutInts(f,&SceneGridIndex,1);
    PutByte(f,FData); PutByte(f,DSceneGridSteps);	PutSize(f,4); PutInts(f,&SceneGridSteps,1);
    PutByte(f,FData); PutByte(f,DNoViewports);		PutSize(f,4); PutInts(f,&NoViewports,1);
    PutByte(f,FData); PutByte(f,DHideSubObjects);	PutSize(f,4); PutInts(f,&HideSubObjects,1);
    PutByte(f,FData); PutByte(f,DHideScene);		PutSize(f,4); PutInts(f,&HideScene,1);

    PutByte(f,FData); PutByte(f,DFileCreateId);		PutSize(f,4); PutInts(f,&cid,1);
    PutByte(f,FData); PutByte(f,DFilePublicDomain);	PutSize(f,4); PutInts(f,&FilePublicDomain,1);
    WriteDataString(f,DFileTitle,FileTitle);
    WriteDataString(f,DFileAuthor,FileAuthor);
    WriteDataString(f,DFileCompany,FileCompany);
    WriteDataString(f,DFileContact,FileContact);
    WriteDataString(f,DFileComment1,FileComment1);
    WriteDataString(f,DFileComment2,FileComment2);
    WriteDataString(f,DFileComment3,FileComment3);
    WriteDataString(f,DFileComment4,FileComment4);
#endif
    PutByte(f,FData); PutByte(f,DObjectIsolated);	PutSize(f,4); PutInts(f,&ObjectIsolated,1);
}
/************************************************************************
Write a medit file...
************************************************************************/
boolean WriteMedit(char *fname)
{
#ifdef MEDIT
    int i;
#endif
    FILE *f;
    int Id = 0;
    char *path,*name,n[MaxFileName];
    MeditObjectPtr o, old = *CurrentObject();

    strcpy(n, fname);
    SplitFileName(n, &path, &name);
#ifndef MEDIT
    strcpy(FilePath, path);
    CheckNewFileStarted("WriteMedit");
    if (!PolygonsAllowed) {
	MLibError(NothingToWrite,NULL);
    }
#endif
    if (!(f = fopen(fname,"wb"))) {
	static char s[MaxFileName+100];
	sprintf(s,"Couldn't write file:  %s\n\nreason: %s",name,sys_errlist[errno]);
#ifdef MEDIT
	TellUserS(s);
#else
	fprintf(stderr,"%s\n",s);
#endif
	return FALSE;
    }
    fprintf(f,"Compatibility: 3.05\n");
    PutByte(f,BinaryFile);

    WriteMaterials(f);
    WriteTextures(f);

    LoopThroughAllObjects(o) {
	*CurrentObject() = o;
	*ObjectId() = Id++;
	WriteObject(f);
    }

    if ((*Scene())->Across) {
	PutByte(f,FScenePush);
	    WriteTree(f,(*Scene())->Across);
	PutByte(f,FScenePop);
    }
    *CurrentObject() = old;

#ifdef MEDIT
    WriteView(f,0x23d,&DefaultScene3D);
    for (i=0; i<NoViews; i++) {
	WriteView(f,0x200+i,DefaultScene2D+i);
    }
    for (i=0; i<MaxViewports; i++) {
	WriteView(f,0x300+i,SceneView+i);
    }
#endif
    WriteDataItems(f);

    PutByte(f,BlockEnd);
    fclose(f);

    return !FileError;
}

