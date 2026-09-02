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
Write a Performer database to a Medit file
************************************************************************/

/************************************************************************
See if a group node contains multiple objects (in our opinion)
************************************************************************/
static int CountObjects(pfNode *node)
{
    pfNode *child;
    int loop, children, objects = 0;

    LoopThroughChildren(node) {
	child = LoopChild;
	if (ItsAGeode(child) OR ItsALod(child) OR ItsATransform(child)) {
	    objects++;
	}
	else if (ItsAGroup(child)) {
	    objects += CountObjects(child);
	}
    }
    return objects;
}
/************************************************************************
Do the conversion
************************************************************************/
static void ParseTree(MBranchPtr scene, pfNode *node, flag hasswitch, float in, float out)
{
    flag IsLod;
    MBranchPtr b;
    pfNode *child;
    float nin, nout;
    GList objlist, lodlist;
    int loop, type, children;

    IsLod = ItsALod(node);
    if (IsLod) {
	DEBUG"Its a lod\n");
	nin = 0.0;
    	InitGList(&lodlist, NodeName(node));
    }
    else {
	LoopThroughChildren(node) {
	    child = LoopChild;
	    if (IsLod) {
		if (loop ISNT (children-1)) {
		    nout = pfGetLODRange((pfLOD*)node, loop+1);
		    GrabGeometry(child, &lodlist, TRUE, nin, nout);
		    nin = nout;
		}
	    }
	    else {
		if (ItsAGeode(child)) {
		    InitGList(&objlist, NodeName(child));
		    GeometryGrabber(child, &objlist, FALSE, 0.0, 0.0);
		    AttachGeometry(scene, &objlist);
		}
		else {
		    if (CountObjects(child) > 1) {
			b = Medit_AddBranch(scene,ACROSS);
			b->Name = NodeName(child);
			ParseTree(b, child, hasswitch, in, out);
		    }
		    else {
			InitGList(&objlist, NodeName(child));
			GrabGeometry(child, &objlist, FALSE, 0.0, 0.0);
			AttachGeometry(scene, &objlist);
		    }
		}
	    }
	}
    }
    if (IsLod) {
	AttachGeometry(scene, &lodlist);
    }
}
static boolean ConvertToMedit(pfNode *root)
{
    NewMeditFile();
    MaterialsCreated = ObjectsCreated = ObjectsReplicated = PolygonsCreated = 0;

    pfFlatten(root, 0);
    if (ItsAGeode(root)) {
	GList objlist;
	objlist.Name = NodeName(root);
	objlist.FirstGeom = NULL;
	GrabGeometry(root, &objlist, FALSE, 0.0, 0.0);
	AttachGeometry(CurrentMeditFile->Scene, &objlist);
    }
    else if (pfGetNumChildren(root) > 0) {
	ObjectIsolated = FALSE;
	ParseTree(CurrentMeditFile->Scene, root, FALSE, 0.0, 0.0);
    }
    else {
	return FALSE;
    }
    return (ObjectsCreated > 0);
}
