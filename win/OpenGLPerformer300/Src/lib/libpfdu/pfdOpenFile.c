/*
 * Copyright 1995, Silicon Graphics, Inc.
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
 * pfdOpenFile.c
 *
 * $Revision: 1.7 $
 * $Date: 2002/11/21 02:19:58 $
 *
 * Search for and open a file.  The file is sought in the 
 * directories named in the curren IRIS Performer file path 
 * (see pfFilePath for more details).
 */

#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <Performer/pf.h>
#include <Performer/pfdu-DLL.h>

#ifdef WIN32
#define READ_MODE "rb"
#define WRITE_MODE "wb"
#else
#define READ_MODE "r"
#define WRITE_MODE "w"
#endif

/*
 * FUNCTION:
 *	pfdOpenFile
 *
 * DESCRIPTION:
 *	Search for and open a file.  The file is sought in the 
 *	directories named in the curren IRIS Performer file path 
 *	(see pfFilePath for more details).
 */

extern PFDUDLLEXPORT FILE *
pfdOpenFile (char *fileName)
{
    FILE	*file = NULL;
    char	 filePath[512];

    /* check argument */
    if (fileName == NULL || *fileName == '\0')
	return NULL;

    /* find file in IRIS Performer directory-search path */
    if (!pfFindFile(fileName, filePath, R_OK))
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "openFile: Could not find file \"%s\"", fileName);
	return NULL;
    }

    /* open  file */
    if ((file = fopen(filePath, READ_MODE)) == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE,
	    "openFile: Could not open file \"%s\"", filePath);
	return NULL;
    }

    return file;
}
