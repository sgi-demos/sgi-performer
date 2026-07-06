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
#include <osgDB/WriteFile>
#include <osgGA/TrackballManipulator>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

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

int main(int argc, char** argv)
{
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
}
