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
Remove all unneeded SCS nodes from the tree after a flatten
************************************************************************/
static float IdentityMatrix[4][4] = {
{ 1.0, 0.0, 0.0, 0.0},
{ 0.0, 1.0, 0.0, 0.0},
{ 0.0, 0.0, 1.0, 0.0},
{ 0.0, 0.0, 0.0, 1.0} };

static void RemoveSCSs(pfNode *node)
{
    int i, nc;
    pfMatrix scsMat;
    pfNode *child, *grandchild;

    if (pfIsOfType(node, pfGetGroupClassType())) {
	nc = pfGetNumChildren(node);
	for (i=0; i<nc; i++) {
            child = pfGetChild(node, i);
	    if (pfIsOfType(child, pfGetSCSClassType())) {
		pfGetSCSMat((pfSCS*)child, scsMat);
		if (PFEQUAL_MAT(scsMat, IdentityMatrix)) {
		    if (pfGetNumChildren(child) ISNT 1) {
			printf("Error in remove SCSs!\n");
			exit(666);
		    }
		    grandchild = pfGetChild(child, 0);
		    pfRemoveChild(child, grandchild);
		    pfReplaceChild(node, child, grandchild);
		    pfDelete(child);
		}
	    }
	    else {
		RemoveSCSs(child);
	    }
	}
    }
}


