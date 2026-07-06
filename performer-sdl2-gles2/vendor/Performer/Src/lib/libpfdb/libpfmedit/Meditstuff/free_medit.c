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
Free up the storage used by a .medit file
************************************************************************/
static void FreePolygon(MPolygonPtr p)
{
    register MPolygonPtr		nextp;
    register MPolygonBeadPtr	pb,nextpb;
    while (p) {
	pb = p->FirstBead;
	while (pb) {
	    nextpb = pb->Next;
	    Free(pb);
	    pb = nextpb;
	}
	nextp = p->Next;
	Free(p);
	p = nextp;
    }
}
static void FreeTree(MBranchPtr b)
{
    register MBranchPtr next;
    while (b) {
	if (b->Across) {
	    FreeTree(b->Across);
	}
	if (b->Name) {
	    Free(b->Name);
	}
	next = b->Down;
	Free(b);
	b = next;
    }
}

void Medit_FreeFile(MeditFilePtr File)
{
    register MeditObjectPtr	o,nexto;
    register MLodPtr		l,nextl;
    register MLodBeadPtr	lb,nextlb;
    register MSubObjectPtr	s,nexts;
    register MMaterialPtr	m,nextm;
    register MTexturePtr	t,nextt;

    o = File->FirstObject;
    while (o) {
	l = o->FirstLod;				/* Free all LODs */
	while (l) {
	    lb = l->FirstBead;
	    while (lb) {
		nextlb = lb->Next;
		Free(lb);
		lb = nextlb;
	    }
	    Free(l->Name);
	    nextl = l->Next;
	    Free(l);
	    l = nextl;
	}
	s = o->FirstSubObject;			/* Free up subobjects */
	while (s) {
	    FreePolygon(s->FirstPolygon);
	    Free(s->Name);
	    nexts = s->Next;
	    Free(s);
	    s = nexts;
	}
	Free(o->Name);					/* Free object */
	nexto = o->Next;
	Free(o);
	o = nexto;
    }
    FreeTree(File->Scene);
    m = File->FirstMaterial;
    while (m) {
	nextm = m->Next;
	if (m->Name) {
	    Free(m->Name);
	}
	Free(m);
	m = nextm;
    }
    t = File->FirstTexture;
    while (t) {
	nextt = t->Next;
	if (t->File) {
	    Free(t->File);
	}
	Free(t);
	t = nextt;
    }
    Free(File);
}
