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
#ifndef __TVSET_H__
#define __TVSET_H__

///////////////////////////////////////////////////////////////////////////////

#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfDCS.h>

#include <Performer/pfv/pfvXml.h>

///////////////////////////////////////////////////////////////////////////////

class pfvXmlNode;
class pfNode;
class pfDCS;
class pfList;

///////////////////////////////////////////////////////////////////////////////

class tvset
{
public:
	tvset();
	tvset(char*filename);
	tvset(pfvXmlNode* xml);
	~tvset();

	pfDCS* realize();
	void setUpInteractors( pfvPicker* p );
	
	//pfDCS* getDCS() { return dcs; }
	pfGroup* getRoot() { return root; }
	pfList* getListTex() { return listTex; }
	
	void update();
	void setChannel(int c);
	int getCurrChannel() { return curChan; }

	void setOnOff( int on_off );
	int getOnOff() { return onOff; }
	
	float screenW;
	float screenAR;
	float border;
	float depth;
	float btnAR;
	float btnGapRatio;

private:

//friend class tvsetInteractor;
friend class tvbtnInteractor;

	void construct();

	void parseXML( pfvXmlNode*xmlRoot);
	void makeGStates();
	void makeBox();
	void makeScreen();
	void makeFrontPanel();
	void makeButtons();

	pfList* listTex;
	pfList* listTexNames;
	pfGroup* root;
	//pfDCS* dcs;

	int onOff;
	pfGeode* geodeOnOff;
	
	//pfList* listIaData;
	pfList* listGeodeBtn;

	pfGeoState* gstateTV;
	pfGeoState* gstateScreen;

	float x[6];
	float y[2];
	float *z;
	int nz;

	pfVec4* screenCol;


	int nChans;
	int curChan;

	pfHighlight *btn_hl;
};
	

///////////////////////////////////////////////////////////////////////////////

class tvbtnInteractor : public pfvInteractor
{
public:
	tvbtnInteractor(tvset*tv,int c,pfNode*_node);
	~tvbtnInteractor();
	
	static void init();
	static pfType* getClassType(){ return classType; }
	virtual const char* getTypeName() const {return "tvbtnInteractor";}

	int getActive(pfvPicker*p);
	int startHlite( pfvPicker* picker, int permissions );
	int updateHlite( pfvPicker* p,int ev,int prmsn, pfvPickerRequest*r );
	void endHlite( pfvPicker* picker );
	int specialFocus(pfvPicker*p,int ev,int prmsn,pfvPickerRequest*r);


private:
	
	static pfType *classType;
	tvset* tv;
	int index; // chan index, or -1 for on-off btn
	pfNode* node; // ptr to button geometry
	
};

///////////////////////////////////////////////////////////////////////////////

/**
<tvset>

	<screenWidth>0.6</screenWidth>
	<border>0.02</border>
	<depth>0.4</depth>
		
	<texture>jay_leno.rgb</texture>
	<texture>jay_leno.rgb</texture>
	<texture>jay_leno.rgb</texture>
				
</tvset>
**/
	
///////////////////////////////////////////////////////////////////////////////


#endif /* end of __TVSET_H__ */



