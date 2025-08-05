/*
 * Copyright 1998, 1999, 2000, Silicon Graphics, Inc.
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

#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include "pov.h"

pov::pov(void)
{
  pfVec4 colour;

  accelRate = 1.0f;
  MaxSpeed  = 257.22f; // 500 knots
  MinSpeed  = -257.22f;
  PitchAuthority = 10.0f;
  RollAuthority = 10.0f;
  TwistAuthority = 10.0f;
  Trange = 10.0f;
  Thead = 0.0f;
  Trate = 30.0f;
  TPamp = 0.4f;
  TPrate = 17.0f;
  wingcount = 0;

  reset();
}

void pov::reset(pfVec3 xyz)
{
  int i;

  subcoord.xyz = xyz;
  forward.set(0.0f, 1.0f, 0.0f);
  up.set(0.0f, 0.0f, 1.0f);
  speed = 0.0f;
  for(i=0;i<NUMTRAILS; i++)
  {
    simulate(0.0f, 0.0f, 0.0f, 0.016666667f);
  }
}

void pov::reset(void)
{
  pfVec3 position(0.0f, 0.0f, 0.0f);
  reset(position);
}

void pov::accelerate(float thrust)
{
  speed += thrust * accelRate;
  if(speed > MaxSpeed)
    speed = MaxSpeed;
  else
  if(speed < MinSpeed)
    speed = MinSpeed;
}

void pov::speedlimit(float factor)
{
  if(factor > 0.0f)
  {
    if(speed > MaxSpeed * factor)
      speed = MaxSpeed*factor;
  }
  else
  {
    if(speed < MinSpeed*factor*-1.0f)
      speed = MinSpeed*factor*-1.0f;
  }
}

void pov::simulate(float joyx, float joyy, float twist, float dtime)
{
  int i;
  float speedorientscale;
  pfVec3 starboard, norolup, norolstbd, calcvec;
  pfMatrix calcmat;
  float dot1, dot2, roll1, roll2;


  if(joyx > 1.0f)
    joyx = 1.0f;
  if(joyy > 1.0f)
    joyy = 1.0f;
  if(twist > 1.0f)
    twist = 1.0f;
  if(joyx < -1.0f)
    joyx = -1.0f;
  if(joyy < -1.0f)
    joyy = -1.0f;
  if(twist < -1.0f)
    twist = -1.0f;


  // pitch and roll & twist the viewpoint
  // use the forward and up vectors for this

  // speedorientscale determines actual orientation
  // authorities based on vehicle speed
/*
  speedorientscale = speed * .5f + 1.5f;
  if(speedorientscale < 1.5f)
*/
    speedorientscale = 1.5f;

  calcmat.makeRot(TwistAuthority * speedorientscale * dtime * -twist,
               up[0], up[1], up[2]);
  forward.xformVec(forward, calcmat);

  starboard.cross(forward, up);
  calcmat.makeRot(PitchAuthority * speedorientscale * dtime * -joyy,
               starboard[0], starboard[1], starboard[2]);
  forward.xformVec(forward, calcmat);
  up.xformVec(up, calcmat);

  calcmat.makeRot(RollAuthority * speedorientscale * dtime * joyx,
               forward[0], forward[1], forward[2]);
  up.xformVec(up, calcmat);

  // ensure forward & up are at right angles by generating the
  // cross products & then back
  starboard.cross(forward, up);
  up.cross(starboard, forward);

  // normalise forward & up vectors
  forward.normalize();
  up.normalize();

  // move the view forward by speed along the orientation vector
  subcoord.xyz += forward * (speed * dtime);

  // convert vector representation to Euler
  // heading
  subcoord.hpr[0] = pfArcTan2(-forward[0], forward[1]);
  // pitch
  subcoord.hpr[1] = pfArcSin(forward[2]);
  // roll
  calcmat.makeEuler(subcoord.hpr[0], subcoord.hpr[1], 0.0f);
  calcvec.set(0.0f, 0.0f, 1.0f);
  norolup.xformVec(calcvec, calcmat);
  calcvec.set(1.0f, 0.0f, 0.0f);
  norolstbd.xformVec(calcvec, calcmat);

  // the angle between norolup and up
  // and between norolstbd and up now
  // holds the required roll

  dot1 = up.dot(norolup);
  if (dot1 > 1.0f) dot1 = 1.0f;
  if (dot1 < -1.0f) dot1 = -1.0f;
  dot2 = up.dot(norolstbd);
  if (dot2 > 1.0f) dot2 = 1.0f;
  if (dot2 < -1.0f) dot2 = -1.0f;
  roll1 = pfArcCos( dot1 );
  roll2 = pfArcCos( dot2 );

  if(roll2 > 90.0f)
    roll1 = -roll1;
  subcoord.hpr[2] = roll1;

  // update wingman data for posview method
  wing[wingcount] = subcoord;
  wingcount++;
  if(wingcount >= NUMTRAILS)
    wingcount = 0;

  // update tether data for posview method
  TPloop += TPrate * dtime;
  while(TPloop > 360.0f)
    TPloop -= 360.0f;

  Thead += Trate * dtime;
  while(Thead > 360.0f)
    Thead -= 360.0f;

}

void pov::posview( pfCoord *view, int mode )
{
  float cval, sval, pval, attnval;

  switch(mode)
  {
    case(SUB_VIEW_POV):
      view->hpr = subcoord.hpr;
      view->xyz = subcoord.xyz;
    break;
    case(SUB_VIEW_WINGMAN):
      view->hpr = wing[wingcount].hpr;
      view->xyz = wing[wingcount].xyz;
    break;
    case(SUB_VIEW_TETHERHIGH):
      pfSinCos(Thead, &pval, &attnval);
      view->hpr.set(-Thead+180.0f, -80.0f, 0.0f);
      pfSinCos(Thead, &sval, &cval);
      view->xyz.set(Trange * sval *.2f, Trange * cval *.2f, Trange * 1.8f);
      view->xyz += subcoord.xyz;
    break;
    case(SUB_VIEW_TETHER):
      pfSinCos(Thead, &pval, &attnval);
      view->hpr.set(-Thead+180.0f, -pfArcSin(pval * TPamp), 0.0f);
      pfSinCos(Thead, &sval, &cval);
      view->xyz.set(Trange * sval, Trange * cval, Trange * pval * TPamp);
      view->xyz += subcoord.xyz;
    break;
    case(SUB_VIEW_FWDSHIFT):
      view->hpr = subcoord.hpr;
      view->xyz = subcoord.xyz + forward*1.0f;
    break;
  }
}

