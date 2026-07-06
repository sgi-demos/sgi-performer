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

#include <Performer/pfutil.h>

#include "pfvmSnapshot.h"

///////////////////////////////////////////////////////////////////////////////

pfType* pfvmSnapshot::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void pfvmSnapshot::init()
{
    if(classType==NULL)
    {
	pfvModule::init();
	classType= new pfType(pfvModule::getClassType(), "pfvmSnapshot" );
    }
}

///////////////////////////////////////////////////////////////////////////////

int pfvmSnapshot::handleEvent(int ev, char key)
{
    chanIndex=pfvInputMngr::getFocusChanIndex();
    
    pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"pfvmSnapshot::handleEvent called. "
	"scheduling snapshot for chan: %d", chanIndex );

    if(chanIndex>=0)
	bindCallback(PFV_CB_PREDRAW);
    return 1;
}
    
///////////////////////////////////////////////////////////////////////////////

void pfvmSnapshot::preDraw(pfvDispChan*chan)
{
    if(chan->getIndex()==chanIndex)
    {
	char filename[512];
	sprintf(filename,"%s%d.rgb%c", baseName, filesSaved++, saveAlpha?'a':'\0');

	if(singleChan) /* only save snapshot of channel on which key was pressed */
	{
	    pfNotify(PFNFY_INFO,PFNFY_PRINT,"pfvmSnapshot::handleEvent called. "
	    	"saving snapshot \"%s\" for channel: %d", filename, chanIndex);
	    float chanL,chanR,chanB,chanT;
	    chan->getViewport(&chanL,&chanR,&chanB,&chanT);
	    int pwinW,pwinH;
	    chan->getPWin()->getSize(&pwinW,&pwinH);
	    chanL*=pwinW; chanR*=pwinW;
	    chanB*=pwinH; chanT*=pwinH;
	    pfuSaveImage( filename, (int)chanL, (int)chanB, 
			((int)(chanR-chanL))+1, ((int)(chanT-chanB))+1, saveAlpha );
	}
	else /* save a snapshot of the entire pipe-window */
	{
	    pfNotify(PFNFY_INFO,PFNFY_PRINT,"pfvmSnapshot::handleEvent called. "
	    	"saving snapshot \"%s\" for pipewindow: %d", 
		    filename, chan->getPWin()->getIndex());
	    int pwinW,pwinH;
	    chan->getPWin()->getSize(&pwinW,&pwinH);
	    pfuSaveImage( filename, 0, 0, pwinW, pwinH, saveAlpha );
	}
	unbindCallback(PFV_CB_PREDRAW);
    }
}

///////////////////////////////////////////////////////////////////////////////

void pfvmSnapshot::postConfig()
{
    pfNotify(PFNFY_INFO,PFNFY_PRINT,
	"pfvmSnapshot::postConfig called. module:%x", this );
    char k[2];
    sprintf(k,"%c",key);
    bindKeys(k);
}

///////////////////////////////////////////////////////////////////////////////

void pfvmSnapshot::construct()
{
    if(classType==NULL)
	init();
    setType(classType);

    sprintf(baseName,"snap");
    filesSaved=0;
    saveAlpha=0;
    singleChan=0;
    key=PFVKEY_F12;
    
    bindCallback(PFV_CB_POSTCONFIG);
}

///////////////////////////////////////////////////////////////////////////////

int pfvmSnapshot::setXmlField(pfvXmlNode* xml)
{
    if(xml==NULL)
	return 0;
	
    if(!xml->nameCmp("BaseName"))
    {
        strcpy(baseName,xml->getValue());
	pfNotify(PFNFY_INFO,PFNFY_PRINT, "pfvmSnapshot::setXmlField"
	    "baseName = <%s>", baseName );
	return 1;
    }	
    if(!xml->nameCmp("SingleChan"))
    {
        xml->getBool(&singleChan);
	pfNotify(PFNFY_INFO,PFNFY_PRINT, "pfvmSnapshot::setXmlField"
	    "singleChan = %s", singleChan? "TRUE":"FALSE");
	return 1;
    }
    if(!xml->nameCmp("SaveAlpha"))
    {
        xml->getBool(&saveAlpha);
	pfNotify(PFNFY_INFO,PFNFY_PRINT, "pfvmSnapshot::setXmlField"
	    "saveAlpha = %s", saveAlpha? "TRUE":"FALSE");
	return 1;
    }  
	
    if(!xml->nameCmp("key"))
    {
       	xml->getInputMngrKey(&key);
	pfNotify(PFNFY_INFO,PFNFY_PRINT, "pfvmSnapshot::setXmlField"
	     "key = %s", pfvInputMngr::getKeyName(key) );
	return 1;
    }  

    else return pfvModule::setXmlField(xml);
}

///////////////////////////////////////////////////////////////////////////////

pfvmSnapshot::pfvmSnapshot(pfvXmlNode*xml)
{
    construct();
    if(xml)
	parseXml(xml);
}
	
///////////////////////////////////////////////////////////////////////////////

extern "C" PFVM_DLLEXPORT
pfvModule* pfvLoadModule_pfvmSnapshot(pfvXmlNode*xml)
{
    pfNotify(PFNFY_INFO,PFNFY_PRINT,
	"pfvLoadModule_pfvmSnapshot from %s called!\n",__FILE__);
    pfvmSnapshot*m= new pfvmSnapshot(xml);
    return (pfvModule*)m;
}

///////////////////////////////////////////////////////////////////////////////

