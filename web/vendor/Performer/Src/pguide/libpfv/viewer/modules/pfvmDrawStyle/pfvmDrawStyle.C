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
#include <Performer/pfutil.h>

#include <Performer/pfv/pfvXml.h>
#include <Performer/pfv/pfvViewer.h>

#include "pfvmDrawStyle.h"

///////////////////////////////////////////////////////////////////////////////

pfType* pfvmDrawStyle::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void pfvmDrawStyle::init()
{
	if(classType==NULL)
	{
		pfvModule::init();
		classType= new pfType(pfvModule::getClassType(), "pfvmDrawStyle" );
	}
}

///////////////////////////////////////////////////////////////////////////////

void pfvmDrawStyle::construct()
{
	if(classType==NULL)
		init();
	setType(classType);

	scribeColor.set(1.0f,1.0f,1.0f,1.0f);
	drawStyle = PFUSTYLE_FILLED;
	backface=0;
	lighting=1;
	texture=1;
	
	bindCallback(PFV_CB_PREDRAW);
	bindCallback(PFV_CB_POSTDRAW);
	bindKeys("wWtlb");
}

///////////////////////////////////////////////////////////////////////////////

int pfvmDrawStyle::setXmlField(pfvXmlNode* xml)
{
	if(xml==NULL)
		return 0;
	
	if(!xml->nameCmp("Name"))
		setName(xml->getValue());
	
	else if(!xml->nameCmp("Texture"))
		xml->getBool(&texture);
	
	else if(!xml->nameCmp("Backface"))
		xml->getBool(&backface);

	else if(!xml->nameCmp("Lighting"))
		xml->getBool(&lighting);

	else if(!xml->nameCmp("DrawStyle"))
	{
		xml->getInt(&drawStyle);
		if(drawStyle>=PFUSTYLE_COUNT)
			drawStyle=PFUSTYLE_FILLED;
	}

	else
	{
		pfNotify(PFNFY_WARN,PFNFY_PRINT,"pfvmDrawStyle::setXmlField - "
				"invalid xml tag <%s>", xml->getName());
		return 0;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
#if 0
int DrawStyle::parseXml(pfvXmlNode* xmlRoot)
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

pfvmDrawStyle::pfvmDrawStyle(pfvXmlNode*xml)
{
	construct();
	if(xml)
		parseXml(xml);
}
	
///////////////////////////////////////////////////////////////////////////////

pfvmDrawStyle::~pfvmDrawStyle()
{
	;
}

///////////////////////////////////////////////////////////////////////////////

int pfvmDrawStyle::handleEvent(int evType, char key)
{
	if( key=='w' )
	{
		if(drawStyle==PFUSTYLE_FILLED)
			drawStyle=PFUSTYLE_LINES;
		else
			drawStyle=PFUSTYLE_FILLED;
		return 1;
	}
	
	if( key=='W' )
	{
		drawStyle++;
		if(drawStyle>=PFUSTYLE_COUNT)
			drawStyle=PFUSTYLE_FILLED;
		/*printf("drawStyle=%d\n",drawStyle);*/
		return 1;
	}

	if( key=='t' )
	{
		texture ^= 1;
		return 1;
	}

	if( key=='b' )
	{
		backface ^= 1;
		return 1;
	}

	if( key=='l' )
	{
		lighting ^= 1;
		return 1;
	}

	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void pfvmDrawStyle::preDraw(pfvDispChan*chan)
{
	//printf("pfvmDrawStyle::preDraw called\n");

	if(backface)
	{
	    /* leave backface removal to loader state */
	    pfOverride(PFSTATE_CULLFACE, 0); 
	    pfCullFace(PFCF_BACK);
	}
	else
	{
	    pfOverride(PFSTATE_CULLFACE, 0);
		pfCullFace(PFCF_OFF);
	    /* really force backface removal off */
	    pfOverride(PFSTATE_CULLFACE, 1); 
	}

	if (lighting == 0)
	{
	    pfOverride(PFSTATE_ENLIGHTING | PFSTATE_FRONTMTL, 0);
	    pfDisable(PFEN_LIGHTING);
	    pfOverride(PFSTATE_ENLIGHTING | PFSTATE_FRONTMTL, 1);
	}
	else
	    pfOverride(PFSTATE_ENLIGHTING | PFSTATE_FRONTMTL, 0);

	if (texture == 0)
	{
	    pfOverride(PFSTATE_ENTEXTURE | PFSTATE_TEXTURE | PFSTATE_ENTEXGEN, 0);
	    pfDisable(PFEN_TEXTURE);
	    pfDisable(PFEN_TEXGEN);
	    pfOverride(PFSTATE_ENTEXTURE | PFSTATE_TEXTURE | PFSTATE_ENTEXGEN, 1);
	}
	else
	    pfOverride(PFSTATE_ENTEXTURE | PFSTATE_TEXTURE | PFSTATE_ENTEXGEN, 0);
	
	curpfvmDrawStyle=drawStyle;
	pfuPreDrawStyle(curpfvmDrawStyle, scribeColor);
}

///////////////////////////////////////////////////////////////////////////////

void pfvmDrawStyle::postDraw(pfvDispChan*chan)
{
	//printf("pfvmDrawStyle::postDraw called\n");
	pfuPostDrawStyle(curpfvmDrawStyle);
}

///////////////////////////////////////////////////////////////////////////////

extern "C" PFVM_DLLEXPORT
pfvModule* pfvLoadModule_pfvmDrawStyle(pfvXmlNode*xml)
{
    pfNotify(PFNFY_INFO,PFNFY_PRINT,
	"pfvLoadModule_pfvmDrawStyle from %s called!\n",__FILE__);
    pfvmDrawStyle*m= new pfvmDrawStyle(xml);
    return (pfvModule*)m;
}

///////////////////////////////////////////////////////////////////////////////

