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

////////////////////////////////////////////////////////////////////////
//
// $RCSfile: pfuAutoList.C,v $
// $Date: 2002/05/15 00:31:09 $
// $Revision: 1.7 $
//
// Description:
//  	Performer Utility Auto-delete List class
//
// Notes:
//  	A list for pfMemory objects that are owned by the list.
//
//  	Loaders can use this to avoid resource leaks by attaching
//  	objects of this class as pfNode::userData and adding
//  	pfMemory objects to this list. When the node is deleted the
//  	user data resources will also be deleted.
//
// 
//  	init() must be called before any use of this class!
//
// To Do:
//
////////////////////////////////////////////////////////////////////////

#define __BUILDCAPI__

#include "pfuAutoList.h"

extern "C"
{
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

PFUDLLEXPORT pfType* pfuAutoList::classType = 0;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void
pfuAutoList::
init ()
{
    if ( ! classType )
    {
        pfObject::init();

        classType = new pfType( pfObject::getClassType(), "pfuAutoList" );
    }
}


PFUDLLEXPORT void
pfuInitAutoListClass ()
{
    pfuAutoList::init();
}

////////////////////////////////////////////////////////////////////////

extern "C"
{

PFUDLLEXPORT pfType*
pfuGetAutoListClassType ()
{
    return pfuAutoList::getClassType();
}

}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

pfuAutoList::
~pfuAutoList ()
{
    destroyElements( List );
    List->checkDelete();
}

////////////////////////////////////////////////////////////////////////

pfuAutoList::
pfuAutoList ()
    : List( new ( getArena() ) pfList )
{
    setType( classType );
}

pfuAutoList::
pfuAutoList ( int howMany )
    : List( new ( getArena() ) pfList( sizeof( pfMemory* ), howMany ) )
{
    setType( classType );
}

extern "C"
{

PFUDLLEXPORT pfuAutoList*
pfuNewAutoList ( int howMany, void* arena )
{
    return new ( arena ) pfuAutoList( howMany );
}

}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// delete each element in the list

void
pfuAutoList::
destroyElements ( pfList* list )
{
    int howMany = list->getNum();

    for ( int which = 0; which < howMany; ++ which )
    {
	pfMemory*   elt = ( pfMemory* ) list->get( which );

	if ( elt )
	    elt->unrefDelete();
    }
}

////////////////////////////////////////////////////////////////////////
// reference count each element in the list

void
pfuAutoList::
refElements ()
{
    int howMany = getNum();

    for ( int which = 0; which < howMany; ++ which )
    {
	pfMemory*   elt = get( which );

	if ( elt )
	    elt->ref();
    }
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// add element to the end (tail) of the list.

void
pfuAutoList::
add( pfMemory* elt )
{
    List->add( elt );
    elt->ref();
}

extern "C"
{

PFUDLLEXPORT void
pfuAdd ( pfuAutoList* self, void* elt )
{
    pfType* memType = pfMemory::getClassType();
    
    if ( pfMemory::isOfType( elt, memType ) )
    {
    	self->add( pfMemory::getMemory( elt ) );
    }
    else
    {
    	pfNotify( PFNFY_WARN, PFNFY_USAGE,
    	    	  "%s::add() - ( void* ) 0x%p is not a %s",
    	    	  self->getTypeName(),
    	    	  elt,
    	    	  memType->getName() );
    }
}

}

////////////////////////////////////////////////////////////////////////
// copy list according to pfList::copy()
// NOTE: pfObject::this->userData is not owned or deleted!!

int
pfuAutoList::
copy( pfMemory const* rhs )
{
    int ok = parentCopy( rhs, 0 );

    if ( ok )
    {
	pfuAutoList const*  other = ( pfuAutoList* ) rhs;

    	pfList* tmp = new ( getArena() ) pfList( other->List->getEltSize(),
    	    	    	    	    	    	 other->List->getNum() );

	ok = tmp->copy( other->List );

	if ( ok )
	{
    	    pfList* old = List;
    	    
     	    List = tmp;
	    refElements();

    	    destroyElements( old ); // exception safe now
    	    old->checkDelete();
	}
	else
	{
	    tmp->checkDelete();
	}
    }
    return ok;
}

extern "C"
{

//  works with pfCopy()

}

////////////////////////////////////////////////////////////////////////
// insert element into list at position given.

void
pfuAutoList::
insert ( int which, pfMemory* elt )
{
    List->insert( which, elt );
    elt->ref();
}

extern "C"
{

PFUDLLEXPORT void
pfuInsert ( pfuAutoList* self, int which, void* elt )
{
    pfType* memType = pfMemory::getClassType();
    
    if ( pfMemory::isOfType( elt, memType ) )
    {
    	self->insert( which, pfMemory::getMemory( elt ) );
    }
    else
    {
    	pfNotify( PFNFY_WARN, PFNFY_USAGE,
    	    	  "%s::insert() - ( void* ) 0x%p is not a %s",
    	    	  self->getTypeName(),
    	    	  elt,
    	    	  memType->getName() );
    }
}

}

////////////////////////////////////////////////////////////////////////
// move (insert) first occurance of element to position given.

extern "C"
{

PFUDLLEXPORT int
pfuMove ( pfuAutoList* self, int which, void* elt )
{
    pfType* memType = pfMemory::getClassType();
    
    if ( pfMemory::isOfType( elt, memType ) )
    {
    	return self->move( which, pfMemory::getMemory( elt ) );
    }
    else
    {
    	pfNotify( PFNFY_WARN, PFNFY_USAGE,
    	    	  "%s::move() - ( void* ) 0x%p is not a %s",
    	    	  self->getTypeName(),
    	    	  elt,
    	    	  memType->getName() );
    }
    return -1;
}

}

////////////////////////////////////////////////////////////////////////
// print list ... as well as pfList::print() does at least.

int
pfuAutoList::
print ( uint, uint, char* prefix, FILE* file )
{
    return fprintf( file ? file : stderr,
    		    "%s%s: 0x%p %d elts of %d bytes\n",
    		    prefix ? prefix : "",
    		    getTypeName(),
    		    this,
    		    getNum(),
    		    sizeof( pfMemory* ) );
}

extern "C"
{

// works with pfPrint()

}

////////////////////////////////////////////////////////////////////////
// remove element from tail of list. return element if any.

pfMemory*
pfuAutoList::
rem ()
{
    int     	which = getNum() - 1;
    pfMemory*	elt = get( which );

    if ( elt )
    {
    	List->removeIndex( which );
    	elt->unref();
    }
    return elt;
}

extern "C"
{

PFUDLLEXPORT void*
pfuRem ( pfuAutoList* self )
{
    pfMemory*	elt = self->rem();

    // NOTE: return getData when dealing with pfMemory
    return elt && elt->isExactType( pfMemory::getClassType() )
    	?  elt->getData()
    	:  elt;
}

}

////////////////////////////////////////////////////////////////////////
// remove element from list if present.

int
pfuAutoList::
remove ( pfMemory* elt )
{
    int ok = List->remove( elt );

    if ( ok >= 0 )
    	elt->unref();

    return ok;
}

extern "C"
{

PFUDLLEXPORT int
pfuRemove ( pfuAutoList* self, void* elt )
{
    pfType* memType = pfMemory::getClassType();
    
    if ( pfMemory::isOfType( elt, memType ) )
    {
    	return self->remove( pfMemory::getMemory( elt ) );
    }
    else
    {
    	pfNotify( PFNFY_WARN, PFNFY_USAGE,
    	    	  "%s::remove() - ( void* ) 0x%p is not a %s",
    	    	  self->getTypeName(),
    	    	  elt,
    	    	  memType->getName() );
    }
    return -1;
}

}

////////////////////////////////////////////////////////////////////////
// remove element at position given from list if present.

void
pfuAutoList::
removeIndex ( int which )
{
    pfMemory*	elt = get( which );

    if ( elt )
    {
    	List->removeIndex( which );
    	elt->unref();
    }
}

extern "C"
{

PFUDLLEXPORT void
pfuRemoveIndex ( pfuAutoList* self, int which )
{
    self->removeIndex( which );
}

}

////////////////////////////////////////////////////////////////////////
// replace first instance of old element with new element.

int
pfuAutoList::
replace ( pfMemory* oldElt, pfMemory* newElt )
{
    int ok = List->replace( oldElt, newElt );
    
    if ( ok >= 0 )
    {
    	oldElt->unref();
    	newElt->ref();
    }
    return ok;
}

extern "C"
{

PFUDLLEXPORT int
pfuReplace ( pfuAutoList* self, void* oldElt, void* newElt )
{
    pfType* memType = pfMemory::getClassType();
    
    if ( pfMemory::isOfType( oldElt, memType ) )
    {
    	if ( pfMemory::isOfType( newElt, memType ) )
	{
	    return self->replace( pfMemory::getMemory( oldElt ),
    	    	    		  pfMemory::getMemory( newElt ) );
	}
	else
	{
    	    pfNotify( PFNFY_WARN, PFNFY_USAGE,
    	    	      "%s::replace() - ( void* ) 0x%p is not a %s",
    	    	      self->getTypeName(),
    	    	      newElt,
    	    	      memType->getName() );
	}
    }
    else
    {
    	pfNotify( PFNFY_WARN, PFNFY_USAGE,
    	    	  "%s::replace() - ( void* ) 0x%p is not a %s",
    	    	  self->getTypeName(),
    	    	  oldElt,
    	    	  memType->getName() );
    }
    return -1;
}

}

////////////////////////////////////////////////////////////////////////
// reset list to empty state.

void
pfuAutoList::
reset ()
{
    destroyElements( List );
    List->reset();
}

extern "C"
{

PFUDLLEXPORT void
pfuResetList ( pfuAutoList* self )
{
    self->reset();
}

}

////////////////////////////////////////////////////////////////////////
// set indicated element in list with given element.
// release existing element if any.

void
pfuAutoList::
set ( int which, pfMemory* elt )
{
    pfMemory*	oldElt = get( which );

    if ( oldElt )
    	oldElt->unref();

    List->set( which, elt );
    elt->ref();
}

extern "C"
{

PFUDLLEXPORT void
pfuSet ( pfuAutoList* self, int which, void* elt )
{
    pfType* memType = pfMemory::getClassType();
    
    if ( pfMemory::isOfType( elt, memType ) )
    {
    	self->set( which, pfMemory::getMemory( elt ) );
    }
    else
    {
    	pfNotify( PFNFY_WARN, PFNFY_USAGE,
    	    	  "%s::set() - ( void* ) 0x%p is not a %s",
    	    	  self->getTypeName(),
    	    	  elt,
    	    	  memType->getName() );
    }
}

}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// compare two lists for recursive, bit-wise equality
// NOTE: consistant with pfCopy(), but not a proper element-wise compare

int
pfuAutoList::
compare ( pfMemory const* rhs ) const
{
    int ok = parentCompare( rhs );

    if ( ok == 0 )
    {
	pfuAutoList const*  other = ( pfuAutoList* ) rhs;

	ok = List->compare( other->List );
	if ( ok == 0 )
	{
	    // compare other data members
	}
    }
    return ok;
}

extern "C"
{

// works with pfCompare()

}

////////////////////////////////////////////////////////////////////////
// get indicated element from list. return element if any.

extern "C"
{

PFUDLLEXPORT void*
pfuGet( pfuAutoList const* self, int which )
{
    pfMemory*	elt = self->get( which );

    // NOTE: return getData when dealing with pfMemory
    return elt && elt->isExactType( pfMemory::getClassType() )
    	?  elt->getData()
    	:  elt;
}

}

////////////////////////////////////////////////////////////////////////
// get number of elements in list.

extern "C"
{

PFUDLLEXPORT int
pfuGetNum ( pfuAutoList const* self )
{
    return self->getNum();
}

}

////////////////////////////////////////////////////////////////////////
// search for first instance of indicated element.
// return index if found.

extern "C"
{

PFUDLLEXPORT int
pfuSearch ( pfuAutoList const* self, void* elt )
{
    pfType* memType = pfMemory::getClassType();
    
    if ( pfMemory::isOfType( elt, memType ) )
    {
    	return self->search( pfMemory::getMemory( elt ) );
    }
    else
    {
    	pfNotify( PFNFY_WARN, PFNFY_USAGE,
    	    	  "%s::search() - ( void* ) 0x%p is not a %s",
    	    	  self->getTypeName(),
    	    	  elt,
    	    	  memType->getName() );
    }
    return -1;
}

}

////////////////////////////////////////////////////////////////////////
// search for first instance of indicated type.
// return index if found.

int
pfuAutoList::
search ( pfType* type ) const
{
    int howMany = getNum();

    for ( int which = 0; which < howMany; ++ which )
    {
	pfMemory*   elt = get( which );

	if ( elt->isOfType( type ) )
	{
	    return which;
	}
    }
    return -1;
}

extern "C"
{

PFUDLLEXPORT int
pfuSearchForType ( pfuAutoList const* self, pfType* type )
{
    return self->search( type );
}

}

