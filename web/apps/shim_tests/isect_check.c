/* isect_check.c - shim regression test for pfNodeIsectSegs/pfQueryHit.
 * Builds a unit quad at z = 3 and fires a downward segment through it. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <Performer/pf.h>

static pfVec3 quad[] = { { -1.0f, -1.0f, 3.0f }, {  1.0f, -1.0f, 3.0f },
                         {  1.0f,  1.0f, 3.0f }, { -1.0f,  1.0f, 3.0f } };

int main(void)
{
    pfScene* scene;
    pfGeode* geode;
    pfGeoSet* gset;
    pfSegSet segset;
    pfHit** hits[PFIS_MAX_SEGS];
    pfVec3 pnt;
    int isect;

    pfInit();
    pfMultiprocess(PFMP_APPCULLDRAW);
    pfConfig();

    gset = pfNewGSet(pfGetSharedArena());
    pfGSetPrimType(gset, PFGS_QUADS);
    pfGSetNumPrims(gset, 1);
    pfGSetAttr(gset, PFGS_COORD3, PFGS_PER_VERTEX, (void*)quad, NULL);
    geode = pfNewGeode();
    pfAddGSet(geode, gset);
    scene = pfNewScene();
    pfAddChild(scene, geode);

    segset.mode = PFTRAV_IS_PRIM;
    segset.userData = NULL;
    segset.activeMask = 1;
    segset.isectMask = 0xFFFF;
    segset.bound = NULL;
    segset.discFunc = NULL;
    pfSetVec3(segset.segs[0].pos, 0.25f, 0.25f, 100.0f);
    pfSetVec3(segset.segs[0].dir, 0.0f, 0.0f, -1.0f);
    segset.segs[0].length = 1000.0f;

    isect = pfNodeIsectSegs(scene, &segset, hits);
    if (!isect) {
        fprintf(stderr, "isect_check: FAIL - no intersection found\n");
        return 1;
    }
    pfQueryHit(*hits[0], PFQHIT_POINT, &pnt);
    if (fabsf(pnt[0] - 0.25f) > 1e-4f || fabsf(pnt[1] - 0.25f) > 1e-4f ||
        fabsf(pnt[2] - 3.0f) > 1e-4f) {
        fprintf(stderr, "isect_check: FAIL - hit at (%g %g %g), expected "
                        "(0.25 0.25 3)\n", pnt[0], pnt[1], pnt[2]);
        return 1;
    }

    /* miss case: segment that stops short of the quad */
    segset.segs[0].length = 5.0f;   /* from z=100 down 5 units: no hit */
    isect = pfNodeIsectSegs(scene, &segset, hits);
    if (isect) {
        fprintf(stderr, "isect_check: FAIL - phantom hit\n");
        return 1;
    }

    printf("isect_check: PASS\n");
    return 0;
}
