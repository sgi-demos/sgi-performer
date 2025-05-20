/*
 * Copyright 1993, 1995, Silicon Graphics, Inc.
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

#include <stdio.h>

#include <Performer/pf.h>
#include <Performer/pr/pfTexture.h>
#include <Performer/pr/pfList.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pfutil/pfuClipCenterNode.h>
#include <Performer/pf/pfMPClipTexture.h>
#include <Performer/pfdu.h>

#include "pfuTextureManager.h"

pfType *pfuTextureManager::classType = 0;
pfType *pfuTextureManager::pfuTexList::classType = 0;
pfType *pfuTextureManager::pfuTex::classType = 0;

#ifndef _LIBGEN_H
static inline char *basename( char *path )
{
    char *ptr;
    int len;

    if( path == 0L ) 
	return (char *)0L;

    len = 0;
    for( ptr = path; *ptr; ptr++ ) 
	len++;
    if( len == 0 ) 
        return path;
    ptr = &path[len];
    while( ptr > path &&  ptr[-1] != '/' ) 
	ptr--;
    return ptr;
}
#endif


void pfuTextureManager::init( void )
{
    if( classType == 0 )
    {
        pfObject::init();

        classType = new pfType( pfObject::getClassType(), "pfuTextureManager" );
    }
    pfuTextureManager::pfuTexList::init();
    pfuTextureManager::pfuTex::init();
    
}

pfuTextureManager::pfuTextureManager( void )
{
    pfQuerySys( PFQSYS_TEXTURE_MEMORY_BYTES, &tmSize );

    masterList     = new pfuTextureManager::pfuTexList( "master-list" );
    masterPfList   = new pfList;
    texLists       = new pfList;
    texLists->add( masterList );
}

pfuTextureManager::~pfuTextureManager( void )
{
}

static inline uint *translateImageData( uint *data, int comp, int ns, int nt, int nr, int tcomp )
{
    uint *tdata;
    unsigned char *cptr;
    unsigned char *tcptr;
    int i;

    switch( comp )
    {
        case 1:
            switch( tcomp )
            {
                case 1 :
                    tdata = data;
                    break;
    
                case 2 :
                    tdata = (uint *)pfMalloc( (sizeof( uint ) * ns * nt * nr)/2, pfGetSharedArena() );
                    cptr  = (unsigned char *)data;
                    tcptr = (unsigned char *)tdata;
                    for( i = 0; i < (ns * nt * nr); i++ )
                    {
                        *(tcptr++) = *(cptr);
                        *(tcptr++) = *(cptr++);
                    }
                    break;
                
                case 3 :
                    tdata = (uint *)pfMalloc( (sizeof( uint ) * ns * nt * nr), pfGetSharedArena() );
                    cptr  = (unsigned char *)data;
                    tcptr = (unsigned char *)tdata;
                    for( i = 0; i < (ns * nt * nr); i++ )
                    {
                        *(tcptr++) = *(cptr);
                        *(tcptr++) = *(cptr);
                        *(tcptr++) = *(cptr++);
                    }
                    break;
                
                case 4 :
                    tdata = (uint *)pfMalloc( (sizeof( uint ) * ns * nt * nr), pfGetSharedArena() );
                    cptr  = (unsigned char *)data;
                    tcptr = (unsigned char *)tdata;
                    for( i = 0; i < (ns * nt * nr); i++ )
                    {
                        *(tcptr++) = *(cptr);
                        *(tcptr++) = *(cptr);
                        *(tcptr++) = *(cptr);
                        *(tcptr++) = *(cptr++);
                    }
                    break;
            }
            break;

        case 2 :
            switch( tcomp )
            {
                case 1 :
                    tdata = (uint *)pfMalloc( (sizeof( uint ) * ns * nt * nr)/4, pfGetSharedArena() );
                    cptr  = (unsigned char *)data;
                    tcptr = (unsigned char *)tdata;
                    for( i = 0; i < (ns * nt * nr); i++ )
                    {
                        *(tcptr++) = *(cptr++);
                        cptr++;
                    }
                    break;
    
                case 2 :
                    tdata = data;
                    break;
                
                case 3 :
                    tdata = (uint *)pfMalloc( (sizeof( uint ) * ns * nt * nr), pfGetSharedArena() );
                    cptr  = (unsigned char *)data;
                    tcptr = (unsigned char *)tdata;
                    for( i = 0; i < (ns * nt * nr); i++ )
                    {
                        *(tcptr++) = *(cptr);
                        *(tcptr++) = *(cptr++);
                        *(tcptr++) = *(cptr++);
                    }
                    break;
                
                case 4 :
                    tdata = (uint *)pfMalloc( (sizeof( uint ) * ns * nt * nr), pfGetSharedArena() );
                    cptr  = (unsigned char *)data;
                    tcptr = (unsigned char *)tdata;
                    for( i = 0; i < (ns * nt * nr); i++ )
                    {
                        *(tcptr++) = *(cptr);
                        *(tcptr++) = *(cptr);
                        *(tcptr++) = *(cptr++);
                        *(tcptr++) = *(cptr++);
                    }
                    break;
            }
            break;

        case 3:
            switch( tcomp )
            {
                case 1 :
                    tdata = (uint *)pfMalloc( (sizeof( uint ) * ns * nt * nr)/4, pfGetSharedArena() );
                    cptr  = (unsigned char *)data;
                    tcptr = (unsigned char *)tdata;
                    for( i = 0; i < (ns * nt * nr); i++ )
                    {
                        unsigned char r, g, b;
    
                        r = *(cptr++);
                        g = *(cptr++);
                        b = *(cptr++);
                        *(tcptr++) = (r & 0xe0) | ((g & 0xe0) >> 3) | ((b & 0xC0) >> 5); 
                    }
                    break;
    
                case 2 :
                    tdata = (uint *)pfMalloc( (sizeof( uint ) * ns * nt * nr)/2, pfGetSharedArena() );
                    cptr  = (unsigned char *)data;
                    tcptr = (unsigned char *)tdata;
                    for( i = 0; i < (ns * nt * nr); i++ )
                    {
                        unsigned char r, g, b;
    
                        r = *(cptr++);
                        g = *(cptr++);
                        b = *(cptr++);
                        *(tcptr++) = (r & 0xe0) | ((g & 0xe0) >> 3) | ((b & 0xC0) >> 5); 
                        *(tcptr++) = 0xFF;
                    }
                    break;
                
                case 3 :
                    tdata = data;
                    break;
                
                case 4 :
                    tdata = (uint *)pfMalloc( (sizeof( uint ) * ns * nt * nr), pfGetSharedArena() );
                    cptr  = (unsigned char *)data;
                    tcptr = (unsigned char *)tdata;
                    for( i = 0; i < (ns * nt * nr); i++ )
                    {
                        *(tcptr++) = *(cptr++);
                        *(tcptr++) = *(cptr++);
                        *(tcptr++) = *(cptr++);
                        *(tcptr++) = 0xFF;
                    }
                    break;
            }
            break;

        case 4:
            switch( tcomp )
            {
                case 1 :
                    tdata = (uint *)pfMalloc( (sizeof( uint ) * ns * nt * nr)/4, pfGetSharedArena() );
                    cptr  = (unsigned char *)data;
                    tcptr = (unsigned char *)tdata;
                    for( i = 0; i < (ns * nt * nr); i++ )
                    {
                        unsigned char r, g, b;
    
                        r = *(cptr++);
                        g = *(cptr++);
                        b = *(cptr++);
                        *(tcptr++) = (r & 0xe0) | ((g & 0xe0) >> 3) | ((b & 0xC0) >> 5); 
                        cptr++;
                    }
                    break;
    
                case 2 :
                    tdata = (uint *)pfMalloc( (sizeof( uint ) * ns * nt * nr)/2, pfGetSharedArena() );
                    cptr  = (unsigned char *)data;
                    tcptr = (unsigned char *)tdata;
                    for( i = 0; i < (ns * nt * nr); i++ )
                    {
                        unsigned char r, g, b;
    
                        r = *(cptr++);
                        g = *(cptr++);
                        b = *(cptr++);
                        *(tcptr++) = (r & 0xe0) | ((g & 0xe0) >> 3) | ((b & 0xC0) >> 5); 
                        *(tcptr++) = *(cptr++);
                    }
                    break;
                
                case 3 :
                    tdata = (uint *)pfMalloc( (sizeof( uint ) * ns * nt * nr), pfGetSharedArena() );
                    cptr  = (unsigned char *)data;
                    tcptr = (unsigned char *)tdata;
                    for( i = 0; i < (ns * nt * nr); i++ )
                    {
                        *(tcptr++) = *(cptr++);
                        *(tcptr++) = *(cptr++);
                        *(tcptr++) = *(cptr++);
                        cptr++;
                    }
                    break;
                
                case 4 :
                    tdata = data;
                    break;
            }
            break;
        
    }
    return tdata;
}  

pfuTextureManager::pfuTexList *pfuTextureManager::addTextureList(  pfList *tlist, const char *name, pfTexture *dflt )
{
    int              n;
    pfTexture **tarray;
    pfuTexList   *texList;
    pfuTex       *tex;

    if( tlist == (pfList *)0L )
    {
        pfNotify( PFNFY_WARN, PFNFY_WARN,
                        "pfuTextureManager::addTextureList() :"
                        "pfList of textures is NULL.\n");
        return (pfuTexList *)0L;
    }

    if( (n = tlist->getNum()) == 0 ) 
    {
        pfNotify( PFNFY_WARN, PFNFY_WARN,
                        "pfuTextureManager::addTextureList() :"
                        "pfList of textures is of 0 length.\n");
        return (pfuTexList *)0L;
    }

    tarray = (pfTexture **)tlist->getArray();

    texList = new pfuTexList( name );

    for( int i = 0; i < n; i++ )
    {
        pfTexture *pftex = (pfTexture *)tarray[i];

        if( (tex = masterList->isInList( pftex )) == (pfuTex *)0L )
        {
            if( dflt != (pfTexture *)0L )
            {
                uint *image;
                int comp, ns, nt, nr ;
                int tcomp;

                pftex->getImage( &image, &tcomp, &ns, &nt, &nr );
                dflt->getImage( &image, &comp, &ns, &nt, &nr ); 

                if( tcomp == comp )
                    tex = new pfuTex( pftex, dflt );
                else
                {
                    if( texList->defaults[comp-1] == (pfTexture *)0L )
                    {
                        texList->defaults[comp-1] = new pfTexture;
                        image = translateImageData( image, comp, ns, nt, nr, tcomp ); 
                        texList->defaults[comp-1]->setImage( image, tcomp, ns, nt, nr );
                    }

                    tex = new pfuTex( pftex, texList->defaults[comp-1] );
                }
                
            }
            else
                tex = new pfuTex( pftex );
            masterList->addTex( tex );
            masterPfList->add( pftex );
        }
        if( texList->addTex( tex ) )
            tex->incRef();
    }

    texLists->add( texList );

    return texList;
}


void pfuTextureManager::traverseAndAssembleTextureList( pfNode *node, pfList *tlist )
{
    int i;

    if (node->isOfType(pfuClipCenterNode::getClassType()))
    {
	pfuClipCenterNode *ccn = (pfuClipCenterNode *) node;
	pfClipTexture *ct = ccn->getClipTexture();
	pfTexture *tex = ct;

	if( tex != (pfTexture *)0L )
	{
	    int found = 0;

	    for( int i = 0; i < tlist->getNum(); i++ )
	    {
		pfTexture *ttex = (pfTexture *)tlist->get( i );
		if( tex == ttex || !strcmp( tex->getName(), ttex->getName() ) )
		{
		    found = 1;
		    break;
		}
	    }
	    if( !found )
		tlist->add( tex );
	}
    }
    else if (node->isOfType(pfGroup::getClassType()))
    {
        pfGroup *group = (pfGroup *) node;
        for(i = 0; i < group->getNumChildren(); i++)
            traverseAndAssembleTextureList( group->getChild(i), tlist );

    }
    else if( node->isOfType( pfGeode::getClassType() ))
    {
        pfGeode *geode = (pfGeode *)node;

        for( i = 0; i < geode->getNumGSets(); i++ )
        {
            pfGeoSet *gset      = (pfGeoSet *)geode->getGSet( i );
            pfGeoState  *gstate = gset->getGState();
            pfTexture   *tex    = (pfTexture *)gstate->getAttr( PFSTATE_TEXTURE );

            if( tex != (pfTexture *)0L )
            {
                int found = 0;

                for( int i = 0; i < tlist->getNum(); i++ )
                {
                    pfTexture *ttex = (pfTexture *)tlist->get( i );
                    if( tex == ttex || !strcmp( tex->getName(), ttex->getName() ) )
                    {
                        found = 1;
                        break;
                    }
                }
                if( !found )
                    tlist->add( tex );
            }
        }
    }
}

void pfuTextureManager::pfuTexList::init( void )
{
    if( classType == 0 )
    {
        pfObject::init();

        classType = new pfType( pfObject::getClassType(), "pfuTexList" );
    }
}

pfuTextureManager::pfuTexList *pfuTextureManager::addTextureList(  pfNode *node, const char *name, pfTexture *dflt )
{
    pfuTexList *ret    = (pfuTexList *)0L;
    pfList *texList = (pfList *)0L;

    texList = new pfList;
   // pfuNodeTraverse( node, PFT_GSETS, assembleTextureList, (void *)texList ); 

    traverseAndAssembleTextureList( node, texList );

    if( texList->getNum() > 0 )
        ret =  addTextureList( texList, name, dflt );

    delete texList;

    return ret;
}

pfuTextureManager::pfuTexList *pfuTextureManager::addTextureList(  pfList *tlist, const char *name )
{
    return addTextureList( tlist, name, (pfTexture *)0L );
}

pfuTextureManager::pfuTexList *pfuTextureManager::addTextureList(  pfNode *node, const char *name )
{
    return addTextureList( node, name, 0L );
}


void pfuTextureManager::deleteTextureList( pfuTexList *texList )
{
    if( texList == (pfuTexList *)0L ) 
    {
        pfNotify( PFNFY_WARN, PFNFY_WARN,
                        "pfuTextureManager::deleteTextureList() :"
                        "Texture list is NULL.\n");
        return;
    }

    int n            = texList->getNum();
    pfuTex **texArray   = texList->getArray();
    int i;

    for( i = 0; i < n; i++ )
    {
        pfuTex *tex = texArray[i];
        tex->decRef();
    }

    do 
    {
        n = masterList->getNum();
        texArray = masterList->getArray();

        for( i = 0; i < n; i++ )
        {
            pfuTex *tex = texArray[i];

            if( tex->getRefCnt() == 0 )
            {
                masterList->remove( tex );
                delete tex;
                break;
            }
        }
    }while( i < n );

    texLists->remove( texList );

    delete texList;
    texList = 0L;
}


void pfuTextureManager::load( pfuTexList *tlist )
{
    if( tlist == (pfuTexList *)0L )
    {
        pfNotify( PFNFY_DEBUG, PFNFY_WARN,
                        "pfuTextureManager::load() :"
                        "Texture list is NULL.\n");
        return;
    }

    if( tlist->getNum() == 0 ) 
    {
        pfNotify( PFNFY_DEBUG, PFNFY_WARN,
                        "pfuTextureManager::load() :"
                        "Texture list is of 0 length.\n");
        return;                            
    }

    if( texLists->search( tlist ) == -1 )
    {
        pfNotify( PFNFY_WARN, PFNFY_WARN,
                        "pfuTextureManager::load() :"
                        "Texture list is not being managed by this Texture Manager.\n");
        return;                            
    }

    tlist->load();
}

float pfuTextureManager::load( pfuTexList *tlist, float deadline )
{
    if( tlist == (pfuTexList *)0L ) 
    {
        pfNotify( PFNFY_DEBUG, PFNFY_WARN,
                        "pfuTextureManager::load() :"
                        "Texture list is NULL.\n");
        return 0.0f;
    }

    if( tlist->getNum() == 0 ) 
    {
        pfNotify( PFNFY_DEBUG, PFNFY_WARN,
                        "pfuTextureManager::load() :"
                        "Texture list is of 0 length.\n");
        return 0.0f;                            
    }
    if( texLists->search( tlist ) == -1 )
    {
        pfNotify( PFNFY_WARN, PFNFY_WARN,
                        "pfuTextureManager::load() :"
                        "Texture list is not being managed by this Texture Manager.\n");
        return 0.0f ;                            
    }

    return tlist->load( deadline );
}

float pfuTextureManager::load( pfuTexList *tlist, int bytes )
{
    if( tlist == (pfuTexList *)0L ) 
    {
        pfNotify( PFNFY_DEBUG, PFNFY_WARN,
                        "pfuTextureManager::load() :"
                        "Texture list is NULL.\n");
        return 0.0f;
    }

    if( tlist->getNum() == 0 ) 
    {
        pfNotify( PFNFY_DEBUG, PFNFY_WARN,
                        "pfuTextureManager::load() :"
                        "Texture list is of 0 length.\n");
        return 0.0f;                            
    }
    if( texLists->search( tlist ) == -1 )
    {
        pfNotify( PFNFY_WARN, PFNFY_WARN,
                        "pfuTextureManager::load() :"
                        "Texture list is not being managed by this Texture Manager.\n");
        return 0.0f ;                            
    }

    return tlist->load( bytes );
}

void pfuTextureManager::unload( pfuTexList *tlist )
{
    if( tlist == (pfuTexList *)0L ) 
    {
        pfNotify( PFNFY_DEBUG, PFNFY_WARN,
                        "pfuTextureManager::unload() :"
                        "Texture list is NULL.\n");
        return;
    }

    if( tlist->getNum() == 0 ) 
    {
        pfNotify( PFNFY_DEBUG, PFNFY_WARN,
                        "pfuTextureManager::unload() :"
                        "Texture list is of 0 length.\n");
        return;                            
    }

    if( texLists->search( tlist ) == -1 )
    {
        pfNotify( PFNFY_WARN, PFNFY_WARN,
                        "pfuTextureManager::unload() :"
                        "Texture list is not being managed by this Texture Manager.\n");
        return;                            
    }

    tlist->unload();

}

void pfuTextureManager::loadUnload( pfuTexList *loadList, pfuTexList *unloadList )
{
    if( loadList == (pfuTexList *)0L && unloadList == (pfuTexList *)0L ) 
    {
        pfNotify( PFNFY_DEBUG, PFNFY_WARN,
                        "pfuTextureManager::loadUnload() :"
                        "Texture lists are NULL.\n");
        return;
    }

    if( loadList == (pfuTexList *)0L || loadList->getNum() == 0) 
    {
        unload( unloadList );
        return;                            
    }

    if( unloadList == (pfuTexList *)0L || unloadList->getNum() == 0 )
    {
        load( loadList );
        return;
    }

    if( texLists->search( loadList ) == -1 || texLists->search( unloadList ) == -1 )
    {
        pfNotify( PFNFY_WARN, PFNFY_WARN,
                        "pfuTextureManager::loadUnload() :"
                        "Texture lists are not both being managed by this Texture Manager.\n");
        return;                            
    }

    load( loadList );
    unload( unloadList );
}


void pfuTextureManager::Print( int level )
{
    int i;
    float totalSize = 0;

    switch( level )
    {
        case 1 :
            printf( "%20s%15s%15s\n", "Name", "Size", "Loaded" );
            printf( "%20s%15s%15s\n", "----", "----", "------" );
            for( i = 0; i < texLists->getNum(); i++ )
            {
                pfuTexList *tl = (pfuTexList *)texLists->get( i );
                tl->Print( level );
            } 
            
            break;

        case 2 :
            printf( "%20s%8s%8s%8s%8s%10s\n", "TextureName", "Size", "Ref", "Load", "Loaded", "State" );
            printf( "%20s%8s%8s%8s%8s%10s\n", "-----------", "----", "---", "----", "------", "-----" );
            masterList->Print( level );
            break;

        case 3 :
            printf( "%20s%20s%8s%8s%8s%8s%10s\n", 
                    "List Name", "TextureName", "Size", "Ref", "Load", "Loaded", "State" );
            printf( "%20s%20s%8s%8s%8s%8s%10s\n", 
                    "---------", "-----------", "----", "---", "----", "------", "-----" );
            for( i = 0; i < texLists->getNum(); i++ )
            {
                pfuTexList *tl = (pfuTexList *)texLists->get( i );
                tl->Print( level );
            } 

            break;

    }

    totalSize = (float)getTextureMemoryUsed();

    printf( "\nTexture memory : %8.3f MB of %8.3f MB filled = %8.3f %%\n", 
                    (float)totalSize/(1024.0 * 1024.0), 
                    (float)tmSize/(1024.0 * 1024.0),
                    tmSize > 0 ? 100.0f * (float)totalSize/(float)tmSize : 0.0f );
}


int pfuTextureManager::getTextureMemorySize( void )
{
    return tmSize;
}

int pfuTextureManager::getTextureMemoryUsed( void )
{
    return (int)(masterList->isLoaded() * (float)masterList->getSizeInBytes());
}

int pfuTextureManager::getTextureMemoryAvailable( void )
{
    return tmSize - getTextureMemoryUsed();
}

float pfuTextureManager::getTextureMemoryUsedPercent( void )
{
    if( tmSize <= 0 )
        return 0.0f;
    else
        return 100.0f * (float)getTextureMemoryUsed()/(float)tmSize;
}


//////////////////////////////  pfuTexList definitions ///////////////////


pfuTextureManager::pfuTexList::pfuTexList( const char *nm )
{
    tlist       = new pfList;
    sizeInBytes = 0;
    name         = (char *)nm;
    state        = TEXLIST_STATE_UNKNOWN;

    defaults[0]  = (pfTexture *)0L;        // 1 component default texture
    defaults[1]  = (pfTexture *)0L;        // 2 component default texture
    defaults[2]  = (pfTexture *)0L;        // 3 component default texture
    defaults[3]  = (pfTexture *)0L;        // 4 component default texture
}


pfuTextureManager::pfuTexList::~pfuTexList( void )
{
}


int pfuTextureManager::pfuTexList::addTex( pfuTex *tex )
{
    if( !isInList( tex ) )
    {
        tlist->add( tex );
        sizeInBytes += tex->getSizeInBytes();
        return 1;
    }

    return 0;
}

void pfuTextureManager::pfuTexList::remove( pfuTextureManager::pfuTex *tex )
{
    int s = tex->getSizeInBytes();

    if( tlist->remove( tex ) != -1 )
        sizeInBytes -= s;
}

pfuTextureManager::pfuTex * pfuTextureManager::pfuTexList::isInList( pfuTextureManager::pfuTex *tex )
{
    int n = tlist->getNum();
    pfuTex **tarray = (pfuTex **)tlist->getArray();

    for( int i = 0; i < n; i++ )
        if( !strcmp( tex->getName(), tarray[i]->getName() ) )
            return tarray[i];

    return (pfuTex *)0L;
}

pfuTextureManager::pfuTex *pfuTextureManager::pfuTexList::isInList( pfTexture *tex )
{
    int n = tlist->getNum();
    pfuTex **tarray = (pfuTex **)tlist->getArray();

    for( int i = 0; i < n; i++ )
        if( !strcmp( tex->getName(), tarray[i]->getName() ))
            return tarray[i];

    return (pfuTex *)0L;
}

void pfuTextureManager::pfuTexList::load( void )
{
    int n = tlist->getNum();
    pfuTex **tarray = (pfuTex **)tlist->getArray();
    int i;

    for( i = 0; i < n; i++ )
    {
        tarray[i]->load();
    }

    state = TEXLIST_STATE_LOADED;
}

float pfuTextureManager::pfuTexList::load( float deadline )
{
    int n = tlist->getNum();
    pfuTex **tarray = (pfuTex **)tlist->getArray();
    int i;

    if( isLoaded() == 1.0 )
    {
        state = TEXLIST_STATE_LOADED;
        return 1.0;
    }


    if( state != TEXLIST_STATE_LOADING )
    {
        state = TEXLIST_STATE_LOADING;
        nextToLoad = 0;
    }

    for( i = nextToLoad; i < n; i++ )
    {
        tarray[i]->load();

        nextToLoad = i + 1;

        if( pfGetTime() >= deadline )
            break;
    }

    if( nextToLoad >= n )
    {
        state = TEXLIST_STATE_LOADED;
    }

    return isLoaded();
}

float pfuTextureManager::pfuTexList::load( int bytes )
{
    int n = tlist->getNum();
    pfuTex **tarray = (pfuTex **)tlist->getArray();
    int bytesLeft = bytes;

    state = TEXLIST_STATE_LOADED;

    if( isLoaded() == 1.0 )
    {
        return 1.0;
    }

    for( int i = 0; i < n; i++ )
    {
	if (tarray[i]->isLoaded() < 1.0f) {
	    int ret = tarray[i]->load(bytesLeft);
	    if (tarray[i]->isLoaded() < 1.0f)
		state = TEXLIST_STATE_LOADING;
	    bytesLeft -= ret;
	    if (bytesLeft <= 0)
		break;
	}
    }

    return isLoaded();
}

void pfuTextureManager::pfuTexList::unload( void )
{
    int n = tlist->getNum();
    pfuTex **tarray = (pfuTex **)tlist->getArray();
    int i;

    for( i = 0; i < n; i++ )
    {
        tarray[i]->idle();
    }

    state = TEXLIST_STATE_IDLE;
}

float pfuTextureManager::pfuTexList::isLoaded( void )
{
    int n = tlist->getNum();
    pfuTex **tarray = (pfuTex **)tlist->getArray();
    int         i;
    int         totalResident;

    if( sizeInBytes == 0 || n == 0 )
        return 0.0f;

    totalResident = 0;
    for( i = 0; i < n; i++ )
    {
        if( tarray[i]->isLoaded() == 1 )
            totalResident += tarray[i]->getSizeInBytes();
    }

    return (float)totalResident/(float)sizeInBytes;
}

void pfuTextureManager::pfuTexList::Print( int level )
{
    int i;

    switch( level )
    {
        case 1 :
            printf( "\n%20s%15d%15.4f\n", basename( (char *)name ), sizeInBytes, isLoaded() );    
            break;

        case 2 :
            for( i = 0; i < tlist->getNum(); i++ )
            {
                pfuTex *tex = (pfuTex *)tlist->get( i );

                tex->Print( level );
            }
            break;

        case 3 :
            printf( "\n%20s\n", name );
            for( i = 0; i < tlist->getNum(); i++ )
            {
                pfuTex *tex = (pfuTex *)tlist->get( i );

                tex->Print( level );
            }

            printf( "\n%20s%20s%8d%8s%8s%8.4f\n", " ", "Total -", sizeInBytes, " ", " ", isLoaded() );
            break;
    }
}


/////////////////// pfTextureManager::pfuTex definitions /////////////////////////////

struct _texInfo {
    int      internalFormat;
    int      externalFormat;
    uint     *image;
    int      levels;
    uint     **mipmaps;
    int      comp, ns, nt, nr;
    uint     glhandle;
};

static inline int bytesInFormat( int format )
{
    switch( format )
    {
          case PFTEX_RGB_5  :        //      16-bit texels.
                return 2;

          case PFTEX_RGB_4  :        // 16-bit texels.
                return 2;

          case PFTEX_IA_8   :        // 16-bit texels.
                return 2;

          case PFTEX_RGB5_A1:        // 16-bit texels.
                return 2;

          case PFTEX_RGBA_4 :        // 16-bit texels.
                return 2;

          case PFTEX_I_12A_4:        // 16-bit texels.
                return 2;

          case PFTEX_I_8    :        // 16-bit texels.
                return 2;

          case PFTEX_I_16   :        // 16-bit texels.
                return 2;

          case PFTEX_IA_12  :        // 24-bit texels.
                return 3;

          case PFTEX_RGBA_8 :        // 32-bit texels.
                return 4;

          case PFTEX_RGB_12 :        // 48-bit texels.
                return 6;

          case PFTEX_RGBA_12:        // 48-bit texels.
                return 6;

    }
    pfNotify( PFNFY_WARN, PFNFY_WARN, "pfuTextureManager bytesInFormat "
	      "unknown format 0x%x\n", format);
    fprintf( stderr, "XXX - pfuTextureManager bytesInFormat "
	      "unknown format 0x%x\n", format);
    return 0;
}


static inline int isMipMapped( int filter )
{
    switch ( filter )
    {
	// only true on systems where mipmap is default
        case PFTEX_DEFAULT:

        case PFTEX_MIPMAP:
        case PFTEX_MIPMAP_LINEAR:
        case PFTEX_MIPMAP_BILINEAR:
        case PFTEX_MIPMAP_TRILINEAR:
            return 1;
        default:
            return 0;
    }
}

static inline int mipmapsize( int ns, int nt, int nr )
{
    int n = 0;

    while( (ns > 1) || (nt > 1) || (nr > 1))
    {
	if (ns > 1)
	    ns >>= 1;
	if (nt > 1)
	    nt >>= 1;
	if (nr > 1)
	    nr >>= 1;

        n += ns * nt * nr;
    }
    return n;
}

static inline int numlevels( int ns, int nt, int nr )
{
    int nn;
    if ((ns >= nt) && (ns >= nr))
	nn = ns;
    else if ((nt >= ns) && (nt >= nr))
	nn = nt;
    else
	nn = nr;

    int n = 0;
    while (nn) {
	nn >>= 1;
	n++;
    }
	
//     return (int) log((double)n) / M_LN2;
    return n;
}

static inline int topow2( int x )
{
    if (x <= 0)
	return 0;

    int n = -1;
    while (x) {
	x >>= 1;
	n++;
    }
	
    return 1 << n;
}

void pfuTextureManager::pfuTex::init( void )
{
    if( classType == 0 )
    {
        pfObject::init();

        classType = new pfType( pfObject::getClassType(), "pfuTex" );
    }
}

// #define GENERATING_MIPS

// need to add "LLDLIBS += -lgutil" to Makefile
#ifdef GENERATING_MIPS
// izoom stuff
extern "C" {
    extern void filterzoom(int (*getfunc)(short *buf, int y),
			   int (*putfunc)(short *buf, int y),
			   int anx, int any,
			   int bnx, int bny, int filttype, float blur);
}

#define IMPULSE		1
#define BOX		2
#define TRIANGLE	3
#define QUADRATIC	4
#define MITCHELL	5
#define GAUSSIAN	6
#define LANCZOS2	7
#define LANCZOS3	8

static short *srcbuf = NULL;
static int srcx = 0;
static short *dstbuf = NULL;
static int dstx = 0;
static int getimgrow(short *buf, int y)
{
    bcopy(&srcbuf[y*srcx], buf, srcx);
    return 0;
}

static int putimgrow(short *buf, int y)
{
    bcopy(buf, &dstbuf[y*dstx], dstx);
    return 0;
}
#endif // GENERATING_MIPS

pfuTextureManager::pfuTex::pfuTex( pfTexture *t )
{
    pftexture = t;
    uint *image;
    int comp, ns, nt, nr;

    t->getImage( &image, &comp, &ns, &nt, &nr );

    primarytex = (texInfo *)0L;
    idletex    = (texInfo *)0L;
    
    primarytex = (texInfo *)pfMalloc( sizeof( texInfo ), pfGetSharedArena() );

    primarytex->internalFormat = t->getFormat( PFTEX_INTERNAL_FORMAT );
    primarytex->externalFormat = t->getFormat( PFTEX_EXTERNAL_FORMAT );
    primarytex->image = image;
    primarytex->comp = comp;
    primarytex->ns = ns;
    primarytex->nt = nt;
    primarytex->nr = nr;

    int bif = bytesInFormat( t->getFormat( PFTEX_INTERNAL_FORMAT ) );
    sizeInBytes = ns * nt * nr * bif;

    if (pftexture->isOfType(pfClipTexture::getClassType())) {
	pfClipTexture *ct = (pfClipTexture *)pftexture;
	sizeInBytes *= ct->getNumEffectiveLevels();
    }

    if( isMipMapped( t->getFilter( PFTEX_MINFILTER ) ) ) {
	int levels = numlevels(ns, nt, nr);

	primarytex->levels = levels;
	primarytex->mipmaps = (uint **)
	    pfMalloc( sizeof( uint *) * levels+1 );
	primarytex->mipmaps[0] = image;

        sizeInBytes += mipmapsize( ns, nt, nr ) * bif;

#if 0
	fprintf(stderr, ">> generating mip maps for texture %s\n",
		basename( (char *)t->getName()));
	fprintf(stderr, "   external format 0x%x, image format 0x%x, "
		"internal format 0x%x\n",
		primarytex->externalFormat,
		t->getFormat( PFTEX_IMAGE_FORMAT ),
		primarytex->internalFormat);
	fprintf(stderr, "   %i levels, %ix%ix%i - %i comp, %i bif\n",
		levels, ns, nt, nr, comp, bif);
	
	for (int ii = 0; ii < 16; ii++) {
// 	    if ((ii % bif) == 0)
// 		fprintf(stderr, " -");
	    fprintf(stderr, " %0.2hx", ((char *)image)[ii]);
	}
	fprintf(stderr, "\n");
#endif
	
	// generate the mipmaps now (once)
	pfdImage *to_mip = (pfdImage *)pfMalloc(sizeof(pfdImage),
						pfGetSharedArena());
	// XXX - pfdImage isn't safe
	bzero(to_mip, sizeof(pfdImage));

	pfdImageFromPfTexture(to_mip, t);

	pfdImageGenMipmaps(to_mip, NULL, NULL, NULL);

	for( int i = 1; i < levels; i++ ) {
	    int nns = ns >> i;
	    int nnt = nt >> i;
	    int nnr = nr >> i;
	    if (nns <= 0)
		nns = 1;
	    if (nnt <= 0)
		nnt = 1;
	    if (nnr <= 0)
		nnr = 1;

	    // make mipmap level i
	    pfTexture *miptex = new pfTexture;
	    miptex->setFormat( PFTEX_INTERNAL_FORMAT,
			       primarytex->internalFormat );
	    miptex->setFormat( PFTEX_EXTERNAL_FORMAT,
			       primarytex->externalFormat );

	    pfdImageToPfTexture( &to_mip->mipmaps[i-1], miptex );

	    uint *mipimage;

#if 0
	    for( int y = 0; y < nnt; y++ )
		for( int x = 0; x < nns; x++ ) {
		    // 0xF801 - red in 5551 ?
		    ((unsigned char *)mipimage)
			[y*nns+x*bif] = 0xff;
		    ((unsigned char *)mipimage)
			[y*nns+x*bif+1] = 0xaa;
		    ((unsigned char *)mipimage)
			[y*nns+x*bif+2] = 0xbb;
		}
#endif

#if 0
	    fprintf(stderr, ">> level %i [%i - %i * %i]\n", i,
		    PF_MIN2(16, nns * bif), nns, bif );
	    fprintf(stderr, "?? pfd says %ix%ix%i\n",
		    to_mip->mipmaps[i-1].xsize, to_mip->mipmaps[i-1].ysize,
		    to_mip->mipmaps[i-1].zsize);

	    for (int ii = 0; ii < PF_MIN2(16, nns * bif); ii++)
		fprintf(stderr, " %0.2hx", ((char *)mipimage)[ii]);
	    fprintf(stderr, "\n");
#endif

	    miptex->getImage( &mipimage, &comp, &nns, &nnt, &nnr );
	    miptex->setImage( NULL, comp, nns, nnt, nnr );

	    primarytex->mipmaps[i] = mipimage;
	    t->setLevel( i, miptex );

	} // for i

	pfdDelImage(to_mip);

	// set up for subloading - so don't automatically generate
	// mipmaps (causes frames to be dropped since done on
	// texture download)
	t->setFormat(PFTEX_SUBLOAD_FORMAT, TRUE);
	t->setFormat(PFTEX_GEN_MIPMAP_FORMAT, FALSE);
	t->setImage( NULL, comp, ns, nt, nr );
    } // if mipmapped

    refcnt  = 0;
    loadcnt = 0;

    state = TEX_STATE_UNKNOWN;
    loadedBytes = 0;
}


pfuTextureManager::pfuTex::pfuTex( pfTexture *t, pfTexture *it )
{
    pftexture = t;
    uint *image;
    int comp, ns, nt, nr;

    t->getImage( &image, &comp, &ns, &nt, &nr );

    primarytex = (texInfo *)0L;
    idletex    = (texInfo *)0L;
    
    primarytex = (texInfo *)pfMalloc( sizeof( texInfo ), pfGetSharedArena() );

    primarytex->internalFormat = t->getFormat( PFTEX_INTERNAL_FORMAT );
    primarytex->externalFormat = t->getFormat( PFTEX_EXTERNAL_FORMAT );
    primarytex->image = image;
    primarytex->comp = comp;
    primarytex->ns = ns;
    primarytex->nt = nt;
    primarytex->nr = nr;

    int bif = bytesInFormat( primarytex->internalFormat );
    sizeInBytes = ns * nt * nr * bif;

    if (pftexture->isOfType(pfClipTexture::getClassType())) {
	pfClipTexture *ct = (pfClipTexture *)pftexture;
	sizeInBytes *= ct->getNumEffectiveLevels();
    }

    if( isMipMapped( t->getFilter( PFTEX_MINFILTER ) ) )
        sizeInBytes += mipmapsize( ns, nt, nr ) * bif;

    refcnt  = 0;
    loadcnt = 0;

    setIdleTexture( it );

    pftexture->setFormat( PFTEX_INTERNAL_FORMAT, idletex->internalFormat );
    pftexture->setFormat( PFTEX_EXTERNAL_FORMAT, idletex->externalFormat );
    pftexture->setImage(
                    idletex->image, 
                    idletex->comp, 
                    idletex->ns, 
                    idletex->nt, 
                    idletex->nr  );
    state = TEX_STATE_IDLE;
}

pfuTextureManager::pfuTex::~pfuTex( void )
{
}


void pfuTextureManager::pfuTex::setIdleTexture( pfTexture *t )
{
    uint *image;
    int comp, ns, nt, nr;

    t->getImage( &image, &comp, &ns, &nt, &nr );

    if( idletex != (texInfo *)0L )
        delete idletex;

    idletex = (texInfo *)pfMalloc( sizeof( texInfo ), pfGetSharedArena() );

    idletex->internalFormat = t->getFormat( PFTEX_INTERNAL_FORMAT );
    idletex->externalFormat = t->getFormat( PFTEX_EXTERNAL_FORMAT );
    idletex->image = image;
    idletex->comp = comp;
    idletex->ns = ns;
    idletex->nt = nt;
    idletex->nr = nr;
}

void pfuTextureManager::pfuTex::incRef()
{
    refcnt++;
}

void pfuTextureManager::pfuTex::decRef()
{
    if( refcnt > 0 )
        refcnt--;
}

void pfuTextureManager::pfuTex::load( void )
{
    loadcnt++;
    if( loadcnt == 1 )
    {
        state = TEX_STATE_LOADING;

	// XXX - HACK pfClipTexture creates GL handle if none exists
	// this functionality will later have its own method
	int handle = pftexture->getGLHandle();

	if (!handle)
	    pfNotify( PFNFY_WARN, PFNFY_WARN, "pfuTextureManager::pfTex::load "
		      "getGLhandle returned 0x%x\n", handle);

        pftexture->setFormat( PFTEX_INTERNAL_FORMAT, primarytex->internalFormat );
        pftexture->setFormat( PFTEX_EXTERNAL_FORMAT, primarytex->externalFormat );
        pftexture->setImage(
                    primarytex->image, 
                    primarytex->comp, 
                    primarytex->ns, 
                    primarytex->nt, 
                    primarytex->nr  );

	if (pftexture->isOfType(pfClipTexture::getClassType())) {
	    pfClipTexture *ct = (pfClipTexture *)pftexture;
	    ct->invalidate();
	}

        pftexture->load();
        state = TEX_STATE_LOADED;
	loadedBytes = sizeInBytes;
    }
}

int pfuTextureManager::pfuTex::load( int bytes )
{
    if (pftexture->isOfType(pfClipTexture::getClassType())) {
	load();
	return 0;
    }

    if (state != TEX_STATE_LOADING) {
	loadcnt++;
	if( loadcnt == 1 )
	{
	    state = TEX_STATE_LOADING;
	    loadedBytes = 0;

	    // XXX - HACK pfClipTexture creates GL handle if none exists
	    // this functionality will later have its own method
	    int handle = pftexture->getGLHandle();

	    if (!handle)
		pfNotify( PFNFY_WARN, PFNFY_WARN,
			  "pfuTextureManager::pfTex::load "
			  "getGLhandle returned 0x%x\n", handle);

	    pftexture->setFormat( PFTEX_INTERNAL_FORMAT,
				  primarytex->internalFormat );
	    pftexture->setFormat( PFTEX_EXTERNAL_FORMAT,
				  primarytex->externalFormat );

	pftexture->setFormat(PFTEX_SUBLOAD_FORMAT, TRUE);
	pftexture->setFormat(PFTEX_GEN_MIPMAP_FORMAT, FALSE);
// 	pftexture->setFormat(PFTEX_INTERNAL_FORMAT, PFTEX_RGB5_A1);
// 	pftexture->setFormat(PFTEX_IMAGE_FORMAT, PFTEX_RGBA);
// 	pftexture->setFormat(PFTEX_EXTERNAL_FORMAT,
// 		     PFTEX_UNSIGNED_SHORT_5_5_5_1);

	    pftexture->setImage(
// 				primarytex->image,
				NULL,
				primarytex->comp, 
				primarytex->ns,
				primarytex->nt,
				primarytex->nr );
// 	    if (isMipMapped( pftexture->getFilter( PFTEX_MINFILTER ) )) {
// 		int levels =
// 		    numlevels(primarytex->ns, primarytex->nt, primarytex->nr);
// 		for( int i = 0; i < levels; i++ )
// 		    pftexture->setLevel(i, primarytex->mipmaps[i]);
// 	    }
	    pftexture->apply();
	}
	else
	    if ( (loadcnt == 2) && (! isLoaded() ) )
		fprintf( stderr, "*** texture %s didn't load correctly\n",
			 basename( (char *)pftexture->getName() ) );
    }

    if ( state == TEX_STATE_LOADING )
    {
	int bytesLeft = bytes;
	int bytesSoFar = 0;
	int tmpBytes = 0;
	
	pfTexture *t = pftexture;
	uint *image;
	int comp, ns, nt, nr;
	t->getImage( &image, &comp, &ns, &nt, &nr );

// 	fprintf(stderr, "%s 0x%x %i\n",
// 		basename( (char *)t->getName()),
// 		t->getFilter( PFTEX_MINFILTER ),
// 		isMipMapped( t->getFilter( PFTEX_MINFILTER ) ) );

	int levels;
	if (! isMipMapped( t->getFilter( PFTEX_MINFILTER ) ))
	    levels = 1;
	else
	    levels = numlevels(ns, nt, nr);
	for( int i = 0; i < levels; i++ ) {
	    uint *mipimage;
	    mipimage = primarytex->mipmaps[i];
	    int nns = ns >> i;
	    int nnt = nt >> i;
	    int nnr = nr >> i;
	    if (nns <= 0)
		nns = 1;
	    if (nnt <= 0)
		nnt = 1;
	    if (nnr <= 0)
		nnr = 1;
// 	    if (miptex)
// 		miptex->getImage( &mipimage, &comp, &nns, &nnt, &nnr );
// 	    else {
// // 		pfNotify( PFNFY_WARN, PFNFY_WARN,
// 		fprintf(stderr,
// 			"pfuTextureManager::pfuTex::Load() :"
// 			"%s mip level %i/%i is NULL.\n",
// 			basename( (char *)t->getName()), i+1, levels);
// 		return bytesSoFar;
// 	    }
	    int bif = bytesInFormat( t->getFormat( PFTEX_INTERNAL_FORMAT ) );
	    int bytesInLevel = nns * nnt * nnr * bif;
	    // if not done loading this level yet, try to load some
	    if (tmpBytes + bytesInLevel > loadedBytes) {
		// if haven't loaded anything this level and
		// can do whole level...
		int bytesAlreadyThisLevel = loadedBytes - tmpBytes;
		int linesdone = bytesAlreadyThisLevel / (nns * bif);
		int numlines = bytesLeft / (nns * bif);
		if (linesdone + numlines > nnt)
		    numlines = nnt - linesdone;
#if 0
		fprintf(stderr,
			">> %s [%i/%i] - %ix%i - %i (%i lines) so far, "
			"%i allowed - 0x%x - (%i) %i\n",
			basename( (char *)t->getName()), i+1, levels,
			nns, nnt,
			bytesAlreadyThisLevel, linesdone,
			bytesLeft, mipimage, numlines,
			topow2(numlines)
			);
#endif

		numlines = topow2(numlines);

		if (mipimage) {
#if 0
		    for (int ii = 0; ii < PF_MIN2(16, nns * bif); ii++)
			fprintf(stderr, " %0.2hx", ((char *)mipimage)[ii]);
		    fprintf(stderr, "\n");
#endif
		    t->subloadLevel(PFTEX_SOURCE_IMAGE, // int _source
				    mipimage, // uint *image
				    0, // int _xsrc
				    linesdone, // inc _ysrc
				    -1, // srcwid
				    0, // int _xdst
				    linesdone, // int _ydst
				    nns, // int xsize
				    numlines, // int ysize
				    i // int level
				    );
		}

		bytesLeft -= numlines * nns * bif;
		bytesSoFar += numlines * nns * bif;
		loadedBytes += numlines * nns * bif;

		if ((numlines + linesdone < nnt) || (bytesLeft <= 0))
		    break;
	    }
	    tmpBytes += bytesInLevel;
	} // for i
	    
	if (loadedBytes == sizeInBytes)
	    state = TEX_STATE_LOADED;

	return bytesSoFar;
    }

    // texture doesn't need to be paged any
    // but load was still called to increment loadcnt
    return 0;
}

void pfuTextureManager::pfuTex::idle( void )
{
    if( loadcnt > 0 )
        loadcnt--;

    if( loadcnt == 0 )
    {
        state = TEX_STATE_UNLOADING;
        if( pftexture->isLoaded() )
            pftexture->idle();
        pftexture->deleteGLHandle();

        if( idletex != (texInfo *)0L )
        {
            pftexture->setFormat( PFTEX_INTERNAL_FORMAT, idletex->internalFormat );
            pftexture->setFormat( PFTEX_EXTERNAL_FORMAT, idletex->externalFormat );
            pftexture->setImage(
                    idletex->image, 
                    idletex->comp, 
                    idletex->ns, 
                    idletex->nt, 
                    idletex->nr  );

        }
        state = TEX_STATE_IDLE;
    }
}


int pfuTextureManager::pfuTex::shouldBeLoaded()
{
    if( loadcnt > 0 ) 
        return 1;
    else 
        return 0;
}


int pfuTextureManager::pfuTex::isLoaded()
{
    // return 0 if not loaded or state == TEX_STATE_LOADING, 
    // -1 if loaded and state == TEX_STATE_IDLE
    // and 1 if loaded but state != TEX_STATE_IDLE

    if (pftexture->isOfType(pfClipTexture::getClassType()) &&
	(state == TEX_STATE_LOADED))
	return 1;
    if (state == TEX_STATE_LOADING)
	return 0;

    // for now ignore what the texture says...
//     if (state == TEX_STATE_LOADED)
// 	return 1;
//     else if (state == TEX_STATE_IDLE)
// 	return 0;

    return (pftexture->isLoaded() ? state == TEX_STATE_IDLE ? -1 : 1 : 0);
}


void pfuTextureManager::pfuTex::Print( int level )
{
    static char *states[] = {
        "UNKNOWN",
        "IDLE",
        "LOADING",
        "LOADED",
        "UNLOADING",
    };

    switch( level )
    {
        case 1 :
            break;

        case 2 :
            printf( "%20s%8d%8d%8d%8d%10s\n",  basename( (char *)pftexture->getName()), 
                                sizeInBytes, refcnt, loadcnt, isLoaded(),
                                states[state] );    
            break;

        case 3 :
            printf( "%20s%20s%8d%8d%8d%8d%10s\n", " ", basename( (char *)pftexture->getName()), 
                                sizeInBytes, refcnt, loadcnt, isLoaded(),
                                states[state] );
            break;
    } 
}


/*
 * C bindings
 */

PFUDLLEXPORT void 
pfuInitTextureManagerClass(void)
{
    if( pfIsConfiged() )
        pfNotify(PFNFY_DEBUG, PFNFY_USAGE,
        "pfuTextureManagerClasse) should be called before pfConfig()"
        "for multiprocessed operation");
    pfuTextureManager::init();
}

