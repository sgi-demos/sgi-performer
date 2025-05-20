
/*
 * Copyright 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * This source code ("Source Code") was originally derived from a
 * code base owned by Silicon Graphics, Inc. ("SGI")
 * 
 * LICENSE: SGI grants the user ("Licensee") permission to reproduce,
 * distribute, and create derivative works from this Source Code,
 * provided that: (1) the user reproduces this entire notice within
 * both source and binary format redistributions and any accompanying
 * materials such as documentation in printed or electronic format;
 * (2) the Source Code is not to be used, or ported or modified for
 * use, except in conjunction with OpenGL Performer; and (3) the
 * names of Silicon Graphics, Inc.  and SGI may not be used in any
 * advertising or publicity relating to the Source Code without the
 * prior written permission of SGI.  No further license or permission
 * may be inferred or deemed or construed to exist with regard to the
 * Source Code or the code base of which it forms a part. All rights
 * not expressly granted are reserved.
 * 
 * This Source Code is provided to Licensee AS IS, without any
 * warranty of any kind, either express, implied, or statutory,
 * including, but not limited to, any warranty that the Source Code
 * will conform to specifications, any implied warranties of
 * merchantability, fitness for a particular purpose, and freedom
 * from infringement, and any warranty that the documentation will
 * conform to the program, or any warranty that the Source Code will
 * be error free.
 * 
 * IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
 * LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
 * ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
 * SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
 * OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
 * PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
 * OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
 * USE, THE SOURCE CODE.
 * 
 * Contact information:  Silicon Graphics, Inc., 
 * 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
 * or:  http://www.sgi.com
 */
///////////////////////////////////////////////////////////////////////////////

#include <Performer/pf.h>
#include <Performer/pr.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pf/pfTraverser.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfSCS.h>
#include <Performer/pf/pfDCS.h>
#include "myTransProjector.h"
#include <Performer/pfutil.h>
#include "math.h"

///////////////////////////////////////////////////////////////////////////////

myTransProjector::myTransProjector()
{
    startPt.set(0.0f,0.0f,1.0f);
    BBox.makeEmpty();
	path = new pfPath;
}

///////////////////////////////////////////////////////////////////////////////

myTransProjector::~myTransProjector()
{
}

///////////////////////////////////////////////////////////////////////////////

void myTransProjector::setPath( pfPath* _path )
{
	path->reset();
	int i,n=_path->getNum();
	for(i=0;i<n;i++)
		path->add( _path->get(i) );
	node = (pfNode*)(path->get(n-1));
}	

///////////////////////////////////////////////////////////////////////////////

pfVec3 myTransProjector::project(pfVec2&pt)
{
	if(chan && node)
	{
		chan->getView( chanxyz, chanhpr );
		chan->getNearFar( &chanNearPlaneDist, &chanFarPlaneDist );
        chanRot.makeEuler( chanhpr[0], chanhpr[1], chanhpr[2] );

		pfuTravCalcBBox( node,  &BBox );
		// Compute BBOX Center in world space
        worldCenter = pfVec3( (BBox.max[0]+BBox.min[0])/2.0f,
						 	  (BBox.max[1]+BBox.min[1])/2.0f,
						 	  (BBox.max[2]+BBox.min[2])/2.0f );

		int pathLen = path->getNum();
		/*fprintf(stderr, "pathLen %d\n", pathLen);*/
		pfMatrix holder;
		startMatrix.makeIdent();
        pathMatrix.makeIdent();
		
		for( int r=0; r<pathLen; r++ )
		{
		    pfNode* pare = (pfNode*)path->get( r );
		    //fprintf(stderr, "Object Type = %s\n", pare->getTypeName());
		    if( pfIsOfType(pare, pfSCS::getClassType() ) ) 
		    {
		        ((pfSCS*)pare)->getMat(holder);
				startMatrix.preMult(holder);
				
				if(r<(pathLen-1))
					pathMatrix.preMult(holder);
				else
					origMatrix = holder;			
		    }
		}
	}
	
	startPt.set(pt[0],pt[1], 0.0f);
	return startPt;
}

///////////////////////////////////////////////////////////////////////////////

pfVec3 myTransProjector::projectAndGetRotation(pfVec2&pt,pfMatrix&rotmat)
{
    // 1. Compute mouse space vec and length
    pfVec2 axial( (pt[0]-startPt[0]), (pt[1]-startPt[1]) );
    float len = axial.length();

    // 2. Skip calc unless there's something to process.
    if( len >= 0.001 ) {
	
	// 3. Compute range correction factor
	pfVec3 rangeV = chanxyz;
	rangeV += worldCenter;
	float range = rangeV.length();
	//fprintf(stderr,"RANNNGE %f %f\n", range, chanNearPlaneDist );
	range /= chanNearPlaneDist;
	range *= len;

	// 4. Compute quaternion transform
	pfQuat q, qinv, qvec;
	qvec.set( axial[0], 0.0f, axial[1], 0.0f );
	chanRot.getOrthoQuat(qinv);
	q.invert(qinv);
	q *= qvec;
	q *= qinv;
	q[0] *= range; q[1] *= range; q[2] *= range;    


    // 5. Apply to object space
	rotmat.makeTrans( q[0], q[1], q[2] );
    rotmat.preMult(startMatrix);
	
	} else
        rotmat = origMatrix /*startMatrix*/;
    
    return startPt;    
}

///////////////////////////////////////////////////////////////////////////////

pfVec3 myTransProjector::projectAndGetRotation( pfVec2& pt, pfMatrix& rotmat, 
                                           int which )
{
    // Compute mouse space vec and length
    pfVec2 axial( (pt[0]-startPt[0])/2, (pt[1]-startPt[1])/2 );
    /*fprintf( stderr, "DRAGEE %f %f\n", axial[0], axial[1] ); */
    float len = axial.length();
    
    // Compute range correction factor
    pfVec3 rangeV = chanxyz;
    rangeV -= worldCenter;
    float range = rangeV.length();
    //fprintf(stderr,"RANNNGE %f %f\n", range, chanNearPlaneDist );
    range /= chanNearPlaneDist;
    range *= len;

    if( which ) 
	{
		/*printf("TRANSLATING ON XY PLANE\n");*/

		float s,c;
		pfSinCos(chanhpr[0],&s,&c);

		pfVec3 vecT( axial[0]*c-axial[1]*s, axial[1]*c+axial[0]*s, 0.0f );
		vecT*=8.0f; // speeding up motion which is too slow

		pfMatrix invPathMat;
		invPathMat.invertFull(pathMatrix);
		
		pfVec3 vecTrans;
		vecTrans.xformVec(vecT,invPathMat);

		pfMatrix transMat;
		transMat.makeTrans(vecTrans[0],vecTrans[1],vecTrans[2]);

		rotmat.mult(origMatrix,transMat);
    } 
	else 
	{	
		// Compute quaternion transform
	    pfQuat q, qinv, qvec;
	    qvec.set( axial[0], 0.0f, axial[1], 0.0f );
	    chanRot.getOrthoQuat(qinv);
	    q.invert(qinv);
	    q *= qvec;
	    q *= qinv;
	    q[0] *= range; q[1] *= range; q[2] *= range;    

		pfVec3 vecT(q[0],q[1],q[2]);

		pfMatrix invPathMat;
		invPathMat.invertFull(pathMatrix);
		
		pfVec3 vecTrans;
		vecTrans.xformVec(vecT,invPathMat);

		pfMatrix transMat;
		transMat.makeTrans(vecTrans[0],vecTrans[1],vecTrans[2]);

		rotmat.mult(origMatrix,transMat);
    }
    
    return startPt;    
}

///////////////////////////////////////////////////////////////////////////////











