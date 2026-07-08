# Performer → SDL2 + GLES2 port

Goal: run classic OpenGL Performer demos (perfly + Performer Town) on modern platforms — native Windows/macOS/Linux and the web via Emscripten — by reimplementing the Performer API subset those demos need.

## Strategy

Build just enough of a Performer implementation — on OpenSceneGraph + SDL2 + GLES2 — to run the demos we want to run. The public surface is the original Performer API ([vendor/Performer/Include](vendor/Performer/README.md)), consumed verbatim so SGI's shipped demo programs compile unmodified; the implementation behind it (the pfosg shim) maps that API onto OSG, with SDL2 owning the window/input and the GL usage kept inside the GLES2-compatible subset so the same stack can target Emscripten/WebGL for the web builds.

Scope is demo-driven, not completeness-driven: whatever perfly and the sample programs actually call gets a real implementation; IRIX hardware machinery they merely mention gets accepted-and-ignored stubs.

Key simplifications vs. real Performer:

- Single process only (`PFMP_APPCULLDRAW`) — mandatory for wasm, supported by perfly; APP/CULL/DRAW run inline in `pfFrame()`.
- One pfPipe / one pfPipeWindow / one SDL2 window.
- GL strategy: code strictly to the GLES2 subset. Native ES2 contexts where drivers offer them (Linux/Windows/ANGLE), desktop GL 2.1 fallback on macOS, WebGL1 on Emscripten. VBO-only drawing (avoids Emscripten `-sFULL_ES2`).

## Layout

```
apps/hello_pf/        M0 bring-up: SDL2 window + ES2-subset shader triangle (native + wasm)
apps/townview/        standalone OSG viewer for .pfb databases
apps/simple/          SGI pguide sample, compiled unmodified against the shim
apps/pguide/          more unmodified SGI samples: texture, lod_func, earthsky, intersect
apps/perfly/          SGI's perfly, compiled unmodified against the shim
apps/shim_tests/      shim regression tests (intersection)
src/loaders/pfb2osg/  direct .pfb/.pfi → OSG loader
src/pfosg/            the stage-A shim: Performer C API over OSG + SDL2
src/compat/           shims that let the original Windows-flavored headers compile on modern clang
src/libpr/            object-system experiments (not part of the demo build)
vendor/Performer/     SGI's open-sourced Performer headers, utilities, and demos verbatim
data/town/            the Performer Town demo database, textures, paths, vehicles
data/town-osg.perfly  demo config: drive mode at the shipped street-level start
data/town-tether.perfly  demo config: original tether start - ride the Esprit
```

## Building

Everything builds from a bare clone; the SGI headers/sources and the town demo data are in the repo.

Native (macOS/Linux; needs CMake, SDL2 and OpenSceneGraph — e.g. `brew install cmake ninja sdl2 open-scene-graph`):

```
cmake -B build -G Ninja && cmake --build build
./build/apps/perfly/perfly data/town-osg.perfly              # the classic demo
./build/apps/townview/townview data/town/town_ogl_pfi.pfb    # standalone viewer
```

perfly starts in drive mode at the shipped street-level position: left mouse accelerates, pointer steers, right mouse brakes/reverses, quick middle-click stops. Pass `-f` for fly mode or a bare `.pfb` path for trackball mode.

### Web build

The town's 3D scene runs in the browser via OpenSceneGraph cross-compiled for
Emscripten with the GLES2/WebGL profile. First build OSG from source for
Emscripten (once):

```
git clone --depth 1 --branch OpenSceneGraph-3.6.5 \
    https://github.com/openscenegraph/OpenSceneGraph.git external/OpenSceneGraph
cd external/OpenSceneGraph
emcmake cmake -B build-em -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=14 \
    -DOPENGL_PROFILE=GLES2 -DDYNAMIC_OPENSCENEGRAPH=OFF \
    -DOSG_WINDOWING_SYSTEM=None -DBUILD_OSG_APPLICATIONS=OFF \
    -DOSG_GL1_AVAILABLE=OFF -DOSG_GL2_AVAILABLE=OFF -DOSG_GLES2_AVAILABLE=ON \
    -DOSG_GL_DISPLAYLISTS_AVAILABLE=OFF -DOSG_GL_MATRICES_AVAILABLE=OFF \
    -DOSG_GL_VERTEX_FUNCS_AVAILABLE=OFF -DOSG_GL_FIXED_FUNCTION_AVAILABLE=OFF \
    -DEGL_LIBRARY=EGL -DOPENGL_egl_LIBRARY=EGL
cmake --build build-em --target osg osgDB osgUtil osgGA osgViewer
cd ../..
```

Then build the web app (townview + the town data bundled into the wasm FS):

```
emcmake cmake -B build-web -G Ninja -DPF_EM_OSG=$PWD/external/OpenSceneGraph
cmake --build build-web --target townview
emrun build-web/apps/townview/townview.html          # the town, in a browser
```

The M0 hello-triangle still builds too:
`cmake --build build-web --target hello_pf && emrun build-web/apps/hello_pf/hello_pf.html`

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
- **M3** town on the web — **scene renders in a browser** 🎉:
  - OpenSceneGraph 3.6.5 cross-compiled from source for Emscripten with the **GLES2 profile** (no fixed-function pipeline), static libs, windowing-system `None` — see [Web build](#web-build) below
  - `townview` builds to wasm+WebGL: the town database (bundled into the wasm virtual FS) renders through OSG with VBO-only geometry, vertex-attribute aliasing, and a hand-written GLES2 uber shader (OSG 3.6.5's own ShaderGen emits desktop GLSL that WebGL rejects) — sky, mountain backdrop, roads, textured terrain, trees, water, and the circulating vehicles all draw; trackball navigation via SDL2 + the Emscripten main loop
  - scene-first: the perfly GUI panel and stats overlay use immediate-mode GL (`glBegin`/`glVertex`, incl. SGI's unmodified `gui.c`) that GLES2 lacks; a follow-on immediate-mode emulation layer (or Emscripten's `regal` port) brings those to the web
- **M4** more demos: bring up further shipped SGI samples and databases as desired

## Provenance & licensing

SGI released the Performer header files, the utility libraries (libpfutil, libpfui, libpfdu), the database loaders (libpfdb, including the .pfb loader source), the sample programs (perfly and the pguide examples), and the demo databases (Performer Town) as open/community-shared content — see [data/town/README.md](data/town/README.md) for citations to the archived `oss.sgi.com/projects/performer` project. Those components are committed verbatim in [vendor/Performer](vendor/Performer/README.md) and [data/town](data/town/README.md).

Only the *implementation* of the core libraries (libpf/libpr) remained closed-source. Nothing from the core binaries, or derived from them, is included in this repository: this port implements the Performer API on OpenSceneGraph, against the open headers.
