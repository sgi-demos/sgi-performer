/*
 * pr_math_test - link + behavior smoke test for the reconstructed libpr math.
 *
 * Exercises out-of-line code from the reconstructed slices (pfMatrix rotation
 * and full inversion, pfQuat<->pfMatrix round trip, pfSphere/pfBox/pfPlane
 * containment) so a regression in the reconstruction shows up as a FAIL or a
 * link error, not just a compile success.
 */
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>
#include <stdio.h>
#include <math.h>

static int failures = 0;
#define CHECK(cond) \
    do { if (!(cond)) { fprintf(stderr, "FAIL(%d): %s\n", __LINE__, #cond); failures++; } } while (0)

static bool near1(float a, float b, float tol = 1e-4f) { return fabsf(a - b) <= tol; }

int main(void)
{
    /* pfVec3 basics (header inlines + C wrappers share these code paths) */
    pfVec3 a(1.0f, 2.0f, 3.0f), b(4.0f, 5.0f, 6.0f), c;
    c.add(a, b);
    CHECK(c[0] == 5.0f && c[1] == 7.0f && c[2] == 9.0f);
    CHECK(near1(a.dot(b), 32.0f));

    /* pfMatrix::makeRot + xformVec: 90 deg about +Z maps +X to +Y */
    pfMatrix m;
    m.makeRot(90.0f, 0.0f, 0.0f, 1.0f);
    pfVec3 r;
    r.xformVec(pfVec3(1.0f, 0.0f, 0.0f), m);
    CHECK(near1(r[0], 0.0f) && near1(r[1], 1.0f) && near1(r[2], 0.0f));

    /* full inverse: M * M^-1 == I */
    pfMatrix t, mt, inv, id;
    t.makeTrans(10.0f, -3.0f, 0.5f);
    mt.mult(m, t);
    CHECK(inv.invertFull(mt));
    id.mult(mt, inv);
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            CHECK(near1(id[i][j], (i == j) ? 1.0f : 0.0f));

    /* pfQuat round trip: matrix -> quat -> matrix */
    pfQuat q;
    q.makeRot(m);
    pfMatrix m2;
    q.getRot(m2);
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            CHECK(near1(m[i][j], m2[i][j]));

    /* pfSphere / pfBox containment (reconstructed pfGeoMath slices) */
    pfSphere s;
    s.center.set(0.0f, 0.0f, 0.0f);
    s.radius = 10.0f;
    pfBox box;
    box.min.set(-1.0f, -1.0f, -1.0f);
    box.max.set(1.0f, 1.0f, 1.0f);
    CHECK(s.contains(&box) != 0);          /* small box inside big sphere  */
    box.min.set(20.0f, 20.0f, 20.0f);
    box.max.set(22.0f, 22.0f, 22.0f);
    CHECK(s.contains(&box) == 0);          /* far box outside              */

    /* pfSeg / pfSphere intersection */
    pfSeg seg;
    seg.pos.set(-20.0f, 0.0f, 0.0f);
    seg.dir.set(1.0f, 0.0f, 0.0f);
    seg.length = 40.0f;
    s.radius = 5.0f;
    float d1 = 0.0f, d2 = 0.0f;
    CHECK(s.isect(&seg, &d1, &d2) != 0);
    CHECK(near1(d1, 15.0f) && near1(d2, 25.0f));

    if (failures == 0) {
        printf("pr_math_test: all checks passed\n");
        return 0;
    }
    fprintf(stderr, "pr_math_test: %d check(s) FAILED\n", failures);
    return 1;
}
