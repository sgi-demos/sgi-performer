
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

#include <Performer/pfutil.h>

#include "myRotProjector.h"

#include "math.h"

///////////////////////////////////////////////////////////////////////////////

myRotProjector::myRotProjector()
{
    lastPt.set(0.0f,0.0f,1.0f);
    BBox.makeEmpty();
	path = new pfPath;
}

///////////////////////////////////////////////////////////////////////////////

myRotProjector::~myRotProjector()
{
}

///////////////////////////////////////////////////////////////////////////////

void myRotProjector::setPath( pfPath* _path )
{
	path->reset();
	int i,n=_path->getNum();
	for(i=0;i<n;i++)
		path->add( _path->get(i) );
	node = (pfNode*)(path->get(n-1));
}	

///////////////////////////////////////////////////////////////////////////////

pfVec3 myRotProjector::project(pfVec2&pt)
{
	if(chan)
	{
		chan->getView( chanxyz, chanhpr );
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

	itre = 0.0;
	lastPt.set(pt[0],pt[1], 0.0f);

	return lastPt;
}

///////////////////////////////////////////////////////////////////////////////

pfVec3 myRotProjector::projectAndGetRotation(pfVec2&pt,pfMatrix&rotmat)
{
    // Compute rot vec/angle
    pfVec3 axial( pt[0]-lastPt[0], pt[1]-lastPt[1], 0.0f );
    itre = axial.length()*48.0f;
    /*fprintf( stderr, "DaFacts %f %f %f\n", itre, axial[0], axial[1] );*/
    
	if( itre > 0.0f ) {

		// Compute quaternion transform
		pfQuat q, qinv, qvec;
		qvec.set( -axial[1], 0.0f, axial[0], 0.0f );
		chanRot.getOrthoQuat(qinv);
		q.invert(qinv);
		q *= qvec;
		q *= qinv;

		// 2. Compute transform
		pfMatrix matter, rematter, rotor;
		
		matter.makeTrans( -worldCenter[0], -worldCenter[1], -worldCenter[2] );
		rematter.makeTrans( worldCenter[0], worldCenter[1], worldCenter[2] );
		
		
		
		pfMatrix invPathMat;
		invPathMat.invertFull(pathMatrix);
		pfVec3 axis ( q[0], q[1], q[2] );
		pfVec3 axisT;
		axisT.xformVec(axis,invPathMat);
		q[0]=axisT[0]; q[1]=axisT[1]; q[2]=axisT[2];
		
			
		
		
		
		rotor.makeRot( itre, q[0], q[1], q[2] );
		matter.postMult(rotor);
		matter.postMult(rematter);
		matter.preMult(origMatrix/*startMatrix*/);
		rotmat = matter;

    } else
        rotmat = origMatrix /*startMatrix*/;

	return lastPt;
}

///////////////////////////////////////////////////////////////////////////////











