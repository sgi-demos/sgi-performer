/*
 * townview - OSG viewer for .pfb databases (M1 bring-up app).
 *
 * Usage:
 *   townview <file.pfb>                          interactive viewer
 *   townview <file.pfb> --screenshot out.png     render offscreen, write PNG
 *   townview <file.pfb> --dump                   print scene stats and exit
 */
#include "pfb2osg.h"

#include <osg/ComputeBoundsVisitor>
#include <osg/CullFace>
#include <osgDB/WriteFile>
#include <osgGA/TrackballManipulator>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <osgUtil/ShaderGen>          /* GLES2 has no fixed-function pipeline */
#endif

#include <SDL.h>

#include <cstdio>
#include <cstring>
#include <string>

struct Stats : public osg::NodeVisitor {
    int groups = 0, geodes = 0, drawables = 0;
    Stats() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {}
    void apply(osg::Group& g) override { groups++; traverse(g); }
    void apply(osg::Geode& g) override
    {
        geodes++;
        drawables += (int)g.getNumDrawables();
        traverse(g);
    }
};

#ifdef __EMSCRIPTEN__
/* per-frame state for the browser main loop (emscripten_set_main_loop needs
 * a plain callback; stash everything the frame body touches here) */
struct WebApp {
    osgViewer::Viewer* viewer;
    osgViewer::GraphicsWindowEmbedded* gw;
    SDL_Window* window;
    float sx, sy;
    osg::Vec3d center;              /* orbit target (downtown, ground level) */
    double angle = 3.9;            /* current orbit azimuth */
    bool userControl = false;      /* mouse drag hands control to trackball */
};
static void web_frame(void* arg)
{
    WebApp* a = (WebApp*)arg;
    SDL_Event ev;
    osgGA::EventQueue* q = a->gw->getEventQueue();
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_MOUSEMOTION:
            q->mouseMotion(ev.motion.x * a->sx, ev.motion.y * a->sy);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            int b = ev.button.button == SDL_BUTTON_LEFT   ? 1
                  : ev.button.button == SDL_BUTTON_MIDDLE ? 2 : 3;
            if (ev.type == SDL_MOUSEBUTTONDOWN) {
                a->userControl = true;      /* stop auto-orbit, hand to user */
                q->mouseButtonPress(ev.button.x * a->sx, ev.button.y * a->sy, b);
            } else
                q->mouseButtonRelease(ev.button.x * a->sx, ev.button.y * a->sy, b);
            break;
        }
        case SDL_MOUSEWHEEL:
            a->userControl = true;
            q->mouseScroll(ev.wheel.y > 0
                               ? osgGA::GUIEventAdapter::SCROLL_UP
                               : osgGA::GUIEventAdapter::SCROLL_DOWN);
            break;
        case SDL_WINDOWEVENT:
            if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                int dw = 0, dh = 0;
                SDL_GL_GetDrawableSize(a->window, &dw, &dh);
                a->gw->resized(0, 0, dw, dh);
                q->windowResize(0, 0, dw, dh);
            }
            break;
        }
    }
    if (!a->userControl) {
        /* slow cinematic orbit around downtown; keep the trackball's own
         * state in sync so the handoff on first drag is seamless */
        a->angle += 0.0018;
        double R = 1500.0, H = 620.0;
        osg::Vec3d eye = a->center +
            osg::Vec3d(R * cos(a->angle), R * sin(a->angle), H);
        osg::Matrixd vm;
        vm.makeLookAt(eye, a->center, osg::Vec3d(0, 0, 1));
        a->viewer->getCameraManipulator()->setByInverseMatrix(vm);
    }
    a->viewer->frame();
    SDL_GL_SwapWindow(a->window);
}
#endif

int main(int argc, char** argv)
{
#ifdef __EMSCRIPTEN__
    /* the town database is bundled into the wasm virtual FS at build time */
    if (argc < 2) { static const char* def[] = {argv[0],
        (char*)"/data/town/town_ogl_pfi.pfb"}; argv = (char**)def; argc = 2; }
#endif
    if (argc < 2) {
        fprintf(stderr, "usage: %s <file.pfb> [--screenshot out.png] [--dump]\n",
                argv[0]);
        return 2;
    }
    const char* screenshot = nullptr;
    bool dump = false, unlit = false, haveEye = false;
    osg::Vec3d eye, at;
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "--screenshot") && i + 1 < argc)
            screenshot = argv[++i];
        else if (!strcmp(argv[i], "--dump"))
            dump = true;
        else if (!strcmp(argv[i], "--unlit"))
            unlit = true;
        else if (!strcmp(argv[i], "--eye") && i + 6 < argc) {
            /* --eye ex ey ez ax ay az : explicit camera for screenshots */
            eye.set(atof(argv[i + 1]), atof(argv[i + 2]), atof(argv[i + 3]));
            at.set(atof(argv[i + 4]), atof(argv[i + 5]), atof(argv[i + 6]));
            haveEye = true;
            i += 6;
        }
    }

    osg::ref_ptr<osg::Node> scene = pfb2osgLoadFile(argv[1]);
    if (!scene) {
        fprintf(stderr, "townview: failed to load \"%s\"\n", argv[1]);
        return 1;
    }

    osg::BoundingSphere bs = scene->getBound();
    printf("townview: loaded \"%s\"\n", argv[1]);
    printf("  bound: center (%.1f %.1f %.1f) radius %.1f\n",
           bs.center().x(), bs.center().y(), bs.center().z(), bs.radius());
    Stats st;
    scene->accept(st);
    printf("  %d groups, %d geodes, %d drawables\n",
           st.groups, st.geodes, st.drawables);
    if (dump) return 0;

    if (unlit)
        scene->getOrCreateStateSet()->setMode(GL_LIGHTING,
            osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    osgViewer::Viewer viewer;
    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
    viewer.setSceneData(scene.get());
    /* Performer viewers cull backfaces by default (perfly: pfCullFace
     * PFCF_BACK); town's sloppy FLAT strips have backfacing overlap
     * triangles that must be culled */
    scene->getOrCreateStateSet()->setAttributeAndModes(
        new osg::CullFace(osg::CullFace::BACK));

    /* sun instead of headlight (perfly gets this from pfEarthSky) */
    viewer.setLightingMode(osg::View::SKY_LIGHT);
    osg::Light* sun = viewer.getLight();
    sun->setPosition(osg::Vec4(0.3f, -0.4f, 1.0f, 0.0f));   /* directional */
    sun->setAmbient(osg::Vec4(0.35f, 0.35f, 0.35f, 1.0f));
    sun->setDiffuse(osg::Vec4(1.0f, 1.0f, 0.95f, 1.0f));
    viewer.setCameraManipulator(new osgGA::TrackballManipulator);

    if (screenshot) {
        /* offscreen render: pbuffer context, a few frames, then capture */
        /* macOS pbuffers are defunct; render in a (briefly visible) window */
        viewer.setUpViewInWindow(40, 40, 1280, 960);
        viewer.realize();

        /* Performer databases are Z-up; look from the south-west, elevated */
        if (!haveEye) {
            eye = bs.center() + osg::Vec3d(-bs.radius() * 0.7,
                                           -bs.radius() * 0.9,
                                           bs.radius() * 0.45);
            at = bs.center();
        }
        viewer.getCameraManipulator()->setHomePosition(
            eye, at, osg::Vec3d(0, 0, 1));
        viewer.home();

        std::string out(screenshot);
        std::string ext = "png";
        size_t dot = out.find_last_of('.');
        if (dot != std::string::npos) {
            ext = out.substr(dot + 1);
            out = out.substr(0, dot);
        }
        osg::ref_ptr<osgViewer::ScreenCaptureHandler::WriteToFile> writer =
            new osgViewer::ScreenCaptureHandler::WriteToFile(
                out, ext, osgViewer::ScreenCaptureHandler::WriteToFile::OVERWRITE);
        osg::ref_ptr<osgViewer::ScreenCaptureHandler> capture =
            new osgViewer::ScreenCaptureHandler(writer.get());
        viewer.addEventHandler(capture.get());

        for (int i = 0; i < 8; i++) viewer.frame();
        capture->captureNextFrame(viewer);
        viewer.frame();
        printf("townview: wrote %s_*.%s\n", out.c_str(), ext.c_str());
        return 0;
    }

    /* Interactive mode: SDL2 window + embedded OSG viewer.  OSG's own Cocoa
     * backend is not HiDPI-aware (viewport in points, framebuffer in pixels
     * -> scene squeezed into the lower-left quadrant on Retina); SDL reports
     * window and drawable sizes separately, so we can do it right. */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "townview: SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#ifdef __EMSCRIPTEN__
    /* WebGL1 == GLES2 */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
#endif
    SDL_Window* window = SDL_CreateWindow("townview",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 960,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_GLContext glctx = window ? SDL_GL_CreateContext(window) : nullptr;
    if (!glctx) {
        fprintf(stderr, "townview: no GL context: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_SetSwapInterval(1);

    int dw = 0, dh = 0, ww = 0, wh = 0;
    SDL_GL_GetDrawableSize(window, &dw, &dh);
    SDL_GetWindowSize(window, &ww, &wh);
    float sx = ww ? (float)dw / ww : 1.0f;     /* point -> pixel scale */
    float sy = wh ? (float)dh / wh : 1.0f;

    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> gw =
        viewer.setUpViewerAsEmbeddedInWindow(0, 0, dw, dh);
    viewer.addEventHandler(new osgViewer::StatsHandler);       /* 's' key */
    viewer.getCamera()->setProjectionMatrixAsPerspective(
        45.0, (double)dw / dh, 1.0, 30000.0);
    viewer.home();

#ifdef __EMSCRIPTEN__
    /* frame the town with a Z-up eye (Performer databases are Z-up; OSG's
     * auto-home assumes Y-up and stares at empty sky).  The bound center is
     * up in the mountain/sky geometry (z~600), so aim at the downtown grid
     * at ground level for an elevated 3/4 view of the buildings. */
    {
        osg::Vec3d townAt(2500.0, 2450.0, 40.0);
        osg::Vec3d e = townAt + osg::Vec3d(-1100.0, -1400.0, 700.0);
        viewer.getCameraManipulator()->setHomePosition(
            e, townAt, osg::Vec3d(0, 0, 1));
        viewer.home();
    }
    /* WebGL forbids client-side vertex arrays and has no display lists, so
     * every drawable must use VBOs (pfb2osg defaults to display lists) */
    {
        struct UseVBO : osg::NodeVisitor {
            UseVBO() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {}
            void apply(osg::Geode& g) override
            {
                for (unsigned i = 0; i < g.getNumDrawables(); i++)
                    if (osg::Geometry* geom = g.getDrawable(i)->asGeometry()) {
                        geom->setUseVertexBufferObjects(true);
                        geom->setUseDisplayList(false);
                    }
                traverse(g);
            }
        } vbo;
        scene->accept(vbo);
    }
    /* GLES2 has no fixed-function pipeline.  OSG 3.6.5's ShaderGen emits
     * desktop GLSL (gl_LightSource, gl_NormalMatrix, ...) that WebGL rejects,
     * so attach our own GLES2 uber shader to the scene root instead.  It uses
     * the osg_* attributes/uniforms that OSG feeds when vertex-attribute
     * aliasing + MVP uniforms are enabled (set after realize, below).  A 1x1
     * white default texture on the root makes untextured geometry sample
     * white, so one program covers textured and flat-colored surfaces. */
    {
        static const char* vsrc =
            "precision highp float;\n"
            "attribute vec4 osg_Vertex;\n"
            "attribute vec3 osg_Normal;\n"
            "attribute vec4 osg_Color;\n"
            "attribute vec4 osg_MultiTexCoord0;\n"
            "uniform mat4 osg_ModelViewProjectionMatrix;\n"
            "uniform mat3 osg_NormalMatrix;\n"
            "varying vec2 uv;\n"
            "varying vec4 vcolor;\n"
            "varying float diffuse;\n"
            "void main() {\n"
            "  gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
            "  vec3 n = normalize(osg_NormalMatrix * osg_Normal);\n"
            "  vec3 L = normalize(vec3(0.3, -0.4, 1.0));\n"
            "  diffuse = 0.95 + 0.45 * max(dot(n, L), 0.0);\n"
            "  uv = osg_MultiTexCoord0.xy;\n"
            "  vcolor = osg_Color;\n"
            "}\n";
        static const char* fsrc =
            "precision highp float;\n"
            "uniform sampler2D tex;\n"
            "varying vec2 uv;\n"
            "varying vec4 vcolor;\n"
            "varying float diffuse;\n"
            "void main() {\n"
            "  vec4 t = texture2D(tex, uv);\n"
            "  vec3 c = t.rgb * vcolor.rgb * diffuse * 1.25;\n"   /* daylight gain */
            "  gl_FragColor = vec4(c, t.a * vcolor.a);\n"
            "}\n";
        osg::Program* prog = new osg::Program;
        prog->addShader(new osg::Shader(osg::Shader::VERTEX, vsrc));
        prog->addShader(new osg::Shader(osg::Shader::FRAGMENT, fsrc));
        osg::StateSet* ss = scene->getOrCreateStateSet();
        ss->setAttributeAndModes(prog);
        ss->addUniform(new osg::Uniform("tex", 0));

        /* 1x1 white fallback texture at the root (overridden per-drawable) */
        osg::Image* wimg = new osg::Image;
        wimg->allocateImage(1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE);
        ((unsigned char*)wimg->data())[0] = 255;
        ((unsigned char*)wimg->data())[1] = 255;
        ((unsigned char*)wimg->data())[2] = 255;
        ((unsigned char*)wimg->data())[3] = 255;
        osg::Texture2D* white = new osg::Texture2D(wimg);
        white->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
        white->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
        ss->setTextureAttributeAndModes(0, white);
    }
    viewer.realize();
    /* GLES2 has no fixed-function vertex setup (glVertexPointer/glNormal...)
     * and no built-in gl_ModelViewProjectionMatrix: drive geometry through
     * generic vertex attributes and matrix uniforms instead */
    if (osg::State* state = gw->getState()) {
        state->setUseModelViewAndProjectionUniforms(true);
        state->setUseVertexAttributeAliasing(true);
    }
    static WebApp app{&viewer, gw.get(), window, sx, sy,
                      osg::Vec3d(2500.0, 2450.0, 60.0)};
    emscripten_set_main_loop_arg(web_frame, &app, 0, 1);
    return 0;                       /* not reached; browser drives the loop */
#else
    viewer.realize();

    bool running = true;
    while (running && !viewer.done()) {
        SDL_Event ev;
        osgGA::EventQueue* q = gw->getEventQueue();
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEMOTION:
                q->mouseMotion(ev.motion.x * sx, ev.motion.y * sy);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                int b = ev.button.button == SDL_BUTTON_LEFT    ? 1
                      : ev.button.button == SDL_BUTTON_MIDDLE  ? 2 : 3;
                if (ev.type == SDL_MOUSEBUTTONDOWN)
                    q->mouseButtonPress(ev.button.x * sx, ev.button.y * sy, b);
                else
                    q->mouseButtonRelease(ev.button.x * sx, ev.button.y * sy, b);
                break;
            }
            case SDL_MOUSEWHEEL:
                q->mouseScroll(ev.wheel.y > 0
                                   ? osgGA::GUIEventAdapter::SCROLL_UP
                                   : osgGA::GUIEventAdapter::SCROLL_DOWN);
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                SDL_Keycode k = ev.key.keysym.sym;
                if (k == SDLK_ESCAPE) { running = false; break; }
                if (k < 128) {         /* printable ascii incl. space, 's' */
                    if (ev.type == SDL_KEYDOWN) q->keyPress((int)k);
                    else                        q->keyRelease((int)k);
                }
                break;
            }
            case SDL_WINDOWEVENT:
                if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    SDL_GL_GetDrawableSize(window, &dw, &dh);
                    SDL_GetWindowSize(window, &ww, &wh);
                    sx = ww ? (float)dw / ww : 1.0f;
                    sy = wh ? (float)dh / wh : 1.0f;
                    gw->resized(0, 0, dw, dh);
                    q->windowResize(0, 0, dw, dh);
                }
                break;
            }
        }
        viewer.frame();
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(glctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
#endif
}
