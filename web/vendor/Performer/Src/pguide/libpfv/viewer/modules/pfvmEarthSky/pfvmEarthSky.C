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

#include <stdio.h>
#include <Performer/pf.h>
#include <Performer/pr.h>

#include <Performer/pf/pfChannel.h>

#include <Performer/pfv/pfvXml.h>
#include <Performer/pfv/pfvViewer.h>

#include "pfvmEarthSky.h"

///////////////////////////////////////////////////////////////////////////////

pfType* pfvmEarthSky::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void pfvmEarthSky::init()
{
    if(classType==NULL)
    {
        pfvModule::init();
        classType= new pfType(pfvModule::getClassType(), "pfvmEarthSky" );
    }
}

///////////////////////////////////////////////////////////////////////////////

void pfvmEarthSky::construct()
{
    if(classType==NULL)
        init();
    setType(classType);

    bindCallback(PFV_CB_POSTCONFIG);
    
    esky=NULL;
    
    /*clearMode = PFES_FAST;*/
    /*clearMode = PFES_SKY_CLEAR;*/
    
    clearMode = PFES_SKY_GRND;
    cloudMode = PFES_OVERCAST;

    // paolo. Default values taken from pfEarthSky.C
    colSkyTop.set(0.0, 0.0, 0.44, 1.0);
    colSkyBot.set(0.0, 0.4, 0.7, 1.0);
    colHoriz.set(0.8, 0.8, 1.0, 1.0);
    colGrndFar.set(0.4, 0.2, 0.0, 1.0);
    colGrndNear.set(0.5, 0.3, 0.0, 1.0);
    colCloudBot.set(0.8, 0.8, 0.8, 1.0);
    colCloudTop.set(0.8, 0.8, 0.8, 1.0);
    colClear.set(0.0, 0.0, 0.0, 1.0);
    
    grndHt=0.0f;
    horizAngle=10.0f;
    cloudTop=20000.0f;
    cloudBot=20000.0f;
    tZoneBot=15000.0f;
    tZoneTop=25000.0f;
    grndFogTop=1000.0f;
    distFactor=1.0f;
}

///////////////////////////////////////////////////////////////////////////////

int pfvmEarthSky::setXmlField(pfvXmlNode* xml)
{
    if(xml==NULL)
        return 0;

    if(!xml->nameCmp("mode")){
        char* s = xml->getValue();
#ifdef WIN32
#define strcasecmp stricmp
#endif
        if(!strcasecmp(s,"fast")){
            clearMode = PFES_FAST;
            return 1;
        }
        if(!strcasecmp(s,"tag")){
            clearMode = PFES_TAG;
            return 1;
        }
        if(!strcasecmp(s,"sky")){
            clearMode = PFES_SKY;
            return 1;
        }
        if(!strcasecmp(s,"skyGrnd")){
            clearMode = PFES_SKY_GRND;
            return 1;
        }
        if(!strcasecmp(s,"skyClear")){
            clearMode = PFES_SKY_CLEAR;
            return 1;
        }
        pfNotify(PFNFY_WARN,PFNFY_PRINT, "pfvmEarthSky::setXmlField"
            " - invalid clearMode setting \"%s\"", s );
        return 0;
    }
    if(!xml->nameCmp("SkyTopColor")){
        xml->getFloatArray(4,&(colSkyTop[0]));
        return 1;
    }
    if(!xml->nameCmp("SkyBotColor")){
        xml->getFloatArray(4,&(colSkyBot[0]));
        return 1;
    }
    if(!xml->nameCmp("HorizColor")){
        xml->getFloatArray(4,&(colHoriz[0]));
        return 1;
    }
    if(!xml->nameCmp("GrndFarColor")){
        xml->getFloatArray(4,&(colGrndFar[0]));
        return 1;
    }
    if(!xml->nameCmp("GrndNearColor")){
        xml->getFloatArray(4,&(colGrndNear[0]));
        return 1;
    }
    if(!xml->nameCmp("CloudBotColor")){
        xml->getFloatArray(4,&(colCloudBot[0]));
        return 1;
    }
    if(!xml->nameCmp("CloudTopColor")){
        xml->getFloatArray(4,&(colCloudTop[0]));
        return 1;
    }
    if(!xml->nameCmp("ClearColor")){
        xml->getFloatArray(4,&(colClear[0]));
        return 1;
    }
    if(!xml->nameCmp("GrndHt")){
        xml->getFloat(&grndHt);
        return 1;
    }
    if(!xml->nameCmp("HorizAngle")){
        xml->getFloat(&horizAngle);
        return 1;
    }
    if(!xml->nameCmp("CloudTop")){
        xml->getFloat(&cloudTop);
        return 1;
    }
    if(!xml->nameCmp("CloudBot")){
        xml->getFloat(&cloudBot);
        return 1;
    }
    if(!xml->nameCmp("TZoneBot")){
        xml->getFloat(&tZoneBot);
        return 1;
    }
    if(!xml->nameCmp("TZoneTop")){
        xml->getFloat(&tZoneTop);
        return 1;
    }
    if(!xml->nameCmp("GrndFogTop")){
        xml->getFloat(&grndFogTop);
        return 1;
    }
    if(!xml->nameCmp("DistFactor")){
        xml->getFloat(&distFactor);
        return 1;
    }

    return pfvModule::setXmlField(xml);
}

///////////////////////////////////////////////////////////////////////////////

pfvmEarthSky::pfvmEarthSky(pfvXmlNode*xml)
{
    construct();
    if(xml)
        parseXml(xml);
}

///////////////////////////////////////////////////////////////////////////////

void pfvmEarthSky::enterView(pfvView*view)
{
	if(view==NULL)
		return;
	
	view->setChanShare( view->getChanShare()| PFCHAN_EARTHSKY);
    if(view->getMasterChan()==NULL)
        pfNotify(PFNFY_WARN,PFNFY_PRINT, 
            "pfvmEarthSky::enterView - NULL pfChannel");
    else
    {
	    pfNotify(PFNFY_DEBUG,PFNFY_PRINT, 
	        "pfvmEarthSky::enterView - master channel=%x (esky=%x)", 
			    view->getMasterChan(), esky );
        view->getMasterChan()->setESky(esky);
    }
}

///////////////////////////////////////////////////////////////////////////////

void pfvmEarthSky::exitView(pfvView*view)
{
	if(view==NULL)
		return;
	
    if(view->getMasterChan()==NULL)
        pfNotify(PFNFY_WARN,PFNFY_PRINT, 
            "pfvmEarthSky::exitView - NULL pfChannel");
    else
    {
	    pfNotify(PFNFY_DEBUG,PFNFY_PRINT, 
	        "pfvmEarthSky::exitView - master channel=%x", 
			    view->getMasterChan());
        view->getMasterChan()->setESky(NULL);
    }
}

///////////////////////////////////////////////////////////////////////////////

void pfvmEarthSky::postConfig()
{
    pfvView*view=NULL;
    
    pfNotify(PFNFY_INFO, PFNFY_PRINT, "pfvmEarthSky::postConfig called");
    
    if(getScope()==PFV_SCOPE_WORLD)
    {
		pfNotify(PFNFY_INFO, PFNFY_PRINT, 
				"MOdule scoped to world %s", getScopeTarget()->getName() );
        bindCallback(PFV_CB_ENTER_VIEW); 
		bindCallback(PFV_CB_EXIT_VIEW);
		pfNotify(PFNFY_INFO, PFNFY_PRINT, "ENTER/EXIT VIEW CBS set for esky module");
	}
	else 
	{	
	    if(getScope()==PFV_SCOPE_GLOBAL)
            view = pfvViewer::getViewer()->getView(0);
	    else
            view = (pfvView*)getScopeTarget();
	
        if(view==NULL)
        {
            pfNotify(PFNFY_WARN,PFNFY_PRINT, 
                "pfvmEarthSky::postConfig - null view");
            return;
        }
        else
	        pfNotify(PFNFY_INFO,PFNFY_PRINT, 
	            "pfvmEarthSky::postConfig - view=%x", view);
    }
	
    esky = new pfEarthSky;
    if(esky==NULL)
    {
        pfNotify(PFNFY_WARN,PFNFY_PRINT, 
            "pfvmEarthSky::postConfig - failed to create pfEarthSky");
        return;
    }


	pfNotify(PFNFY_DEBUG,PFNFY_PRINT, 
            "pfvmEarthSky::postConfig - created esky at %x", esky );


	
    esky->setMode(PFES_BUFFER_CLEAR,clearMode);

    esky->setColor(PFES_SKY_TOP,
            colSkyTop[0], colSkyTop[1], colSkyTop[2], colSkyTop[3] );
    esky->setColor(PFES_SKY_BOT,
            colSkyBot[0], colSkyBot[1], colSkyBot[2], colSkyBot[3] );
    esky->setColor(PFES_HORIZ,
        colHoriz[0], colHoriz[1], colHoriz[2], colHoriz[3] );
    esky->setColor(PFES_GRND_FAR,
        colGrndFar[0], colGrndFar[1], colGrndFar[2], colGrndFar[3] );
    esky->setColor(PFES_GRND_NEAR,
        colGrndNear[0], colGrndNear[1], colGrndNear[2], colGrndNear[3] );
    esky->setColor(PFES_CLOUD_BOT,
        colCloudBot[0], colCloudBot[1], colCloudBot[2], colCloudBot[3] );
    esky->setColor(PFES_CLOUD_TOP,
        colCloudTop[0], colCloudTop[1], colCloudTop[2], colCloudTop[3] );
    esky->setColor(PFES_CLEAR,
        colClear[0], colClear[1], colClear[2], colClear[3] );

    esky->setAttr(PFES_GRND_HT , grndHt );
    esky->setAttr(PFES_HORIZ_ANGLE , horizAngle );
    esky->setAttr(PFES_CLOUD_TOP , cloudTop );
    esky->setAttr(PFES_CLOUD_BOT , cloudBot );
    esky->setAttr(PFES_TZONE_TOP , tZoneTop );
    esky->setAttr(PFES_TZONE_BOT , tZoneBot );
    esky->setAttr(PFES_GRND_FOG_TOP , grndFogTop );
    esky->setAttr(PFES_TENT_DISTANCE_FACTOR , distFactor );


    if(view)
	{
		enterView(view);
	}
}    

///////////////////////////////////////////////////////////////////////////////

extern "C" PFVM_DLLEXPORT
pfvModule* pfvLoadModule_pfvmEarthSky(pfvXmlNode*xml)
{
    pfNotify(PFNFY_INFO,PFNFY_PRINT,
	"pfvLoadModule_pfvmEarthSky from %s called!\n",__FILE__);
    pfvmEarthSky*m= new pfvmEarthSky(xml);
    return (pfvModule*)m;
}

///////////////////////////////////////////////////////////////////////////////

