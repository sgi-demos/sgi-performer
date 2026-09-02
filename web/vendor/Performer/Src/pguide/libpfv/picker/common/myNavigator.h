
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
#ifndef __MY_NAVIGATOR_H__
#define __MY_NAVIGATOR_H__

///////////////////////////////////////////////////////////////////////////////

#include <Performer/pf/pfNode.h>
#include <Performer/pr/pfHighlight.h>

#include <Performer/pfutil.h>

///////////////////////////////////////////////////////////////////////////////

class pfvXmlNode;
class pfNode;

///////////////////////////////////////////////////////////////////////////////

class myNavigator
{
public:	
	myNavigator( pfvXmlNode* xml);
	~myNavigator();

	//static void init();
	//static pfType* getClassType(){ return classType; }
	//virtual const char* getTypeName() const {return "myNavigator";}

	int update(pfuMouse*m,pfuEventStream*events,pfChannel*chan);

	void setCoord( pfVec3* _xyz, pfVec3* _hpr ) { 
		if(_xyz)xyz.copy(_xyz[0]); if(_hpr)hpr.copy(_hpr[0]); 
	}
	
	void getCoord( pfVec3* _xyz, pfVec3* _hpr ) { 
		if(_xyz)_xyz[0].copy(xyz); 
		if(_hpr)_hpr[0].copy(hpr); 
	}


	void setDefaultCoord( pfVec3* _xyz, pfVec3* _hpr ) { 
		if(_xyz)default_xyz.copy(_xyz[0]); 
		if(_hpr)default_hpr.copy(_hpr[0]); 
	}
	
	void getDefaultCoord( pfVec3* _xyz, pfVec3* _hpr ) { 
		if(_xyz)_xyz[0].copy(default_xyz); 
		if(_hpr)_hpr[0].copy(default_hpr); 
	}


	float dzone[2];

	float deltaH;
	float deltaP;

	float initSpeed;
	float minSpeed;
	float maxSpeed;
	float deltaSpeed; /* note: speed is multiplied by this on each frame */

	float deltaStrafe;
	float deltaZ;

private:

	pfVec3 default_xyz;
	pfVec3 default_hpr;

	pfVec3 xyz;
	pfVec3 hpr;

	float speed;

	
};

///////////////////////////////////////////////////////////////////////////////

#endif // end of __MY_SELECTOR_H__






