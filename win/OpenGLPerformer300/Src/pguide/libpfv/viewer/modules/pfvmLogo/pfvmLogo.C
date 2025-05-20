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
#include <Performer/pr/pfTexture.h>
#include <Performer/pr.h>
#include <Performer/pfv/pfvXml.h>
#include <Performer/pfv/pfvViewer.h>

#include "pfvmLogo.h"

///////////////////////////////////////////////////////////////////////////////

pfType* pfvmLogo::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void pfvmLogo::init()
{
    if(classType==NULL)
    {
	pfvModule::init();
	classType= new pfType(pfvModule::getClassType(), "pfvmLogo" );
    }
}

///////////////////////////////////////////////////////////////////////////////

void pfvmLogo::construct()
{
    if(classType==NULL)
	init();
    setType(classType);

    listLogos = new pfList;
    PFASSERTALWAYS(listLogos);

    bindCallback(PFV_CB_POSTCONFIG);
}

///////////////////////////////////////////////////////////////////////////////

void pfvmLogo::overlay(pfvDispChan*chan)
{
    /*printf("pfvmLogo::overlay CALLED!!!\n");*/

    int i,n=listLogos->getNum();
    if(n>0)
    {
	pfPushState();
	pfEnable(PFEN_TEXTURE);
	pfTransparency(PFTR_HIGH_QUALITY);
	
	for(i=0;i<n;i++)
	{
	    LOGO* logo = (LOGO*)(listLogos->get(i));
	    /*printf("pfvmLogo::overlay LOGO[%d]=%x tex=%x!!!\n", 
		i, logo, logo->tex);*/
	    logo->tex->apply();
	    glColor4fv((GLfloat*)(&(logo->color[0])));
	    
	    glBegin(GL_QUADS);
	    
	    glTexCoord2f(logo->tcoords[0],logo->tcoords[2]);
	    glVertex2f(logo->pos[0],logo->pos[2]); // bottom-left
	    
	    glTexCoord2f(logo->tcoords[1],logo->tcoords[2]);
	    glVertex2f(logo->pos[1],logo->pos[2]); // bottom-right
	    
	    glTexCoord2f(logo->tcoords[1],logo->tcoords[3]);
	    glVertex2f(logo->pos[1],logo->pos[3]); // top-right
	    
	    glTexCoord2f(logo->tcoords[0],logo->tcoords[3]);
	    glVertex2f(logo->pos[0],logo->pos[3]); // top-left
	    
	    glEnd();
	}

	pfPopState();		
    }
}

///////////////////////////////////////////////////////////////////////////////

void pfvmLogo::postConfig()
{
    int i,n=listLogos->getNum();
    for(i=0;i<n;i++)
    {
	LOGO* logo = (LOGO*)(listLogos->get(i));
	logo->tex = new pfTexture;
	PFASSERTALWAYS(logo->tex);
	
	if(!logo->tex->loadFile(logo->texName))
	{
		pfNotify(PFNFY_WARN,PFNFY_PRINT, "pfvmLogo::postConfig - "
				"Failed to load texture file \"%s\"", logo->texName );
		listLogos->remove(logo);
		n--;
		i--;
		continue;
	}
	pfNotify(PFNFY_INFO,PFNFY_PRINT, "pfvmLogo::postConfig - "
		"Texture %x: Load image: \"%s\"", logo->tex, logo->texName );
    }
		
    if(n>0)
	bindCallback(PFV_CB_OVERLAY);

}

///////////////////////////////////////////////////////////////////////////////

int pfvmLogo::setXmlField(pfvXmlNode* xml)
{
	if(xml==NULL)
		return 0;
	
	if(!xml->nameCmp("logo"))
	{
		char texName[256];
		texName[0]='\0';
		
		float pos[4]={0.1,0.4,0.1,0.4};
		float tcoords[4]={0.0,1.0,0.0,1.0};
		float color[4]={1.0,1.0,1.0,1.0};
		
		int i,n=xml->getNumChildren();
		for(i=0;i<n;i++)
		{
			pfvXmlNode* child = xml->getChild(i);
			if(!child->nameCmp("TexName"))
			{
				strcpy(texName,child->getValue());
				continue;
			}
			if(!child->nameCmp("Pos"))
			{
				child->getFloatArray(4,pos);
				continue;
			}
			if(!child->nameCmp("TexCoords"))
			{
				child->getFloatArray(4,tcoords);
				continue;
			}
			if(!child->nameCmp("Color"))
			{
				child->getFloatArray(4,color);
				continue;
			}
			pfNotify(PFNFY_WARN,PFNFY_PRINT,"pfvmLogo::setXmlField - "
					"invalid xml token <%s>", child->getName() );
		}

		if(texName[0]!='\0')
		{
			LOGO* logo = new LOGO(texName);
			PFASSERTALWAYS(logo);
			logo->pos.set(pos[0],pos[1],pos[2],pos[3]);
			logo->tcoords.set(tcoords[0],tcoords[1],tcoords[2],tcoords[3]);
			logo->color.set(color[0],color[1],color[2],color[3]);
			
			pfNotify(PFNFY_INFO,PFNFY_PRINT,"pfvmLogo::setXmlField - "
					"Adding pfvmLogo=%x to list (tex=\"%s\")", logo, texName );

			listLogos->add(logo);
		}
		else
			pfNotify(PFNFY_WARN,PFNFY_PRINT,"pfvmLogo::setXmlField - "
					"missing TexName in pfvmLogo section" );
		return 1;
	}
	
	else return pfvModule::setXmlField(xml);
}

///////////////////////////////////////////////////////////////////////////////

pfvmLogo::pfvmLogo(pfvXmlNode*xml)
{
	construct();
	if(xml)
		parseXml(xml);
}
	
///////////////////////////////////////////////////////////////////////////////

extern "C" PFVM_DLLEXPORT
pfvModule* pfvLoadModule_pfvmLogo(pfvXmlNode*xml)
{
    pfNotify(PFNFY_INFO,PFNFY_PRINT,
	"pfvLoadModule_pfvmLogo from %s called!\n",__FILE__);
    pfvmLogo*m= new pfvmLogo(xml);
    return (pfvModule*)m;
}

///////////////////////////////////////////////////////////////////////////////

