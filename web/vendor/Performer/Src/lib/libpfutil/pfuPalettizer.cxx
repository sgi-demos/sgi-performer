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
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <math.h>
#include <malloc.h>

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pfdu.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfGroup.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfTexture.h>

#include "pfuPalettizer.h"

#define TCOORD_EPS 0.01f
#define TCOORD_MIN TCOORD_EPS
#define TCOORD_MAX (1.0f - TCOORD_EPS)

static char *defaultTexturePrefix = "texture";
static uint defaultS    = 512;
static uint defaultT    = 512;
static uint defaultMargin = 0;
static uint defaultBackground     = 0x0;



pfuPalettizer::Grid::Grid( int s, int t )
{
    int i, j;

    S = s;
    T = t;

    grid = new int[S * T];

    for( i = 0; i < T; i++ )
    {
        for( j = 0; j < S; j++ )
        {
            int index = i * S + j;
            grid[index] = 0;
        }
    }

}

pfuPalettizer::Grid::~Grid( void )
{
    delete grid;
}

void pfuPalettizer::Grid::print( FILE *fp )
{
    int i, j;

    for( i = T - 1; i >= 0; i-- )
    {
        for( j = 0; j < S; j++ )
        {
            int index = i * S + j;
            if( grid[index] == 0 )
                fprintf( fp, " 0" );
            else
                fprintf( fp, " 1" );
        } 
        fprintf( fp, "\n" );
    }
}
    

int pfuPalettizer::Grid::findNextHole( int *xx, int *yy )
{
    int x, y;
    int ret = 0;
    int index;

    for( y = 0; y < T; y++ )
    {
        for( x = 0; x < S; x++ )
        {
            index = y * S + x;
            if( grid[index] == 0 )
            {
                ret = 1;
                break;
            }
        }

        if( ret ) break;
    }

    if( ret )
    {
        *xx = x;
        *yy = y;
    }

    return ret;
}

void pfuPalettizer::Grid::findWxHat( int x, int y, int *w, int *h )
{
    int i;
    int index;

    *w = 0;
    *h = 0;

    for( i = y; i < T; i++ )
    {
        index = i * S + x;
        if( grid[index] != 0 )
            break;

        (*h)++;
    }

    for( i = x; i < S; i++ )
    {
        index = y * S + i;
        if( grid[index] != 0 )
            break;

        (*w)++;
    }
}


void pfuPalettizer::Grid::setRect( int x, int y, int width, int height )
{
    int x1, y1, x2, y2;
    int i, j;

    x1 = x;
    y1 = y;

    x2 = x + width;
    y2 = y + height;


    if( x1 > S ) x1 = S;
    if( x2 > S ) x2 = S;
    if( y1 > S ) y1 = T;
    if( y2 > S ) y2 = T;

    for( i = y1; i < y2; i++ )
    {
        for( j = x1; j < x2; j++ )
        {
            int index = i * S + j;
            grid[index] = 1;
        }
    }
}

void pfuPalettizer::Grid::set( int x, int y )
{
    int index = y * S + x;
    grid[index] = 1;
}

/////////////////////// end grid ///////////////////////


pfuPalettizer::Baseline::Baseline( int s )
{
    baseline = new int[s];
    dim = s;
}

pfuPalettizer::Baseline::~Baseline( void )
{
    delete baseline;
}

void pfuPalettizer::Baseline::set( int x1, int x2, int y )
{
    for( int i = x1; i < dim && i < x2; i++ )
        baseline[i] = y;
}

void pfuPalettizer::Baseline::reset( int y )
{
    for( int i = 0; i < dim; i++ )
        baseline[i] = y;
}

void pfuPalettizer::Baseline::softReset( int oldy, int y )
{
    for( int i = 0; i < dim; i++ )
        if( baseline[i] == oldy )
            baseline[i] = y;
}

int pfuPalettizer::Baseline::get( int x )
{
    return baseline[x];
}


int pfuPalettizer::Baseline::lessThan( int y )
{
    int x;
    for (x = 0; x < dim; x++ )
        if( baseline[x] < y )
            break;

    return x;
}
    
/////////////////////// end baseline //////////////////



void pfuPalettizer::buildTtexList( pfGeoSet *gset )
{
    pfGeoState  *gstate  = gset->getGState();
    pfTexture   *tex     = (pfTexture *)gstate->getAttr( PFSTATE_TEXTURE );
    Ttex        *t;
    int          i;

    if( tex != NULL )
    {
        // First check to see if texture is already in list
        int found = 0;

        for( i = 0; i < binList->getNum(); i++ )
        {
            t = (Ttex *)binList->get( i );

           // Assumption: texture with same name is same texture
            if( !strcmp( tex->getName(), t->tex->getName() ) )
            {

               // Add geoset to gsetList only if it is not already on there.
                if( t->gsetList->search( gset ) == -1 )
                    t->gsetList->add( (void *)gset );
                found = 1;
                break;
            }
        }

        if( !found )
        {
            t = (Ttex *)pfMalloc( sizeof( Ttex ), pfGetSharedArena() );
            pfRef( t );
            t->tex = tex;
            tex->getImage( &t->image, &t->comp, &t->ns, &t->nt, &t->nr );

            // if getImage fails attempt to load the file... if that fails, give up
            if( t->image == (uint *)0L )
            {
                tex->loadFile( tex->getName() );
                tex->getImage( &t->image, &t->comp, &t->nt, &t->ns, &t->nr );
                if( t->image == (uint *)0L )
                {
                    pfNotify( PFNFY_NOTICE, PFNFY_PRINT, "pfuPalettizer->palettize() : Unable to load texture \"%s\"\n", tex->getName() );
                    return;                   // give up
                }
            }

            t->gsetList = new pfList;
            pfRef( t->gsetList );
            t->gsetList->add( (void *)gset );
            t->x = t->y = 0;
            t->margin = margin;
            t->is = t->ns + (2 * t->margin); 
            t->it = t->nt + (2 * t->margin); 
            t->area = t->is * t->it;

            t->placed = 0;

            binList->add( (void *)t );
        }
    }
}

void pfuPalettizer::traverseNodeAndBuildTextureList( pfNode *node )
{
    int i;

    if (node->isOfType(pfGroup::getClassType()))
    {
        pfGroup *group = (pfGroup *) node;

        for(i = 0; i < group->getNumChildren(); i++)
            traverseNodeAndBuildTextureList( group->getChild(i) );

    }
    else if( node->isOfType( pfGeode::getClassType() ))
    {
        pfGeode *geode = (pfGeode *)node;

        for( i = 0; i < geode->getNumGSets(); i++ )
        {
               pfGeoSet    *gset   = geode->getGSet(i);
            buildTtexList( gset );
        }
    }
}


//
// Return number of vertices in a geoset
//

static inline int getNumVerts( pfGeoSet *gset )
{
    int nv;
    int np;
    int *lens;
    int i;

    np = gset->getNumPrims();
    nv = 0;

    switch( gset->getPrimType() )
    {
        case PFGS_POINTS :
            nv = np;
            break;

        case PFGS_LINES :
            nv = 2 * np;
            break;

        case PFGS_TRIS :
            nv = 3 * np;
            break;

        case PFGS_QUADS :
            nv = 4 * np;
            break;

        case PFGS_TRISTRIPS :
        case PFGS_FLAT_TRISTRIPS :
        case PFGS_POLYS :
        case PFGS_LINESTRIPS :
        case PFGS_FLAT_LINESTRIPS :

            lens = gset->getPrimLengths();
            for( i = 0; i < np; i++ )
                nv += lens[i];
            break;

    }
    return nv;
}




/////////// bin pack //////////////////

//
// Set texture locations in pallette
//
inline void pfuPalettizer::place( int ss, int tt, int x, int y, Ttex *t, int *placecnt )
/*
                          pfuPalettizer::Baseline *baseline, 
                          pfuPalettizer::Grid *grid, 
*/
{
    t->ix = x;
    t->iy = y;

    t->x = ((float)x + (float)t->margin)/(float)ss;
    t->y = ((float)y + (float)t->margin)/(float)tt;

    t->xscale = (float)t->ns/(float)ss;
    t->yscale = (float)t->nt/(float)tt;

    baseline->set( t->ix, t->ix + t->is, t->iy + t->it ); 
    grid->setRect( t->ix, t->iy, t->is, t->it );

    t->placed = ++(*placecnt);
}

//
// Sort binable list by height (t) primarily and width (s) secondarily
//

void pfuPalettizer::sortList( void )
{
    int i, j;
    Ttex **array;

    int n = binList->getNum();

    array = (Ttex **)binList->getArray();

    for( i = 0; i < n-1; i++ )
    {
        for( j = i + 1; j < n; j++ )
        {
            Ttex     *t1 = array[i],
                    *t2 = array[j];

            if( t1->it < t2->it || ((t1->it == t2->it) && (t1->is < t2->is)) )
            {
                binList->move( j, t1 );
                binList->move( i, t2 );
            }
            
        }
    }
}

//
// Bin packing algorithm
//

int pfuPalettizer::binPack( int s, int t )
{
    int     i, j;
    int     x, y;
    int     nexty;
    Ttex     *ti, *tj;
    Ttex    **tarray;
    int        ntlist;
//    Grid     *grid;
//    Baseline *baseline;
    int     placecnt = 0;

   // If we only have ONE texture to pack return
    ntlist = binList->getNum();

    if( ntlist <= 1 ) 
        return 0;

    // Create grid and baseline data structures to keep track of packing
    grid      = new Grid( s, t );
    baseline = new Baseline( s );

   // Sort the eligible list
    sortList( );

    x = 0;
    tarray = (Ttex **)binList->getArray();

   // nexty is the y value of the next available "row".
    nexty = tarray[0]->it;
    if( nexty > t )
        nexty = t;

   // Set baseline to the bottom of the palette
    baseline->reset( 0 );
    y = baseline->get( x );

    placecnt = 0;

    for( i = 0; i < ntlist; i++ )
    {
        ti = tarray[i];

        // If the texture has already been placed, ignore it
        if( ti->placed != 0 )
            continue;

        // If it fits in current x, y location place it there
        if( x + ti->is <= s && y + ti->it <= nexty )
        {
            place( s, t, x, y, ti, /*baseline, grid,*/ &placecnt );
            pfNotify( PFNFY_DEBUG, PFNFY_PRINT, "pfuPalettizer()->binPack :"
                                            " %d of %d eligible textures palettized\n", placecnt, ntlist );
            //
            // increment x and y
            //
            x += ti->is;
            y = baseline->get( x );
        }

        // If we are at the end of a "row"
        if( x + ti->is > s )
        {
            int found;

            // loop while textures fit
            do
            {
                found = 0;

                // First check to see what textures fit at end of row
                // textures after index 'i' are "guaranteed" to be <= height
                for( j = i; j < ntlist; j++ )
                {
                    tj = tarray[j];

                    if( tj->placed != 0 )
                        continue ;

                    if( x + tj->is <= s && y + tj->it <= nexty )
                    {
                        found = 1;
                        place( s, t, x, y, tj, /*baseline, grid,*/ &placecnt );
                        pfNotify( PFNFY_DEBUG, PFNFY_PRINT, "pfuPalettizer()->binPack : "
                                                            "%d of %d eligible textures palettized\n", placecnt, ntlist );
                        x += tj->is;
                        y = baseline->get( x );
                    }
                }
        
                // We can't fit anymore between current x and right side
                // of the palette, go back and find spaces in the current
                // "row"
                x = baseline->lessThan( nexty );
                y = baseline->get( x );

                // allow for margins.  If we create "subrows" within a
                // "row", then we must bump up nexty to compensate for
                // margins
                if( x < s && nexty + 2 * margin <= t )
                {
                    baseline->softReset( nexty, nexty + 2 * margin ); 
                    nexty += 2 * margin;    
                }

                // With new x and y locations, continue to look for textures that
                // fit
                for( j = i; j < ntlist; j++ )
                {
                    tj = tarray[j];

                    if( tj->placed != 0 )
                        continue ;

                    if( x + tj->is <= s && y + tj->it <= nexty )
                    {
                        found = 1;
                        place( s, t, x, y, tj, /*baseline, grid,*/ &placecnt );

                        pfNotify( PFNFY_DEBUG, PFNFY_PRINT, "pfuPalettizer()->binPack : "
                                                            "%d of %d eligible textures palettized\n", placecnt, ntlist );
                        x += tj->is;
                        y = baseline->get( x );
                    }
                }


            }while( found );


            x = 0;
            baseline->reset( nexty );
            y = baseline->get( x );

            nexty += ti->it;
            if( nexty > t ) nexty = t;
        }

    }

    // Last algorithm here ... find any remaining holes...
    if( placecnt < ntlist )
    {
        // findNextHole traverses the grid bottom to top,
        // left to right.  returns first avaiable space
        while( grid->findNextHole( &x, &y ) )
        {
            int found = 0;
            int w, h;

            // get the width and height at this location
            grid->findWxHat( x, y, &w, &h );

            // See if any textures fit here
            for( i = 0; i < ntlist; i++ )
            {
                ti = tarray[i];
                
                if( ti->placed != 0 )
                    continue;

                if( ti->is <= w && ti->it <= h )
                {
                    place( s, t, x, y, ti, /*baseline, grid,*/ &placecnt );  

                    pfNotify( PFNFY_DEBUG, PFNFY_PRINT, "pfuPalettizer()->binPack : "
                                                        "%d of %d eligible textures palettized\n", placecnt, ntlist );
                    found = 1;
                    break;
                }
            }

            // If no textures fit, fill in the grid
            if(!found ) 
                grid->setRect( x, y, w, h );
        }
    }

    // clean up
    delete grid;
    delete baseline;

    return placecnt;
}

/////////////  end bin packing ///////////////////


////////////  Misc utilities /////////////////////


// Finds most efficient size for given bin list
void pfuPalettizer::findBestST( int *s, int *t )
{
    int i;
    int totalArea = 0;

    *s = maxS;
    *t = maxT;

    for( i = 0; i < binList->getNum(); i++ )
    {
        Ttex *tt = (Ttex *)binList->get( i );
        totalArea += tt->area;
    }


    while( totalArea <= (*s * *t)/2 )
    {
        if( *t < *s )
            *s >>= 1;
        else
            *t >>= 1;
    }
}

//
// removes textures from bin list which have s or t larger than
// maxS and maxT
//

void  pfuPalettizer::removeOversizedTextures(void )
{
    int flag;

    do
    {
        flag = 0;
        for( int i = 0; i < binList->getNum(); i++ )
        {
            Ttex *t = (Ttex *)binList->get( i );
    
            if( t->is > maxS || t->it > maxT )
            {
                pfNotify( PFNFY_INFO, PFNFY_PRINT, "pfuPalettizer->palettize() :\n" );
                pfNotify( PFNFY_INFO, PFNFY_MORE,  "\t\tcannot palettize texture \"%s\"\n", t->tex->getName() );
                pfNotify( PFNFY_INFO, PFNFY_MORE, "\t\t will not fit in %d X %d palette.\n", maxS, maxT );
                completeList->add( t->tex );
                binList->remove( t );
                flag = 1;
                break;
            }
        }
    }while( flag );
}

//
// Removes ineligible textures
//
// Ineligible textures are textures which :
//
//      - have detail textures
//        - have geosets with texture coordinates greater than 
//        1.0 or less than 0.0
//

void  pfuPalettizer::removeIneligibleTextures( void )
{

    int i, j, k;
    Ttex *t;
    int flag = 0;

    do
    {
        flag = 0;

        for( i = 0; i < binList->getNum(); i++ )
        {
            int eligible = 1;
    
            t = (Ttex *)binList->get( i );
    
            // Check if texture has detail
            if( eligible )
            {
                int level;
                pfTexture *detail;
    
                t->tex->getDetail( &level, &detail );
                if( detail != (pfTexture *)0L )
                {
                    pfNotify( PFNFY_INFO, PFNFY_PRINT, "pfuPalettizer->palettize() :\n" );
                    pfNotify( PFNFY_INFO, PFNFY_MORE,    "\t\ttexture \"%s\" cannot be palettized\n", t->tex->getName() );
                    pfNotify( PFNFY_INFO, PFNFY_MORE,    "\t\thas detail texture."  );
                    eligible = 0;
                }
            }
    
            // Check if texture coordinates repeat
            if( eligible )
            {
                for( j = 0; j < t->gsetList->getNum(); j ++ )
                {
                    pfGeoSet     *gset   = (pfGeoSet *)t->gsetList->get( j );
                    int         nv      = getNumVerts( gset );
                    pfVec2      *tlist  = (pfVec2 *)0L;
                    ushort      *ilist  = (ushort *)0L;
		    int		noise = 0; // clamp noisy tcoords to [0,1]
    
                    tlist = (pfVec2 *)0L;
                    gset->getAttrLists( PFGS_TEXCOORD2,  (void **)&tlist, &ilist );
    
                    if( tlist != (pfVec2 *)0L )
                    {

                        // Assume if an index list is specified that the number of texture coordinates
                        // is the same as the largest value in the index list + 1.
                        if( ilist != (ushort *)0L )
                        {
                            int max;
    
                            max = 0;
                            for( k = 0; k < nv; k++ )
                                if( ilist[k] > max ) max = ilist[k];
                            nv = max + 1;
                        }
    
                        for( k = 0; k < nv; k++ )
                        {
                            if( tlist[k][0] > 1.0f || tlist[k][0] < 0.0f ||
                                tlist[k][1] > 1.0f || tlist[k][1] < 0.0f )

                            {
				if (PF_ABSLT(tlist[k][0], TCOORD_EPS) || 
				    PF_ABSLT(tlist[k][1], TCOORD_EPS) ||
				    PF_ABSLT(tlist[k][0], 1.0f + TCOORD_EPS) || 
				    PF_ABSLT(tlist[k][1], 1.0f + TCOORD_EPS))
				{ // tcoords are close enough to [0,1] 
				    noise = 1;
				}
				else
				{
				    pfNotify( PFNFY_INFO, PFNFY_PRINT, "pfuPalettizer->palettize() :\n" );
				    pfNotify( PFNFY_INFO, PFNFY_MORE,    "\t\ttexture \"%s\" cannot be palettized\n", t->tex->getName() );
				    pfNotify( PFNFY_INFO, PFNFY_MORE,    "\t\treferenced by texture coordinates outside the range 0.0 - 1.0.\n"  );
				    pfNotify( PFNFY_DEBUG, PFNFY_MORE,    "\t\t\t(%8.12f %8.12f)\n", tlist[k][0], tlist[k][1]  );
				    eligible = 0;
				    break;
				}
                            }
    
                        }
			// clean up noisy tcoords so we don't get bleeding
			if (eligible /* && noise */) 
			{
			    for( k = 0; k < nv; k++ )
			    {
				if (tlist[k][0] < TCOORD_EPS)
				    tlist[k][0] = TCOORD_MIN;
				else if (tlist[k][0] > (1.0f - TCOORD_EPS))
				    tlist[k][0] = TCOORD_MAX;
				if (tlist[k][1] < TCOORD_EPS)
				    tlist[k][1] = TCOORD_MIN;
				else if (tlist[k][1] > (1.0f - TCOORD_EPS))
				    tlist[k][1] = TCOORD_MAX;
			    }
			    noise = 0;
			}
    
                    }
                    if (eligible == 0) 
			break;
                }
            }
    
    
            if( !eligible )
            {
                completeList->add( t->tex );
                binList->remove( t );
                pfUnrefDelete( t );
                flag = 1;
                break;
            }
        }

    } while( flag );
}


//
// Creates new pfTexture.  Currently simply creates an rgba
// file.  Will be enhanced to use pfI format
//


pfTexture *pfuPalettizer::buildNewTexture( int s, int t )
{
    static int texCount = 0;
    uint         *data;
    unsigned char    *cptr;
    int         i;
    char texName[40];


    data = (uint *)pfMalloc( s * t * sizeof( uint ), pfGetSharedArena() );
    pfRef( data );

    for( i = 0; i < (s * t); i++ )
        data[i] = background;


    for( i = 0; i < binList->getNum(); i++ ) 
    {
        Ttex  *tt = (Ttex *)binList->get( i );
        int   index;
        int   x, y;
        int   xx, yy;
        int   mm;

        if( tt->placed == 0 )
            continue;

        pfNotify( PFNFY_INFO, PFNFY_PRINT, "pfuPalettizer->palettize() :\n" );
        pfNotify( PFNFY_INFO, PFNFY_MORE, "\t\tpalettizing \"%s\"\n", tt->tex->getName() );

        xx = (int)((float)s * tt->x);
        yy = (int)((float)t * tt->y);

        mm = (int)margin;

        for( y = -(mm); y < (tt->nt + mm); y++ )
        {

            if( y <= 0 )
            {
                cptr = (unsigned char *)tt->image;
            }
            else if( y >= tt->nt )
            {
                cptr = (unsigned char *)tt->image;
                cptr += (tt->ns * tt->nt * tt->comp) - (tt->ns * tt->comp);
            }

            for( x = -(mm); x < (tt->ns + (mm)); x++ )
            {
                unsigned char r, g, b, a; 

                index = ((yy + y) * s) + (xx + x);
                if( x == -(mm) || (x > 0 && x < tt->ns) )
                {
                    switch( tt->comp )
                    {
                        case 1 :
                            r = *cptr++;
                            g = b = r;
                            a = 0xFF;
                            break;
    
                        case 2 : 
			    r = *cptr++;
                            a = *cptr++;
                            g = b = r;
                            break;
    
                        case 3 :
			    r = *cptr++;
                            g = *cptr++;
                            b = *cptr++;
                            a = 0xFF;
                            break;
                    
                        case 4 :
                            r = *cptr++;
                            g = *cptr++;
                            b = *cptr++;
                            a = *cptr++;
                            break;
                    }
                }
		data[index] = ((uint)r<<24) | ((uint)g<<16) | ((uint)b<<8) | a;
            }
        }
    }

    pfTexture *tex = new pfTexture;
    pfRef( tex );
    tex->setImage( (uint *)data, 4, s, t, 1 );
    sprintf( texName, "%s%03d.rgba", texturePrefix, ++texCount );
    tex->setName( texName );

    tex->setFormat( PFTEX_EXTERNAL_FORMAT,  PFTEX_PACK_8 );
    tex->setFormat( PFTEX_INTERNAL_FORMAT,  PFTEX_RGB5_A1 );
   // tex->setFormat( PFTEX_IMAGE_FORMAT,     PFTEX_DEFAULT );

    if (!(mode & _PFUPAL_MODEMASK_MIPMAP))
	tex->setFilter( PFTEX_MINFILTER, PFTEX_BILINEAR );

    return tex;
}


//
// Set texture coordinates in GeoSets to appropriate offsets into
// palette.  Set the new palette as the current texture. 
// Unref Delete the old texture
//

void pfuPalettizer::changeTextureCoordinates( pfTexture *tex )
{
    int i, j, k;

    for( i = 0; i < binList->getNum(); i++ )
    {
        Ttex *t = (Ttex *)binList->get( i );

        if( t->placed )
        {
            for( j = 0; j < t->gsetList->getNum(); j++ )
            {
                pfGeoSet     *gset     = (pfGeoSet *)t->gsetList->get( j );
                pfGeoState   *gstate   = gset->getGState();
                int          nv        = getNumVerts( gset );
                pfVec2       *tlist    = (pfVec2 *)0L;
                ushort       *ilist    = (ushort *)0L;

                gset->getAttrLists( PFGS_TEXCOORD2,  (void **)&tlist, &ilist );
                //
                // Don't change a texture coordinate list more than once.  tlistChanged is
                // a list that keeps track of what tlists have already been changed.
                //
                // CAVEAT : if geosets share texture lists, but don't share textures, results
                // are undefined.
                //
                if( tlist != (pfVec2 *)0L && tlistChanged->search( tlist ) == -1 )
                {
                //
                // Assume if an index list is specified that the number of texture coordinates
                // is the same as the largest value in the index list + 1.
                //
                if( ilist != (ushort *)0L )
                {
                    int max;
    
                    max = 0;
                    for( k = 0; k < nv; k++ )
                        if( ilist[k] > max ) max = ilist[k];
                    nv = max + 1;
                }
    
                for( k = 0; k < nv; k++ )
                {
                    tlist[k][0] = t->x + (tlist[k][0] * t->xscale); 
                    tlist[k][1] = t->y + (tlist[k][1] * t->yscale); 
                }


                pfRef( tlist );
                tlistChanged->add( tlist );
            }

            gstate->setAttr( PFSTATE_TEXTURE, tex);
        }
      }
    }
}

//
// Clean up the bin list after each new palette creation
// Remove textures that have already been placed
//

void pfuPalettizer::cleanBinList( void )
{
    int flag = 0;

//
// This while loop is necessary since order of the pfList
// changes after each ->remove
//
    do
    {
        flag = 0;
        for( int i = 0; i < binList->getNum(); i++ )
        {
            Ttex *t = (Ttex *)binList->get( i );

            if( t->placed )
            {
                pfUnrefDelete( t->tex );
                binList->remove( t );    
                pfUnrefDelete( t );
                flag = 1;
                break;
            }
        }
    }while( flag );

}


//
// Constructor
//

pfuPalettizer::pfuPalettizer( void )
{
    completeList = (pfList *)0L;
    paletteList  = (pfList *)0L;
    texturePrefix = defaultTexturePrefix;

    maxS         = defaultS;
    maxT         = defaultT;
    margin         = defaultMargin;
    background     = defaultBackground;
    mode	 = 0;
}

pfuPalettizer::~pfuPalettizer( void )
{
    if( completeList != (pfList *)0L )
        delete completeList;

    if( paletteList != (pfList *)0L )
        delete paletteList;

    if( texturePrefix != defaultTexturePrefix )
        delete texturePrefix;
}


int pfuPalettizer::palettize( pfNode *node )
{
    // Set up data structures
    tlistChanged   = new pfList;
    pfRef( tlistChanged );

    binList           = new pfList;
    pfRef( binList );

// see note below on pfUnrefDelete
//    if( completeList != (pfList *)0L )
//       pfUnrefDelete( completeList );

    completeList = new pfList;
    pfRef( completeList );

// see note below on pfUnrefDelete
//    if( paletteList != (pfList *)0L )
//       pfUnrefDelete( paletteList );

    paletteList = new pfList;
    pfRef( paletteList );

    // uh... traverse the pfNode and build the texture list
    traverseNodeAndBuildTextureList( node );

    nTotal = binList->getNum();

    // uh.. remove any oversized textures
    removeOversizedTextures();

    // Remove ineligible textures.
    // Ineligible texture refer to textures that :
    //
    //  a. have detail
    //  b. are referred to by geosets containing texture coordinates
    //     outside of the range of 0.0 - 1.0
    removeIneligibleTextures();

    nEligible = binList->getNum();
    nPalettized = 0;

    pfNotify( PFNFY_DEBUG, PFNFY_PRINT, "pfuPalettize->palettize() : %d textures of %d are eligible for palettization\n",
                nEligible, nTotal );


    // Loop until bin list is exhausted
    while( binList->getNum() )
    {
        int s, t;
        int placed;
        int i;

        // Find best size for new palette
        findBestST( &s, &t ); 

        // pack textures (including margins)
        placed = binPack( s, t );


        // If more than one texture has been binpacked,
        // create a new texture, change texture coordinates
        if( placed > 1 )
        {
            nPalettized += placed;

            pfTexture *tex = buildNewTexture( s, t );
            changeTextureCoordinates(  tex );
            completeList->insert( 0, tex );
            paletteList->insert( 0, tex );
        }

        // If only one texture has bin bin packed, use the original
        // texture.
        else if( placed == 1 )
        {
            for( i = 0; i < binList->getNum(); i++ )
            {
                Ttex *t = (Ttex *)binList->get( i );

                if( t->placed )
                {
                    pfRef( t->tex );
                    completeList->add( t->tex );
                }
            }
        }

        // If no textures have been bin packed, binpacking has 
        // failed.  Move all remaining textures to final list
        else
        {
            for( i = 0; i < binList->getNum(); i++ )
            {
                Ttex *t = (Ttex *)binList->get( i );

                pfRef( t->tex );
                completeList->add( t->tex );

                // set "placed" so that cleanBinList will remove this node
                t->placed = 1;
            }
        }

        cleanBinList( );
    }

//
// Clean up ...
//
// Note 9/10 - commenting out pfUnrefDelete due to unstable behavior 
// of pfLists, when you do this.
//
//   pfUnrefDelete( binList );
//   pfUnrefDelete( tlistChanged );

//
// ... and report
//
    pfNotify( PFNFY_NOTICE, PFNFY_PRINT, "pfuPalettizer->palettize() : %3d textures submitted\n", nTotal );
    pfNotify( PFNFY_NOTICE, PFNFY_MORE , "                              %3d textures eligible for palettizing\n", nEligible );
    pfNotify( PFNFY_NOTICE, PFNFY_MORE , "                              %3d textures palettized\n", nPalettized );
    pfNotify( PFNFY_NOTICE, PFNFY_MORE , "                              %3d new texture palette%s created\n", 
                                                                        paletteList->getNum(),
                                                                        paletteList->getNum() > 1 ? "s":"" );
            

    return nPalettized;
}

void pfuPalettizer::setST( uint ss, uint tt )
{
    maxS = ss;
    maxT = tt;
}

void pfuPalettizer::setMargin( uint m )
{
    margin = m;
}

void pfuPalettizer::setTexturePrefix( char *prefix )
{
    if( texturePrefix != defaultTexturePrefix )
        delete texturePrefix;

    texturePrefix = strdup( prefix );
}

pfList *pfuPalettizer::getPaletteList( void )
{
    return  paletteList;
}

pfList *pfuPalettizer::getCompleteList( void )
{
    return completeList;
}


void pfuPalettizer::setBackground( float rf, float gf, float bf, float af )
{
    unsigned char r, g, b, a;


    r = (unsigned char)(255. * rf) & 0xFF;
    g = (unsigned char)(255. * gf) & 0xFF;
    b = (unsigned char)(255. * bf) & 0xFF;
    a = (unsigned char)(255. * af) & 0xFF;
    background = ((uint)r<<24) | ((uint)g<<16) | ((uint)b<<8) | a;
}

void 
pfuPalettizer::setMode(int _mode,  int _val)
{
    switch (_mode)
    {
	case PFUPAL_MODE_MIPMAP:
	    PFFLAG_BOOL_SET(mode, _PFUPAL_MODEMASK_MIPMAP, _val);
	    break;
	default:
	    pfNotify(PFNFY_NOTICE, PFNFY_WARN, 
	    "pfuPalettizer::setMode - unknown mode %d", _mode);
	    break;
    }
}

int 
pfuPalettizer::getMode(int _mode)
{
    switch (_mode)
    {
	case PFUPAL_MODE_MIPMAP:
	    return PFFLAG_BOOL_GET(mode, _PFUPAL_MODEMASK_MIPMAP);
	default:
	    pfNotify(PFNFY_NOTICE, PFNFY_WARN, 
	    "pfuPalettizer::getMode - unknown mode %d", _mode);
	    return -1;
    }
}
