/*
 * WARNING! DO NOT EDIT!
 * pfiPickD.h automatically generated from (../libpfui/pfiPick.h)
 */
/*
 * Copyright 1993, 1994, 1995, Silicon Graphics, Inc.
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


/*
 * file: pfiPickD.h
 * ----------------
 *
 * pfiPickD Class Delcarations
 *
 * $Revision: 1.8 $
 * $Date: 2002/09/18 02:47:08 $
 *
 */

#ifndef __PICK_D_H__
#define __PICK_D_H__
 
#include <Performer/pfuiD.h>

#if PF_CPLUSPLUS_API

class PFUIDLLEXPORT pfiPickD : public pfObject
{
  private:
    static pfType *classType;
  public:
    int			 pPickMode;
    int			 pIsectMode;
    int			 pCount;
    pfChannel		*pChan;
    pfNode		*pNode;
    pfGeoSet		*pGSet;
    pfPath		*pPath;
    pfList		*pChanList; 
    pfList		*pNodeList;
    pfList		*pPathList;
  public:	/* constructor/destructors */
    pfiPickD(void);
    virtual ~pfiPickD(void);
  public:	/****** callbacks and associated data *****/
    void		 *pHitCBData;
    pfiPickDFuncType	 pHitCB;
  public:	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiPickD"; }
  public:	/***** sets/gets *****/
    void		 setMode(int _mode, int _val);
    int			 getMode(int _mode) const;
    void		 setHitFunc(pfiPickDFuncType _func, void *_data) {
	pHitCB = _func;
	pHitCBData = _data;
    }
    void		 getHitFunc(pfiPickDFuncType *_func, void **_data) {
	*_func = pHitCB;
	*_data = pHitCBData;
    }
 public:	/***** main interface ****/
    void		 addChan(pfChannel *_chan);
    void		 insertChan(int _index, pfChannel *_chan);
    void		 removeChan(pfChannel *_chan);
    int			 getCount(void) const { return pCount; }
    pfNode*		 getNode(void) const { return pNode; }
    pfPath*		 getPath(void) const { return pPath; }
    pfGeoSet*		 getGSet(void) const { return pGSet; }
    void		 setupChans(void);
    int		 	 doPick(int _x, int _y);
    void		 reset(void);
  private:
    int		 _doPick(pfChannel *_chan, int _x, int _y);
};  

#endif /* PF_CPLUSPLUS_API */
#endif /* __PICK_D_H__ */
