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

#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfDCS.h>

#include <Performer/pfv/pfvXml.h>
#include <Performer/pfv/pfvViewer.h>

#include "pfvmLoader.h"

///////////////////////////////////////////////////////////////////////////////

pfType* pfvmLoaderModel::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void pfvmLoaderModel::init()
{
	if(classType==NULL)
	{
		pfvObject::init();
		classType= new pfType(pfvObject::getClassType(), "pfvmLoaderModel" );
	}
}

///////////////////////////////////////////////////////////////////////////////

void pfvmLoaderModel::construct()
{
	if(classType==NULL)
		init();
	setType(classType);

	filePath=NULL;
	fileName=NULL;
	addToScene=1;
	dcs=NULL;
	trans.set(0.0f,0.0f,0.0f);
	rot.set(0.0f,0.0f,0.0f);
	scale.set(1.0f,1.0f,1.0f);
	root=NULL;
}


///////////////////////////////////////////////////////////////////////////////

int pfvmLoaderModel::setXmlField(pfvXmlNode* xml)
{
	float f[3];
	if(xml==NULL)
		return 0;
	
	if(!xml->nameCmp("Name"))
		setName(xml->getValue());
	
	else if(!xml->nameCmp("FileName"))
		setFileName(xml->getValue());

	else if(!xml->nameCmp("FilePath"))
		setFilePath(xml->getValue());

	else if(!xml->nameCmp("Trans"))
	{
		if(xml->getFloatArray(3,f)!=3)
		{
			pfNotify(PFNFY_WARN,PFNFY_PRINT,"pfvmLoaderModel::setXmlField - "
					"invalid Trans setting %s\n", xml->getValue());
		}
		else
			trans.set(f[0],f[1],f[2]);
	}
		
	else if(!xml->nameCmp("Rot"))
	{
		if(xml->getFloatArray(3,f)!=3)
		{
			pfNotify(PFNFY_WARN,PFNFY_PRINT,"pfvmLoaderModel::setXmlField - "
					"invalid Rot setting %s\n", xml->getValue());
		}
		else
			rot.set(f[0],f[1],f[2]);
	}

	else if(!xml->nameCmp("Scale"))
	{
		int n=xml->getFloatArray(3,f);
		if(n==1)
			scale.set(f[0],f[0],f[0]);
		else if(n==3)
			scale.set(f[0],f[1],f[2]);
		else
		{
			pfNotify(PFNFY_WARN,PFNFY_PRINT,"pfvmLoaderModel::setXmlField - "
					"invalid Scale setting %s\n", xml->getValue());
		}		
	}

	
	else
	{
		pfNotify(PFNFY_WARN,PFNFY_PRINT,"pfvmLoader::setXmlField - "
				"invalid xml tag <%s>", xml->getName());
		return 0;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
#if 0
int pfvmLoaderModel::parseXml(pfvXmlNode*xmlRoot)
{
	if(xmlRoot==NULL)
		return 0;
	
    int i, n = xmlRoot->getNumChildren();
    for(i=0;i<n;i++)
    {
		pfvXmlNode* xml = xmlRoot->getChild(i);
		setXmlField(xml);
	}
	return 1;
}
#endif
///////////////////////////////////////////////////////////////////////////////

pfvmLoaderModel::pfvmLoaderModel(pfvXmlNode*xml)
{
	construct();
	if(xml)
		parseXml(xml);
}

///////////////////////////////////////////////////////////////////////////////

pfvmLoaderModel::~pfvmLoaderModel()
{
	if(filePath)pfFree(filePath);
	if(fileName)pfFree(fileName);
	// should we delete dcs and root?
}

///////////////////////////////////////////////////////////////////////////////

void pfvmLoaderModel::postConfig(pfGroup*rootNode)
{
	if(fileName==NULL || *fileName=='\0')
	{
		pfNotify(PFNFY_WARN,PFNFY_PRINT,
			"pfvmLoaderModel::postConfig - NULL fileName");
		return;
	}
	char* oldPath=NULL;
	if(filePath)
	{
		oldPath=(char*)pfGetFilePath();
		int oldlen = oldPath? strlen(oldPath) : 0;
		int len = oldlen+strlen(filePath)+3;
		char*newPath = (char*)calloc(len, sizeof(char));
		PFASSERTALWAYS(newPath);
		sprintf(newPath,"%s:%s",oldPath?oldPath:"",filePath);
		pfFilePath(newPath);
		free(newPath);
	}
	
	root = pfdLoadFile(fileName);

	if(filePath)
		pfFilePath(oldPath?oldPath:"");
	
	if(root==NULL)
	{
		pfNotify(PFNFY_WARN,PFNFY_PRINT,
			"pfvmLoaderModel::postConfig - failed to load model %s", fileName);
		return;
	}

	if( (trans[0]!=0.0f) || (trans[1]!=0.0f) || (trans[2]!=0.0f) ||
		(rot[0]!=0.0f) || (rot[1]!=0.0f) || (rot[2]!=0.0f) ||
		(scale[0]!=1.0f) || (scale[1]!=1.0f) || (scale[2]!=1.0f) )
	{
		dcs = new pfDCS;
		PFASSERTALWAYS(dcs);
		dcs->setTrans(trans[0],trans[1],trans[2]);
		dcs->setRot(rot[0],rot[1],rot[2]);
		dcs->setScale(scale[0],scale[1],scale[2]);
		dcs->addChild(root);
	}

	if(addToScene && (rootNode!=NULL))
	{
		rootNode->addChild( dcs? dcs:root );
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

pfType* pfvmLoader::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void pfvmLoader::init()
{
	if(classType==NULL)
	{
		pfvModule::init();
		classType= new pfType(pfvModule::getClassType(), "pfvmLoader" );
	}
}

///////////////////////////////////////////////////////////////////////////////

void pfvmLoader::construct()
{
	if(classType==NULL)
		init();
	setType(classType);

	filePath=NULL;
	listModels= new pfList;
	PFASSERTALWAYS(listModels);

	bindCallback(PFV_CB_PRECONFIG);
	bindCallback(PFV_CB_POSTCONFIG);
}

///////////////////////////////////////////////////////////////////////////////

int pfvmLoader::setXmlField(pfvXmlNode* xml)
{
	if(xml==NULL)
		return 0;
	
	if(!xml->nameCmp("Name"))
		setName(xml->getValue());
	
	else if(!xml->nameCmp("FilePath"))
		setFilePath(xml->getValue());
	
	else if(!xml->nameCmp("Model"))
	{
		pfvmLoaderModel* m = new pfvmLoaderModel(xml);
		if(m)
		{
			///m->setIndex(listModels->getNum());
			listModels->add(m);
		}
	}
	
	else
	{
		pfNotify(PFNFY_WARN,PFNFY_PRINT,"pfvmLoader::setXmlField - "
				"invalid xml tag <%s>", xml->getName());
		return 0;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

int pfvmLoader::parseXml(pfvXmlNode* xmlRoot)
{
	if(xmlRoot==NULL)
		return 0;
	
    int i, n = xmlRoot->getNumChildren();
    for(i=0;i<n;i++)
    {
		pfvXmlNode* xml = xmlRoot->getChild(i);
		setXmlField(xml);
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

pfvmLoader::pfvmLoader(pfvXmlNode*xml)
{
	construct();
	if(xml)
		parseXml(xml);
}

///////////////////////////////////////////////////////////////////////////////

pfvmLoader::~pfvmLoader()
{
	;
}

///////////////////////////////////////////////////////////////////////////////
	
void pfvmLoader::preConfig()
{
	int i,n=getNumModels();
	for(i=0;i<n;i++)
	{
		pfvmLoaderModel*m=getModel(i);
		m->preConfig();
	}
}
	
///////////////////////////////////////////////////////////////////////////////
	
void pfvmLoader::postConfig()
{
	pfGroup* scene=NULL;

	if(getScope()==PFV_SCOPE_WORLD)
	{
		pfvWorld* w = (pfvWorld*)getScopeTarget();
		if(w)
		    scene = (pfGroup*)(w->getScene());
		// printf("\n\npfvmLoader::postConfig - world:%x  scene:%x\n\n\n",w,scene);
	}
	else if(getScope()==PFV_SCOPE_VIEW)
	{
	    pfvView* view = (pfvView*)getScopeTarget();
	    scene = view->getRootNode();

#if 0
	    pfvWorld* w = view->getWorld();
	    if(w)
		scene = w->getScene();
#endif
	}
	else
	{
	    pfvViewer* viewer = pfvViewer::getViewer();
	    pfvWorld* w = viewer->getWorld(0);
	    if(w)
		scene = w->getScene();
	}
	
	if(scene==NULL)
		pfNotify(PFNFY_WARN,PFNFY_PRINT,"pfvmLoader::postConfig - scene is NULL");


	char* oldPath=NULL;
	if(filePath)
	{
		pfNotify( PFNFY_INFO, PFNFY_PRINT, 
		    "pfvmLoader::postConfig - filepath=<%s>",  filePath );

		oldPath=(char*)pfGetFilePath();
		int oldlen = oldPath? strlen(oldPath) : 0;
		int len = oldlen+strlen(filePath)+3;
		char*newPath = (char*)calloc(len, sizeof(char));
		PFASSERTALWAYS(newPath);
		sprintf(newPath,"%s:%s",oldPath?oldPath:"",filePath);
		pfFilePath(newPath);
		free(newPath);
	}

	int i,n=getNumModels();
	for(i=0;i<n;i++)
	{
		pfvmLoaderModel*m=getModel(i);
		m->postConfig(scene);
	}

	if(filePath)
	    pfFilePath(oldPath?oldPath:"");	
}

///////////////////////////////////////////////////////////////////////////////

extern "C" PFVM_DLLEXPORT
pfvModule* pfvLoadModule_pfvmLoader(pfvXmlNode*xml)
{
    pfNotify(PFNFY_INFO,PFNFY_PRINT,
	"pfvLoadModule_pfvmLoader from %s called!\n",__FILE__);
    pfvmLoader*m= new pfvmLoader(xml);
    return (pfvModule*)m;
}

///////////////////////////////////////////////////////////////////////////////

