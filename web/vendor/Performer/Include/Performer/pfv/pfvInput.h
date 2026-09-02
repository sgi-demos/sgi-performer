//
// Copyright 2001, 2002 Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
// in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
// in similar or successor clauses in the FAR, or the DOD or NASA FAR
// Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
// 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
// PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
// GRAPHICS, INC.
//
// pfvInput.h
//
// $Revision: 1.5 $
// $Date: 2002/12/03 02:21:58 $
//

#ifndef __PFV_INPUT_MNGR_H__
#define __PFV_INPUT_MNGR_H__ 

///////////////////////////////////////////////////////////////////////////////

#include <Performer/pf.h>
#include <Performer/pfutil.h>

#include <Performer/pr/pfList.h>

#include <Performer/pfv/pfvDLL.h>

///////////////////////////////////////////////////////////////////////////////

#define PFV_MAX_KEYVAL		127

#define PFVKEY_F1		1 /* same as PFUDEV_F1KEY */
#define PFVKEY_F2		2 /* same as PFUDEV_F2KEY */
#define PFVKEY_F3		3 /* same as PFUDEV_F3KEY */
#define PFVKEY_F4		4 /* same as PFUDEV_F4KEY */

#define PFVKEY_F5		5 /* same as PFUDEV_F5KEY */
#define PFVKEY_F6		6 /* same as PFUDEV_F6KEY */
#define PFVKEY_F7		7 /* same as PFUDEV_F7KEY */

#define PFVKEY_F8		8 /* same as PFUDEV_F8KEY */
#define PFVKEY_F9		9 /* same as PFUDEV_F9KEY */
#define PFVKEY_F10		10 /* same as PFUDEV_F10KEY */
#define PFVKEY_F11		11 /* same as PFUDEV_F11KEY */
#define PFVKEY_F12		12 /* same as PFUDEV_F12KEY */

#define PFVKEY_ENTER		13 /* Enter/Return key */

/* NOTE: PFU defines F8 and F9 as values 8 and 9.. I have also
* defined F* keys as 1-12, so I assign two new values to the
* backspace and tab keys (14 and 15)
* 
* key 8 --> Backspace
* key 9 --> Tab
*/
#define PFVKEY_BACKSPACE	14
#define PFVKEY_TAB		15

#if 0
#define PFVKEY_ESCAPE		16 /* same as PFUDEV_ESCKEY */
#endif

#define PFVKEY_LEFTARROW	17 /* same as PFUDEV_LEFTARROWKEY */
#define PFVKEY_DOWNARROW	18 /* same as PFUDEV_DOWNARROWKEY */
#define PFVKEY_RIGHTARROW	19 /* same as PFUDEV_RIGHTARROWKEY */
#define PFVKEY_UPARROW		20 /* same as PFUDEV_UPARROWKEY */

#define PFVKEY_PRINTSCREEN	21 /* same as PFUDEV_PRINTSCREENKEY */

#define PFVKEY_ESCAPE		27 /* ESC key */

#define PFVKEY_SPACE		32 /* ascii ' ' */
#define PFVKEY_EXCLAM		33 /* ascii '!' */
#define PFVKEY_DOUBLEQUOTE	34 /* ascii '"' */
#define PFVKEY_HASH		35 /* ascii '#' */
#define PFVKEY_POUND		35 /* ascii '#' */
#define PFVKEY_DOLLAR		36 /* ascii '$' */
#define PFVKEY_PERCENT		37 /* ascii '%' */
#define PFVKEY_AND		38 /* ascii '&' */
#define PFVKEY_SINGLE_QUOTE	39 /* ascii '\'' */

#define PFVKEY_OPENROUND	40 /* ascii '(' */
#define PFVKEY_CLOSEROUND	41 /* ascii ')' */

#define PFVKEY_ASTERISC		42 /* ascii '*' */
#define PFVKEY_PLUS		43 /* ascii '+' */
#define PFVKEY_COMMA		44 /* ascii ',' */
#define PFVKEY_MINUS		45 /* ascii '-' */
#define PFVKEY_PERIOD		46 /* ascii '.' */
#define PFVKEY_FULLSTOP		46 /* ascii '.' */

#define PFVKEY_SLASH		47 /* ascii '/' */

#define PFVKEY_0		48 /* ascii '0' */
#define PFVKEY_1		49 /* ascii '1' */
#define PFVKEY_2		50 /* ascii '2' */
#define PFVKEY_3		51 /* ascii '3' */
#define PFVKEY_4		52 /* ascii '4' */
#define PFVKEY_5		53 /* ascii '5' */
#define PFVKEY_6		54 /* ascii '6' */
#define PFVKEY_7		55 /* ascii '7' */
#define PFVKEY_8		56 /* ascii '8' */
#define PFVKEY_9		57 /* ascii '9' */

#define PFVKEY_COLON		58 /* ascii ':' */
#define PFVKEY_SEMICOLON	59 /* ascii ';' */

#define PFVKEY_SMALLER		60 /* ascii '<' */
#define PFVKEY_EQUAL		61 /* ascii '=' */
#define PFVKEY_GREATER		62 /* ascii '>' */

#define PFVKEY_QUESTION		63 /* ascii '?' */
#define PFVKEY_AT		64 /* ascii '@' */

#define PFVKEY_A		65 /* ascii 'A' */
#define PFVKEY_B		66 /* ascii 'B' */
#define PFVKEY_C		67 /* ascii 'C' */
#define PFVKEY_D		68 /* ascii 'D' */
#define PFVKEY_E		69 /* ascii 'E' */
#define PFVKEY_F		70 /* ascii 'F' */
#define PFVKEY_G		71 /* ascii 'G' */
#define PFVKEY_H		72 /* ascii 'H' */
#define PFVKEY_I		73 /* ascii 'I' */
#define PFVKEY_J		74 /* ascii 'J' */
#define PFVKEY_K		75 /* ascii 'K' */
#define PFVKEY_L		76 /* ascii 'L' */
#define PFVKEY_M		77 /* ascii 'M' */
#define PFVKEY_N		78 /* ascii 'N' */
#define PFVKEY_O		79 /* ascii 'O' */
#define PFVKEY_P		80 /* ascii 'P' */
#define PFVKEY_Q		81 /* ascii 'Q' */
#define PFVKEY_R		82 /* ascii 'R' */
#define PFVKEY_S		83 /* ascii 'S' */
#define PFVKEY_T		84 /* ascii 'T' */
#define PFVKEY_U		85 /* ascii 'U' */
#define PFVKEY_V		86 /* ascii 'V' */
#define PFVKEY_W		87 /* ascii 'W' */
#define PFVKEY_X		88 /* ascii 'X' */
#define PFVKEY_Y		89 /* ascii 'Y' */
#define PFVKEY_Z		90 /* ascii 'Z' */

#define PFVKEY_OPENSQUARE	91 /* ascii '[' */
#define PFVKEY_BACKSLASH	92 /* ascii '\' */
#define PFVKEY_CLOSESQUARE	93 /* ascii ']' */
#define PFVKEY_HAT		94 /* ascii '^' */
#define PFVKEY_UNDERSCORE	95 /* ascii '_' */
#define PFVKEY_APOSTROPHE	96 /* ascii '`' */

#define PFVKEY_a		97 /* ascii 'a' */
#define PFVKEY_b		98 /* ascii 'b' */
#define PFVKEY_c		99 /* ascii 'c' */
#define PFVKEY_d		100 /* ascii 'd' */
#define PFVKEY_e		101 /* ascii 'e' */
#define PFVKEY_f		102 /* ascii 'f' */
#define PFVKEY_g		103 /* ascii 'g' */
#define PFVKEY_h		104 /* ascii 'h' */
#define PFVKEY_i		105 /* ascii 'i' */
#define PFVKEY_j		106 /* ascii 'j' */
#define PFVKEY_k		107 /* ascii 'k' */
#define PFVKEY_l		108 /* ascii 'l' */
#define PFVKEY_m		109 /* ascii 'm' */
#define PFVKEY_n		110 /* ascii 'n' */
#define PFVKEY_o		111 /* ascii 'o' */
#define PFVKEY_p		112 /* ascii 'p' */
#define PFVKEY_q		113 /* ascii 'q' */
#define PFVKEY_r		114 /* ascii 'r' */
#define PFVKEY_s		115 /* ascii 's' */
#define PFVKEY_t		116 /* ascii 't' */
#define PFVKEY_u		117 /* ascii 'u' */
#define PFVKEY_v		118 /* ascii 'v' */
#define PFVKEY_w		119 /* ascii 'w' */
#define PFVKEY_x		120 /* ascii 'x' */
#define PFVKEY_y		121 /* ascii 'y' */
#define PFVKEY_z		122 /* ascii 'z' */

#define PFVKEY_OPENCURLY	123 /* ascii '{' */
#define PFVKEY_OR		124 /* ascii '|' */
#define PFVKEY_CLOSECURLY	125 /* ascii '}' */
#define PFVKEY_TILDA		126 /* ascii '~' */

#define PFVKEY_DELETE		127 /* Delete key */

///////////////////////////////////////////////////////////////////////////////

class pfvDisplayMngr;
class pfvDispChan;

///////////////////////////////////////////////////////////////////////////////

typedef int (*pfvInputMngrCBFunc_t)(int,char,void*); 

///////////////////////////////////////////////////////////////////////////////

typedef struct _pfvSharedInputData pfvSharedInputData;

///////////////////////////////////////////////////////////////////////////////

class PFV_DLLEXPORT pfvInputMngrCallback
{
friend class pfvInputMngr;

private:
    pfvInputMngrCallback(pfvInputMngrCBFunc_t func, void*data);
    ~pfvInputMngrCallback();

    static int dispatchEvents();
    static int dispatchKeyEvent(int key,uint64_t vMask);

public:	
    static void init();

    void enable() { enabled=1; }
    void disable() {enabled=0; }
    int getEnabled() { return enabled; }

    uint64_t getViewMask() { return viewMask; }
    void setViewMask(uint64_t mask) {viewMask=mask; }

    char* getEventMask() { return evMask; }
    void setEventMask(char* str);

    int bindKeys(char*keys);
    int unbindKeys(char*keys);
	
private:

    int enabled;
	    
    uint64_t viewMask;
    char* evMask;

    pfvInputMngrCBFunc_t func;
    void* cbData;

    static pfList* listCBs; /* a list of all user callbacks */
    static pfList** cbTable; /* array of lists of cbs relevant to each 'key' */	
};

///////////////////////////////////////////////////////////////////////////////

class PFV_DLLEXPORT pfvInputMngr
{
public:
	
    static int init();
    
    static int collectEvents();
    
    static pfuMouse* getMouse() { return &mouse; }
    static pfuEventStream* getEvents() { return &events; }

    static int getFocusViewIndex() { return focusViewInd; }
    static int getViewNormXY( float*x, float*y ) {
	if(x)*x=viewNormXY[0]; if(y)*y=viewNormXY[1]; return focusViewInd; }

    static int getFocusChanIndex() { return focusChanInd; }
    static pfvDispChan* getFocusChan() { return focusChan; }
    static int getChanNormXY( float*x, float*y ) {
	if(x)*x=chanNormXY[0]; if(y)*y=chanNormXY[1]; return focusChanInd; }
    static int getChanAbsXY( int*x, int*y ) {
	if(x)*x=chanAbsXY[0]; if(y)*y=chanAbsXY[1]; return focusChanInd; }

    static int getFocusPWinIndex() { return focusPWinInd; }
    static int getPWinNormXY( float*x, float*y ) {
	    if(x)*x=pwinNormXY[0]; if(y)*y=pwinNormXY[1]; return focusPWinInd; }
    static int getPWinAbsXY( int*x, int*y ) {
	    if(x)*x=pwinAbsXY[0]; if(y)*y=pwinAbsXY[1]; return focusPWinInd; }
	    
    // user callmack related methods
    static pfvInputMngrCallback* addCallback( pfvInputMngrCBFunc_t func,
	void* data, char* evMask, uint64_t viewMask=0 );
    
    static int deleteCallback( pfvInputMngrCallback* cb );
    static int deleteCallback( pfvInputMngrCBFunc_t func, void* data ){
	return deleteCallback( findCallback(func,data) );
    }
    
    static pfvInputMngrCallback* findCallback(pfvInputMngrCBFunc_t func,void*data);

    static int getNumCallbacks() { return listCBs->getNum(); }
    static pfvInputMngrCallback* getCallback(int i){
	return (pfvInputMngrCallback*)(listCBs->get(i));
    }

    static int dispatchEvents(){ 
	return pfvInputMngrCallback::dispatchEvents(); 
    }
    static int dispatchKeyEvent(int key,int viewMask=0){ 
	return pfvInputMngrCallback::dispatchKeyEvent(key,viewMask);
    }

    static char* getKeyName(int key){
        return ((key<1)||(key>PFV_MAX_KEYVAL))? (char*)"" : keyNames[key];
	}

    static int getKeyFromName(char*keyName);
	
private:
    
    static pfvDisplayMngr* dm;
    
    static pfuMouse mouse;
    static pfuEventStream events;

    static int focusViewInd;
    static pfVec2 viewNormXY;
    
    static int focusChanInd;
    static pfvDispChan* focusChan;
    static pfVec2 chanNormXY;
    static int chanAbsXY[2];
    
    static int focusPWinInd;
    static pfVec3 pwinNormXY;
    static int pwinAbsXY[2];
    
    static char* keyNames[PFV_MAX_KEYVAL+1];
	
    // user callbacks related data
    static pfList* listCBs; /* a list of all user callbacks */
    static pfList** cbTable; /* array of lists of cbs relevant to each 'key' */

    static pfvSharedInputData* sharedData;
    static int onlyUnmanagedWins;

    static int findUtilDataPool();
    static void collectUnmanagedEvents();

};

///////////////////////////////////////////////////////////////////////////////

#endif /* end of __PFV_INPUT_MNGR_H__ */

