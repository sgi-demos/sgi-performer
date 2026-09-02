/*
 * Copyright 1997, MultiGen, Inc.
 * Copyright 1997, Paradigm Simulation, Inc.
 * ALL RIGHTS RESERVED
 *
 * UNPUBLISHED -- Rights reserved under the copyright laws of the United
 * States.   Use of a copyright notice is precautionary only and does not
 * imply publication or disclosure.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
 * in similar or successor clauses in the FAR, or the DOD or NASA FAR
 * Supplement.
 */

/*
 * $RCSfile: pfuAutoList.h,v $
 * $Revision: 1.7 $ 
 * $Date: 2002/09/15 13:18:31 $ 
 *
 * Description:
 *  	Performer Utility Auto-delete List class
 *
 * Notes:
 *  	A list for pfMemory objects that are owned by the list.
 *
 *  	Loaders can use this to avoid resource leaks by attaching
 *  	objects of this class as pfNode::userData and adding
 *  	pfMemory objects to this list. When the node is deleted the
 *  	user data resources will also be deleted.
 *
 *  	init() must be called before any use of this class!
 */

#ifndef __PFU_AUTOLIST_H__
#define __PFU_AUTOLIST_H__

#include <Performer/pr.h>
#include <Performer/pfutil-DLL.h>

#ifdef __cplusplus
extern "C" {     // EXPORT to CAPI
#endif
/* ----------------------- pfuAutoList Decl-s ----------------------- */

PFUDLLEXPORT void pfuInitAutoListClass(void);

#ifdef __cplusplus
} // extern "C" END of C include export
#endif


#ifdef __cplusplus

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfList.h>


#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

class PFUDLLEXPORT pfuAutoList : public pfObject
{
 public:
 
    ////////////
    // SERVICES
    ////////////

    static void     init ();
    static pfType*  getClassType () { return classType; }

    /////////
    // CTORS
    /////////

    virtual ~pfuAutoList ();

    pfuAutoList ();
    pfuAutoList ( int howMany );

    /////////
    // MTORS
    /////////

//MCB: not possible in typical expression, i.e. requires (*this) = rhs syntax
//MCB: besides no other Performer derived class overrides it either!
//MCB-VEGA    vgList&	  operator = ( const vgList &rhs );
//    virtual pfMemory* operator = ( pfMemory const* rhs );

    void    add ( pfMemory* elt );

    virtual int copy ( pfMemory const* rhs );

    void    insert ( int which, pfMemory* elt );

    int     move ( int which, pfMemory* elt )
    	    { return List->move( which, elt ); }
    
//MCB: not really implemented for pfList
//MCB-VEGA    void	    print ( int level );
    virtual int print ( uint travMode, uint verbose, char* prefix, FILE* );

    pfMemory*	rem ();

    int     remove ( pfMemory* elt );
    void    removeIndex ( int which );
    int     replace ( pfMemory* oldElt, pfMemory* newElt );
    void    reset ();
    void    set ( int which, pfMemory* elt );

    //////////
    // ACCESS
    //////////

    virtual int compare ( pfMemory const* rhs ) const;

    pfMemory*	get ( int which ) const
    	    	{ return ( pfMemory* ) List->get( which ); }

    int     	getNum () const { return List->getNum(); }

    int     search ( pfMemory* elt ) const { return List->search( elt ); }
    int     search ( pfType* type ) const;

    ////////
    // CONV
    ////////

 protected:

 private:

    /////////
    // MTORS
    /////////

    void    destroyElements ( pfList* );
    void    refElements ();

    ///////
    // DATA
    ///////

    pfList* List;

    static pfType*  classType;
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


#ifndef __BUILDCAPI__
extern "C" {     // EXPORT to CAPI
/* ------------------ pfuAutoList Macros ---------------------------- */
#if defined(__STDC__) || defined(__cplusplus)

#define pfuResetList(_list)		pfuResetList((pfuAutoList*)_list)
#define pfuGetNum(_list)		pfuGetNum((pfuAutoList*)_list)
#define pfuAdd(_list, _elt)		pfuAdd((pfuAutoList*)_list, _elt)
#define pfuInsert(_list, _index, _elt)	pfuInsert((pfuAutoList*)_list, _index, _elt)
#define pfuSearch(_list, _elt)		pfuSearch((pfuAutoList*)_list, _elt)
#define pfuSearchForType(_list, _type)	pfuSearchForType((pfuAutoList*)_list, _type)
#define pfuRemove(_list, _elt)		pfuRemove((pfuAutoList*)_list, _elt)
#define pfuReplace(_list, _old, _new)	pfuReplace((pfuAutoList*)_list, _old, _new)
#define pfuSet(_list, _index, _elt)	pfuSet((pfuAutoList*)_list, _index, _elt)
#define pfuGet(_list, _index)		pfuGet((pfuAutoList*)_list, _index)

#endif /* defined(__STDC__) || defined(__cplusplus) */
}
#endif // !__BUILDCAPI__

#endif
#endif // __PFU_AUTOLIST_H__
