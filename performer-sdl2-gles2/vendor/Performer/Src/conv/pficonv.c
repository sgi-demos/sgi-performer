/*
 * Copyright 1997, Silicon Graphics, Inc.
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

/*
 *  file: pficonv.c		$Revision: 1.6 $
 *  --------------
 *
 *  utility for converting .rgb image files to .pfi image files.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef WIN32
#include <Performer/pfutil/getopt.h>
#else
#include <getopt.h>
#endif

#include <Performer/pfdu.h>

int save_mipmaps = 0;
int format = 0;
int alignment = -1;

int format_table[] =
{
    12,
    PFTEX_UNSIGNED_BYTE,
    PFTEX_BYTE,
    PFTEX_UNSIGNED_SHORT,
    PFTEX_SHORT,
    PFTEX_UNSIGNED_INT,
    PFTEX_INT,
    PFTEX_FLOAT,
    PFTEX_UNSIGNED_BYTE_3_3_2,
    PFTEX_UNSIGNED_SHORT_4_4_4_4,
    PFTEX_UNSIGNED_SHORT_5_5_5_1,
    PFTEX_UNSIGNED_INT_8_8_8_8,
    PFTEX_UNSIGNED_INT_10_10_10_2,
};


/*
 *  local prototypes
 */
static int parse_args(int argc, char *argv[], char ***files);
static void usage(char *prog_name);


main(int argc, char **argv)
{
    char **files;
    int i, num_files;
    pfdImage *image, *itmp;

    if ((num_files = parse_args(argc, argv, &files)) < 2)
	usage(argv[0]);

    /*
     *  initialize OpenGL Performer
     */
    pfInit();
    pfInitArenas();
    pfNotifyLevel(PFNFY_INFO);
    pfFilePath("/usr/share/Performer/data:"
	       "/usr/demos/models:"
	       "/usr/demos/data/flt:"
	       "/usr/demos/data/textures");

    if ((image = pfdLoadImage(files[0])) == NULL)
	exit(1);

    if (num_files > 2)
    {
	for (i = 1, itmp = image; i < num_files-1; i++)
	{
	    if ((itmp->next = pfdLoadImage(files[i])) == NULL)
		exit(1);
	    itmp = itmp->next;
	}
    }

    if (save_mipmaps)
	pfdImageGenMipmaps(image, NULL, NULL, NULL);

    if (format)
	pfdImagePack(image, format_table[format]);

    if (alignment != -1)
	pfdImageAlignment(image, alignment);

    pfdStoreImage(image, files[num_files-1]);
}


/*
 *  parse commnand line arguments
 */
static int parse_args(int argc, char *argv[], char ***files)
{
    int opt;

    /*
     *  process command-line arguments
     */
    while ((opt = getopt(argc, argv, "a:f:mk:")) != -1)
    {
	switch (opt)
	{
	    case 'a':
		alignment = atoi(optarg);
		if (alignment < 0)
		    usage(argv[0]);
		break;
	    case 'f':
		format = atoi(optarg);
		if (format < 1 || format > 12)
		    usage(argv[0]);
		break;
	    case 'm':			/* save mipmaps */
		save_mipmaps = 1;
		break;
	    case 'k':		/* right half of symmetric separable kernel */
#if 0
		assert(10 <= MAX_HALF_KERNEL_SIZE);
		half_kernel_size = sscanf(optarg,
			"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
			&half_kernel[0],
			&half_kernel[1],
			&half_kernel[2],
			&half_kernel[3],
			&half_kernel[4],
			&half_kernel[5],
			&half_kernel[6],
			&half_kernel[7],
			&half_kernel[8],
			&half_kernel[9]);
		if (half_kernel_size < 1)
		    usage(argv[0]);
#endif
		break;
	    default:
		usage(argv[0]);
	}
    }

    *files = &(argv[optind]);   /* return address of filename list */

    return(argc - optind);      /* number of files */
}


/*
 *  print usage
 */
static void usage(char *prog_name)
{
    char *base_name;
    static const char *help_list =
	"usage:\n"
	"       %s [options] infile1 [infile2 ...] outfile\n"
	"\n"
	"options:\n"
	"    -a <alignment>          -Set alignment.\n"
	"    -f <format>             -Set packing format.\n"
	"        1 -> PFTEX_UNSIGNED_BYTE\n"
	"        2 -> PFTEX_BYTE\n"
	"        3 -> PFTEX_UNSIGNED_SHORT\n"
	"        4 -> PFTEX_SHORT\n"
	"        5 -> PFTEX_UNSIGNED_INT\n"
	"        6 -> PFTEX_INT\n"
	"        7 -> PFTEX_FLOAT\n"
	"        8 -> PFTEX_UNSIGNED_BYTE_3_3_2\n"
	"        9 -> PFTEX_UNSIGNED_SHORT_4_4_4_4\n"
	"       10 -> PFTEX_UNSIGNED_SHORT_5_5_5_1\n"
	"       11 -> PFTEX_UNSIGNED_INT_8_8_8_8\n"
	"       12 -> PFTEX_UNSIGNED_INT_10_10_10_2\n"
	"    -m                      -Generate mipmaps.\n"
	"    -k <right half of symmetric separable convolution kernel>\n"
	"";

    /*
     *  just use program's base name: /usr/fred/base -> base
     */
    if (prog_name == NULL)
	base_name = "UNKNOWN";
    else if ((base_name = strrchr(prog_name, '/')) == NULL)
	base_name = prog_name;
    else
	base_name++;

    /*
     *  print list of command-line options and arguments
     */
    fprintf(stderr, help_list, base_name);
    fflush(stderr);
    exit(1);
}
