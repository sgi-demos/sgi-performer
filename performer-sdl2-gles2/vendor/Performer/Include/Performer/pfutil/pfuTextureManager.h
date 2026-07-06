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

#ifndef __PFU_TEX_MANAGER_H
#define __PFU_TEX_MANAGER_H

#ifdef __cplusplus

#include <Performer/pf.h>
#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfMemory.h>
#include <Performer/pr/pfTexture.h>

#define TEXLIST_STATE_UNKNOWN	0
#define TEXLIST_STATE_LOADING	1
#define TEXLIST_STATE_LOADED	2
#define TEXLIST_STATE_UNLOADING	3
#define TEXLIST_STATE_IDLE		4


#define TEX_STATE_UNKNOWN	0
#define TEX_STATE_IDLE		1
#define TEX_STATE_LOADING 	2
#define TEX_STATE_LOADED	3
#define TEX_STATE_UNLOADING 4

typedef struct _texInfo texInfo;

class PFUDLLEXPORT pfuTextureManager : public pfObject
{
	public  :
		class pfuTex   : public pfObject
		{

			private :
				int 		sizeInBytes;
				pfTexture 	*pftexture;
				int			refcnt;
				int			loadcnt;
				texInfo		*primarytex;
				texInfo		*idletex;
				int			state;
				int             loadedBytes;

				static pfType *classType;

			public :
				pfuTex( pfTexture *t );
				pfuTex( pfTexture *t, pfTexture *idleTexture );
				~pfuTex( void );

        		static pfType* getClassType( void ) { return classType; }
				static void init( void );

				void setIdleTexture( pfTexture *t );
				int getSizeInBytes( void ) { return sizeInBytes; }
				const char *getName( void ) { return pftexture->getName(); }

				void getImage( uint **i, int *c, int *s, int *t, int *r )
					{ pftexture->getImage( i, c, s, t, r ); }
		
				void incRef( void );
				void decRef( void );
				void load( void );
				int  load( int bytes );
				void idle( void );
				int  shouldBeLoaded( void );
				int  isLoaded(); 
				int  getRefCnt( void ) {return refcnt;}
				int  getLoadCnt( void ) {return loadcnt;}


				void Print( int level );
		};

		class pfuTexList  : public pfObject 
		{
			private :
				char 	*name;
				pfList 	*tlist;
				int 	sizeInBytes;
				int		state;
				int		nextToLoad;

				static pfType *classType;

			public :

				pfTexture *defaults[4];

			public:
				pfuTexList( const char *name );
				~pfuTexList( void );

        		static pfType* getClassType( void ) { return classType; }
				static void init( void );

				int	    addTex( pfuTex * );
				pfuTex *	isInList( pfTexture *);
				pfuTex *	isInList( pfuTex *);
				int		getNum( void )   { return tlist->getNum(); }
				pfuTex     *get( int index ) { return (pfuTex *)tlist->get( index ); }
				pfuTex     **getArray( void ) { return (pfuTex **)tlist->getArray(); }
				void    remove( pfuTex *tex ); 
				const char    *getName( void ) { return name; }
				void	load( void );
				float	load( float deadline );
				float   load( int bytes );
				void	unload( void );
				float   isLoaded();
				int		getSizeInBytes( void ) { return sizeInBytes; }
				int		getState( void ) { return state; }
				void	Print( int level );

				pfList *getTexList( void ) { return  tlist; }

		};


    private :
        int     tmSize;        // texture memory size
        pfuTexList 	*masterList;   
        pfList 		*masterPfList;   
        pfList  *texLists;

		static pfType *classType;

        void traverseAndAssembleTextureList( pfNode *node, pfList *tlist );

    public :
        pfuTextureManager( void );
        ~pfuTextureManager( void );

        static pfType* getClassType( void ) { return classType; }
		static void init( void );


        int   getTextureMemoryAvailable( void );
        int   getTextureMemoryUsed( void );
        float getTextureMemoryUsedPercent( void );

        pfuTexList *addTextureList(  pfList *texList, const char *name, pfTexture *dflt );
        pfuTexList *addTextureList(  pfNode *node, const char *name, pfTexture *dflt );
        pfuTexList *addTextureList(  pfList *texList, const char *name );
        pfuTexList *addTextureList(  pfNode *node, const char *name );
		void	deleteTextureList( pfuTexList *tlist );
        void    load( pfuTexList *texList );
        float   load( pfuTexList *texList, float deadline );
        float   load( pfuTexList *texList, int bytes );
        void    unload( pfuTexList *texList );
        void    loadUnload( pfuTexList *loadList, pfuTexList *unloadList );
        int     getTextureMemorySize( void );
		float   getTextureMemoryUsage( void );
        int     willFit( pfuTexList *texList ) { return (texList->getSizeInBytes() < tmSize ); }
        float   isListLoaded( pfuTexList *texList ) { return texList->isLoaded(); }
        void    Print( int level );

		pfList *getMasterList( void ) { return masterPfList; }
};


typedef pfuTextureManager::pfuTexList TexList;

#endif


#ifdef __cplusplus
extern "C" {
#endif
extern PFUDLLEXPORT void pfuInitTextureManagerClass(void);
#ifdef __cplusplus
}
#endif

#endif /* __PFU_TEX_MANAGER_H */

