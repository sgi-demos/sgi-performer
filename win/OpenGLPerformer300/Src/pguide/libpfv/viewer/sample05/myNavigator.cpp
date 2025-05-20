
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

#include <Performer/pfv/pfvXml.h>

#include <Performer/pfv/pfvDisplay.h>
#include <Performer/pfv/pfvInput.h>

#include "myNavigator.h"

///////////////////////////////////////////////////////////////////////////////

myNavigator::myNavigator(pfvXmlNode* root)
{
	dzone[PF_X] = dzone[PF_Y] = 0.1f;
	
	deltaH = 0.1;
	deltaP = 0.1f;
	
	initSpeed = 0.001;
	minSpeed = 0.0004;
	maxSpeed = 0.03f;	
	deltaSpeed = 1.03;
	speed = 0.0f;
	
	deltaStrafe = 0.05f;
	deltaZ = 0.05f;
	
	default_xyz.set(0.0f,-10.0f,0.0);
	default_hpr.set(0.0f,0.0f,0.0f);

	viewInd = 0;


	bindCallback(PFV_CB_FRAME);

	
	if(root==NULL)
		return;
	
	//printf("myNavigator::myNavigator - looking at xml nodes\n");
			
    int i, n = root->getNumChildren();
    for(i=0;i<n;i++)
    {
		pfvXmlNode* xml = root->getChild(i);

		printf("navigator xml: child %d is <%s>\n", i, xml->getName() );
		
		if(!xml->nameCmp("viewIndex"))
		{
			xml->getInt(&viewInd);
			printf("Navigator::viewIndex = %d\n", viewInd );
		}  	
		else if(!xml->nameCmp("dzone"))
		{
			xml->getFloat((float*)(&dzone));
		}
		else if(!xml->nameCmp("deltaH"))
		{
			xml->getFloat(&deltaH);
		}
		else if(! xml->nameCmp("deltaP"))
		{
			xml->getFloat(&deltaP);
		}
		else if(! xml->nameCmp("initSpeed"))
		{
			xml->getFloat(&initSpeed);
		}
		else if(! xml->nameCmp("minSpeed"))
		{
			xml->getFloat(&minSpeed);
		}
		else if(! xml->nameCmp("maxSpeed"))
		{
			xml->getFloat(&maxSpeed);
		}

		else if(! xml->nameCmp("deltaSpeed"))
		{
			xml->getFloat(&deltaSpeed);
		}

		else if(! xml->nameCmp("deltaStrafe"))
		{
			xml->getFloat(&deltaStrafe);
		}

		else if(! xml->nameCmp("deltaZ"))
		{
			xml->getFloat(&deltaZ);
		}
		
		else if(! xml->nameCmp("default_xyz"))
		{
			xml->getFloatArray(3,(float*)(&default_xyz));
			printf("navigator: default_xyz= %f %f %f\n", 
				default_xyz[PF_X], default_xyz[PF_Y], default_xyz[PF_Z] );
		}

		else if(! xml->nameCmp("default_hpr"))
		{
			xml->getFloatArray(3,(float*)(&default_hpr));
			printf("navigator: default_hpr= %f %f %f\n", 
				default_hpr[PF_H], default_hpr[PF_P], default_hpr[PF_R] );
		}


		else
		{
			pfNotify( PFNFY_WARN, PFNFY_PRINT,
				"Invalid xlm token <%s> in navigator definition", 
				xml->getName());
		}
	}
	
	xyz.copy(default_xyz);
	hpr.copy(default_hpr);
}

///////////////////////////////////////////////////////////////////////////////

myNavigator::~myNavigator()
{
	return;
}

///////////////////////////////////////////////////////////////////////////////

float DZONE(float v, float r, float dz)
{
	if(v>0.0f)
	{
		if(v>dz) return (v-dz)/(r-dz);
		else return 0.0f;
	}
	else
	{
		if(v<-dz) return (v+dz)/(r-dz);
		else return 0.0f;
	}
}
			
///////////////////////////////////////////////////////////////////////////////
		
void myNavigator::frame()
{
/*
	printf("myNavigator::frame called - viewInd=%d FocusView=%d\n",
			pfvInputMngr::getFocusViewIndex(),viewInd);
*/

	if(pfvInputMngr::getFocusViewIndex()!=viewInd)
		return;

	pfvDisplayMngr::getMngr()->getView(viewInd)->getEye(xyz,hpr);
	
	pfuMouse*m = pfvInputMngr::getMouse();
	
	int btn = m->flags&PFUDEV_MOUSE_DOWN_MASK;
	float x, y;

	pfvInputMngr::getViewNormXY(&x,&y);
	
	
#if 0
	pfuCalcNormalizedChanXY( &x, &y, chan, m->xpos, m->ypos );
#endif
	
	x*=2.0f; x-=1.0f;
	y*=2.0f; y-=1.0f;

	x = DZONE(x,1.0f,dzone[PF_X]/2.0f);
	y = DZONE(y,1.0f,dzone[PF_Y]/2.0f);
	
	//printf("\n\nbtn=%d mousex=%f mousey=%f\n", btn, x, y);
	
	if(btn&PFUDEV_MOUSE_MIDDLE_DOWN)
	{
		hpr[PF_H] -= (x*deltaH);
		while(hpr[PF_H]>180.0f)
			hpr[PF_H]-=360.0f;
		while(hpr[PF_H]<-180.0f)
			hpr[PF_H]+=360.0f;
		
		hpr[PF_P] += (y*deltaP);
		if(hpr[PF_P]>90.0f)
			hpr[PF_P]=90.0f;
		if(hpr[PF_P]<-90.0f)
			hpr[PF_P]=-90.0f;

		if(btn==(PFUDEV_MOUSE_LEFT_DOWN|PFUDEV_MOUSE_MIDDLE_DOWN))
		{
			if(speed==0.0f)
				speed=initSpeed;
			if(speed>0.0f)
			{
				speed*=deltaSpeed;
				if(speed>maxSpeed)
					speed=maxSpeed;
			}
			else
			{
				speed/=deltaSpeed;
				if(speed>-minSpeed)
					speed=0.0f;
			}
		}
		else if(btn==(PFUDEV_MOUSE_RIGHT_DOWN|PFUDEV_MOUSE_MIDDLE_DOWN)) 	
		{
			if(speed==0.0f)
				speed=-initSpeed;

			if(speed>0.0f)
			{
				speed/=deltaSpeed;
				if(speed<minSpeed)
				speed=0.0f;
			}
			else
			{
				speed*=deltaSpeed;
				if(speed<-maxSpeed)
					speed=-maxSpeed;
			}
		}
	}
	else
	{
		speed = 0.0f;
		
		if(btn==(PFUDEV_MOUSE_LEFT_DOWN|PFUDEV_MOUSE_RIGHT_DOWN))
		{
			float s,c;
			pfSinCos(hpr[PF_H],&s,&c);
			//printf("sin=%f cos=%f strafe: %f, %f\n", s, c,
			//		(x*c*deltaStrafe), (y*s*deltaStrafe) );

			xyz[PF_X] += (x*c*deltaStrafe);
			xyz[PF_Y] += (x*s*deltaStrafe);

			xyz[PF_Z] += (y*deltaZ);
		}
	}

	if(btn==(PFUDEV_MOUSE_LEFT_DOWN  |
			 PFUDEV_MOUSE_MIDDLE_DOWN |
			 PFUDEV_MOUSE_RIGHT_DOWN  ))
	{
		xyz.copy(default_xyz);
		hpr.copy(default_hpr);
		speed=0.0f;
	}
	
	if(speed!=0.0f)
	{
		float s,c;
		//printf("speed=%f\n",speed);
		pfSinCos(hpr[PF_H],&s,&c);			
		xyz[PF_X] -= (speed*s);
		xyz[PF_Y] += (speed*c);		
	}
	
	//chan->setView(xyz,hpr);
	
	pfvDisplayMngr::getMngr()->getView(viewInd)->setEye(xyz,hpr);
	return;
	
}

///////////////////////////////////////////////////////////////////////////////


