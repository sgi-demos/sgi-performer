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
** rsets is used to build all the clipmap data files from either a single
** level 0 file (if the filename argument is provided) or from collection of
** level 0 icache files in rgb format (if no filename is provided).
**
** If given a filename argument, it firsts divides that file into subfiles
** of tilesize, using the format string argument to name the files
** properly.
**
** If no filename is given, it assumes that tile files for the top level
** exists, and proceeds to create the rest of the data files.
**
** It combines four adjacent tiles,
** then shrinks 50% down to the clipmap level. When it gets to the clipmap
** level, it shrinks each level down the pyramid until it makes a 1x1 file.
**
** rsets accepts the following parameters in order:
** 
** filename - if this argument is supplied, it is the name of a single
**            data file containing level zero as a single tile. The program
**	      will break this file up into individual image files of size
**            tilesize, using the format string to compute the file names.
**
** format string - a scanf-style string describing the data file filenames.
**              The string should contin three parameters, in this order:
**		level size, row tile coordinate, column tile coordinate.
**
** clip texture size - size of level 0 of clip texture, in texels. 
**              All levels must be the same in both dimensions, and be
**              a power of 2.
**
** tile size - size of each tile in texels. All tiles must be square,
**		the same size, and a power of 2.
**     
**
** Example: rsets hl.%d.r%03d.c%03d.rgb 4096 512
*/

#include<stdio.h>

int main(int argc, char *argv[])
{
    int i, j, k;
    int clipsize, tilesize;
    char *fname;
    char *format;
    int maxtiles;
    char cmd[256];
    char infile[256], outfile[256];

    switch(argc) {
    case 4:
	format = argv[1];
	clipsize = atoi(argv[2]);
	tilesize = atoi(argv[3]);
	break;
    case 5:
	fname = argv[1];
	format = argv[2];
	clipsize = atoi(argv[3]);
	tilesize = atoi(argv[4]);
	break;
    default:
	fprintf(stderr, 
		"Usage: %s [datafile] fnameformat clipmapsize tilesize\n\n"
		"Use datafile if all data is currently stored in a single\n"
		"file. Otherwise, data must be in files matching fnameformat\n"
		"fnameformat is a scanfstyle string with 3 numeric arguments:\n"
		"levelsize, tile row, tile column\n"
		"clipmapsize is the level 0 clipmap size in texels\n"
		"tilesize is the size of each tile in texels\n"	,
		argv[0]);
	return -1;
    }

    /* Parameter checking */

    if(tilesize >= clipsize) {
	fprintf(stderr, "Tilesize %d, must be smaller than clipsize %d\n",
		tilesize, clipsize);
	return -1;
    }

    if(tilesize & (tilesize - 1)) {
	fprintf(stderr, "Tilesize %d, must be a power of two\n",
		tilesize);
	return -1;
    }

    if(clipsize & (clipsize - 1)) {
	fprintf(stderr, "Clipsize %d, must be a power of two\n",
		clipsize);
	return -1;
    }

    /* If one big data file, break it up into pieces */
    if(argc == 5) {
	maxtiles = clipsize/tilesize;
	for(i = 0; i < maxtiles; i++) {
	    for(j = 0; j < maxtiles; j++) {
		sprintf(outfile, format, clipsize, j, i);
		sprintf(cmd, "subimg %s %s %d %d %d %d",
			fname, outfile, 
			i * tilesize,
			(i + 1) * tilesize - 1,
			j * tilesize,
			(j + 1) * tilesize - 1);
		printf("%s\n", cmd);
		system(cmd);
	    }
	}
    }


    /* Create the levels */
    for(; clipsize > tilesize; clipsize /= 2) {
	maxtiles = clipsize/(tilesize * 2); /* dimensions of next level down */
	for(i = 0;i < maxtiles; i++) {
	    for(j = 0;j < maxtiles; j++) {
		/*
		** choose 4 adjacent tiles, and create 4 temporary files
		** create 4 temporary files of half the size.
		*/
		sprintf(infile,  format, clipsize, j * 2 , i * 2);
		sprintf(cmd, "izoom %s tmp_ll.rgb 0.5 0.5", infile);
		printf("%s\n", cmd);
		system(cmd);

		sprintf(infile,  format, clipsize, j * 2 , i * 2 + 1);
		sprintf(cmd, "izoom %s tmp_lr.rgb 0.5 0.5", infile);
		printf("%s\n", cmd);
		system(cmd);

		sprintf(infile,  format, clipsize, j * 2 + 1 , i * 2);
		sprintf(cmd, "izoom %s tmp_ul.rgb 0.5 0.5", infile);
		printf("%s\n", cmd);
		system(cmd);

		sprintf(infile,  format, clipsize, j * 2 + 1 , i * 2 + 1);
		sprintf(cmd, "izoom %s tmp_ur.rgb 0.5 0.5", infile);
		printf("%s\n", cmd);
		system(cmd);

		/*
		** Combine 4 1/2 size temporary files to create a new
		** file for the next level down.
		*/
		sprintf(outfile, format, clipsize/2, j, i);
		sprintf(cmd, 
			"assemble 2 2 %s "
			"tmp_ll.rgb tmp_lr.rgb tmp_ul.rgb tmp_ur.rgb",
			outfile);
		printf("%s\n", cmd);
		system(cmd);

	    }
	}
    }
    /* Remove temporary files */
    sprintf(cmd, "rm -f tmp_ll.rgb tmp_lr.rgb tmp_ul.rgb tmp_ur.rgb");
    printf("%s\n", cmd);
    system(cmd);

    /*
    ** Make the pyramid tiles
    */
    for(; clipsize > 1; clipsize /= 2) {
	
	sprintf(infile, format, clipsize, 0, 0),
	sprintf(outfile, format, clipsize/2, 0, 0);
        sprintf(cmd, "izoom %s %s 0.5 0.5", infile, outfile);
	printf("%s\n", cmd);
        system(cmd);
    }

    return 0;
}
