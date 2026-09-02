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
// pfGeode.h		Text include file
//
// $Revision: 1.19 $
// $Date: 2002/05/30 00:33:56 $
//
#ifndef _PF_TEXT_H
#define _PF_TEXT_H

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfLists.h>


class pfBuffer;

#define PFTEXT ((pfText*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFTEXTBUFFER ((pfText*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfText : public pfNode
{
public:

    int addString(pfString *str)  {
        return PFTEXT->nb_addString(str);
    }

    int insertString(int index, pfString *str)  {
        return PFTEXT->nb_insertString(index, str);
    }

    int replaceString(pfString *oldgs, pfString *newgs)  {
        return PFTEXT->nb_replaceString(oldgs, newgs);
    }

    int removeString(pfString *str)  {
        return PFTEXT->nb_removeString(str);
    }

    pfString* getString(int i) const  {
        return PFTEXT->nb_getString(i);
    }

    int getNumStrings() const  {
        return PFTEXT->nb_getNumStrings();
    }
public:		// Constructors/Destructors
    //CAPI:updatable
    //CAPI:newargs
    pfText();
    virtual ~pfText();

protected:
    pfText(pfBuffer *buf);
    pfText(const pfText *prev, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:			// pfMemory virtual functions

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	    pf_applyUpdate(const pfUpdatable *prev, int upId);
    virtual pfUpdatable*    pf_bufferClone(pfBuffer *buf);

PFINTERNAL:		// pfNode virtual traversals
    virtual void	    nb_clean(uint64_t cleanBits, pfNodeCounts *counts);
    virtual int 	    nb_cull(int mode, int cullResult, _pfCuller *trav);
    virtual int 	    nb_cullProgram(int mode, int cullResult, _pfCullPgInfo *cullPgInfo, _pfCuller *trav);
    virtual int 	    nb_flatten(pfTraverser *trav);
    virtual int 	    nb_intersect(_pfIsector *isector);
    virtual void 	    nb_setTravMask(int which, uint bits, 
					int setWho, int lop);
    virtual void	    nb_write(pfTraverser *trav, uint which, 
				  uint verbose);

PFINTERNAL:		// pfText virtual functions
    //CAPI:virtual
    //CAPI:basename
    virtual int 	    nb_addString(pfString *str);
    virtual int 	    nb_insertString(int index, pfString *str);
    virtual int 	    nb_replaceString(pfString *oldgs, pfString *newgs);
    virtual int 	    nb_removeString(pfString *str);

PFINTERNAL:		// specific sets and gets
    pfString*		    nb_getString(int i) const {
	return (pfString*)strList[i];
    }
    int 		    nb_getNumStrings() const {
	return strList.getNum();
    }


protected:
    _pfVoidList   strList;	// List of pfStrings

private:
    static pfType *classType;
};

#endif // !__PFTEXT_H__
