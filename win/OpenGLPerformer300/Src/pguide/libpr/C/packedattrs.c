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
 * vertarray.c: simple Performer program to demonstrate use of vertex arrays
 *
 * $Revision: 1.5 $ 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <Performer/pr.h>

#define MAX_SHORT 32767
#define MAX_SHORT_INV (1.0f/MAX_SHORT)

pfVec3          coords[] ={     {-1.0f,  -1.0f,  1.0f },
                                { 1.0f,  -1.0f,  1.0f },
                                { 1.0f,  1.0f,   1.0f },
                                {-1.0f,  1.0f,   1.0f } };

pfVec4		colors[] = {{1.0f, 1.0f, 1.0f, 1.0f}};

pfVec3		norms[] = {{0.0f, 0.0f, -1.0f}};

uint          pcolors[] = {0xffffffff,
				0x0000ffff,
				0xff0000ff,
				0x00ff00ff};

short		pnorms[][4] = {
			{0, 0, -MAX_SHORT, 0},
			{0, 0, -MAX_SHORT, 0},
			{0, 0, -MAX_SHORT, 0},
			{0, 0, -MAX_SHORT, 0}
};

pfVec2		texcoords[] = {
			{0, 0},
			{1, 0},
			{1, 1},
			{0, 1}
};

float		ptexcoords[][2] = {
			{0, 0},
			{1.0f, 0},
			{1.0f, 1.0f},
			{0, 1.0f}
};

short		sptexcoords[][2] = {
			{0, 0},
			{MAX_SHORT, 0},
			{MAX_SHORT, MAX_SHORT},
			{0, MAX_SHORT}
};

pfMatrix	stmat = {
	{MAX_SHORT_INV, 0.0f, 0.0f, 0.0f},
	{0.0f, MAX_SHORT_INV, 0.0f, 0.0f},
	{0.0f, 0.0f, MAX_SHORT_INV, 0.0f},
	{0.0f, 0.0f, 0.0f, 1.0f},
};


pfVec3          stripcoords[4][4] ={     
		    {{-1.0f,  -1.0f,  1.0f },
		     {-1.0f,  1.0f,   1.0f }, 
		     {-0.5f,  -1.0f,  1.0f },
		     {-0.5f,  1.0f,   1.0f }}, 
		    {{-0.5f,  -1.0f,  1.0f },
		     {-0.5f,  1.0f,   1.0f }, 
		     {0.0f,  -1.0f,  1.0f },
		     {0.0f,  1.0f,   1.0f }}, 
		    {{0.0f,  -1.0f,  1.0f },
		     {0.0f,  1.0f,   1.0f }, 
		     {0.5f,  -1.0f,  1.0f },
		     {0.5f,  1.0f,   1.0f }, 
		     }, 
		    {{0.5f,  -1.0f,  1.0f },
		     {0.5f,  1.0f,   1.0f }, 
		     { 1.0f,  -1.0f,  1.0f },
		     { 1.0f,  1.0f,   1.0f }},
		     };
int lengths[5] = {4, 4, 4, 4};

#define NUM_GSETS 11

int main (void)
{
    pfGeoSet	*gset[NUM_GSETS];
    pfGeoState  *gstate[NUM_GSETS];
    pfFrustum	*frust;
    pfWindow	*win;
    pfTexture   *tex;
    pfLight   	*lt;
    uint	i, j, t;
    float       time;
    void	*cntv, *cntvp, *cnt, *cntp, *strip, *stripp, *sstrip, *sstripp;

    pfInit();
    /* Initialize Performer */
    pfInitState(NULL);

    pfFilePath(".:/usr/share/Performer/data:/usr/demos/data/textures");
    
    /* Initialize GL */
    win = pfNewWin(NULL);
    pfWinOriginSize(win, 0, 0, 300, 300);
    pfWinName(win, "Performer packed attribute test");
    pfWinType(win, PFWIN_TYPE_X);
    pfOpenWin(win);
    pfInitGfx();
    
    frust = pfNewFrust(NULL);
    pfApplyFrust(frust);
    pfDelete(frust);
    
    pfAntialias(PFAA_ON);
    pfCullFace(PFCF_OFF);

    pfApplyTEnv(pfNewTEnv(NULL));
    pfApplyMtl(pfNewMtl(NULL));
    pfApplyLModel(pfNewLModel(NULL));
    lt = pfNewLight(NULL);
    pfLightPos(lt, 0, 0, -1, 0);
    pfLightOn(lt);

    tex = pfNewTex(NULL);
    pfLoadTexFile(tex, "brick.rgba");
    
    cntp = cnt = (void *)pfCalloc(4, 8 * sizeof(float), NULL);
    cntvp = cntv = (void *)pfCalloc(4, 12 * sizeof(float), NULL);
    stripp = strip = (void *)pfCalloc(4, 4*12 * sizeof(float), NULL);
    sstripp = sstrip = (void *)pfCalloc(4, 4*12 * sizeof(float), NULL);
    
    /* set up data for quads */
    for (i=0; i < 4; i++)
    {
	*((uint*)cntp) = (uint) pcolors[i];
	cntp = (void*) (((char*)cntp) + sizeof(int));
	*((short*)cntp) = pnorms[i][0];
	cntp = (void*) (((char*)cntp) + sizeof(short));
	*((short*)cntp) = pnorms[i][1];
	cntp = (void*) (((char*)cntp) + sizeof(short));
	*((short*)cntp) = pnorms[i][2];
	cntp = (void*) (((char*)cntp) + sizeof(short));
	cntp = (void*) (((char*)cntp) + sizeof(short));
	*((float*)cntp) = ptexcoords[i][0];
	cntp = (void*) (((char*)cntp) + sizeof(float));
	*((float*)cntp) = ptexcoords[i][1];
	cntp = (void*) (((char*)cntp) + sizeof(float));
    }
    for (i=0; i < 4; i++)
    {
	*((long*)cntvp) = pcolors[i];
	cntvp = (void*) (((char*)cntvp) + sizeof(int));
	*((short*)cntvp) = pnorms[i][0];
	cntvp = (void*) (((char*)cntvp) + sizeof(short));
	*((short*)cntvp) = pnorms[i][1];
	cntvp = (void*) (((char*)cntvp) + sizeof(short));
	*((short*)cntvp) = pnorms[i][2];
	cntvp = (void*) (((char*)cntvp) + sizeof(short));
	cntvp = (void*) (((char*)cntvp) + sizeof(short)); /* pad to word-align next attr */
	*((float*)cntvp) = ptexcoords[i][0];
	cntvp = (void*) (((char*)cntvp) + sizeof(float));
	*((float*)cntvp) = ptexcoords[i][1];
	cntvp = (void*) (((char*)cntvp) + sizeof(float));
	*((float*)cntvp) = coords[i][0];
	cntvp = (void*) (((char*)cntvp) + sizeof(float));
	*((float*)cntvp) = coords[i][1];
	cntvp = (void*) (((char*)cntvp) + sizeof(float));
	*((float*)cntvp) = coords[i][2];
	cntvp = (void*) (((char*)cntvp) + sizeof(float));
    }
    
    /* set up data for tstrip */
    for (j=0; j < 4; j++)
    {
        for (i=0; i < 4; i++)
	{
	switch(i)
	{
	    case 0: t = 0; break;
	    case 1: t = 3; break;
	    case 2: t = 1; break;
	    case 3: t = 2; break;
	}
	*((long*)stripp) = pcolors[i];
	stripp = (void*) (((char*)stripp) + sizeof(int));
	*((short*)stripp) = pnorms[i][0];
	stripp = (void*) (((char*)stripp) + sizeof(short));
	*((short*)stripp) = pnorms[i][1];
	stripp = (void*) (((char*)stripp) + sizeof(short));
	*((short*)stripp) = pnorms[i][2];
	stripp = (void*) (((char*)stripp) + sizeof(short)); 
	stripp = (void*) (((char*)stripp) + sizeof(short)); /* pad to word-align next attr */
	*((float*)stripp) = ptexcoords[t][0];
	stripp = (void*) (((char*)stripp) + sizeof(float));
	*((float*)stripp) = ptexcoords[t][1];
	stripp = (void*) (((char*)stripp) + sizeof(float));
	*((float*)stripp) = stripcoords[j][i][0];
	stripp = (void*) (((char*)stripp) + sizeof(float));
	*((float*)stripp) = stripcoords[j][i][1];
	stripp = (void*) (((char*)stripp) + sizeof(float));
	*((float*)stripp) = stripcoords[j][i][2];
	stripp = (void*) (((char*)stripp) + sizeof(float));
	}
    }

    /* set up data for short texcoord tstrip */
    for (j=0; j < 4; j++)
    {
        for (i=0; i < 4; i++)
	{
	switch(i)
	{
	    case 0: t = 0; break;
	    case 1: t = 3; break;
	    case 2: t = 1; break;
	    case 3: t = 2; break;
	}
	*((long*)sstripp) = pcolors[i];
	sstripp = (void*) (((char*)sstripp) + sizeof(int));
	*((short*)sstripp) = pnorms[i][0];
	sstripp = (void*) (((char*)sstripp) + sizeof(short));
	*((short*)sstripp) = pnorms[i][1];
	sstripp = (void*) (((char*)sstripp) + sizeof(short));
	*((short*)sstripp) = pnorms[i][2];
	sstripp = (void*) (((char*)sstripp) + sizeof(short)); /* pad to word-align next attr */
	sstripp = (void*) (((char*)sstripp) + sizeof(short)); 
	*((short*)sstripp) = sptexcoords[t][0]; /* short tcoord */
	sstripp = (void*) (((char*)sstripp) + sizeof(short));
	*((short*)sstripp) = sptexcoords[t][1]; /* short tcoord */
	sstripp = (void*) (((char*)sstripp) + sizeof(short));
	*((float*)sstripp) = stripcoords[j][i][0];
	sstripp = (void*) (((char*)sstripp) + sizeof(float));
	*((float*)sstripp) = stripcoords[j][i][1];
	sstripp = (void*) (((char*)sstripp) + sizeof(float));
	*((float*)sstripp) = stripcoords[j][i][2];
	sstripp = (void*) (((char*)sstripp) + sizeof(float));
	}
    }
    
    /* Set up geosets */
    for (i=0; i < NUM_GSETS; i++)
    {
	gset[i] = pfNewGSet(NULL);
	pfGSetAttr(gset[i], PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
	
	pfGSetPrimType(gset[i], PFGS_QUADS);
	pfGSetNumPrims(gset[i], 1);

	/* Set up a geostate, backface removal turned off */
	gstate[i] = pfNewGState (NULL);
	pfGStateMode(gstate[i], PFSTATE_CULLFACE, PFCF_OFF);
	pfGSetGState (gset[i], gstate[i]);
	pfGSetDrawMode (gset[i], PFGS_PACKED_ATTRS, 1);


	switch(i)
	{
	case 0: 
	case 1: 
	case 2: 
		pfGSetAttr(gset[i], PFGS_PACKED_ATTRS, PFGS_PA_C4UBN3ST2F, cnt, NULL); 
		pfGSetAttr(gset[i], PFGS_COLOR4, PFGS_PER_VERTEX, NULL, NULL);
		pfGSetAttr(gset[i], PFGS_TEXCOORD2, PFGS_PER_VERTEX, NULL, NULL);
		pfGSetAttr(gset[i], PFGS_NORMAL3, PFGS_PER_VERTEX, NULL, NULL);
		pfGStateAttr(gstate[i], PFSTATE_TEXTURE, tex);
		pfGStateMode(gstate[i], PFSTATE_ENTEXTURE, PF_ON);
		pfGStateMode(gstate[i], PFSTATE_ENLIGHTING, PF_ON);
		pfGStateMode(gstate[i], PFSTATE_ENTEXTURE, 1);
		break;
	case 3: 
		pfGSetAttr(gset[i], PFGS_COLOR4, PFGS_PER_VERTEX, NULL, NULL);
		pfGSetAttr(gset[i], PFGS_PACKED_ATTRS, PFGS_PA_C4UBN3ST2F, pcolors, NULL); 
		pfGStateMode(gstate[i], PFSTATE_ENLIGHTING, 0);
		pfGStateMode(gstate[i], PFSTATE_ENTEXTURE, 0);
		break;
	case 4: 
		pfGSetAttr(gset[i], PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
		pfGSetAttr(gset[i], PFGS_NORMAL3, PFGS_PER_VERTEX, NULL, NULL);
		pfGSetAttr(gset[i], PFGS_PACKED_ATTRS, PFGS_PA_C4UBN3ST2F, pnorms, NULL); 
		pfGStateMode(gstate[i], PFSTATE_ENLIGHTING, 1);
		pfGStateMode(gstate[i], PFSTATE_ENTEXTURE, 0);
		break;
	case 5: 
		pfGSetAttr(gset[i], PFGS_COLOR4, PFGS_OVERALL, colors, NULL);
		pfGSetAttr(gset[i], PFGS_TEXCOORD2, PFGS_PER_VERTEX, NULL, NULL);
		pfGSetAttr(gset[i], PFGS_PACKED_ATTRS, PFGS_PA_C4UBN3ST2F, ptexcoords, NULL); 
		pfGStateAttr(gstate[i], PFSTATE_TEXTURE, tex);
		pfGStateMode(gstate[i], PFSTATE_ENTEXTURE, PF_ON);
		pfGStateMode(gstate[i], PFSTATE_ENLIGHTING, PF_OFF);
		break;
	
	
	case 6: 
	case 7: 
	case 8: 
		pfGSetAttr(gset[i], PFGS_PACKED_ATTRS, PFGS_PA_C4UBN3ST2FV3F, cntv, NULL); 
		pfGSetAttr(gset[i], PFGS_COLOR4, PFGS_PER_VERTEX, NULL, NULL);
		pfGSetAttr(gset[i], PFGS_TEXCOORD2, PFGS_PER_VERTEX, NULL, NULL);
		pfGSetAttr(gset[i], PFGS_NORMAL3, PFGS_PER_VERTEX, NULL, NULL);
		pfGStateAttr(gstate[i], PFSTATE_TEXTURE, tex);
		pfGStateMode(gstate[i], PFSTATE_ENTEXTURE, PF_ON);
		pfGStateMode(gstate[i], PFSTATE_ENLIGHTING, 0);
		pfGStateMode(gstate[i], PFSTATE_ENTEXTURE, 1);
		break;
		
	case 9: /* this one is a tristrip */
		pfGSetAttr(gset[i], PFGS_PACKED_ATTRS, PFGS_PA_C4UBN3ST2FV3F, strip, NULL); 
		pfGSetAttr(gset[i], PFGS_COLOR4, PFGS_PER_VERTEX, NULL, NULL);
		pfGSetAttr(gset[i], PFGS_TEXCOORD2, PFGS_PER_VERTEX, NULL, NULL);
		pfGSetAttr(gset[i], PFGS_NORMAL3, PFGS_PER_VERTEX, NULL, NULL);
		pfGStateAttr(gstate[i], PFSTATE_TEXTURE, tex);
		pfGStateMode(gstate[i], PFSTATE_ENTEXTURE, PF_ON);
		pfGStateMode(gstate[i], PFSTATE_ENLIGHTING, 0);
		pfGStateMode(gstate[i], PFSTATE_ENTEXTURE, 1);
		pfGSetPrimType(gset[i], PFGS_TRISTRIPS);
		pfGSetPrimLengths(gset[i], lengths);
		pfGSetNumPrims(gset[i], 4);
		break;
		
	case 10: /* this one is a tristrip with short tcoords */
		pfGSetAttr(gset[i], PFGS_PACKED_ATTRS, PFGS_PA_C4UBN3ST2SV3F, sstrip, NULL); 
		pfGSetAttr(gset[i], PFGS_COLOR4, PFGS_PER_VERTEX, NULL, NULL);
		pfGSetAttr(gset[i], PFGS_TEXCOORD2, PFGS_PER_VERTEX, NULL, NULL);
		pfGSetAttr(gset[i], PFGS_NORMAL3, PFGS_PER_VERTEX, NULL, NULL);
		pfGStateAttr(gstate[i], PFSTATE_TEXTURE, tex);
		pfGStateMode(gstate[i], PFSTATE_ENTEXTURE, PF_ON);
		pfGStateMode(gstate[i], PFSTATE_ENLIGHTING, 0);
		pfGStateMode(gstate[i], PFSTATE_ENTEXTURE, 1);
		/* we must scale the short tcoords into a proper float range */
		pfGStateMode(gstate[i], PFSTATE_ENTEXMAT, 1);
		pfGStateAttr(gstate[i], PFSTATE_TEXMAT, stmat);
		pfGSetPrimType(gset[i], PFGS_TRISTRIPS);
		pfGSetPrimLengths(gset[i], lengths);
		pfGSetNumPrims(gset[i], 4);
		break;
	}
    }

    pfTranslate (0.0f, 0.0f, -30.0f);
    
    pfInitClock (0.0f);
    while (time < 5.f)
    {
	static pfVec4 clr = {0.1f, 0.0f, 0.4f, 1.0f};
	
	time = pfGetTime();
	pfClear(PFCL_COLOR | PFCL_DEPTH, clr);
	pfRotate (PF_Y, 1.0f);
	
	pfPushMatrix();
	
	pfTranslate (-2.5f, 0.0f, 0.0f);

#if 1
	/* draw the 3x3 block of quads */
	for (i=0; i < 3; i++)
        {
	    pfPushMatrix();
	    pfTranslate (0.0f, -2.5f, 0.0f);
	    for (j=0; j < 3; j++)
	    {
		pfDrawGSet(gset[i*3 + j]);
		pfTranslate (0.0f, 2.5f, 0.0f);
	    }
	    pfPopMatrix();
	    pfTranslate (2.5f, 0.0f, 0.0f);
	}
#endif	
	pfTranslate(-5.0f, -5.0f, 0.0f);
	/* draw tristrip across the bottom */
	pfPushMatrix();
	pfScale(3.0f, 1.0f, 1.0f);
	pfDrawGSet(gset[9]);
	pfPopMatrix();

	/* draw a short-texcoord tristrip across the bottom */
	pfTranslate(0.0f, -3.0f, 0.0f);
	pfPushMatrix();
	pfScale(3.0f, 1.0f, 1.0f);
	pfDrawGSet(gset[10]);
	pfPopMatrix();
	
	pfPopMatrix();
	
	pfSwapWinBuffers(win);
    }
    return 0;
}

