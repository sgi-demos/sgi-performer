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

#ifndef __PFPALLETIZER_H
#define __PFPALLETIZER_H

#ifdef WIN32
#include <Performer/pfutil-DLL.h>
#endif

#define PFUPAL_MODE_MIPMAP  1

#define _PFUPAL_MODEMASK_MIPMAP  1

typedef struct _ttex {
    pfList        *gsetList;        // GeoSets that reference this texture
    pfTexture     *tex;             // pfTexture
    uint          *image;           // Image Data
    int           comp, ns, nt, nr; // dimensions
    int           area;             // total area (for sorting)
    uint          is, it;           // Actual dimensions (including margins)
    int           ix, iy;           // relocation variables (discrete)
    uint           margin;          // margin surrounding texture. Space between adjacent
                                    // texture is 2 * margin
    float         x, y;             // relocation variables (normalized)
    float         xscale, yscale;   // Texture coords scale
    int           placed;
}Ttex;

class PFUDLLEXPORT pfuPalettizer {
    private :
        pfList  *completeList;
        pfList  *paletteList;
        pfList  *binList;
        pfList  *tlistChanged;
        char    *texturePrefix;
        uint    margin;
        uint    maxS, maxT;
	int	mode;
        int     nTotal;
        int     nEligible;
        int     nPalettized;
        uint    background;

        void traverseNodeAndBuildTextureList( pfNode *node );
        void buildTtexList( pfGeoSet *gset );
        void removeOversizedTextures( void );
        void removeIneligibleTextures( void );
        void findBestST( int *s, int *t );
        int  binPack( int s, int t );
        void sortList( void );
        uint findGCD( void );
        pfTexture *buildNewTexture( int s, int t );
        void changeTextureCoordinates( pfTexture *tex );
        void cleanBinList( void );
        inline void place( int ss, int tt, int x, int y, Ttex *t, int *placecnt );

    /////////////////////////////////////////////////////////////////
    //
    // Grid structure.  Originally intended to be a grid of
    // gcd X gcd (gcd = greatest common divisor of all texture
    // s and t dimensions).  When margins were implemented it
    // became apparant that the "grid" needed to be the same
    // S and T dimensions as the palette itself.  
    //
    // Grid structure for simple management of a int array
    //
    class Grid {

        private :
            int *grid;
            int S;
            int T;
    
    
        public :
            Grid( int s, int t );
            ~Grid( void );
    
            void print( FILE * );
            int findNextHole( int *, int * );
            void findWxHat( int x, int y, int *w, int *h );
            void setRect( int x, int y, int width, int height );
            void set( int x, int y );
    };

    /////////////////////////  baseline ///////////////////
    //
    // Baseline structure for simple management of a one
    // dimensional int array.  Keeps track of the "floor"
    // as textures are bin-packed into palette
    //
    ///////////////////////////////////////////////////////
    
    class Baseline {
    
        private :
            int *baseline;    
            int dim;
    
        public :
    
            Baseline( int );
            ~Baseline( void );
    
            void set( int x1, int x2, int y );
            void reset( int y );
            void softReset( int oldy, int y );
            int  get( int x );
            int  lessThan( int y );
    
    };

		Grid	*grid;
		Baseline *baseline;

    public  :
        pfuPalettizer( void );
        ~pfuPalettizer( void );

        int palettize( pfNode *node );
        void setST( uint s, uint t );
        void setMargin( uint margin );
        void setTexturePrefix( char *prefix );
        void setBackground( float r, float g, float b, float a );
	void setMode(int _mode,  int _val);
	int getMode(int _mode);
        pfList *getPaletteList( void );
        pfList *getCompleteList( void );
};

#endif
