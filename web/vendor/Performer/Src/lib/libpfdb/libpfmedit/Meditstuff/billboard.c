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
Special case for billboards...
************************************************************************/
static void AddBillboard(pfGroup *base, MeditObjectPtr o)
{
    pfLOD *lod;
    pfVec4 *colour;
    pfGeoSet *geoset;
    pfVec2 *tcoords,*t;
    pfBillboard *board;
    pfVec3 *vertices,*normals,*v;

    int count;
    MLodPtr l;
    MPolygonPtr p;
    flag coloured;
    MTexturePtr tex;
    MMaterialPtr mat;
    MPolygonBeadPtr pb;
    static pfVec3 centre = { 0.0,0.0,0.0 };


    l = o->FirstLod;
    p = l->FirstBead->SubObject->FirstPolygon;

    count = 0;
    pb = p->FirstBead;
    while (pb) {
	count++;
	pb = pb->Next;
    }
    if ((count < 3) OR (count > 4) OR p->Next OR p->Child) {
	fprintf(stderr,"Error in billboard %s\n",o->Name);
	fprintf(stderr,"Performer billboards may only contain 1 polygon,\n");
	fprintf(stderr,"and that polygon must be a triangle or a rectangle!\n");
    }
    else {
	if (LoadingTextures) {
	    if (tex = p->Texture) {
		mat = p->TextureMaterial;
	    }
	    else {
		mat = p->NormalMaterial;
	    }
	}
	else {
	    tex = NULL;
	}
	coloured = (p->Lighting IS Unlit);

	vertices = pfMalloc(count*sizeof(pfVec3), MeditArena);
	if (coloured OR FastLighting) {
	    colour = pfMalloc(sizeof(pfVec4), MeditArena);
	    (*colour)[0] = mat->Diffuse[0];
	    (*colour)[1] = mat->Diffuse[1];
	    (*colour)[2] = mat->Diffuse[2];
	    (*colour)[3] = mat->Alpha;
	}
	if (!coloured) {
	    normals = pfMalloc(sizeof(pfVec3), MeditArena);
	}
	if (tex) {
	    tcoords = pfMalloc(count*sizeof(pfVec2), MeditArena);
	}

	v = vertices;
	t = (tex)?tcoords:NULL;
	pb = p->FirstBead;
	if (p->Lighting IS Lit) {
	    CopyXYZ(normals[0],p->Normal);
	}
	else if (p->Lighting IS Smooth) {
	    CopyXYZ(normals[0],pb->Normal);
	}
	while (pb) {
	    CopyXYZ((*v),pb->Coordinate);			v++;
	    if (tex) { CopyXY((*t),pb->TextureCoord);	t++; }
	    pb = pb->Next;
	}

	if (count IS 4) {		/* A square... */
	    geoset = pfNewGSet(MeditArena);
	    pfGSetPrimType(geoset, PFGS_QUADS);
	    pfGSetNumPrims(geoset, 1);
	    pfGSetAttr(geoset, PFGS_COORD3, PFGS_PER_VERTEX, vertices, NULL);
	}
	else {				/* A triangle... */
	    geoset = pfNewGSet(MeditArena);
	    pfGSetPrimType(geoset, PFGS_TRIS);
	    pfGSetNumPrims(geoset, 1);
	    pfGSetAttr(geoset, PFGS_COORD3, PFGS_PER_VERTEX, vertices, NULL);
	}

	if (coloured OR FastLighting) {
	    pfGSetAttr(geoset, PFGS_COLOR4, PFGS_OVERALL, colour, NULL);
	}
	if (!coloured) {
	    pfGSetAttr(geoset, PFGS_NORMAL3, PFGS_PER_PRIM, normals, NULL);
	}
	if (tex) pfGSetAttr(geoset, PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, NULL);
	pfGSetGState(geoset, MakeGeostate(coloured,mat,tex,FALSE));

	board = pfNewBboard();
	pfAddGSet((pfGeode*)board, geoset);

	lod = pfNewLOD();		/* Set up for switchout */
	pfAddChild(base, lod);
	pfAddChild(lod, board);
	pfLODRange(lod,0,0.0);
	pfLODCenter(lod,centre);

	pfAddChild(lod, pfNewGroup());
	pfLODRange(lod,1,l->SwitchOut);
    }
}
