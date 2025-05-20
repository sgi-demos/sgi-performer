//
//
// Copyright 1995, Silicon Graphics, Inc.
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
// pfCullProgram.h		Channel class include file
//
// $Revision: 1.21 $
// $Date: 2002/11/10 02:00:36 $
//

#ifndef __PF_CULLPROGRAM_H__
#define __PF_CULLPROGRAM_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <math.h>
#include <string.h>

#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfLists.h>
#include <Performer/pf/pfChannel.h>


class pfCullProgram;

typedef int (*_pfCullPgInstruction)(pfCullProgram *, int);

// _pfIntList doesn't allow us to get info about the number of elements,
// it returns only the size of the allocated array
typedef _pfTemplateList<int> _pfIntegerList;

// run-time (of a cull program) information. This is the information that
// gets past from parents to children as the tree is traversed and the
// program is run and /or some polytope tests are performed.
typedef struct _pfCullPgInfo{
    uint64_t          cullResult;     // results of polytope tests

    uint64_t          childOrderMask;  
    int               sortOrderParent;// parent from which we inherit sort
    uint64_t          binParentsMask; // mask of the first 63 parents
    _pfUint64List     *binParents;    // mask of all additional bin parents
    int               bin;            // stores the last bin
    _pfIntegerList    *nonexclusiveParents; 
#if _PFCULLPROGRAM_PUSH_TEST==1
    int               push;           // should the value be pushed on the 
                                      // stack?
#endif
    int               (*runProgramNodePtr)(pfCullProgram *, _pfCullPgInfo *, pfSphere *);
} _pfCullPgInfo;


// type of cull instructions
struct _pfCullInstructionType {
    _pfCullPgInstruction instruction;
    int    data;
};

typedef _pfTemplateList<_pfCullInstructionType> _pfCullInstructionList;
typedef _pfTemplateList<pfPolytope *> _pfPolytopeList;


extern "C" {
/* cull program instruction opcodes */
#define PFCULLPG_TEST_POLYTOPE            0

#define PFCULLPG_ASSIGN_BIN_MAYBE         1
#define PFCULLPG_ASSIGN_BIN_TRUE          1  // true is the same as maybe
#define PFCULLPG_ASSIGN_BIN_ALL_IN        2
#define PFCULLPG_ASSIGN_BIN_ALL_OUT       3
#define PFCULLPG_ASSIGN_BIN_FALSE         3  // false is the same as all out
#define PFCULLPG_ASSIGN_BIN               4

#define PFCULLPG_ADD_BIN_MAYBE            5
#define PFCULLPG_ADD_BIN_TRUE             5  // true is the same as maybe
#define PFCULLPG_ADD_BIN_ALL_IN           6
#define PFCULLPG_ADD_BIN_ALL_OUT          7
#define PFCULLPG_ADD_BIN_FALSE            7  // false is the same as all out
#define PFCULLPG_ADD_BIN                  8

#define PFCULLPG_JUMP_MAYBE               9
#define PFCULLPG_JUMP_TRUE                9  // true is the same as maybe
#define PFCULLPG_JUMP_ALL_IN             10
#define PFCULLPG_JUMP_ALL_OUT            11
#define PFCULLPG_JUMP_FALSE              11
#define PFCULLPG_JUMP                    12

#define PFCULLPG_TEST_IS_SUBBIN_OF       13

#define PFCULLPG_RETURN                  14

/* bit flag for return instruction */
#define PFCULLPG_TEST_TRANSPARENCY       (1<<0)
#define PFCULLPG_DONT_TEST_TRANSPARENCY  (1<<1)
#define PFCULLPG_TEST_LPOINTS            (1<<2)
#define PFCULLPG_DONT_TEST_LPOINTS       (1<<3)
#define PFCULLPG_CULL_ON_ALL_IN          (1<<4) // cull based on the last test
#define PFCULLPG_CULL_ON_ALL_OUT         (1<<5) // cull based on the last test
#define PFCULLPG_CULL                    (1<<6) // cull 

/* flags stored with cull program */
#define PFCULLPG_GEOSET_PROGRAM_ACTIVATED (1<<0)
#define PFCULLPG_NODE_PROGRAM_ACTIVATED   (1<<1)
#define PFCULLPG_DONOT_XFORM_BBOXES       (1<<2)

/* flags used in resetCullProgram */
#define PFCULLPG_GEOSET_PROGRAM           (1<<0)
#define PFCULLPG_NODE_PROGRAM             (1<<1)
}

//////////////////////////////////////////////


#define PFCULLPROGRAM ((pfCullProgram*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFCULLPROGRAMBUFFER ((pfCullProgram*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfCullProgram : public pfUpdatable
{
 private:
    _pfCullInstructionList *cullProgram;
    _pfCullInstructionList *cullProgramNode;
    _pfPolytopeList     *cullPolytopes;

 public:
    // run-time methods and values, used by instructions
    int                  currentResult; // result of the last polytope test
    const pfBox          *bbox;         // geoset's or string's bounding box
    int                  flags;

 private:
    int                  cullPC;        // program counter
    int                  polytopesTested; // set to 1 if no tests needed

    _pfBinList           *chanBins;     // points to channel's list of bins
    pfChannel            *channel;      // points to channel

    _pfCullPgInfo         pgInfo;
    
 public: 
    int                  testPolytope(int index);   
    void                 addBinParent(int bin);
    int                  isSubbinOf(int bin);
    void                 resetBinParents(void);

    // methods used to setup cull program and polytopes it refers to 
// CAPI:verb CullProgramResetPgm
    void                 resetCullProgram(int flags);
// CAPI:verb CullProgramAddPgmOpcode
    void                 addCullProgramOpcode(int opcode, int data);
// CAPI:verb CullProgramAddPgmInstruction
    void                 addCullProgramInstruction(_pfCullPgInstruction instruction, int data);
// CAPI:noverb 
    void                 setNumPolytopes(int i);
    int                  getNumPolytopes(void);
    void                 setPolytope(int index, pfPolytope *pol);
    pfPolytope *         getPolytope(int index);
    void                 setFlags(int which, int value);
    int                  getFlags(int which);

 protected:
    pfCullProgram(pfBuffer *buf, pfChannel *chan, _pfBinList *bins);
    pfCullProgram(const pfCullProgram *src, pfBuffer *dstBuf);

};


#endif /* __PF_CULLPROGRAM_H__ */
