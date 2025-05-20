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

#include "pfvmWorldSwitcher.h"

///////////////////////////////////////////////////////////////////////////////

pfType* pfvmWorldSwitcher::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void pfvmWorldSwitcher::init()
{
    if(classType==NULL)
    {
        pfvModule::init();
        classType= new pfType(pfvModule::getClassType(), "pfvmWorldSwitcher" );
    }
}

///////////////////////////////////////////////////////////////////////////////

pfvmWorldSwitcher::pfvmWorldSwitcher(pfvXmlNode*xml)
{
    construct();
    if(xml)
        parseXml(xml);
}
	
///////////////////////////////////////////////////////////////////////////////

void pfvmWorldSwitcher::construct()
{
    if(classType==NULL)
        init();
    setType(classType);
	
    listWS = new pfList;
    PFASSERTALWAYS(listWS);
    curSwitch=NULL;
	
    bindCallback(PFV_CB_POSTCONFIG);
}

///////////////////////////////////////////////////////////////////////////////

int pfvmWorldSwitcher::setXmlField(pfvXmlNode* xml)
{
    if(xml==NULL)
        return 0;
	
    if(!xml->nameCmp("switch"))
    {
        int k=0;
        pfvWorld* w=NULL;
        pfVec4 col(0.0,0.0,0.0,1.0);
        float dur=-1.0;
		
        int i,n=xml->getNumChildren();
        for(i=0;i<n;i++)
        {
            pfvXmlNode*child = xml->getChild(i);
            if(!child->nameCmp("key"))
            {
                child->getInputMngrKey(&k);
                continue;
            }
            if(!child->nameCmp("world"))
            {
                w = viewer->findWorld(child->getValue());
                continue;
            }
            if(!child->nameCmp("fadetime"))
            {
                child->getFloat(&dur);
                continue;
            }
            if(!child->nameCmp("fadecol"))
            {
                child->getFloatArray(4,(float*)(&col[0]));
                continue;
            }
			
            pfNotify(PFNFY_WARN,PFNFY_PRINT,
                "pfvmWorldSwitcher::setXmlField - invalid xml token"
                " <%s> in switch definition", child->getName());
        }
        if( (k!=0) && (w!=NULL) )
        {
            pfNotify(PFNFY_DEBUG,PFNFY_PRINT,
                "pfvmWorldSwitcher::setXmlField - "
                "adding a WorldSwitch (%s,%s,%f) to list",
                 pfvInputMngr::getKeyName((int)k), w->getName(),dur );

             WorldSwitch*WS = new WorldSwitch(k,w,dur,col);
             listWS->add( WS );
        }
        else
            pfNotify(PFNFY_WARN,PFNFY_PRINT,
                "pfvmWorldSwitcher::setXmlField - "
                "Incomplete WorldSwitch data (%d,%x) to list", k, w );

        return 1;
    }		
    
    return pfvModule::setXmlField(xml);
}

///////////////////////////////////////////////////////////////////////////////

void pfvmWorldSwitcher::overlay(pfvDispChan*chan)
{
    if(curSwitch==NULL)
        return;

    if(chan->getView()!=fadeView)
        return;
    
    pfPushState();
    pfTransparency(PFTR_HIGH_QUALITY);

    glColor4f( curSwitch->fadeCol[0], 
                curSwitch->fadeCol[1],
                curSwitch->fadeCol[2],
                curSwitch->fadeCol[3]*alpha );
	
    glBegin(GL_QUADS);
	glVertex2f(0.0,0.0);
	glVertex2f(1.0,0.0);
	glVertex2f(1.0,1.0);
	glVertex2f(0.0,1.0);
    glEnd();
		
    pfPopState();		
}


///////////////////////////////////////////////////////////////////////////////

void pfvmWorldSwitcher::frame()
{
    if(curSwitch==NULL)
        return;

    float time = pfGetFrameTimeStamp();
	
    if(time<=fadeStart)
        return;

    alpha = (time-fadeStart) / curSwitch->fadeDur;

    if(switchDone==0)
    {
        if(alpha>1.0f)
        {
            fadeView->setTargetWorld(curSwitch->world);
            switchDone=1;
            alpha=1.0f;
            fadeStart=time;
        }
    }
    else
    {
        alpha=1.0-alpha;
		
        if(alpha<0.0f)
        {
            alpha=0.0f;
            curSwitch=NULL;
            unbindCallback(PFV_CB_OVERLAY);
            unbindCallback(PFV_CB_FRAME);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void pfvmWorldSwitcher::postConfig()
{
    char keys[128];
    char keyNames[1024];
    keyNames[0]='\0';

    int i,n=listWS->getNum();
    for(i=0;i<n;i++)
    {
        char buf[32];
        WorldSwitch* ws = (WorldSwitch*)(listWS->get(i));
        keys[i] = (char)(ws->key);
        sprintf(buf,"%s%s", (i==0)?"":" ", pfvInputMngr::getKeyName(ws->key) );
        strcat(keyNames,buf);
    }
    if(n>0)
    {
        pfNotify(PFNFY_DEBUG,PFNFY_PRINT,
            "pfvmWorldSwitcher::postConfig - binding keys: %s", keyNames );
        keys[n]='\0';
        bindKeys(keys);
    }
    else
        pfNotify(PFNFY_WARN,PFNFY_PRINT,
            "pfvmWorldSwitcher::postConfig - NOT binding ANY keys");
}
	
///////////////////////////////////////////////////////////////////////////////

int pfvmWorldSwitcher::handleEvent(int evType, char key)
{
    /*pfNotify(PFNFY_DEBUG,PFNFY_PRINT,"pfvmWorldSwitcher::handleEvent - "
        "key = %s", pfvInputMngr::getKeyName(key) );*/

    if(curSwitch!=NULL) // currently fading..
        return 1; // grab event anyways
	
    pfvView*view=viewer->getView(pfvInputMngr::getFocusViewIndex());

    if(view==NULL)
        return 0;

    int i,n=listWS->getNum();
    for(i=0;i<n;i++)
    {
        WorldSwitch* ws = (WorldSwitch*)(listWS->get(i));
        if(ws->key==key)
        {
            if(view->getTargetWorld()!=ws->world)
            {
                if(ws->fadeDur<=0.0f)
                    view->setTargetWorld(ws->world);
                else
                {
                    fadeStart=pfGetFrameTimeStamp();
                    bindCallback(PFV_CB_FRAME);
                    bindCallback(PFV_CB_OVERLAY);
                    switchDone=0;
                    fadeView=view;
                    curSwitch=ws;
                }
                return 1;
            }
        }	
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" PFVM_DLLEXPORT
pfvModule* pfvLoadModule_pfvmWorldSwitcher(pfvXmlNode*xml)
{
    pfNotify(PFNFY_INFO,PFNFY_PRINT,
	"pfvLoadModule_pfvmWorldSwitcher from %s called!\n",__FILE__);
    pfvmWorldSwitcher*m= new pfvmWorldSwitcher(xml);
    return (pfvModule*)m;
}

///////////////////////////////////////////////////////////////////////////////

