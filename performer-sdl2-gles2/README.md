# Performer → SDL2 + GLES2 port

Goal: run classic OpenGL Performer demos (perfly + Performer Town) on modern platforms — native Windows/macOS/Linux and the web via Emscripten — by reimplementing the Performer API subset those demos need.

**SGI's `perfly` — compiled unmodified — now runs in a web browser**, riding the Esprit through Performer Town with its full GUI control panel, stats overlay, animated traffic, cycling stoplights, and EarthSky. The same code runs natively on desktop GL and on a GLES2 backend (ANGLE/Metal), from one shared source tree. See [Demos](#demos) below.

## Strategy

Build just enough of a Performer implementation — on OpenSceneGraph + SDL2 + GLES2 — to run the demos we want to run. The public surface is the original Performer API ([vendor/Performer/Include](vendor/Performer/README.md)), consumed verbatim so SGI's shipped demo programs compile unmodified; the implementation behind it (the pfosg shim) maps that API onto OSG, with SDL2 owning the window/input and the GL usage kept inside the GLES2-compatible subset so the same stack can target Emscripten/WebGL for the web builds.

Scope is demo-driven, not completeness-driven: whatever perfly and the sample programs actually call gets a real implementation; IRIX hardware machinery they merely mention gets accepted-and-ignored stubs.

Key simplifications vs. real Performer:

- Single process only (`PFMP_APPCULLDRAW`) — mandatory for wasm, supported by perfly; APP/CULL/DRAW run inline in `pfFrame()`.
- One pfPipe / one pfPipeWindow / one SDL2 window.
- GL strategy: code strictly to the GLES2 subset. Native ES2 contexts where drivers offer them (Linux/Windows, or ANGLE-on-Metal for macOS), desktop GL 2.1 fallback on macOS, WebGL1 on Emscripten. VBO-only drawing (avoids Emscripten `-sFULL_ES2`). One `PFOSG_GLES2` code path serves both the web and native-GLES2 builds — the immediate-mode overlays (GUI/stats/messages) draw through a batched mini-GL so even SGI's `gui.c` compiles into the GLES2 targets unchanged.

## Layout

```
apps/hello_pf/        M0 bring-up: SDL2 window + ES2-subset shader triangle (native + wasm)
apps/townview/        standalone OSG viewer for .pfb databases (native + wasm)
apps/webfly/          SGI's perfly in the browser: town tether ride, GUI-and-all (wasm)
apps/perfly/          SGI's perfly, compiled unmodified against the shim (native GL + GLES2)
apps/simple/          SGI pguide sample, compiled unmodified against the shim
apps/pguide/          more unmodified SGI samples: texture, lod_func, earthsky, intersect
apps/shim_tests/      shim regression tests (intersection)
src/loaders/pfb2osg/  direct .pfb/.pfi/.rgb → OSG loader (also the plugin-less image reader)
src/pfosg/            the pfosg shim: Performer C API over OSG + SDL2 (GL + GLES2)
src/compat/           shims that let the original Windows-flavored headers compile on modern clang
src/libpr/            object-system experiments (not part of the demo build)
vendor/Performer/     SGI's open-sourced Performer headers, utilities, and demos verbatim
data/town/            the Performer Town demo database, textures, paths, vehicles
data/town-tether.perfly  demo config: original tether start — ride the Esprit (webfly default)
data/town-osg.perfly     demo config: drive mode at the shipped street-level start
```

## Demos

Everything builds from a bare clone — the SGI headers/sources and the town demo data are committed. Three build flavors share one source tree:

| flavor | build dir | GL | demos |
|---|---|---|---|
| **native (desktop GL)** | `build/` | system OSG, GL 2.1 / native ES2 | perfly, townview, pguide samples, simple, shim_tests |
| **native GLES2** | `build-gles2/` | OSG-GLES2 + ANGLE (Metal on macOS) | perfly (GUI-and-all) |
| **web** | `build-web/` | OSG-GLES2, WebGL1 | **webfly** (perfly), townview, hello_pf |

The headline demo — **perfly riding the Esprit through Performer Town** — runs on all three, from the same `data/town-tether.perfly` config.

## Building

### Native (desktop GL)

Needs CMake, SDL2 and OpenSceneGraph — e.g. `brew install cmake ninja sdl2 open-scene-graph`:

```
cmake -B build -G Ninja && cmake --build build

./build/apps/perfly/perfly data/town-tether.perfly          # ride the Esprit (GUI, traffic, stoplights)
./build/apps/perfly/perfly data/town-osg.perfly             # drive it yourself
./build/apps/townview/townview data/town/town_ogl_pfi.pfb   # standalone orbit viewer
./build/apps/pguide/earthsky                                # SGI samples: earthsky, texture, lod_func, intersect
./build/apps/simple/simple                                  # the minimal pguide sample
```

- **perfly tether** (`town-tether.perfly`) auto-rides the Esprit; the GUI panel's *View:* menu switches tether angle (defaults to an ahead-of-the-car view), *Motion:* leaves tether for drive/fly, and the other menus toggle texture/fog/lighting/stats live.
- **perfly drive** (`town-osg.perfly`) starts street-level: left mouse accelerates, pointer steers, right mouse brakes/reverses, quick middle-click stops. `-f` for fly mode; a bare `.pfb` path for trackball.

### Native GLES2 (ANGLE)

The same shim on a real GLES2 backend, so the native build exercises the exact render path the web build uses. On macOS this runs GLES2 on Metal via [ANGLE](https://github.com/google/angle) — the prebuilt dylibs from [`opengl-for-mac`](https://github.com/sgi-demos/opengl-for-mac) are the easy route (`PF_ANGLE` defaults to `~/Github/opengl-for-mac`).

Build OSG for GLES2 once (shares the source tree with the web OSG; see [OSG for GLES2](#osg-for-gles2)), pointing its GL header at ANGLE:

```
cmake -S external/OpenSceneGraph -B external/OpenSceneGraph/build-gles2 \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=14 \
    -DOPENGL_PROFILE=GLES2 -DDYNAMIC_OPENSCENEGRAPH=OFF -DDYNAMIC_OPENTHREADS=OFF \
    -DOSG_WINDOWING_SYSTEM=None \
    -DOSG_GL1_AVAILABLE=OFF -DOSG_GL2_AVAILABLE=OFF -DOSG_GLES2_AVAILABLE=ON \
    -DOSG_GL_DISPLAYLISTS_AVAILABLE=OFF -DOSG_GL_MATRICES_AVAILABLE=OFF \
    -DOSG_GL_VERTEX_FUNCS_AVAILABLE=OFF -DOSG_GL_FIXED_FUNCTION_AVAILABLE=OFF \
    -DOPENGL_HEADER1="#include <GLES2/gl2.h>" -DOPENGL_HEADER2="" \
    -DCMAKE_C_FLAGS="-I$HOME/Github/opengl-for-mac/include" \
    -DCMAKE_CXX_FLAGS="-I$HOME/Github/opengl-for-mac/include"
cmake --build external/OpenSceneGraph/build-gles2 \
    --target osg osgDB osgUtil osgGA osgViewer osgText
```

Then the perfly app against it:

```
cmake -B build-gles2 -DPF_GLES2_OSG=$PWD/external/OpenSceneGraph
cmake --build build-gles2 --target perfly

# ANGLE's dylibs load from the cwd-relative install name, so point the loader at them:
DYLD_FALLBACK_LIBRARY_PATH=$HOME/Github/opengl-for-mac/lib \
    ./build-gles2/apps/perfly/perfly data/town-tether.perfly
```

(On Linux/Windows with a native ES2 driver, drop the ANGLE bits: `-DPF_GLES2_OSG=...` still selects the GLES2 path.)

### Web

**webfly** is SGI's perfly compiled to wasm — the town tether ride, GUI panel, stats, and all — with the town database bundled into the wasm virtual filesystem. townview (standalone orbit viewer) and the hello-triangle build for the web too.

<a name="osg-for-gles2"></a>Build OSG from source for Emscripten once (the same clone serves the native-GLES2 build above):

```
git clone --depth 1 --branch OpenSceneGraph-3.6.5 \
    https://github.com/openscenegraph/OpenSceneGraph.git external/OpenSceneGraph
emcmake cmake -S external/OpenSceneGraph -B external/OpenSceneGraph/build-em -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=14 \
    -DOPENGL_PROFILE=GLES2 -DDYNAMIC_OPENSCENEGRAPH=OFF \
    -DOSG_WINDOWING_SYSTEM=None -DBUILD_OSG_APPLICATIONS=OFF \
    -DOSG_GL1_AVAILABLE=OFF -DOSG_GL2_AVAILABLE=OFF -DOSG_GLES2_AVAILABLE=ON \
    -DOSG_GL_DISPLAYLISTS_AVAILABLE=OFF -DOSG_GL_MATRICES_AVAILABLE=OFF \
    -DOSG_GL_VERTEX_FUNCS_AVAILABLE=OFF -DOSG_GL_FIXED_FUNCTION_AVAILABLE=OFF \
    -DEGL_LIBRARY=EGL -DOPENGL_egl_LIBRARY=EGL
cmake --build external/OpenSceneGraph/build-em --target osg osgDB osgUtil osgGA osgViewer osgText
```

Then the web apps (town data is bundled into the wasm FS at link time):

```
emcmake cmake -B build-web -G Ninja -DPF_EM_OSG=$PWD/external/OpenSceneGraph
cmake --build build-web --target webfly townview hello_pf

# serve over HTTP (wasm/data can't be fetched from file://) and open in a browser:
python3 -m http.server 8092 --directory build-web/apps/webfly
#   http://localhost:8092/webfly.html      — perfly rides the town, GUI panel and all
python3 -m http.server 8080 --directory build-web/apps/townview
#   http://localhost:8080/townview.html    — standalone orbit viewer
```

webfly loads `data/town-tether.perfly` by default (baked into its shell). Drag to look, scroll to zoom; the GUI panel on the left is fully live (menus, sliders, Reset All, GUI Off).

> Note: a change to bundled data (`data/…`) or the shell HTML doesn't relink on its own — remove the target's `.data`/`.html` (or touch a source file) to force a repackage.

## Roadmap

- **M0** ✅ skeleton; SDL2 + ES2-subset context + shader pipeline on native & web
- **M0.5** ✅ the original Windows-flavored Performer headers compiling on modern clang ([src/compat](src/compat/pfcompat.h)); pragmatic pfType/pfMemory/pfObject/pfNotify object-system core ([src/libpr/pfCore.cpp](src/libpr/pfCore.cpp): heap-backed, single-process, API contracts preserved)
- **M1** ✅ OSG backend, perfly drives town:
  - `townview` renders the town database through a direct pfb→OSG loader ([src/loaders/pfb2osg](src/loaders/pfb2osg/pfb2osg.cpp), format transcribed from SGI's shipped `pfpfb.c`; `.pfi` and `.rgb` images supported); SDL2 window with embedded OSG viewer (HiDPI-correct)
  - **pfosg shim** ([src/pfosg](src/pfosg/pfosg.cpp)): Performer C API over OSG + SDL2. The shim's `pf.h` supplies object handles and tokens; the original `prmath.h`/`pfutil.h`/`pfui.h`/`pfdu.h` are consumed via `#include_next` (`PF_CPLUSPLUS_API=0`), so tokens, struct layouts, and declarations are exactly SGI's.
  - Six **unmodified** shipped programs compile and run: `simple`, `texture`, `lod_func`, `earthsky`, `intersect` — and perfly itself.
  - Real implementations: geoset building, textures, geostates, DCS/SCS/LOD, EarthSky clears, `pfNodeIsectSegs`, the pfu input system (SDL-backed `pfuMouse`/`pfuEventStream`), and pfiXformer motion models (trackball/fly/drive) with control laws transcribed from the shipped libpfui source — fly pitch sense verified against a real Performer install. IRIX hardware machinery (stats, calligraphics, compositors, DVR, clip textures, GUI panels) is accepted and ignored (~200 stubs in [pfosg_perfly.cpp](src/pfosg/pfosg_perfly.cpp)).
- **M2** full demo experience:
  - ✅ pfuPath vehicle animation: SGI's shipped `path.c` compiled in unmodified, driven through real APP-traversal node callbacks — the Esprit and truck circulate the streets, the blimp and Rocket Tux fly overhead, and **tether mode rides the Esprit** (`./build/apps/perfly/perfly data/town-tether.perfly`, the demo's original startup mode)
  - ✅ **perfly's GUI control panel**: SGI's shipped libpfutil `gui.c` compiled in unmodified — the classic widget panel (position readout, motion/texture/fog/lighting menus, sliders, Reset All, GUI Off) drawn by the original immediate-mode code through a real second pfChannel, with a built-in bitmap font replacing the X11 fonts and heap-backed pfDataPools replacing IRIX shared memory. Buttons, menu cycling, sliders, Reset All, and GUI on/off all live
  - ✅ **EarthSky sky/ground bands**: pfEarthSky renders the real eye-following background — sky dome with the horizon band gradient (horizon → sky-bottom → zenith), ground sheet with near→far colors at `PFES_GRND_HT` — driven live by perfly's BG menu (Sky/Grnd, Sky, Clear, Tag) and the Time-of-Day slider (env.c's color ramps and horizon-angle animation run as shipped)
  - ✅ **stats display**: `pfDrawChanStats` draws the classic overlay — frame rate, frame-time strip chart with the 60 Hz reference line, app/cull/draw milliseconds (cull/draw from the OSG renderer), and scene geometry counts when the Gfx class is enabled — toggled live from the panel's Stats buttons
  - ✅ **SGI's shipped .pfb/.pfa reader** ([vendor/Performer/Src/lib/libpfdb/libpfpfb/pfpfb.c](vendor/Performer/Src/lib/libpfdb/libpfpfb/pfpfb.c)), 16k lines compiled **unmodified**, is now the default database loader — it builds scenes through the shim's `pf*` API (geosets, billboards, sequences, LODs, layers/decals, switches), so format coverage is Performer's own. The town, all four vehicles, and the tether demo load and render identically to the direct [pfb2osg](src/loaders/pfb2osg/pfb2osg.cpp) path; set `PFOSG_LOADER=pfb2osg` to use the direct loader instead. Its API surface (~450 tokens/decls beyond the base shim) is harvested from the vendored headers by [tools/harvest_pfb_api.py](tools/harvest_pfb_api.py) into `pf_pfb_api.h` + `pfosg_pfb_stubs.cpp`; the node types the demos use get real implementations in [pfosg_pfb.cpp](src/pfosg/pfosg_pfb.cpp)
- **M3** town on the web — **the full perfly demo runs in a browser** 🎉:
  - OpenSceneGraph 3.6.5 cross-compiled from source for Emscripten with the **GLES2 profile** (no fixed-function pipeline), static libs, windowing-system `None` — see [Web build](#web) above
  - `townview` builds to wasm+WebGL: the town database (bundled into the wasm virtual FS) renders through OSG with VBO-only geometry, vertex-attribute aliasing, and a hand-written GLES2 uber shader (OSG 3.6.5's own ShaderGen emits desktop GLSL that WebGL rejects) — sky, mountain backdrop, roads, textured terrain, trees, water, and the circulating vehicles all draw; trackball navigation via SDL2 + the Emscripten main loop
  - **webfly**: SGI's `perfly` itself, compiled unmodified to wasm — the tether ride through town under Asyncify, with the channel-driven camera, animated traffic, EarthSky, and the **GUI control panel + stats overlay live in the browser**. The immediate-mode overlays (SGI's `gui.c`, the stats/messages drawing) run through a batched GLES2 mini-GL — matrix stacks, `glBegin`/`glEnd` vertex batching, `glPushAttrib`/`glPopAttrib` — so the FFP overlay code compiles into the wasm build unchanged
  - **unified GLES2 path**: one `PFOSG_GLES2` source path drives both the web build and a native GLES2 backend (ANGLE-on-Metal), so the browser render path is exercised natively too. A built-in SGI `.rgb` image reader ([pfb2osg](src/loaders/pfb2osg/pfb2osg.cpp)) covers the vehicle/banner textures on the plugin-less static OSG builds
  - **light points**: stoplight bulbs and street lamps are perspective-scaled light points (from the database's own `pfLPointState` sizing) that fill the lamp lens, on cycling `pfSequence`s driven by the update traversal
- **M4** more demos: bring up further shipped SGI samples and databases as desired

## Provenance & licensing

SGI released the Performer header files, the utility libraries (libpfutil, libpfui, libpfdu), the database loaders (libpfdb, including the .pfb loader source), the sample programs (perfly and the pguide examples), and the demo databases (Performer Town) as open/community-shared content — see [data/town/README.md](data/town/README.md) for citations to the archived `oss.sgi.com/projects/performer` project. Those components are committed verbatim in [vendor/Performer](vendor/Performer/README.md) and [data/town](data/town/README.md).

Only the *implementation* of the core libraries (libpf/libpr) remained closed-source. Nothing from the core binaries, or derived from them, is included in this repository: this port implements the Performer API on OpenSceneGraph, against the open headers.
