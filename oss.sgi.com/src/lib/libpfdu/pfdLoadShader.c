/*
 * pfdLoadShader.c
 *
 * Load a shader description file and return a pointer to 
 * to a pfShader data structure which represents the 
 * described shader.  The file is sought in the directories 
 * named in the active performer file search path (see 
 * pfFilePath for more details). 
 *
 * Copyright 1995, Silicon Graphics, Inc.
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
 * THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
 * INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
 * DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
 * PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
 * GRAPHICS, INC.
 */

#include <stdlib.h>
#include <stdio.h>

#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <Performer/pfutil.h>

/* function prototypes */
int __pfdLoadShader_parse (void);
int __pfdLoadShader_error (const char*, ...);
int __pfdLoadShader_warning (const char*, ...);

/* typedefs
 * 
 * XXX This definition should be in a header file which is
 *     included in pfdLoadShader.c and in pfdLoadShader_Parse.yxx.
 *     Due a build complication, I can't get such an include to
 *     work properly in pfdLoadShader_Parse.yxx.  Since this is
 *     the only negative side effect, I'm leaving it like this.
 */
typedef struct __texIDMapEntry {
    struct __texIDMapEntry *next;
    int src;
    int inUse;
    int texID;
    char *identifierString;
    pfTexture *texture;
} texIDMapEntry;

/* extern globals */
extern FILE* __pfdLoadShader_in;
extern texIDMapEntry* __pfdLoadShader_texIDMap;

/* non-static globals */
pfShader *__pfdLoadShader_shader;
int __pfdLoadShader_lineNum;
char *__pfdLoadShader_fileName;
GLboolean __pfdLoadShader_parseFailed = 0;

pfShader *pfdLoadShader (const char* fileName)
{
    texIDMapEntry *currEntry, *prevEntry;
    __pfdLoadShader_fileName = strdup (fileName);
    
    /* Open fileName with pfdOpenFile which is aware
     * of performer paths.
     */
    __pfdLoadShader_shader = 0;
    __pfdLoadShader_lineNum = 0;
    __pfdLoadShader_parseFailed = 0;
    __pfdLoadShader_in = pfdOpenFile (__pfdLoadShader_fileName);
    if (__pfdLoadShader_in == NULL) {
        pfNotify(PFNFY_WARN, PFNFY_PRINT,
                 "pfdLoadShader: couldn't load shader description file %s",
		 __pfdLoadShader_fileName);
        return NULL;

    }

    __pfdLoadShader_shader = pfNewShader ();

    pfShaderName(__pfdLoadShader_shader,__pfdLoadShader_fileName);

    /* Call the yacc generated parser to actually read the file
     */
    __pfdLoadShader_texIDMap = NULL;
    __pfdLoadShader_lineNum = 1;

    __pfdLoadShader_parse ();

    /* Do cleanup which must always happen */
    free (__pfdLoadShader_fileName);

    currEntry = __pfdLoadShader_texIDMap;
    while (currEntry != NULL) {
	if (!currEntry->inUse) {
	    __pfdLoadShader_warning ("Texture %s declared but never "
				     "referenced.\n",
				     currEntry->identifierString);
	}
	free (currEntry->identifierString);
	if (currEntry->texture != NULL) {
	    pfUnrefDelete (currEntry->texture);
	}
	prevEntry = currEntry;
	currEntry = currEntry->next;
	free (prevEntry);
    }
    
    if (__pfdLoadShader_parseFailed) {
	/* Do failure cleanup */
	pfDelete (__pfdLoadShader_shader);
	return NULL;
    }    

    return (__pfdLoadShader_shader);
}

