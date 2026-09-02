/* sliver_check.c - does this GL stack rasterize huge eye-plane-straddling
 * sliver triangles?
 *
 * Reproduces the exact failing case from Performer Town: road strip gset
 * 1107 drawn from the tether camera. Strip triangle #2 is a 92.5-unit
 * sliver with two vertices behind the eye; on Apple Silicon it is silently
 * dropped, leaving a clear-color wedge. Measured on an M3 (macOS 14.8.3):
 *
 *   Apple GL 2.1 -> Metal                     raw FAIL   subdivided PASS
 *   ANGLE -> Apple GL 4.1 -> Metal (default)  raw FAIL   subdivided PASS
 *   ANGLE -> Metal (ANGLE_DEFAULT_PLATFORM=metal, what Chrome uses)
 *                                             raw FAIL   subdivided PASS
 *
 * i.e. the drop happens in Metal / the Apple GPU's clipless rasterizer, not
 * in any particular GL front end - which is why pfb2osg subdivides long
 * edges (PFOSG_SUBDIV, default 25 units) on every backend, web included.
 *
 * Build (Apple desktop GL):
 *   clang sliver_check.c -o sliver_gl $(sdl2-config --cflags --libs) \
 *         -framework OpenGL -DGL_SILENCE_DEPRECATION
 * Build (ANGLE GLES2; OGL = path to opengl-for-mac):
 *   clang sliver_check.c -DUSE_GLES2 -o sliver_angle \
 *         $(sdl2-config --cflags --libs) -I$OGL/include -L$OGL/lib \
 *         -lGLESv2 -lEGL
 *   DYLD_FALLBACK_LIBRARY_PATH=$OGL/lib [ANGLE_DEFAULT_PLATFORM=metal] \
 *         ./sliver_angle
 *
 * SLIVER_SUBDIV=1 in the environment draws the strip pre-subdivided to
 * <=25-unit edges (the pfb2osg workaround) - expected PASS everywhere.
 */
#include <SDL.h>
#include <math.h>
#include <stdio.h>

#ifdef USE_GLES2
#include <SDL_opengles2.h>
#else
#include <OpenGL/gl.h>
#endif

/* ---- minimal column-major mat4 ---- */
typedef struct { float m[16]; } Mat4;

static Mat4 perspective(float fovyDeg, float aspect, float zn, float zf)
{
    float f = 1.0f / tanf(fovyDeg * (float)M_PI / 360.0f);
    Mat4 r = {{0}};
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (zf + zn) / (zn - zf);
    r.m[11] = -1.0f;
    r.m[14] = 2.0f * zf * zn / (zn - zf);
    return r;
}

/* view for eye at e, forward +y, up +z (the tether camera heading) */
static Mat4 lookNorth(float ex, float ey, float ez)
{
    /* rows: s=(1,0,0), u=(0,0,1), -f=(0,-1,0) */
    Mat4 r = {{1, 0, 0, 0,
               0, 0, -1, 0,
               0, 1, 0, 0,
               0, 0, 0, 1}};
    r.m[12] = -ex;
    r.m[13] = -ez * 0 - ey * 0 - ez;  /* u . -e  = -(e.z) */
    r.m[14] = ey;                     /* -f . -e = e.y */
    /* recompute properly: col-major translation = M_rot * -e */
    r.m[12] = -(1 * ex + 0 * ey + 0 * ez);
    r.m[13] = -(0 * ex + 0 * ey + 1 * ez);
    r.m[14] = -(0 * ex - 1 * ey + 0 * ez);
    return r;
}

static Mat4 mul(Mat4 a, Mat4 b)
{
    Mat4 r;
    for (int c = 0; c < 4; c++)
        for (int rr = 0; rr < 4; rr++) {
            float s = 0;
            for (int k = 0; k < 4; k++)
                s += a.m[k * 4 + rr] * b.m[c * 4 + k];
            r.m[c * 4 + rr] = s;
        }
    return r;
}

/* the road strip, verbatim from the .pfb (z=0 ground) */
static const float STRIP[7][3] = {
    {2592.5f, 2380.5f, 0}, {2592.5f, 2404.5f, 0}, {2577.5f, 2380.5f, 0},
    {2592.5f, 2473.0f, 0}, {2577.5f, 2404.5f, 0}, {2577.5f, 2473.0f, 0},
    {2577.5f, 2445.1f, 0}};

/* probe points: 3 inside the dropped sliver (tri2), 2 inside healthy tri1 */
static const float WEDGE[3][2] = {{2585.0f, 2432.1f}, {2586.0f, 2437.0f},
                                  {2584.5f, 2429.0f}};
static const float CTRL[2][2] = {{2589.5f, 2430.0f}, {2591.0f, 2440.0f}};

int main(void)
{
    SDL_Init(SDL_INIT_VIDEO);
#ifdef USE_GLES2
    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
#endif
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_Window* win = SDL_CreateWindow("sliver_check", SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED, 735, 415,
                                       SDL_WINDOW_OPENGL |
                                           SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    if (!ctx) {
        fprintf(stderr, "no GL context: %s\n", SDL_GetError());
        return 2;
    }
    printf("renderer: %s | %s\n", (const char*)glGetString(GL_RENDERER),
           (const char*)glGetString(GL_VERSION));

    int W = 0, H = 0;
    SDL_GL_GetDrawableSize(win, &W, &H);
    glViewport(0, 0, W, H);

    /* the exact repro projection: fovy 26.51, near 1, far 32357.7 */
    Mat4 mvp = mul(perspective(26.51f, (float)W / (float)H, 1.0f, 32357.7f),
                   lookNorth(2589.0f, 2419.6f, 2.0f));

#ifdef USE_GLES2
    const char* vs_src =
        "attribute vec3 pos; uniform mat4 mvp;"
        "void main(){ gl_Position = mvp * vec4(pos,1.0); }";
    const char* fs_src =
        "precision mediump float;"
        "void main(){ gl_FragColor = vec4(1.0,0.0,0.0,1.0); }";
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, NULL);
    glCompileShader(fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glBindAttribLocation(prog, 0, "pos");
    glLinkProgram(prog);
    glUseProgram(prog);
    glUniformMatrix4fv(glGetUniformLocation(prog, "mvp"), 1, GL_FALSE, mvp.m);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, STRIP);
    glEnableVertexAttribArray(0);
#else
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(mvp.m);
    glColor3f(1, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, STRIP);
#endif

    /* SLIVER_SUBDIV=1: draw the strip pre-subdivided to <=25-unit edges
     * (the pfb2osg workaround) instead of raw */
    int subdiv = getenv("SLIVER_SUBDIV") != NULL;
    static float tris[8192][3];
    int ntri = 0;
    if (subdiv) {
        /* expand strip to triangles, then split longest edge > 25 */
        float stack[64][3][3];
        int sp = 0;
        for (int t = 0; t < 5; t++) {
            int a = t, b = t + 1, c = t + 2;
            if (t & 1) { int tmp = a; a = b; b = tmp; }
            for (int k = 0; k < 3; k++) {
                stack[sp][0][k] = STRIP[a][k];
                stack[sp][1][k] = STRIP[b][k];
                stack[sp][2][k] = STRIP[c][k];
            }
            sp++;
            while (sp > 0) {
                float(*T)[3] = stack[--sp];
                float e[3];
                for (int ed = 0; ed < 3; ed++) {
                    float dx = T[(ed + 1) % 3][0] - T[ed][0];
                    float dy = T[(ed + 1) % 3][1] - T[ed][1];
                    e[ed] = dx * dx + dy * dy;
                }
                int longest = e[0] >= e[1] && e[0] >= e[2] ? 0
                              : e[1] >= e[2]               ? 1
                                                           : 2;
                if (e[longest] <= 25.0f * 25.0f || sp > 60) {
                    for (int v = 0; v < 3; v++)
                        for (int k = 0; k < 3; k++)
                            tris[ntri * 3 + v][k] = T[v][k];
                    ntri++;
                    continue;
                }
                int i0 = longest, i1 = (longest + 1) % 3;
                float mid[3];
                for (int k = 0; k < 3; k++)
                    mid[k] = (T[i0][k] + T[i1][k]) * 0.5f;
                /* two halves: replace i1 with mid; replace i0 with mid */
                float h1[3][3], h2[3][3];
                for (int v = 0; v < 3; v++)
                    for (int k = 0; k < 3; k++) {
                        h1[v][k] = v == i1 ? mid[k] : T[v][k];
                        h2[v][k] = v == i0 ? mid[k] : T[v][k];
                    }
                for (int v = 0; v < 3; v++)
                    for (int k = 0; k < 3; k++) {
                        stack[sp][v][k] = h1[v][k];
                        stack[sp + 1][v][k] = h2[v][k];
                    }
                sp += 2;
            }
        }
#ifdef USE_GLES2
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, tris);
#else
        glVertexPointer(3, GL_FLOAT, 0, tris);
#endif
        printf("subdivided into %d triangles\n", ntri);
    }

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (subdiv)
        glDrawArrays(GL_TRIANGLES, 0, ntri * 3);
    else
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 7);
    glFinish();

    /* project a world point through the same mvp to window coords */
    int fails = 0;
    for (int i = 0; i < 5; i++) {
        const float* p = i < 3 ? WEDGE[i] : CTRL[i - 3];
        float x = p[0], y = p[1], z = 0, w;
        float cx = mvp.m[0] * x + mvp.m[4] * y + mvp.m[8] * z + mvp.m[12];
        float cy = mvp.m[1] * x + mvp.m[5] * y + mvp.m[9] * z + mvp.m[13];
        w = mvp.m[3] * x + mvp.m[7] * y + mvp.m[11] * z + mvp.m[15];
        int px = (int)((cx / w * 0.5f + 0.5f) * W);
        int py = (int)((cy / w * 0.5f + 0.5f) * H);
        if (px < 0 || px >= W || py < 0 || py >= H) {
            printf("%s (%.1f,%.1f) -> px(%d,%d) offscreen, skipped\n",
                   i < 3 ? "wedge" : "ctrl ", p[0], p[1], px, py);
            continue;
        }
        unsigned char rgba[4] = {0};
        glReadPixels(px, py, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
        int red = rgba[0] > 200 && rgba[1] < 50;
        printf("%s (%.1f,%.1f) -> px(%d,%d) rgba(%d,%d,%d) %s\n",
               i < 3 ? "wedge" : "ctrl ", p[0], p[1], px, py, rgba[0],
               rgba[1], rgba[2], red ? "DRAWN" : "MISSING");
        if (!red) fails += (i < 3) ? 1 : 100;  /* ctrl failure = test broken */
    }
    printf(fails == 0     ? "RESULT: PASS (sliver rasterized)\n"
           : fails >= 100 ? "RESULT: TEST-BROKEN (control points missing)\n"
                          : "RESULT: FAIL (sliver dropped by driver)\n");

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return fails == 0 ? 0 : 1;
}
