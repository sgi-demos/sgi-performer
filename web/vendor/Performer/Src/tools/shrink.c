/*
 * Copyright 2000, Silicon Graphics, Inc.
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
** Shrink takes an rgb-style image file, and makes a pyramid using izoom.
**
** It takes a scanf string with one argument describing the file pyramid.
** the argument in the string is the dimension of the file.
**
** The second parameter is the size of the topmost file. It is used
** to complete the filename. For example, to shrink a 1024x1024 file:
**
** shrink foo.%d.rgb 1024
**
** This assumes that foo.1024.rgb exists, and will create:
** foo.512.rgb foo.256.rgb foo.128.rgb foo.64.rgb foo.32.rgb ... foo.1.rgb
**
*/
#include<stdio.h>
enum{COMMAND, SCANF_STRING, TOP_LEVEL_SIZE};

int main(int argc, char *argv[])
{
    int size;
    char infile[256], outfile[256];
    char cmd[256];
    FILE *fp;

    if(argc < 3) {
	fprintf(stderr, 
		"Usage: %s fname size\n"
		"Where fname is a scanf-style string with one numeric "
		"parameter, filesize.\n"
		"Size is the size of the largest tile in pyramid. "
		"it must be a power of two.\n",
		argv[COMMAND]);
	return -1;
    }

    sscanf(argv[TOP_LEVEL_SIZE],"%d", &size);
    if(size & (size - 1)) {
	fprintf(stderr, "%s is not a power of 2\n", argv[TOP_LEVEL_SIZE]);
	return -1;
    }	
    for(; size > 1; size /= 2) {
	
	sprintf(infile, argv[SCANF_STRING], size),
	sprintf(outfile, argv[SCANF_STRING], size/2);
        sprintf(cmd, "izoom %s %s 0.5 0.5", infile, outfile);
	printf("%s\n", cmd);
        system(cmd);
    }
    return 0;

}
