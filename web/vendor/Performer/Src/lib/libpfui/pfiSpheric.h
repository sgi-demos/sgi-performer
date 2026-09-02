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
 * pfiSpheric.h
 *
 * $Revision: 1.11 $
 * $Date: 2002/09/18 02:47:08 $
 *
 */

#ifndef __PFI_SPHERIC_H__
#define __PFI_SPHERIC_H__

#include <Performer/pfui.h>
#include <Performer/pfui/pfiInputXform.h>

#include <Performer/pr/pfList.h>

#if PF_CPLUSPLUS_API

class PFUIDLLEXPORT pfiInputXformSpheric : public pfiInputXform
{
  private:
    static pfType *classType;
  public:
    int			  xMotionMod;
    int                   xTracking;
  public:
    void                  recalc(void); /* recalc values from position */
    void                  generateLocation(void);
    int                   doUpdate(pfiInputCoord *_ic);
  public:
    void                  setWorld(int worldNumber, int in_or_out);
    void                  readPathFile(char *filename);
    void                  printPathStuff(void);
public:
    void                  setParameter(int param, float num);
    float                 getParameter(int param);
    
private:
    float                 radius; /* r */
    float                 minRadius, maxRadius;
    float                 around;	    /* theta? */
    float                 nearMinAround, nearMaxAround;
    float                 farMinAround, farMaxAround;
    float                 tilt;		    /* phi? */
    float                 nearMinTilt, nearMaxTilt;
    float                 farMinTilt, farMaxTilt;
    float                 roll;
    float                 nearMinRoll, nearMaxRoll;
    float                 farMinRoll, farMaxRoll;
    
    float                 nearMaxLinearVelocity;
    float                 nearMaxAngularVelocity;
    float                 nearMaxRollVelocity;
    float                 farMaxLinearVelocity;
    float                 farMaxAngularVelocity;
    float                 farMaxRollVelocity;
private:
    pfList                *pathList;
    int                   numWorlds;
    int                   currentWorld;

  private:       /****** saved positions ******/
    pfVec3                center;
    short                 numChildren;
    short                 hasIn;
    int                   childNum; /* child aiming for */
    float                 inRadius[PFIXSPH_MAX_CHILDREN], outRadius;
    int                   atRadius;
    float                 inAround[PFIXSPH_MAX_CHILDREN], outAround;
    int                   atAround;
    float                 inTilt[PFIXSPH_MAX_CHILDREN], outTilt;
    int                   atTilt;
  public:	/****** constructor/destructors ******/
    pfiInputXformSpheric(void);
  public: 	/**** type ****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiInputXformSpheric"; }
  public: 	/**** generic interface ****/
    virtual void	_setBSphere(pfSphere* _sphere);
    virtual void	setMode(int _mode, int _val);
    virtual int		getMode(int _mode) const;
    virtual void        resetPosition(void);
};

#endif /* PF_CPLUSPLUS_API */

#endif /* __PF_SPHERIC_H__ */
