/*
 * Copyright 1993, 1995, Silicon Graphics, Inc.
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
 * Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
 * 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that (i) the above copyright notices and this
 * permission notice appear in all copies of the software and related
 * documentation, and (ii) the name of Silicon Graphics may not be
 * used in any advertising or publicity relating to the software
 * without the specific, prior written permission of Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR ANY SPECIAL,
 * INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY
 * THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pf/pfPipeWindow.h>
#include <Performer/pr/pfTexture.h>

#include <GL/gl.h>
#include <GL/glu.h>
#ifdef WIN32
#include <wingdi.h>
#endif

static int inited = 0;
static struct _table {
    int     format;
    char    *formatName;
    int        bytesInFormat;
    GLuint  bankA,
            bankB;
}table[] = {
          { PFTEX_RGB_5   , "PFTEX_RGB_5"  , 2,  0,  0 },
          { PFTEX_RGB_4   , "PFTEX_RGB_4"  , 2,  0,  0 },
          { PFTEX_RGB5_A1 , "PFTEX_RGB5_A1", 2,  0,  0 },
          { PFTEX_RGBA_4  , "PFTEX_RGBA_4" , 2,  0,  0 },
          { PFTEX_IA_8    , "PFTEX_IA_8"   , 2,  0,  0 },
          { PFTEX_I_12A_4 , "PFTEX_I_12A_4", 2,  0,  0 },
          { PFTEX_I_8     , "PFTEX_I_8"    , 2,  0,  0 },
          { PFTEX_I_16    , "PFTEX_I_16"   , 2,  0,  0 },
          { PFTEX_IA_12   , "PFTEX_IA_12"  , 3,  0,  0 },
          { PFTEX_RGBA_8  , "PFTEX_RGBA_8" , 4,  0,  0 },
          { PFTEX_RGB_12  , "PFTEX_RGB_12" , 6,  0,  0 },
          { PFTEX_RGBA_12 , "PFTEX_RGBA_12", 6,  0,  0 },
          { -1 }
};

static char fontName[] = "-bitstream-charter-bold-r-normal--25-240-75-75-p-136-iso8859-1";
static int xfont;

static inline void getST(  int tmSize, int bytesInFormat, int *s, int *t )
{
    int ss = 1, tt = 1;
    do
    {
        tt <<= 1;
        if( (ss * tt * bytesInFormat) >= tmSize )
            break;
        ss <<= 1;
    }while( (ss * tt * bytesInFormat) < tmSize );

    *s = ss;
    *t = tt;
}

static inline void square( float x1, float y1, float x2, float y2 )
{
    int i;
    float v[4][3];
    float t[][2] = {
        { 0.0, 0.0 },
        { 0.0, 1.0 },
        { 1.0, 1.0 },
        { 1.0, 0.0 },
    };

    v[0][0] = v[3][0] = x1;
    v[1][0] = v[2][0] = x2;
    v[0][1] = v[1][1] = y1;
    v[2][1] = v[3][1] = y2;

    v[0][2] = v[1][2] = v[2][2] = v[3][2] = 0.0f;


    glBegin( GL_QUADS );
    for( i = 0; i < 4; i++ )
    {
        glTexCoord2fv( t[i] );
        glVertex3fv( v[i] );
    }
    glEnd();
}

#ifdef WIN32
/* defined in xfont.c */
extern "C" {
  int pfuXLFDtoLOGFONT(const char *fontDesc, LOGFONT *destination);
};
#endif

static int loadXFont(const char *fontName)
{

#define FIRSTGLYPH  32
#define LASTGLYPH   128
    int fontDLHandle;

#ifdef WIN32
    /*
      HFONT font = CreateFont(-24,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,
      OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
      ANTIALIASED_QUALITY,FF_DONTCARE|DEFAULT_PITCH,
      fontName);
    */
    
    LOGFONT lf;
    HFONT font;
    
    pfuXLFDtoLOGFONT(fontName, &lf);
    font = CreateFontIndirect(&lf);
    
    if(!font) return 0;
    DeleteObject(SelectObject(wglGetCurrentDC(), font));
    //HFONT oldFont = SelectObject(hDC,font);
    if(fontDLHandle = glGenLists((GLuint) LASTGLYPH + 1) == 0)
       // XXX Alex -- destroy font ... 
       return 0;
      
    wglUseFontBitmaps(wglGetCurrentDC(),
		      FIRSTGLYPH,
		      LASTGLYPH - FIRSTGLYPH + 1, 
		      fontDLHandle);
    
#else
    Display *dpy = pfGetCurWSConnection();
    XFontStruct *fontInfo = XLoadQueryFont(dpy, fontName);

    if( fontInfo == 0L )
        return 0;

    if( (fontDLHandle = glGenLists((GLuint) LASTGLYPH + 1)) == 0 )
        return 0;

    glXUseXFont(fontInfo->fid, FIRSTGLYPH, LASTGLYPH - FIRSTGLYPH + 1,
        fontDLHandle + FIRSTGLYPH);
#endif

    return fontDLHandle;
}

static void drawXFontString( int fontDLHandle, char *s )
{
    glPushAttrib(GL_LIST_BIT);
    glListBase(fontDLHandle);
    glCallLists(strlen(s), GL_UNSIGNED_BYTE, (GLubyte *)s);
    glPopAttrib();
}

static int init( void )
{
    int i;
    struct _table *ptr;
    int tmSize;

    pfQuerySys( PFQSYS_TEXTURE_MEMORY_BYTES, &tmSize );

    if( tmSize == -1 )
    {
        pfNotify( PFNFY_WARN, PFNFY_PRINT, "pfuShowTextureMemory: No Texture Memory" ); 
        return 0;
    }

    for( ptr = table;  ptr->format != -1;  ptr++ )
    {
        GLuint texobjs[2];
        int s, t;

        // Assume that we know that texture memory is split into two banks
        // I/R architexture 10/97
        getST( tmSize/2, ptr->bytesInFormat, &s, &t ); 

        glGenTextures( 2, texobjs );
	
        for( i = 0; i < 2; i++ )
        {
            glBindTexture( GL_TEXTURE_2D, texobjs[i] );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexImage2D( GL_TEXTURE_2D, 0, ptr->format, s, t, 0, 
                        GL_RGBA, GL_UNSIGNED_INT, 0L );
            glBindTexture( GL_TEXTURE_2D, 0 );
        }
        ptr->bankA = glGenLists( 1 );
        glNewList( ptr->bankA, GL_COMPILE );
        glBindTexture( GL_TEXTURE_2D, texobjs[0] );
        square( -0.95, -0.95, -0.05, 0.95 );
        glEndList();

        ptr->bankB = glGenLists( 1 );
        glNewList( ptr->bankB, GL_COMPILE );
        glBindTexture( GL_TEXTURE_2D, texobjs[1] );
        square(  0.05, -0.95, 0.95, 0.95 );
        glEndList();

    }

    inited = 1;

    xfont = loadXFont( fontName );

    return 1;
}


void pfuShowTextureMemory( pfPipeWindow *pw, pfList *texList, int format )
{
    struct _table *ptr;
    int xs, ys;
    char label[80];
    int tmSize;

    if( !inited && !init() )
        return;

    for( int i = 0; i < texList->getNum(); i++ )
    {
        pfTexture *tex = (pfTexture *)texList->get( i ) ;
        tex->deleteGLHandle();
    }

    pw->getSize( &xs, &ys );

    for( ptr = table; ptr->format != -1; ptr++ )
        if( ptr->format == format )
            break;

    if( ptr->format == -1 )
	{
		pfNotify( PFNFY_WARN, PFNFY_PRINT, "pfuShowTextureMemory() : unsupported format : 0x%x\n", format );
        return;
	}

    glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glViewport( 0, 0, xs, ys );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glPushAttrib( GL_ENABLE_BIT );
    glEnable( GL_TEXTURE_2D );

    glCallList( ptr->bankA );
    glCallList( ptr->bankB );

    glPopAttrib();

    sprintf( label, "Bank A (%s)", ptr->formatName );
    glRasterPos2f( -0.94, 0.96 );
    drawXFontString( xfont, label );

    sprintf( label, "Bank B (%s)", ptr->formatName );
    glRasterPos2f(  0.06, 0.96 );
    drawXFontString( xfont, label );

    glBindTexture( GL_TEXTURE_2D, 0 );
}


void pfuClearTextureMemory( void )
{
    int i;
    static GLuint texobjs[2];
    static unsigned char *data = 0L;
    static int s, t;

    if( data == 0L )
    {
        int tmSize;

        pfQuerySys( PFQSYS_TEXTURE_MEMORY_BYTES, &tmSize );
        getST( tmSize/2, 4, &s, &t );

        data = (unsigned char *)pfMalloc( s * t * 4 *
                sizeof( unsigned char ), pfGetSharedArena() );
        memset( data, 0,  s * t * 4 * sizeof( unsigned char ) );

        glGenTextures( 2, texobjs );
    }

    // Assume that we know that texture memory is split into two banks
    // I/R architexture 10/97
    for( i = 0; i < 2; i++ )
    {
      glBindTexture( GL_TEXTURE_2D, texobjs[i] );
      
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8_EXT, s, t, 0,
		    GL_RGBA, GL_UNSIGNED_BYTE, data );
      glBindTexture( GL_TEXTURE_2D, 0 );
    }
    
    glDeleteTextures( 2, texobjs );
    glBindTexture( GL_TEXTURE_2D, 0 );
}
