
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
#include <Performer/pf.h>
#include <Performer/pr.h>
#include <Performer/pfutil.h>

#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfChannel.h>

#include <Performer/pr/pfList.h>
#include <Performer/pr/pfLinMath.h>

#include <Performer/pfv/pfvXml.h>

#include <Performer/pfv/pfvDisplay.h>
#include <Performer/pfv/pfvInput.h>

#include "pfvmTrackball.h"

///////////////////////////////////////////////////////////////////////////////

pfType* pfvmTrackball::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::init()
{
    if(classType==NULL)
    {
	pfvModule::init();
	classType= new pfType(pfvModule::getClassType(), "pfvmTrackball" );
    }
}

///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::construct()
{
    if(classType==NULL)
	init();
    setType(classType);

    viewInd = -1;
    view=NULL;
    mouseBufferLen = 3;
    
    //firstFrame=1;

    dragging=0;
    spinning=0;
    zooming=0;
    rolling=0;
    translating=0;

    rollScale = 180.0f;

    rotDist=7.0f;

    bindCallback(PFV_CB_POSTCONFIG);
    bindCallback(PFV_CB_FRAME);

    char keys[8];
    sprintf(keys," %c%c%c", PFVKEY_ESCAPE,PFVKEY_UPARROW,PFVKEY_DOWNARROW );
    bindKeys(keys);
}

///////////////////////////////////////////////////////////////////////////////

int pfvmTrackball::handleEvent(int evType, char key)
{
    if(key==PFVKEY_ESCAPE)
	pfExit();

    else if(key==PFVKEY_UPARROW)
    {
	//rotDist *= 0.95;
        return 0;
    }
    else if(key==PFVKEY_DOWNARROW)
    {
        //rotDist /= 0.95;
        return 0;
    }	
    else if(key==' ')
    {
	if(view)
	{
	    if( !(pfvInputMngr::getMouse()->modifiers & PFUDEV_MOD_SHIFT))
	    	view->autoPos();
    	    resetPivot();
	}
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

pfvmTrackball::pfvmTrackball(pfvXmlNode* root)
{
    construct();
    
    if(root!=NULL)
	parseXml(root);
}

///////////////////////////////////////////////////////////////////////////////

int pfvmTrackball::setXmlField(pfvXmlNode* xml)
{
    if(!xml->nameCmp("viewIndex"))
    {
	xml->getInt(&viewInd);
	/*printf("pfvmTrackball::setXmlField - viewIndex = %d\n", viewInd );*/
	return 1;
    }  	
    if(!xml->nameCmp("mouseBufferLen"))
    {
	xml->getInt(&mouseBufferLen);
	if(mouseBufferLen>15)
	    mouseBufferLen=15;
	else if(mouseBufferLen<1)
	    mouseBufferLen=1;
	/*printf("pfvmTrackball::setXmlField - mouseBufferLen = %d\n",mouseBufferLen);*/
	return 1;
    }

    return pfvModule::setXmlField(xml);
}

///////////////////////////////////////////////////////////////////////////////

pfvmTrackball::~pfvmTrackball()
{
    return;
}

///////////////////////////////////////////////////////////////////////////////
		
void pfvmTrackball::postConfig()
{
    int i;
    int n=viewer->getNumViews();

    if(getScope()==PFV_SCOPE_VIEW)
    {
	view=(pfvView*)getScopeTarget();
	viewInd=view->getIndex();
    }
    else
    {
    	if(getScope()==PFV_SCOPE_WORLD)
	{
	    pfNotify( PFNFY_WARN, PFNFY_PRINT, "pfvmTrackball::postConfig - "
		"pfvmTrackball module cannot be scoped to a world!\n");
	}
	viewInd=0;
	view=viewer->getView(0);
    }
 }

///////////////////////////////////////////////////////////////////////////////

void ProjectMouseOntoSphere( pfVec3& pt )
{
    pt[1]=0.0f;

    pfvInputMngr::getViewNormXY(&(pt[0]),&(pt[2]));
    pt[0]-=0.5f; 
    pt[0]*=2.0f; 
    pt[2]-=0.5f;
    pt[2]*=2.0f;
	
    float len = pt.length();
    if(len>1.0f)
	pt.normalize();
    else
	pt[1] = 1.0f - (pt[0]*pt[0]) - (pt[2]*pt[2]) ;

    pt.normalize();
    	
   /* pfNotify(PFNFY_ALWAYS, PFNFY_PRINT, "ProjectMouseOntoSphere "
	"result: %f %f %f", pt[0], pt[1], pt[2] );    */
}

///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::resetPivot()
{
    pfScene* s = view->getScene();
    pfBox box;
    pfuTravCalcBBox( s, &box );
    pivot.combine(0.5f,box.min,0.5f,box.max);

    pfVec3 eyeXYZ, eyeHPR;
    view->getEye(eyeXYZ,eyeHPR);
    pfVec3 eye2pivot = pivot - eyeXYZ;
    rotDist=eye2pivot.length();
    if(rotDist)
	eye2pivot /= rotDist;
    eye2pivot.normalize();

    pfMatrix chanMat;
    view->getMasterChan()->getViewMat(chanMat);
    pfMatrix invChanMat;
    invChanMat.invertOrtho(chanMat);
 
    pfVec3 ahead(0.0f,1.0f,0.0f);  
    //ahead.xformVec(ahead,resultMat);
    eye2pivot.xformVec(eye2pivot,invChanMat);
	    
    rotMat.makeVecRotVec(ahead,eye2pivot);
    //rotMat.postMult(invChanMat);
	    
    chanMat.preMult(rotMat);
    chanMat.getOrthoCoord(&resultCoord);
    view->setEye( resultCoord.xyz, resultCoord.hpr );
}

///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::startDragging()
{
    view->getMasterChan()->getViewMat(startChanMat);
    view->getMasterChan()->getView(startChanXYZ,startChanHPR);
    ProjectMouseOntoSphere(startPt);
    for(int i=0;i<mouseBufferLen;i++)
	mouseBuffer[i]=startPt;

    spinning=0;
    rolling=0;
    dragging=1;
    zooming=0;
    translating=0;
}


///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::drag()
{
    pfVec3 pt;
    ProjectMouseOntoSphere(pt);
    for(int i=0;i<mouseBufferLen-1;i++)
	mouseBuffer[i]=mouseBuffer[i+1];
    mouseBuffer[mouseBufferLen-1]=pt;

    
    if( pt.equal(startPt) )
    {
	view->setEye(startChanXYZ,startChanHPR);
	return;
    }
			    
    rotMat.makeVecRotVec(startPt,pt);
    resultMat.makeTrans(0.0f,-rotDist,0.0f);
    resultMat.postMult(rotMat);
    resultMat.postTrans(resultMat,0.0f,rotDist,0.0f);
    resultMat.postMult(startChanMat);

    resultMat.getOrthoCoord(&resultCoord);
    view->setEye( resultCoord.xyz, resultCoord.hpr );
}

///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::startSpinning()
{
    //pfNotify(PFNFY_ALWAYS,PFNFY_PRINT,"pfvmTrackball::startSpinning called");
    
    if(dragging)
    {
	if( mouseBuffer[0].equal(mouseBuffer[mouseBufferLen-1]) )
	{
	    spinning=0;
	}
	else
	{
	    rotMat.makeVecRotVec(mouseBuffer[0],mouseBuffer[mouseBufferLen-1]);
	    spinMat.makeTrans( 0.0,-rotDist,0.0f);
	    spinMat.postMult(rotMat);
	    spinMat.postTrans(spinMat,0.0f,rotDist,0.0f);
	    spinning=1;
	    /*pfNotify(PFNFY_ALWAYS,PFNFY_PRINT,"pfvmTrackball now spinning");*/
	}
    }
    else if(rolling)
    {
	if( yBuffer[0]==yBuffer[mouseBufferLen-1] )
	{
	    spinning=0;
	}
	else
	{
	    float degrees = (yBuffer[mouseBufferLen-1]-yBuffer[0])*rollScale;
    	    spinMat.makeRot(degrees,0.0f,1.0f,0.0f);
	    spinning=1;
	}
    }
    dragging=0;
    rolling=0;
    zooming=0;
    translating=0;
}

///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::spin()
{
    //pfNotify(PFNFY_ALWAYS,PFNFY_PRINT,"pfvmTrackball::spin called");
    pfMatrix mat;
    view->getMasterChan()->getViewMat(mat);
    mat.preMult(spinMat);
    mat.getOrthoCoord(&resultCoord);
    view->setEye( resultCoord.xyz, resultCoord.hpr );
}

///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::startRolling()
{
    view->getMasterChan()->getViewMat(startChanMat);
    pfvInputMngr::getViewNormXY(NULL,&startY);
    for(int i=0;i<mouseBufferLen;i++)
	yBuffer[i]=startY;
    rolling=1;
    spinning=0;
    dragging=0;
    zooming=0;
    translating=0;
}

///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::roll()
{
    float y;
    pfvInputMngr::getViewNormXY(NULL,&y);
    for(int i=0;i<mouseBufferLen-1;i++)
	yBuffer[i]=yBuffer[i+1];
    yBuffer[mouseBufferLen-1]=y;

    float degrees = (y-startY)*rollScale;
    rotMat.makeRot(degrees,0.0f,1.0f,0.0f);
    
    pfMatrix chanMat=startChanMat;
    chanMat.preMult(rotMat);
    chanMat.getOrthoCoord(&resultCoord);
    view->setEye( resultCoord.xyz, resultCoord.hpr );

}

///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::startZooming()
{
    view->getMasterChan()->getViewMat(startChanMat);
    pfvInputMngr::getViewNormXY(NULL,&startY);
    for(int i=0;i<mouseBufferLen;i++)
	yBuffer[i]=startY;
    zoomStart=rotDist;
    rolling=0;
    spinning=0;
    dragging=0;
    zooming=1;
    translating=0;
}

///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::zoom()
{
    float y;
    pfvInputMngr::getViewNormXY(NULL,&y);
    for(int i=0;i<mouseBufferLen-1;i++)
	yBuffer[i]=yBuffer[i+1];
    yBuffer[mouseBufferLen-1]=y;

    float dist = (y-startY)*zoomStart;
    rotDist=zoomStart-dist;
    rotMat.makeTrans(0.0f,dist,0.0f);
    
    pfMatrix chanMat=startChanMat;
    chanMat.preMult(rotMat);
    chanMat.getOrthoCoord(&resultCoord);
    view->setEye( resultCoord.xyz, resultCoord.hpr );
}


///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::startTranslating()
{
    pfChannel* chan = view->getMasterChan();
    chan->getViewMat(startChanMat);
    
    pfVec3 xyz, hpr;
    view->getEye(xyz,hpr);
    rotMat.makeEuler( hpr[0], hpr[1], hpr[2] );
    
    pfvInputMngr::getViewNormXY(&startX,&startY);
    for(int i=0;i<mouseBufferLen;i++)
    {
	xBuffer[i]=startX;
	yBuffer[i]=startY;
    }
    
    float _nearP,_farP;
    chan->getNearFar(&_nearP,&_farP);
    float fovH,fovV;
    chan->getFOV(&fovH,&fovV);
    
    xScale=_nearP*pfTan(fovH*0.5f);
    yScale=_nearP*pfTan(fovV*0.5f);
    transRange = rotDist/_nearP;
  
    rolling=0;
    spinning=0;
    dragging=0;
    zooming=0;
    translating=1;
}

///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::translate()
{
    float x,y;
    pfvInputMngr::getViewNormXY(&x,&y);
    for(int i=0;i<mouseBufferLen-1;i++)
    {
	xBuffer[i]=xBuffer[i+1];
	yBuffer[i]=yBuffer[i+1];
    }
    xBuffer[mouseBufferLen-1]=x;
    yBuffer[mouseBufferLen-1]=y;


    pfVec3 axial( (x-startX)*xScale, 0.0f, (y-startY)*yScale );
    //axial.xformPt(axial, rotMat); /*was vec*/
    axial *= -transRange;

    pfMatrix transMat;
    transMat.makeTrans(axial[0],axial[1],axial[2]);

    pfMatrix chanMat=startChanMat;
    chanMat.preMult(transMat);
    chanMat.getOrthoCoord(&resultCoord);
    view->setEye( resultCoord.xyz, resultCoord.hpr );
}


///////////////////////////////////////////////////////////////////////////////

void pfvmTrackball::frame()
{
    int btn = pfvInputMngr::getMouse()->flags&PFUDEV_MOUSE_DOWN_MASK;
    pfvView* focusView = viewer->getView(pfvInputMngr::getFocusViewIndex());

    if(focusView!=view)
        btn=0;
    	
    if( btn==0 )
    {
	if(dragging||rolling)
	    startSpinning();
	else if(spinning)
	    spin();
	dragging=0;
	rolling=0;
	zooming=0;
    }
    else
	spinning=0;

    if( btn == PFUDEV_MOUSE_MIDDLE_DOWN )
    {
	if(!(pfvInputMngr::getMouse()->modifiers & PFUDEV_MOD_SHIFT))
        {
	    if(dragging)
	    	drag();
	    else
	    	startDragging();
	}
	else
	{
	    if(translating)
		translate();
	    else
		startTranslating();
	}
    }
    else
    {
	dragging=0;
        translating=0;
    }
    
    if( btn == (PFUDEV_MOUSE_MIDDLE_DOWN|PFUDEV_MOUSE_LEFT_DOWN) )
    /*if( btn == PFUDEV_MOUSE_RIGHT_DOWN )*/
    {
	if(rolling)
	    roll();
	else
	    startRolling();
    }
    else
	rolling=0;


    if( btn == (PFUDEV_MOUSE_MIDDLE_DOWN|PFUDEV_MOUSE_RIGHT_DOWN) )
    {
	if(zooming)
	    zoom();
	else
	    startZooming();
    }
    else
	zooming=0;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" PFVM_DLLEXPORT
pfvModule* pfvLoadModule_pfvmTrackball(pfvXmlNode*xml)
{
    pfNotify(PFNFY_INFO,PFNFY_PRINT,
	"pfvLoadModule_pfvmTrackball from %s called!\n",__FILE__);
    pfvmTrackball*m= new pfvmTrackball(xml);
    return (pfvModule*)m;
}

///////////////////////////////////////////////////////////////////////////////
