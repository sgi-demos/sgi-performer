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
#include <Performer/pfv/pfvXml.h>
#include <Performer/pfv/pfvViewer.h>
#include <Performer/pf/pfFrameStats.h>
#include <Performer/pf/pfChannel.h>
#include "pfvmStats.h"

///////////////////////////////////////////////////////////////////////////////

pfType* pfvmStats::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void pfvmStats::init()
{
    if(classType==NULL)
    {
        pfvModule::init();
	classType= new pfType(pfvModule::getClassType(), "pfvmStats" );
    }
}

///////////////////////////////////////////////////////////////////////////////

void pfvmStats::construct()
{
    if(classType==NULL)
	init();
    setType(classType);
    onOff=0;
    key=PFVKEY_s;

    updateFrames = 2.0f;

    flagCpu = 0;
    flagDb = 0;
    flagCull = 0;
    flagFill = 0;
    flagGfx = 0;
    flagTimes = 0;

    flagAttrCounts = 0;
    flagTStrip = 0;
    flagHistory = 1;
    flagTranspFill = 0;
    flagDbVis = 0;
    flagDepthCmp = 0;
    
    bindCallback(PFV_CB_POSTCONFIG);
    /*bindCallback(PFV_CB_POSTDRAW);*/
}

///////////////////////////////////////////////////////////////////////////////

int pfvmStats::setXmlField(pfvXmlNode* xml)
{
    if(xml==NULL)
	return 0;
	
    if(!xml->nameCmp("OnOff"))
    {
	xml->getBool(&onOff);
	return 1;
    }	
    if(!xml->nameCmp("Key"))
    {
        xml->getInputMngrKey(&key);
	return 1;
    }
    
    if(!xml->nameCmp("updateFrames"))
    {
        xml->getFloat(&updateFrames);
	return 1;
    }
    if(!xml->nameCmp("Cpu"))
    {
        xml->getBool(&flagCpu);
	return 1;
    }

    if(!xml->nameCmp("DB"))
    {
        xml->getBool(&flagDb);
	return 1;
    }

    if(!xml->nameCmp("CULL"))
    {
        xml->getBool(&flagCull);
	return 1;
    }

    if(!xml->nameCmp("FILL"))
    {
        xml->getBool(&flagFill);
	return 1;
    }

    if(!xml->nameCmp("GFX"))
    {
        xml->getBool(&flagGfx);
	return 1;
    }

    if(!xml->nameCmp("TIMES"))
    {
        xml->getBool(&flagTimes);
	return 1;
    }

    if(!xml->nameCmp("AttrCounts"))
    {
        xml->getBool(&flagAttrCounts);
	return 1;
    }

    if(!xml->nameCmp("TStrips"))
    {
        xml->getBool(&flagTStrip);
	return 1;
    }

    if(!xml->nameCmp("TimeHist"))
    {
        xml->getBool(&flagHistory);
	return 1;
    }

    if(!xml->nameCmp("TranspFill"))
    {
        xml->getBool(&flagTranspFill);
	return 1;
    }

    if(!xml->nameCmp("DbVis"))
    {
        xml->getBool(&flagDbVis);
	return 1;
    }
    
    if(!xml->nameCmp("Depth"))
    {
        xml->getBool(&flagDepthCmp);
	return 1;
    }
    else return pfvModule::setXmlField(xml);
}

///////////////////////////////////////////////////////////////////////////////

pfvmStats::pfvmStats(pfvXmlNode*xml)
{
    construct();
    if(xml)
	parseXml(xml);
}
	
///////////////////////////////////////////////////////////////////////////////

void pfvmStats::postConfig()
{
    char keybuf[2];
    sprintf(keybuf,"%c",key);
    bindKeys(keybuf);

    if(getScope()==PFV_SCOPE_WORLD)
    {
	bindCallback(PFV_CB_ENTER_VIEW);
	bindCallback(PFV_CB_EXIT_VIEW);
    }
    else
    {
	if(onOff)
	    switchOn();
    }
}
	
///////////////////////////////////////////////////////////////////////////////

void pfvmStats::enterView(pfvView*view)
{
    if(onOff)
	switchOn(view);
}

///////////////////////////////////////////////////////////////////////////////

void pfvmStats::exitView(pfvView*view)
{
    if(onOff)
	switchOff(view);
}

///////////////////////////////////////////////////////////////////////////////

int pfvmStats::handleEvent(int evType, char key)
{
    if(onOff)
	switchOff();
    else
	switchOn();
    onOff^=1;

    return 1;
}

///////////////////////////////////////////////////////////////////////////////

void pfvmStats::postDraw(pfvDispChan*chan)
{
    if(!onOff)
	return;

    pfChannel* pfchan = chan->getHandle();
    pfchan->drawStats();
}

///////////////////////////////////////////////////////////////////////////////

void pfvmStats::switchOn(pfvView*view)
{
    if(view)
    {
	int i, n = view->getNumChans();
	for(i=0;i<n;i++)
	{
	    pfvDispChan*chan = view->getChan(i);
	    pfChannel* pfchan = chan->getHandle();
	    pfFrameStats *frameStats = pfchan->getFStats();

            frameStats->setClass(PFSTATS_ALL, PFSTATS_OFF);
	    
            frameStats->setAttr(PFFSTATS_UPDATE_FRAMES, updateFrames );

	    frameStats->setClass( PFSTATSHW_ENCPU, flagCpu ? PFSTATS_ON:PFSTATS_OFF);
	    frameStats->setClass( PFFSTATS_ENDB, flagDb ? PFSTATS_ON:PFSTATS_OFF);
	    frameStats->setClass( PFFSTATS_ENCULL, flagCull ? PFSTATS_ON:PFSTATS_OFF);
	    frameStats->setClass( PFSTATSHW_ENGFXPIPE_FILL, flagFill ? PFSTATS_ON:PFSTATS_OFF);
	    frameStats->setClass( PFSTATS_ENGFX, flagGfx ? PFSTATS_ON:PFSTATS_OFF);	    
	    frameStats->setClass( PFFSTATS_ENPFTIMES, flagTimes ? PFSTATS_ON:PFSTATS_OFF);
		    

	    frameStats->setClassMode( PFSTATS_GFX,PFSTATS_GFX_ATTR_COUNTS,
						flagAttrCounts ? PFSTATS_ON:PFSTATS_OFF );

	    frameStats->setClassMode( PFSTATS_GFX, PFSTATS_GFX_TSTRIP_LENGTHS,
			    	flagTStrip ? PFSTATS_ON:PFSTATS_OFF );

	    frameStats->setClassMode( PFFSTATS_PFTIMES,PFFSTATS_PFTIMES_HIST,
			    flagHistory ? PFSTATS_ON:PFSTATS_OFF );

 	    frameStats->setClassMode( PFSTATSHW_GFXPIPE_FILL,
		PFSTATSHW_GFXPIPE_FILL_TRANSPARENT, 
		flagTranspFill ? PFSTATS_ON:PFSTATS_OFF );
			    
 	    frameStats->setClassMode( PFFSTATS_DB, PFFSTATS_DB_VIS,
			    flagDbVis ? PFSTATS_ON:PFSTATS_OFF );

	    frameStats->setClassMode( PFSTATSHW_GFXPIPE_FILL, 
			    PFSTATSHW_GFXPIPE_FILL_DEPTHCMP,
			    flagDepthCmp ? PFSTATS_ON:PFSTATS_OFF );	    
        }
    }
    else
    {
	int i, n=viewer->getNumViews();
	uint64_t vmask = getViewMask();
	for(i=0; i<n; i++)
	{
	    if( (vmask==0) || (vmask & (1<<i)) )
	        switchOn(viewer->getView(i));
	}
    }
    bindCallback(PFV_CB_POSTDRAW);
}

///////////////////////////////////////////////////////////////////////////////

void pfvmStats::switchOff(pfvView*view)
{
    if(view)
    {
	int i, n = view->getNumChans();
	for(i=0;i<n;i++)
	{
	    pfvDispChan*chan = view->getChan(i);
	    pfChannel* pfchan = chan->getHandle();
	    pfFrameStats *frameStats = pfchan->getFStats();
            frameStats->setClass(PFSTATS_ALL, PFSTATS_OFF);
        }
    }
    else
    {
	int i, n=viewer->getNumViews();
	uint64_t vmask = getViewMask();
	for(i=0; i<n; i++)
	{
	    if( (vmask==0) || (vmask & (1<<i)) )
	        switchOff(viewer->getView(i));
	}
	unbindCallback(PFV_CB_POSTDRAW);
    }
}

///////////////////////////////////////////////////////////////////////////////

extern "C" PFVM_DLLEXPORT 
pfvModule* pfvLoadModule_pfvmStats(pfvXmlNode*xml)
{
    pfvmStats*m= new pfvmStats(xml);
    return (pfvModule*)m;
}

///////////////////////////////////////////////////////////////////////////////

