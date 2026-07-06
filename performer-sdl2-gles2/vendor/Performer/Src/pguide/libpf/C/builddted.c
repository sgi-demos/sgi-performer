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
 *
 */

#include <stdio.h>
#include <math.h>

#include <Performer/pf.h>
#include <Performer/pfdu.h>


#define TENTHS_ARCSEC_2_RADIANS 0.000000484813681109535

typedef struct {
    char title[80];               /* Heading title info */
    char dsi_tag[3];              /* 'DSI' sentinel tag */
    char security_class[1];       /* Security classification */
    char security_mark[2];        /* Security control & release mark */
    char security_desc[27];       /* Security handling description */
    char reserved1[26];
    char level[5];                /* DMA series designator for level */
    char ref_num[15];             /* Reference number */
    char reserved2[8];
    char edition[2];              /* Data edition */
    char merge_version[1];        /* Match/merge version */
    char maintenance_date[4];     /* Maintenance date (YYMM) */
    char merge_date[4];           /* Match/Merge date (YYMM) */
    char maintenance_desc[4];     /* Maintenance description */
    char producer[8];             /* Producer */
    char reserved3[16];
    char product_num[9];          /* Product specification stock number */
    char product_change[2];       /* Product specification change number */
    char product_date[4];         /* Product specification date (YYMM) */
    char vertical_datum[3];       /* Vertical datum */
    char horizontal_datum[5];     /* Horizontal datum */
    char collection_sys[10];      /* Digitizing collection system */
    char compilation_date[4];     /* Compilation date (YYMM) */
    char reserved4[22];
    char origin_lat[9];           /* Latitude of data origin */
    char origin_long[10];         /* Longitude of data origin */
    char sw_corner_lat[7];        /* Latitude of SW corner */
    char sw_corner_long[8];       /* Longitude of SW corner */
    char nw_corner_lat[7];        /* Latitude of NW corner */
    char nw_corner_long[8];       /* Longitude of NW corner */
    char ne_corner_lat[7];        /* Latitude of NE corner */
    char ne_corner_long[8];       /* Longitude of NE corner */
    char se_corner_lat[7];        /* Latitude of SE corner */
    char se_corner_long[8];       /* Longitude of SE corner */
    char orientation[9];          /* Orientation angle */
    char ns_spacing[4];           /* North-south data spacing (tenths sec) */
    char ew_spacing[4];           /* East-west data spacing (tenths sec) */
    char rows[4];                 /* Number of data rows */
    char cols[4];                 /* Number of data cols */
    char cell_coverage[2];        /* Partial cell indicator */
    char reserved5[357];
} DTED_header_t;


/*
 *  This function converts geodetic coordinates (WGS-84 datum)
 *  to Cartesian coordinates (earth-centered, earth-fixed).
 *
 *  Z-axis points toward the North Pole; X-axis is defined by
 *  the intersection of the plane defined by the prime meridian
 *  and the equatorial plane; Y-axis completes a right handed
 *  coordinate system by a plane 90 degrees east of the X-axis
 *  and its intersection with the equator.
 *
 *  Note that the latitute and longitude values are expected
 *  to be in radians, other values are in meters!
 */

#define WGS84_ES  0.00669437999013   /* Eccentricity squared */
#define WGS84_EER 6378137.0 /* Ellipsoid equatorial radius (semi-major axis) */

void WGS84_to_Cartesian(double latitude, double longitude, double height,
                        double cartesian[3])
{
   double coslat, sinlat;
   double N;                  /* radius of vertical in prime meridian */

   coslat = cos(latitude);
   sinlat = sin(latitude);

   N = WGS84_EER / sqrt(1.0 - WGS84_ES*sinlat*sinlat);

   cartesian[0] = (N + height) * coslat * cos(longitude);
   cartesian[1] = (N + height) * coslat * sin(longitude);
   cartesian[2] = (N*(1.0 - WGS84_ES) + height) * sinlat;
}


static void
Usage (void)
{
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: builddted file.dted tile_pre_name config_filename pagelookahead_filename\n");
    exit(1);
}

void
main(int argc, char *argv[])
{
    FILE *dtedFile;
    DTED_header_t fileheader;
    unsigned int num_rows, num_cols;
    pfVec3 *theData;
    int orig_lat_tas, orig_long_tas;    /* Geodetic origin in 10ths sec. */
    double orig_lat, orig_long;         /* Geodetic origin in radians */
    double origin[3];                   /* Cartesian origin */
    int ns_tas, ew_tas;                 /* Spacing in 10ths sec. */
    double ns, ew;                      /* Spacing in radians */
    double x, y;
    int i, j;
    short *inBuf;                       /* Assumes shorts are 2 bytes! */
    double cart[3];
    int t1, t2;

    int lookahead[2];


    /* check argument */
    if (argc < 5)
        Usage();

    pfInit();

    if ((dtedFile = pfdOpenFile(argv[1])) == NULL)
	return;

    /* Read the DTED file header into a struct */

    if (fread(&fileheader, sizeof(DTED_header_t), 1, dtedFile) != 1)
    {
	 pfNotify(PFNFY_WARN, PFNFY_PRINT,
	          "builddted: Error reading DTED file header");
	return;
    }

    /* Skip the DTED file accuracy information */

    fseek(dtedFile, 2700, SEEK_CUR);

    sscanf(fileheader.rows, "%4d", &num_rows);
    sscanf(fileheader.cols, "%4d", &num_cols);

    if (strncmp(fileheader.orientation, "0000000.0", 9))
	pfNotify(PFNFY_WARN, PFNFY_PRINT,
	   "builddted: Unsupported orientation angle.");

    theData = (pfVec3 *) pfMalloc(sizeof(pfVec3) * num_cols * num_rows, NULL);

    sscanf(fileheader.origin_lat, "%6d", &t1);
    sscanf(&(fileheader.origin_lat[7]), "%1d", &t2);
    orig_lat_tas = (t1/10000)*36000 + ((t1%10000)/100)*600 + (t1%100)*10 + t2;
    if (fileheader.origin_lat[8] == 'S')
	orig_lat_tas *= -1;
    orig_lat = TENTHS_ARCSEC_2_RADIANS * orig_lat_tas;

    sscanf(fileheader.origin_long, "%7d", &t1);
    sscanf(&(fileheader.origin_long[8]), "%1d", &t2);
    orig_long_tas = (t1/10000)*36000 + ((t1%10000)/100)*600 + (t1%100)*10 + t2;
    if (fileheader.origin_long[9] == 'W')
	orig_long_tas *= -1;
    orig_long = TENTHS_ARCSEC_2_RADIANS * orig_long_tas;

    WGS84_to_Cartesian(orig_lat, orig_long, 0.0, origin);

    sscanf(fileheader.ns_spacing, "%4d", &ns_tas);
    sscanf(fileheader.ew_spacing, "%4d", &ew_tas);

    ns = TENTHS_ARCSEC_2_RADIANS * ns_tas;
    ew = TENTHS_ARCSEC_2_RADIANS * ew_tas;

    inBuf = (short *) pfMalloc(sizeof(short) * num_rows, NULL);

    for (x=orig_long, i=0; i<num_cols; x+=ew, ++i)
    {
	fseek(dtedFile, 8, SEEK_CUR);            /* Skip block count fields */
	fread(inBuf, sizeof(short), num_rows, dtedFile);
	fseek(dtedFile, 4, SEEK_CUR);            /* Skip checksum field */

	for (y=orig_lat, j=0; j<num_rows; y+=ns, ++j)
	{
            /* First convert signed-magnitude format to twos-complement */

	    if (inBuf[j] < 0)
                inBuf[j] = - (inBuf[j] & 0x7fff);

	    WGS84_to_Cartesian(y, x, inBuf[j], cart);
	    theData[j*num_cols+i][PF_X] = cart[0] - origin[0];
	    theData[j*num_cols+i][PF_Y] = cart[1] - origin[1];
	    theData[j*num_cols+i][PF_Z] = cart[2] - origin[2];
	}
    }

    pfFree(inBuf);
    fclose(dtedFile);

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "builddted: %s", argv[1]);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "Dimensions: %d x %d vertices",
        num_cols, num_rows);

    lookahead[0] = lookahead[1] = 6;
    pfdBuildASD(num_cols, num_rows, theData, 0, 9, 1, argv[2], argv[3], argv[4], lookahead);
 
    pfFree(theData);

}

