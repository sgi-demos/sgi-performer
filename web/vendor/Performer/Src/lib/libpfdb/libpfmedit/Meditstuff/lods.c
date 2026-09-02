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
Handle LODs. This is more complicated than is seems because we need to
deal with fadeed lods (sharing polygons between them where possible).
************************************************************************/


/************************************************************************
Add the polygons in a sub object to the lists of
polygons with/without sub polygons
************************************************************************/
/* Add all the polygons in a sub object to the lists above */
static void AddSubObjectsPolygons(MSubObjectPtr s)
{
    reg MPolygonPtr p = s->FirstPolygon;
    reg PolyListPtr *normalpolys = &NormalPolys;
    reg PolyListPtr *subfacepolys = &PolysWithSubfaces;
    while (p) {
	if (p->Child) {
	    AddToPolyList(subfacepolys,p);
	}
	else {
	    AddToPolyList(normalpolys,p);
	}
	p = p->Next;
    }
}
/************************************************************************
Bits n' bobs
************************************************************************/
static void ClearAllSubObjectFlags(MeditObjectPtr o)
{
    MSubObjectPtr s = o->FirstSubObject;
    while (s) {
	s->Added = FALSE;
	s = s->Next;
    }
}
static void FlagUsedSubObjects(MeditObjectPtr o)
{
    MLodPtr l;
    MLodBeadPtr lb;
    MSubObjectPtr s = o->FirstSubObject;
    while (s) {
	s->Id = 0;
	s = s->Next;
    }

    l = o->FirstLod;
    while (l) {
	lb = l->FirstBead;
	while (lb) {
	    lb->SubObject->Id = 1;
	    lb = lb->Next;
	}
	l = l->Next;
    }
}
static boolean SubObjectInLod(reg MSubObjectPtr s, reg MLodPtr l)
{
    reg MLodBeadPtr lb = l->FirstBead;
    while (lb) {
	if (lb->SubObject IS s) {
	    return TRUE;
	}
	lb = lb->Next;
    }
    return FALSE;
}
static boolean SubObjectUsed(reg MSubObjectPtr s, reg MLodPtr l)
{
    while (l) {
	if (SubObjectInLod(s,l)) {
	    return TRUE;
	}
	l = l->Next;
    }
    return FALSE;
}
/************************************************************************
Do a literal conversion of the LOD
************************************************************************/
static void ConvertLodNormal(MeditObjectPtr o, pfGroup *base)
{
    uint32 count;
    float distance;

    MLodPtr l;
    MLodPtr next;
    MLodBeadPtr lb;

    pfLOD *lod;
    pfGroup *group;
    static pfVec3 centre = { 0.0,0.0,0.0 };

    count = 0;
    distance = 0.0;
    l = o->FirstLod;
    lod = pfNewLOD();
    pfAddChild(base, lod);
    pfLODCenter(lod,centre);
    while (l) {
	NormalPolys = NULL;
	PolysWithSubfaces = NULL;
	ClearAllSubObjectFlags(o);
	lb = l->FirstBead;
	while (lb) {
	    AddSubObjectsPolygons(lb->SubObject);
	    lb = lb->Next;
	}
	AddPolyList((pfGroup*)lod);
	pfLODRange(lod,count++,distance);
	distance = l->SwitchOut;
	l = l->Next;
    }
    group = pfNewGroup();		/* Final switchout distance */
    pfAddChild(lod, group);
    pfLODRange(lod,count,distance);
}
/************************************************************************
Case when you want to do a LOD fade
************************************************************************/
static void FindMatchingSubObjects(MeditObjectPtr o, MSubObjectPtr s, void (*Callback)(MSubObjectPtr))
{
    reg MLodPtr l;
    reg MSubObjectPtr search;
    reg flag s_used,search_used;
    search = s;
    while (search) {
	l = o->FirstLod;
	if (!search->Added AND (s->Id ISNT 0)) {
	    while (l) {
		s_used = SubObjectInLod(s,l);
		search_used = SubObjectInLod(search,l);
		if (s_used ISNT search_used) {
		    break;
		}
		l = l->Next;
	    }
	    if (!l) {
		Callback(search);
	    }
	}
	search = search->Next;
    }
}
static void FlagSubObject(MSubObjectPtr s)
{
    s->Added = TRUE;
}
static void ConvertLodForFade(MeditObjectPtr o, pfGroup *base)
{
    uint32 count;
    pfLOD *lod;
    reg MLodPtr l;
    reg MSubObjectPtr s;
    reg float SwitchIn,SwitchOut;
    static pfVec3 centre = { 0.0,0.0,0.0 };

    s = o->FirstSubObject;
    FlagUsedSubObjects(o);
    ClearAllSubObjectFlags(o);
    while (s) {
	if (!s->Added AND (s->Id ISNT 0)) {
	    s->Added = TRUE;

	    count = 0;
	    l = o->FirstLod;
	    lod = pfNewLOD();
	    pfAddChild(base, lod);
	    pfLODCenter(lod,centre);
	    while (SubObjectUsed(s,l)) {
		NormalPolys = NULL;
		PolysWithSubfaces = NULL;
		SwitchIn = 0.0;
		while (l) {
		    if (SubObjectInLod(s,l)) {
			break;
		    }
		    SwitchIn = l->SwitchOut;
		    l = l->Next;
		}
		SwitchOut = l->SwitchOut;
		while (l) {
		    if (!SubObjectInLod(s,l)) {
			break;
		    }
		    SwitchOut = l->SwitchOut;
		    l = l->Next;
		}
		AddSubObjectsPolygons(s);
		FindMatchingSubObjects(o,s,AddSubObjectsPolygons);
		AddPolyList((pfGroup*)lod);
		pfAddChild(lod,pfNewGroup());	/* Dummy group for switchout */
		pfLODRange(lod,count++,SwitchIn);
		pfLODRange(lod,count++,SwitchOut);
	    }
	    FindMatchingSubObjects(o,s,FlagSubObject);
	}
	s = s->Next;
    }
}
/************************************************************************
Do a literal conversion of the LOD
************************************************************************/
static void ConvertLodLiteral(MeditObjectPtr o, pfGroup *base)
{
    float distance;
    long count;

    MLodPtr l;
    MLodPtr next;
    MLodBeadPtr lb;

    pfLOD *lod;
    pfGroup *group, *subobj;
    static pfVec3 centre = { 0.0,0.0,0.0 };

    count = 0;
    distance = 0.0;
    l = o->FirstLod;
    lod = pfNewLOD();
    pfAddChild(base, lod);
    pfLODCenter(lod,centre);
    while (l) {
	lb = l->FirstBead;
	group = pfNewGroup();
	pfAddChild(lod, group);
	while (lb) {
	    NormalPolys = NULL;
	    PolysWithSubfaces = NULL;
	    ClearAllSubObjectFlags(o);

	    subobj = pfNewGroup();
	    pfNodeName(subobj, lb->SubObject->Name);
	    pfAddChild(group, subobj);
	    AddSubObjectsPolygons(lb->SubObject);
	    AddPolyList(subobj);
	    lb = lb->Next;
	}
	pfNodeName(pfGetChild((pfGroup*)lod, count), l->Name);
	pfLODRange(lod,count++,distance);
	distance = l->SwitchOut;
	l = l->Next;
    }
    group = pfNewGroup();		/* Final switchout distance */
    pfAddChild(lod, group);
    pfLODRange(lod,count,distance);
}
/************************************************************************
Generate Performer LODs from a Medit object
************************************************************************/
static void LodConvert(MeditObjectPtr o, pfGroup *base)
{
    if (LiteralConvert) {
	ConvertLodLiteral(o,base);
    }
    else if (FadeLods) {
	ConvertLodForFade(o,base);
    }
    else {
	ConvertLodNormal(o,base);
    }
}

