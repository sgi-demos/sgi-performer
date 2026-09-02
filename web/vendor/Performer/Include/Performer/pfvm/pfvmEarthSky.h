/*
 * Copyright 2002, Silicon Graphics, Inc.
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


#ifndef __PFV_ESKY_H__
#define __PFV_ESKY_H__

///////////////////////////////////////////////////////////////////////////////

#include <Performer/pfv/pfvViewer.h>
#include <Performer/pf/pfEarthSky.h> 

///////////////////////////////////////////////////////////////////////////////

class pfvXmlNode;

///////////////////////////////////////////////////////////////////////////////

class pfvmEarthSky : public pfvModule
{
public:	
	pfvmEarthSky( pfvXmlNode* xml=NULL);
	~pfvmEarthSky() {}

	static void init();
	static pfType* getClassType(){ return classType; }
	virtual const char* getTypeName() const {return "pfvmEarthSky";}

	/*int parseXml(pfvXmlNode*xml);*/
	int setXmlField(pfvXmlNode*xml);
		
    void postConfig();
    void enterView(pfvView*v);
	void exitView(pfvView*v);
	
    pfEarthSky* getESky() { return esky; }
	
protected:
	static pfType* classType;
	
private:

	void construct();
    pfEarthSky* esky;

    int clearMode;
    int cloudMode;

	pfVec4 colSkyTop;
    pfVec4 colSkyBot;
    pfVec4 colHoriz;
    pfVec4 colGrndFar;
    pfVec4 colGrndNear;
    pfVec4 colCloudBot;
    pfVec4 colCloudTop;
    pfVec4 colClear;

	
    float grndHt;
    float horizAngle;
    float cloudTop;
    float cloudBot;
    float tZoneBot;
    float tZoneTop;
    float grndFogTop;
    float distFactor;
};

///////////////////////////////////////////////////////////////////////////////

#endif // end of __PFV_ESKY_H__






