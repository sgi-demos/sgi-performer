//
// Copyright 2002, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// This source code ("Source Code") was originally derived from a
// code base owned by Silicon Graphics, Inc. ("SGI")
// 
// LICENSE: SGI grants the user ("Licensee") permission to reproduce,
// distribute, and create derivative works from this Source Code,
// provided that: (1) the user reproduces this entire notice within
// both source and binary format redistributions and any accompanying
// materials such as documentation in printed or electronic format;
// (2) the Source Code is not to be used, or ported or modified for
// use, except in conjunction with OpenGL Performer; and (3) the
// names of Silicon Graphics, Inc.  and SGI may not be used in any
// advertising or publicity relating to the Source Code without the
// prior written permission of SGI.  No further license or permission
// may be inferred or deemed or construed to exist with regard to the
// Source Code or the code base of which it forms a part. All rights
// not expressly granted are reserved.
// 
// This Source Code is provided to Licensee AS IS, without any
// warranty of any kind, either express, implied, or statutory,
// including, but not limited to, any warranty that the Source Code
// will conform to specifications, any implied warranties of
// merchantability, fitness for a particular purpose, and freedom
// from infringement, and any warranty that the documentation will
// conform to the program, or any warranty that the Source Code will
// be error free.
// 
// IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
// LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
// ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
// SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
// OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
// PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
// OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
// USE, THE SOURCE CODE.
// 
// Contact information:  Silicon Graphics, Inc., 
// 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
// or:  http://www.sgi.com
//
//

///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfLightSource.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfTraverser.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfGeode.h>

#include <Performer/pr/pfLinMath.h>   

#include <Performer/pr/pfHighlight.h>

#include <Performer/pf.h>
#include <Performer/pr.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>

#include <Performer/pfv/pfvXml.h>
#include <Performer/pfv/pfvPicker.h>
#include <Performer/pfv/pfvMousePicker.h>

#include "tvset.h"

///////////////////////////////////////////////////////////////////////////////


void tvset::construct()
{
	listTex = new pfList;
	PFASSERTALWAYS(listTex);
	
	listTexNames = new pfList;
	PFASSERTALWAYS(listTexNames);
		
	//listIaData = new pfList;
	//PFASSERTALWAYS(listIaData);

	listGeodeBtn = new pfList;
	PFASSERTALWAYS(listGeodeBtn);

	btn_hl = new pfHighlight;
	PFASSERTALWAYS(btn_hl);
	btn_hl->setMode( PFHL_LINES );
 	btn_hl->setColor( PFHL_FGCOLOR, 0.8f, 0.8f, 0.5f );


	root = NULL;
	//dcs = NULL;

	screenW = 0.6;
	screenAR = 4.0f/3.0f;
	
	border = 0.02;
	depth = 0.4;
	
	btnAR = 1.0f;
	btnGapRatio = 3.0f;
		
	z = NULL;

	nChans = 0;
	curChan=0;
	
	gstateTV = NULL;
	gstateScreen = NULL;

	onOff=1;

}

///////////////////////////////////////////////////////////////////////////////

tvset::tvset()
{
	construct();
}

///////////////////////////////////////////////////////////////////////////////

tvset::tvset(char*filename)
{
	construct();

	if((filename==NULL)||(filename[0]=='\0'))
	{
		printf("tvset::tvset -  NULL filename\n");
		return;
	}

	pfvXmlNode* xml = pfvXmlNode::parseFile(filename);
			
	if(xml==NULL)
	{
		printf("tvset::tvset -  failed to parse xml file %s\n", filename);
		return;
	}
	
	parseXML(xml);
}

///////////////////////////////////////////////////////////////////////////////

tvset::tvset( pfvXmlNode* xmlRoot )
{
	construct();
	parseXML(xmlRoot);
}

///////////////////////////////////////////////////////////////////////////////

void tvset::parseXML( pfvXmlNode* xmlRoot )
{
	if(xmlRoot==NULL)
	{
		printf("tvset::parseXML -  NULL xml node\n" );
		return;
	}
		
	int i, n = xmlRoot->getNumChildren();
    for(i=0;i<n;i++)
    {
		printf("getting child %d\n", i );
		pfvXmlNode* xml = xmlRoot->getChild(i);

		printf("tvset::tvset - node %d is <%s>\n", i, xml->getName());

		if( !xml->nameCmp("on"))
		{
	    	xml->getBool(&onOff);
			printf("tvset will initially be: %s\n", onOff?"ON":"OFF" );
		}

		else if(! xml->nameCmp("curChan"))
		{
	    	xml->getInt(&curChan);
		}
		else if(! xml->nameCmp("screenWidth"))
		{
	    	xml->getFloat(&screenW);
			printf("SCREEN WIDTH: %f\n", screenW );
		}
		else if(! xml->nameCmp("screenAR"))
		{
	    	xml->getFloat(&screenAR);
		}
		else if( !xml->nameCmp("border"))
		{
	    	xml->getFloat(&border);
		}
		else if( !xml->nameCmp("depth"))
		{
			xml->getFloat(&depth);
		}
		else if( !xml->nameCmp("btnAR"))
		{
			xml->getFloat(&btnAR);
		}
		else if( !xml->nameCmp("btnGapRatio"))
		{
			xml->getFloat(&btnGapRatio);
		}
		else if( !xml->nameCmp("texture"))
		{
			printf("<texture>%s</texture>\n",xml->getValue()?xml->getValue():"NULL");
			int len = strlen(xml->getValue());
			printf("strlen=%d\n",len);
			char* texName = (char*)pfCalloc(len+1,sizeof(char),pfGetSharedArena());
			PFASSERTALWAYS(texName);
			strcpy(texName,xml->getValue());
			printf("after name copy: %s\n", texName );
			listTexNames->add(texName);
			
			printf( "tvset::tvset - added texture %s\n", texName );
		}
		else
		{
			printf("tvset::tvset - invalid xml token <%s>\n", xml->getName());
		}
	}
	printf("tvset::tvset - done\n");
}

///////////////////////////////////////////////////////////////////////////////

tvset::~tvset()
{
	// should delete texture names, texList, dcs, rootm textures..
	;
}
		
///////////////////////////////////////////////////////////////////////////////

void tvset::makeGStates()
{
	gstateTV = new pfGeoState;
	gstateTV->makeBasic();
	gstateTV->setMode(PFSTATE_CULLFACE, PFCF_BACK);
	gstateTV->setMode(PFSTATE_ENTEXTURE,0);
	gstateTV->setMode(PFSTATE_ENLIGHTING,0);

		
	gstateScreen = new pfGeoState;
	gstateScreen->makeBasic();
	gstateScreen->setMode(PFSTATE_CULLFACE, PFCF_OFF);

	// load textures

	int i;
	for(i=0; i<nChans; i++)
	{
		pfTexture* tex = new pfTexture;

		printf("now loading texture <%s>\n", (char*)listTexNames->get(i));
		tex->loadFile((char*)listTexNames->get(i));
		printf("texture <%s> loaded\n", (char*)listTexNames->get(i));
		
		tex->setFilter(PFTEX_MINFILTER, PFTEX_BILINEAR);

		listTex->add(tex);
	}

	gstateScreen->setMode(PFSTATE_ENLIGHTING,0);
	
	pfTexEnv *tev = new pfTexEnv;
	gstateScreen->setAttr(PFSTATE_TEXENV, tev);
}

///////////////////////////////////////////////////////////////////////////////

void tvset::makeBox()
{
	pfGeode* geode = new pfGeode;
	pfGeoSet* gset = new pfGeoSet;

	geode->addGSet(gset);
	root->addChild(geode);

    pfVec3 *coords = (pfVec3*)pfCalloc(20,
			sizeof(pfVec3),pfGetSharedArena());
	PFASSERTALWAYS(coords);

	printf("coords allocated\n");
	
	// bottom
    coords[0].set( x[0],  y[0],  z[0] );
    coords[1].set( x[0],  y[1],  z[0] );
    coords[2].set( x[5],  y[1],  z[0] );
	coords[3].set( x[5],  y[0],  z[0] );
 
	// left
    coords[4].set( x[0],  y[0],  z[0] );
    coords[5].set( x[0],  y[0],  z[nz-1] );
    coords[6].set( x[0],  y[1],  z[nz-1] );
    coords[7].set( x[0],  y[1],  z[0] );

	// top
    coords[8].set( x[0],  y[1],  z[nz-1] );
    coords[9].set( x[0],  y[0],  z[nz-1] );
    coords[10].set( x[5],  y[0],  z[nz-1] );
    coords[11].set( x[5],  y[1],  z[nz-1] ); 
    
	// right
    coords[12].set( x[5],  y[1],  z[nz-1] ); 
    coords[13].set( x[5],  y[0],  z[nz-1] );
	coords[14].set( x[5],  y[0],  z[0] );
    coords[15].set( x[5],  y[1],  z[0] );
	  
	// back
    coords[19].set( x[0],  y[1],  z[nz-1] );
    coords[18].set( x[0],  y[1],  z[0] );
    coords[17].set( x[5],  y[1],  z[0] );
    coords[16].set( x[5],  y[1],  z[nz-1] ); 
 

    pfVec4 *colors = (pfVec4*)pfCalloc(5,sizeof(pfVec4),pfGetSharedArena());
	PFASSERTALWAYS(colors);
	
    colors[0].set(0.3f, 0.3f, 0.3f, 1.0f);
    colors[1].set(0.0f, 0.0f, 1.0f, 1.0f);
    colors[2].set(0.0f, 0.0f, 1.0f, 1.0f);
    colors[3].set(0.0f, 0.0f, 1.0f, 1.0f);
    colors[4].set(0.6f, 0.6f, 0.65f, 1.0f);

	printf("setting attrs\n");
	
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setAttr(PFGS_COLOR4, PFGS_PER_PRIM, colors, NULL);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(5);
    gset->setGState(gstateTV);
	
}

///////////////////////////////////////////////////////////////////////////////

void tvset::makeScreen()
{
	pfGeode* geode = new pfGeode;
	pfGeoSet* gset = new pfGeoSet;

	geode->addGSet(gset);
	root->addChild(geode);

    pfVec3 *coords = (pfVec3*)pfCalloc(4,sizeof(pfVec3),pfGetSharedArena());
	PFASSERTALWAYS(coords);

    coords[0].set( x[1],  y[0],  z[nz-2] );
    coords[1].set( x[1],  y[0],  z[1] );
    coords[2].set( x[2],  y[0],  z[1] );
	coords[3].set( x[2],  y[0],  z[nz-2] );
 
    pfVec4 *colors = (pfVec4*)pfCalloc(5,sizeof(pfVec4),pfGetSharedArena());
	PFASSERTALWAYS(colors);
	screenCol=colors;
	colors[0].set(1.0f, 1.0f, 1.0f, 1.0f);


    pfVec2 *tcoords = (pfVec2*)pfCalloc(4,sizeof(pfVec2),pfGetSharedArena());
	PFASSERTALWAYS(tcoords);
	tcoords[0].set( 0.0, 1.0 );
	tcoords[1].set( 0.0, 0.0 );
	tcoords[2].set( 1.0, 0.0 );
	tcoords[3].set( 1.0, 1.0 );
				
	
	
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
    gset->setAttr(PFGS_TEXCOORD2, PFGS_PER_VERTEX, tcoords, NULL);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(1);
    gset->setGState(gstateScreen);

}

///////////////////////////////////////////////////////////////////////////////

void tvset::makeFrontPanel()
{
	pfGeode* geode = new pfGeode;
	pfGeoSet* gset = new pfGeoSet;

	geode->addGSet(gset);
	root->addChild(geode);

    pfVec3 *coords = (pfVec3*)pfCalloc(16*3,
			sizeof(pfVec3),pfGetSharedArena());
	PFASSERTALWAYS(coords);

	printf("coords allocated\n");
	
	// tri 0
    coords[0].set( x[0],  y[0],  z[nz-1] );
    coords[1].set( x[1],  y[0],  z[nz-2] );
    coords[2].set( x[5],  y[0],  z[nz-1] );

	// tri 1
    coords[3].set( x[1],  y[0],  z[nz-2] );
    coords[4].set( x[2],  y[0],  z[nz-2] );
    coords[5].set( x[5],  y[0],  z[nz-1] );

	// tri 2
    coords[6].set( x[2],  y[0],  z[nz-2] );
    coords[7].set( x[3],  y[0],  z[nz-2] );
    coords[8].set( x[5],  y[0],  z[nz-1] );

	// tri 3
    coords[9].set( x[3],  y[0],  z[nz-2] );
    coords[10].set( x[4],  y[0],  z[nz-2] );
    coords[11].set( x[5],  y[0],  z[nz-1] );

	// tri 4
    coords[12].set( x[4],  y[0],  z[nz-2] );
    coords[13].set( x[5],  y[0],  z[0] );
    coords[14].set( x[5],  y[0],  z[nz-1] );

	// tri 5
    coords[15].set( x[4],  y[0],  z[2] );
    coords[16].set( x[4],  y[0],  z[1] );
    coords[17].set( x[5],  y[0],  z[0] );

	// tri 6
    coords[18].set( x[4],  y[0],  z[1] );
    coords[19].set( x[3],  y[0],  z[1] );
    coords[20].set( x[5],  y[0],  z[0] );

	// tri 7
    coords[21].set( x[3],  y[0],  z[1] );
    coords[22].set( x[0],  y[0],  z[0] );
    coords[23].set( x[5],  y[0],  z[0] );

	// tri 8
    coords[24].set( x[2],  y[0],  z[1] );
    coords[25].set( x[0],  y[0],  z[0] );
    coords[26].set( x[3],  y[0],  z[1] );

	// tri 9
    coords[27].set( x[1],  y[0],  z[1] );
    coords[28].set( x[0],  y[0],  z[0] );
    coords[29].set( x[2],  y[0],  z[1] );

	// tri 10
    coords[30].set( x[0],  y[0],  z[nz-1] );
    coords[31].set( x[0],  y[0],  z[0] );
    coords[32].set( x[1],  y[0],  z[1] );

	// tri 11
    coords[33].set( x[0],  y[0],  z[nz-1] );
    coords[34].set( x[1],  y[0],  z[1] );
    coords[35].set( x[1],  y[0],  z[nz-2] );

	// tri 12
    coords[36].set( x[2],  y[0],  z[nz-2] );
    coords[37].set( x[2],  y[0],  z[1] );
    coords[38].set( x[3],  y[0],  z[1] );

	// tri 13
    coords[39].set( x[2],  y[0],  z[nz-2] );
    coords[40].set( x[3],  y[0],  z[1] );
    coords[41].set( x[3],  y[0],  z[2] );

	// tri 14
    coords[42].set( x[2],  y[0],  z[nz-2] );
    coords[43].set( x[3],  y[0],  z[2] );
    coords[44].set( x[3],  y[0],  z[nz-2] );

	// tri 15
    coords[45].set( x[4],  y[0],  z[nz-2] );
    coords[46].set( x[4],  y[0],  z[2] );
    coords[47].set( x[5],  y[0],  z[0] );


    pfVec4 *colors = (pfVec4*)pfCalloc(1,
			sizeof(pfVec4),pfGetSharedArena());
	PFASSERTALWAYS(colors);
	
	printf("colors allocated\n");
	
	colors[0].set(0.5f, 0.5f, 1.0f, 1.0f);


	printf("setting attrs\n");
	
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
    gset->setPrimType(PFGS_TRIS);
    gset->setNumPrims(16);
    gset->setGState(gstateTV);


	// quad geoset for panel between buttons
	gset = new pfGeoSet;
	PFASSERTALWAYS(gset);
	geode->addGSet(gset);

    coords = (pfVec3*)pfCalloc(nChans*4,sizeof(pfVec3),pfGetSharedArena());
	PFASSERTALWAYS(coords);
	
	int i, v;
	for( i=0,v=0; i<nChans; i++,v+=4 )
	{
		int iz = nz-3-(i*2);
		coords[v+0].set( x[3], y[0], z[iz] );
		coords[v+1].set( x[3], y[0], z[iz-1] );
		coords[v+2].set( x[4], y[0], z[iz-1] );
		coords[v+3].set( x[4], y[0], z[iz] );					
	}
	
    colors = (pfVec4*)pfCalloc(1,sizeof(pfVec4),pfGetSharedArena());
	PFASSERTALWAYS(colors);
	//colors[0].set(0.1f, 0.2f, 0.8f, 1.0f);
	colors[0].set(0.5f, 0.5f, 1.0f, 1.0f);


	
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(nChans);
    gset->setGState(gstateTV);
	
}

///////////////////////////////////////////////////////////////////////////////

void tvset::makeButtons()
{
	// on-off button
	
	pfGeode* geode = new pfGeode;
	geodeOnOff = geode;

	pfGeoSet* gset = new pfGeoSet;

	geode->addGSet(gset);
	root->addChild(geode);

    pfVec3 *coords = (pfVec3*)pfCalloc(4,sizeof(pfVec3),pfGetSharedArena());
	PFASSERTALWAYS(coords);

    coords[0].set( x[3],  y[0],  z[2] );
    coords[1].set( x[3],  y[0],  z[1] );
    coords[2].set( x[4],  y[0],  z[1] );
	coords[3].set( x[4],  y[0],  z[2] );
 
    pfVec4 *colors = (pfVec4*)pfCalloc(5,sizeof(pfVec4),pfGetSharedArena());
	PFASSERTALWAYS(colors);
	
	colors[0].set(1.0f, 1.0f, 1.0f, 1.0f);
	
    gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(1);
    gset->setGState(gstateTV);


	// channel buttons
	
	int i;
	for(i=0; i<nChans; i++ )
	{
		int iz = nz-2-(i*2);

		geode = new pfGeode;
		gset = new pfGeoSet;

		geode->addGSet(gset);
		root->addChild(geode);

    	coords = (pfVec3*)pfCalloc(4,sizeof(pfVec3),pfGetSharedArena());
		PFASSERTALWAYS(coords);

    	coords[0].set( x[3],  y[0],  z[iz] );
    	coords[1].set( x[3],  y[0],  z[iz-1] );
    	coords[2].set( x[4],  y[0],  z[iz-1] );
		coords[3].set( x[4],  y[0],  z[iz] );
 
    	colors = (pfVec4*)pfCalloc(5,sizeof(pfVec4),pfGetSharedArena());
		PFASSERTALWAYS(colors);
	
		colors[0].set(1.0f, 1.0f, 1.0f, 1.0f);
	
    	gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    	gset->setAttr(PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
    	gset->setPrimType(PFGS_QUADS);
    	gset->setNumPrims(1);
    	gset->setGState(gstateTV);


		//tvsetIaData* iaData = (tvsetIaData*)
		//	pfCalloc(1,sizeof(tvsetIaData),pfGetSharedArena());
		//PFASSERTALWAYS(iaData);
		//
		//iaData->geode = geode;
		//iaData->chanIndex = i;
		//listIaData->add(iaData);
		
		listGeodeBtn->add(geode);
	}

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

pfType* tvbtnInteractor::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void tvbtnInteractor::init()
{
	if(classType==NULL)
	{
		pfvInteractor::init();
		classType= new pfType(pfvInteractor::getClassType(), "tvbtnInteractor" );
	}
}

///////////////////////////////////////////////////////////////////////////////

tvbtnInteractor::tvbtnInteractor(tvset*_tv,int c,pfNode*_node)
{
	if(classType==NULL)
		init();
	setType(classType);
	tv = _tv;
	index = c;
	node = _node;
}

///////////////////////////////////////////////////////////////////////////////

tvbtnInteractor::~tvbtnInteractor()
{
	;
}

///////////////////////////////////////////////////////////////////////////////

int tvbtnInteractor::startHlite( pfvPicker* picker, int permissions )
{
	
	if( tv->getOnOff() || (index==-1))
	{
		pfuTravNodeHlight( node, tv->btn_hl );
		return 1;
	}
	
	return 0;	
}

///////////////////////////////////////////////////////////////////////////////

int tvbtnInteractor::updateHlite( pfvPicker* p,int ev,int prmsn, pfvPickerRequest*r )
{
	if( ev==LEFT_DOWN )
	{
		if(index==-1)
		{
			tv->setOnOff( tv->getOnOff() );
			r->state |= PFPICKER_GRAB_EVENT;
		}
		else
		{
			tv->setChannel(index);
			r->state &= ~PFPICKER_HLITE;
			r->state |= PFPICKER_GRAB_EVENT;
			r->interactor = NULL;
		}
	}

	return 1;	
}

///////////////////////////////////////////////////////////////////////////////

void tvbtnInteractor::endHlite( pfvPicker* picker )
{
	pfuTravNodeHlight( node, NULL );
}

///////////////////////////////////////////////////////////////////////////////

int tvbtnInteractor::getActive(pfvPicker* p )
{
	return (index==-1)||tv->getOnOff();
}


///////////////////////////////////////////////////////////////////////////////

int 
tvbtnInteractor::specialFocus(pfvPicker*p,int ev,int prmsn,pfvPickerRequest*r)
{
	if(ev==LEFT_DOWN )
	{
		if(index==-1)
		{
			tv->setOnOff( !tv->getOnOff() );
			r->state |= PFPICKER_GRAB_EVENT;
		}
		else if(tv->getOnOff() && (tv->getCurrChannel()!=index) )
		{
			tv->setChannel(index);
			r->state |= PFPICKER_GRAB_EVENT;
			//r->interactor = NULL;
		}
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

void tvset::setUpInteractors( pfvPicker* p )
{
	int i;
	tvbtnInteractor* ia;
	
	for(i=0;i<nChans; i++ )
	{
		ia = new tvbtnInteractor(this,i,(pfNode*)(listGeodeBtn->get(i)));
		PFASSERTALWAYS(ia);
		ia->nodeSetup( (pfNode*)(listGeodeBtn->get(i)), p ); 
	}

	ia = new tvbtnInteractor(this, -1, (pfNode*)geodeOnOff );
	PFASSERTALWAYS(ia);
	ia->nodeSetup( (pfNode*)geodeOnOff, p );		
}

///////////////////////////////////////////////////////////////////////////////


pfDCS* tvset::realize()
{

	//dcs = new pfDCS;
	//PFASSERTALWAYS(dcs);

	root = new pfGroup;
	PFASSERTALWAYS(root);
	
	//dcs->addChild(root);
	
	nChans = listTexNames->getNum();
	if(nChans<=0)
	{
		printf("tvset::realize - tvset has no channels\n");
		return NULL;
	}
	
	nz = 4+(2*nChans);
	z = (float*)pfCalloc( nz, sizeof(float), pfGetSharedArena());
	PFASSERTALWAYS(z);

	float screenH = screenW/screenAR;

	printf("nChans=%d screenW=%f screenH=%f\n", nChans, screenW, screenH);

	float sum = ((nChans+2)*btnGapRatio)+(nChans+1);
	float gapH = screenH/sum;
	float btnH = gapH*btnGapRatio;
	float btnW = btnH*btnAR;

	printf( "btnH=%f btnW=%f gapH=%f\n", btnH, btnW, gapH );
		
	x[0] = ((3*border)+screenW+btnW)/-2.0f;
	x[1] = x[0]+border;
	x[2] = x[1]+screenW;
	x[3] = x[2]+border;
	x[4] = x[3]+btnW;
	x[5] = x[4]+border;

	y[0] = 0.0f;
	y[1] = depth;

	z[0] = 0.0f;
	z[1] = z[0]+border;
	z[2] = z[1]+btnH;
	z[3] = z[2]+((2*gapH)+btnH);
	z[4] = z[3]+btnH;
	
	int i,zi;
	for(i=1,zi=5; i<nChans; i++,zi+=2 )
	{
		z[zi] = z[zi-1]+gapH;
		z[zi+1] = z[zi]+btnH;
	}
	
	printf("nz=%d\n",nz);
	z[nz-1] = z[nz-2]+border;

	for(i=0; i<nz; i++ )
	{
		printf("z[%d] = %f\n",i, z[i]);
	}
	printf("\n");

		

	
	printf("Making GStates:\n");	
	makeGStates();
	
	printf("Making Box:\n");	
	makeBox();

	makeScreen();
	makeFrontPanel();
	makeButtons();

	if(curChan<0 || curChan>=listTex->getNum() )
		curChan=0;

	onOff=!onOff;
	setOnOff(!onOff);

	
}

///////////////////////////////////////////////////////////////////////////////

void tvset::setChannel( int c )
{
	curChan = c;
	gstateScreen->setAttr(PFSTATE_TEXTURE, 
			(pfTexture*)(listTex->get(curChan)));
}

///////////////////////////////////////////////////////////////////////////////

void tvset::setOnOff( int i )
{
	i = (i!=0);
	
	if( i==onOff )
		return;

	if(i) // switch on
	{
		if(curChan<0 || curChan>=nChans)
			curChan=0;

		printf("SWITCHING ON: channel %d\n", curChan );
		
		gstateScreen->setAttr(PFSTATE_TEXTURE, 
			(pfTexture*)(listTex->get(curChan)));
		gstateScreen->setMode(PFSTATE_ENTEXTURE,1);
		screenCol[0].set(1.0f,1.0f,1.0f,1.0f);
	}
	else // switch off
	{
		printf("SWITCHING OFF\n" );
		gstateScreen->setMode(PFSTATE_ENTEXTURE,0);
		gstateScreen->setAttr(PFSTATE_TEXTURE, NULL );
		screenCol[0].set(0.1f,0.1f,0.1f,1.0f);
	}
	onOff=i;

	//if(onOff)
	//	setChannel(0);
}

///////////////////////////////////////////////////////////////////////////////


void tvset::update()
{
}

///////////////////////////////////////////////////////////////////////////////




