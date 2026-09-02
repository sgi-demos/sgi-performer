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
#ifdef WIN32
#include <float.h>
#endif

#include <Performer/pf.h>
#include <Performer/pfdu.h>


#define LDROUND(d) ((d < 0.0) ? ((long)(d-0.5)) : ((long)(d+0.5)))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef struct {
    char filename[144];      /* DEM file name and title info */
    char level[6];           /* DEM Level Code (1,2, or 3)   */
    char pattern[6];         /* Elevation pattern (regular or random) */
    char ground[6];          /* Ground planimetric reference system code */
    char zone[6];            /* Ground planimetric zone code */
    char projection[15][24]; /* Map projection parameters */
    char ground_unit[6];     /* Unit of measure for ground coordinates */
    char elev_unit[6];       /* Unit of measure for elevation coordinates */
    char num_sides[6];       /* Number of sides in coverage polygon */
    char corners[4][2][24];  /* Ground coordinates of the four corners */
    char minmax[2][24];      /* Minimum and Maximum elevations */
    char angle[24];          /* Angle to align local reference system */
    char accuracy[6];        /* Accuracy code for elevations */
    char resolution[3][12];  /* DEM spatial resolution */
    char rows_cols[2][6];    /* Number of rows and columns of profiles */
    char filler[160];        /* Assume DEM header occupies a full 1K ? */
} DEM_header_t;


typedef struct{
    double easting, northing;  /* Coords. of first point in profile */
    double datum;              /* Elevation of local datum for profile */
    int num;                   /* Number of elevations in the profile */
    int *elev;                 /* Array of elevations in the profile */
    long minp;
} prof_t;


int chars2int(char *s, int num)
{
    char *news;
    int i;

    news = (char *) pfMalloc(sizeof(char) * (num+1), NULL);
    strncpy(news, s, num);
    news[num] = '\0';
    sscanf(news, "%d", &i);
    pfFree(news);
    return(i);
}


double chars2double(char *s, int num)
{
    char *news, *t;
    double d;

    news = (char *) pfMalloc(sizeof(char) * (num+1), NULL);
    strncpy(news, s, num);
    news[num] = '\0';
    if (t = strchr(news, 'D'))
       *t = 'E';
    sscanf(news, "%lE", &d);
    pfFree(news);
    return(d);
}


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
    pfNotify(PFNFY_FATAL, PFNFY_USAGE, "Usage: builddem file.dem tile_pre_name config_filename pagelookahead_filename\n");
    exit(1);
}

void
main(int argc, char *argv[])
{
    FILE *demFile;
    DEM_header_t fileheader;
    int ground_unit, elev_unit;
    int geo;
    double xyMult, zMult;
    double sw_easting, sw_northing;
    double min_elev;
    double angle;
    double xres, yres, zres;
    int rows, cols;
    prof_t *prof;
    int seq;
    int prows, pcols;
    char ins[3][30];
    long minp=LONG_MAX;
    long maxp=LONG_MIN;
    double miny=DBL_MAX;
    unsigned int eindex;
    double orig[3];
    int i, j;
    pfVec3 *theData;
    double xval, yval;
    double cart[3];
    char fname[100];
    FILE *f;

    int lookahead[2];


    /* check argument */
    if (argc < 5)
        Usage();

    pfInit();

    if ((demFile = pfdOpenFile(argv[1])) == NULL)
	return;

    if (fread(&fileheader, sizeof(DEM_header_t), 1, demFile) != 1)
    {
        pfNotify(PFNFY_WARN, PFNFY_PRINT,
	         "builddem:  Error reading DEM file header");
        return;
    }

    ground_unit = chars2int(fileheader.ground_unit, 6);

    switch (ground_unit) 
    {
	case 0:           /* Radians */
	    geo = TRUE;
	    xyMult = 1.0;
	    break;
	case 1:           /* Feet */
	    geo = FALSE;
	    xyMult = 0.3048;
	    break;
	case 2:           /* Meters */
	    geo = FALSE;
	    xyMult = 1.0;
	    break;
	case 3:           /* Arc-seconds */
	    geo = TRUE;
	    xyMult = .00000484813681109535;
	    break;
	default:
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "builddem:  Unsupported ground planimetric coordinate unit code: %d", ground_unit);
	    return;
    }

    elev_unit = chars2int(fileheader.elev_unit, 6);

    switch (elev_unit)
    {
	case 1:                /* Feet */
	    zMult = 0.3048;
	    break;
	case 2:                /* Meters */
	    zMult = 1.0; 
	    break;
	default:
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "builddem:  Unsupported elevation coordinate unit code: %d", elev_unit);
	    return;
    }

    sw_easting  = chars2double(fileheader.corners[0][0], 24);
    sw_northing = chars2double(fileheader.corners[0][1], 24);

    min_elev = chars2double(fileheader.minmax[0], 24);

    angle = chars2double(fileheader.angle, 24);

    if (angle != 0.0)
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "builddem:  Unsupported orientation angle: %f", angle);

    xres = chars2double(fileheader.resolution[0], 12);
    yres = chars2double(fileheader.resolution[1], 12);
    zres = chars2double(fileheader.resolution[2], 12);

    rows = chars2int(fileheader.rows_cols[0], 6);
    cols = chars2int(fileheader.rows_cols[1], 6);

    if (rows != 1 || cols < 0)
	pfNotify(PFNFY_WARN, PFNFY_PRINT, "builddem:  Unsupported dimensions for array of profiles: %d x %d\n", rows, cols);

    prof = (prof_t *) pfMalloc(sizeof(prof_t) * cols, NULL);

    for (i=0; i<cols; ++i)
    {
	if (fscanf(demFile, "%*d%d%d%d%s%s%s%*s%*s", &seq, &prows, &pcols,
            ins[0], ins[1], ins[2]) != 6)
	       pfNotify(PFNFY_WARN, PFNFY_PRINT,
	       "builddem: Error reading profile #%d in DEM file header",
	       i+1);

        if (seq != i+1)
            pfNotify(PFNFY_WARN, PFNFY_PRINT,
	      "builddem:  Encountered profile #%d out of sequence", seq);

        if (pcols != 1)
	    pfNotify(PFNFY_WARN, PFNFY_PRINT, "builddem:  Unsupported number of columns (%d) in profile %d", pcols, i+1);

        prof[i].num = prows;

	prof[i].elev = (int *) pfMalloc(sizeof(int) * prows, NULL);

	prof[i].easting  = chars2double(ins[0], strlen(ins[0]));
	prof[i].northing = chars2double(ins[1], strlen(ins[1]));
	prof[i].datum    = chars2double(ins[2], strlen(ins[2]));

	for (j=0; j<prows; ++j)
	    fscanf(demFile, "%d", &(prof[i].elev[j]));

        prof[i].minp = LDROUND(prof[i].northing / yres);

        minp = MIN(minp, prof[i].minp);
	maxp = MAX(maxp, prof[i].minp + prof[i].num - 1);
	miny = MIN(miny, prof[i].northing);
    }

    fclose(demFile);

    rows = (maxp - minp) + 1;

    theData = (pfVec3 *) pfMalloc(sizeof(pfVec3) * cols * rows, NULL);

    if (geo)
        WGS84_to_Cartesian(miny*xyMult, prof[0].easting*xyMult, 0.0, orig);
    else 
    {
	orig[0] = prof[0].easting*xyMult;
	orig[1] = miny*xyMult;
	orig[2] = 0.0;
    }
	
    for (i=0; i<cols; ++i)
    {
	xval = prof[i].easting;

	for (j=0, yval=miny; j<rows; ++j, yval+=yres)
	{
	    if ((j<prof[i].minp-minp) || (j>prof[i].minp+prof[i].num-1-minp))
	    {
		if (geo)  {
		    WGS84_to_Cartesian(yval*xyMult, xval*xyMult,
				       prof[i].datum * zMult, cart);
                    theData[cols*j+i][PF_X] = cart[0] - orig[0];
                    theData[cols*j+i][PF_Y] = cart[1] - orig[1];
                    theData[cols*j+i][PF_Z] = cart[2] - orig[2];
		}
		else
		{
                    theData[cols*j+i][PF_X] = xval*xyMult - orig[0];
                    theData[cols*j+i][PF_Y] = yval*xyMult - orig[1];
                    theData[cols*j+i][PF_Z] = prof[i].datum * zMult - orig[2];
		}
	    }
	    else
	    {
		eindex = j - (prof[i].minp - minp);

		if (geo)  {
		    WGS84_to_Cartesian(yval*xyMult, xval*xyMult, zMult*
		        (prof[i].elev[eindex]*zres + prof[i].datum), cart);
                    theData[cols*j+i][PF_X] = cart[0] - orig[0];
                    theData[cols*j+i][PF_Y] = cart[1] - orig[1];
                    theData[cols*j+i][PF_Z] = cart[2] - orig[2];
		}
		else
		{
                    theData[cols*j+i][PF_X] = xval*xyMult - orig[0];
                    theData[cols*j+i][PF_Y] = yval*xyMult - orig[1];
                    theData[cols*j+i][PF_Z] = zMult *
			(prof[i].elev[eindex]*zres + prof[i].datum) - orig[2];
		}
	    }
	}
	pfFree(prof[i].elev);
    }

    pfFree(prof);

    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "builddem: %s", argv[1]);
    pfNotify(PFNFY_INFO,   PFNFY_MORE,  "Dimensions: %d x %d vertices",
        cols, rows);

    lookahead[0] = lookahead[1] = 6;
    pfdBuildASD(cols, rows, theData, 0, 10, 1, argv[2], argv[3], argv[4], lookahead);

    pfFree(theData);
    /* print statistics */
}

