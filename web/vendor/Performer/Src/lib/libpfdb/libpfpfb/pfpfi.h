/*
 * Copyright (C) 1997, Silicon Graphics, Inc.
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

/*
 *  pfpfi.h $Revision: 1.4 $
 */


#ifndef __PFPFI_H__
#define __PFPFI_H__


#include <Performer/pf.h>
#include <Performer/pfdu.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    uint magic;
    int version;
    int data_start;
    int data_size;
    int size[3];
    int num_comp;
    int format;
    int packing;
    int gl;
    int mipmaps;
    int num_images;
} pfi_header_t;

#define PFI_MAGIC_NUMBER	0xdb0fdb00
#define PFI_VERSION_NUMBER      -9998

#define PFI_FORMAT_UINT_8	1
#define PFI_FORMAT_INT_8	2
#define PFI_FORMAT_UINT_16	3
#define PFI_FORMAT_INT_16	4
#define PFI_FORMAT_UINT_32	5
#define PFI_FORMAT_INT_32	6
#define PFI_FORMAT_FLOAT_32	7

#define PFI_PACK_DEFAULT	0

#define PFI_OPENGL	0
#define PFI_IRISGL	1


/*
 *  prototypes
 */
int pfi_store_file(pfTexture *tex, const char *fileName);
int pfi_load_file(pfTexture *tex, const char *fileName);


#ifdef __cplusplus
}
#endif

#endif /* __PFPFI_H__ */
