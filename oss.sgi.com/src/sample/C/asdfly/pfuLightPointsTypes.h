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

#ifndef PFU_LIGHT_POINTS_TYPES_H
#define PFU_LIGHT_POINTS_TYPES_H

typedef struct
    {
    int                         nofLightPoints;

    float			*original_vertex;
    pfASDRegisteredVertex   	**registration;
    } pfASDTrackingLightPoints;

/*
 *      Hold this structure in the Group node controlling the Z-update. 
 */
typedef struct
    {
    /*
     *  This is where we keep information about light points. Fro each
     *  point we hold preprocessed info about its terrain position.
     */
    pfASDTrackingLightPoints      lightPoints;

    /*
     *  Hold pointer to the terrain structure for altitude queries.
     */

    /*
     *  The two Geosets containing light points.
     *  We use these in order to update the light point Z coordinates.
     */
    pfGeoSet                    *gset[2];

    /*
     *  Hold which buffer is currently active for drawing. The other buffer
     *  is the one we can update.
     */
    int                         active;

    /*
     *  For multi-frame update: how many frames for a single full pass
     */
    int                         nofPasses;
    } pfASDTrackingLightPointInfo;


typedef struct
{

    pfGeoSet		*gset;
    pfFlux		*asd_output;
    float		*v_attr, *c_attr, *t_attr;
    unsigned long	query_mask;

} pfuAppCallbackData;

typedef struct
{
    pfGeoSet            *gset;
    pfGeode		*geode;
    float		*v_attr, *t_attr, *c_attr, *n_attr;
    int			switch_id;  /* Index on the switch */
    int			max_nof_triangles;
    unsigned long	query_mask;
    pfGeoState		*gstate;
    pfBox		box;
    char		texture_filename[300];
} pfuShadowFluxData;

typedef struct
{
    pfGeoSet        *gset;
    float           *v_attr, *t_attr, *c_attr, *n_attr;
} pfuShadowGeometry;

typedef struct
{
    pfuShadowGeometry	*geometry;
    unsigned long	query_mask;
    pfFlux		*switch_flux;
    int			switch_count;
} pfuShadowFluxInfo;

#endif
