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
Convert a Medit object to Performer geometry
Strategy: Work out Lod structure needed, then make lists of all
	  polygons in each Lod. These lists are then processed in
	  addpolys.c above
************************************************************************/
static pfGroup *ConvertObject(MeditObjectPtr o)
{
    pfGroup *base = pfNewGroup();
    pfNodeName(base,o->Name);

    fprintf(stderr, "Object: %s\n",o->Name);
    if (o->Attributes & ObjBillboard) {		/* Billboards are a special case... */
	AddBillboard(base,o);
    }
    else {
	LodConvert(o,base);
    }
    return base;
}
/************************************************************************
Make a Performer scene tree which matches the Medit tree
Add objects as we encounter them
************************************************************************/
static void GenerateScene(MBranchPtr b, pfGroup *base)
{
    pfGroup *across;
    MeditObjectPtr o;
    pfSCS *ObjectTrans;
    char s[M_MaxName+20];
    while (b) {
	if (b->Across) {
	    across = pfNewGroup();
	    pfAddChild(base, across);
	    pfNodeName(across, b->Name);
	    GenerateScene(b->Across, across);
	}
	if (o = b->Obj) {
	    ObjectTrans = pfNewSCS(b->Trans);
	    pfAddChild(base, ObjectTrans);
	    sprintf(s,"%s_transform",o->Name);
	    pfNodeName(ObjectTrans, s);
	    if (ObjectList[o->Id]) {
		pfAddChild(ObjectTrans,pfClone(ObjectList[o->Id],0));
	    }
	    else {
		ObjectList[o->Id] = ConvertObject(o);
		pfAddChild(ObjectTrans,ObjectList[o->Id]);
	    }
	}
	b = b->Down;
    }
}
/************************************************************************
Convert the fils
************************************************************************/
static void ConvertMeditFile(void)
{
    if (CurrentMeditFile->Scene->Across) {
	GenerateScene(CurrentMeditFile->Scene,MeditRoot);
    }
    else {			/* If no scene defined, assume a single object */
	fprintf(stderr, "Adding single object\n");
	if (CurrentMeditFile->FirstObject) {
	    pfAddChild(MeditRoot, ConvertObject(CurrentMeditFile->FirstObject));
	    pfNodeName(MeditRoot, CurrentMeditFile->FirstObject->Name);
	}
    }
}
