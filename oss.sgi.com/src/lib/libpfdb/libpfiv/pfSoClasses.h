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


#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoLevelOfDetail.h>

class GroupLevelOfDetail : public SoLevelOfDetail {

   SO_NODE_HEADER(GroupLevelOfDetail);

 public:
   // Initializes this class for use in scene graphs. This
   // should be called after database initialization and before
   // any instance of this node is constructed.
   static void    initClass();

   // Constructor
   GroupLevelOfDetail();

 protected:
   virtual void   callback(SoCallbackAction *action);

 private:
   // Destructor. Private to keep people from trying to delete
   // nodes, rather than using the reference count mechanism.
   virtual ~GroupLevelOfDetail();
};


#if SO_VERSION_REVISION >= 1 
#include <Inventor/nodes/SoLOD.h>

class GroupLOD : public SoLOD {

   SO_NODE_HEADER(GroupLOD);

 public:
   // Initializes this class for use in scene graphs. This
   // should be called after database initialization and before
   // any instance of this node is constructed.
   static void    initClass();

   // Constructor
   GroupLOD();

 protected:
   virtual void   callback(SoCallbackAction *action);

 private:
   // Destructor. Private to keep people from trying to delete
   // nodes, rather than using the reference count mechanism.
   virtual ~GroupLOD();
};

#endif

class GroupBlinker : public SoBlinker {
   SO_NODE_HEADER(GroupBlinker);
 public:
   static void    initClass();
   GroupBlinker();
 protected:
   virtual void   callback(SoCallbackAction *action);
 private:
   virtual ~GroupBlinker();
};
