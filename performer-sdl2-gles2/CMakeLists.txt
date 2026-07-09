cmake_minimum_required(VERSION 3.16)
project(performer-port C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prefer dylibs (Homebrew) over /Library/Frameworks — stray SDL2.framework
# installs there are often unsigned and refused by macOS at load time.
set(CMAKE_FIND_FRAMEWORK LAST)

option(PF_WITH_OSG "Build the OSG-backed Performer shim (stage A)" ON)

# ---------------------------------------------------------------------------
# SDL2
# ---------------------------------------------------------------------------
if(EMSCRIPTEN)
    # SDL2 comes from Emscripten ports (-sUSE_SDL=2), no find_package needed.
    add_library(pf_sdl2 INTERFACE)
    target_compile_options(pf_sdl2 INTERFACE "-sUSE_SDL=2")
    target_link_options(pf_sdl2 INTERFACE "-sUSE_SDL=2")
    add_library(SDL2::SDL2 ALIAS pf_sdl2)
else()
    find_package(SDL2 REQUIRED CONFIG)
endif()

# ---------------------------------------------------------------------------
# OpenSceneGraph (stage-A backend only; never built for Emscripten)
# ---------------------------------------------------------------------------
if(PF_WITH_OSG AND NOT EMSCRIPTEN)
    find_package(OpenSceneGraph COMPONENTS osg osgDB osgUtil osgViewer osgGA)
    if(OPENSCENEGRAPH_FOUND)
        message(STATUS "OSG ${OPENSCENEGRAPH_VERSION} found - stage-A backend enabled")
    else()
        message(STATUS "OSG not found - stage-A backend disabled")
    endif()
elseif(PF_WITH_OSG AND EMSCRIPTEN AND PF_EM_OSG)
    # M3 web build: an OSG cross-compiled for Emscripten with the GLES2
    # profile (built from external/OpenSceneGraph; see README).  PF_EM_OSG
    # points at that OSG source root (its build tree is <root>/build-em).
    set(_osgsrc ${PF_EM_OSG})
    set(_osglib ${PF_EM_OSG}/build-em/lib)
    if(EXISTS ${_osglib}/libosg.a)
        set(OPENSCENEGRAPH_INCLUDE_DIRS
            ${_osgsrc}/include ${_osgsrc}/build-em/include)
        # static libs, most-dependent first
        set(OPENSCENEGRAPH_LIBRARIES
            ${_osglib}/libosgViewer.a ${_osglib}/libosgGA.a
            ${_osglib}/libosgUtil.a   ${_osglib}/libosgText.a
            ${_osglib}/libosgDB.a     ${_osglib}/libosg.a
            ${_osglib}/libOpenThreads.a)
        set(OPENSCENEGRAPH_FOUND TRUE)
        message(STATUS "Emscripten OSG at ${_osgsrc} - web backend enabled")
    else()
        message(FATAL_ERROR "PF_EM_OSG=${PF_EM_OSG} has no build-em/lib/libosg.a")
    endif()
endif()

# ---------------------------------------------------------------------------
# Original Performer assets (headers, sample source, data) - user-provided,
# dropped into external/.  Targets that need them are only configured when
# they are present.
# ---------------------------------------------------------------------------
set(PF_EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)
# SGI open-sourced the Performer headers, utility libs, loaders, samples and
# demo data; those live vendored in vendor/Performer (committed).  A full
# distribution dropped into external/ (local-only) is used as fallback and
# for anything beyond the open-sourced set.
foreach(_pf_root ${CMAKE_SOURCE_DIR}/vendor/Performer/Include
                 ${PF_EXTERNAL_DIR}/Include
                 ${PF_EXTERNAL_DIR}/include)
    if(EXISTS ${_pf_root}/Performer/pr/pfLinMath.h)
        set(PF_HAVE_HEADERS TRUE)
        set(PF_PERFORMER_INCLUDE_DIR ${_pf_root})
        break()
    endif()
endforeach()
foreach(_pf_src ${CMAKE_SOURCE_DIR}/vendor/Performer/Src
                ${PF_EXTERNAL_DIR}/Src)
    if(EXISTS ${_pf_src}/sample/C/perfly/perfly.c)
        set(PF_PERFORMER_SRC_DIR ${_pf_src})
        break()
    endif()
endforeach()
if(PF_HAVE_HEADERS)
    message(STATUS "Performer headers found at ${PF_PERFORMER_INCLUDE_DIR} - libpr targets enabled")
else()
    set(PF_HAVE_HEADERS FALSE)
    message(STATUS "Performer headers NOT found (vendor/Performer/Include or "
                   "external/Include) - Performer-API targets skipped")
endif()

add_subdirectory(apps/hello_pf)

if(PF_HAVE_HEADERS)
    add_subdirectory(src/libpr)
    if(TARGET pr_math)              # only with the local-only slices present
        add_subdirectory(apps/pr_math_test)
    endif()
endif()

if(OPENSCENEGRAPH_FOUND)
    # self-contained: needs only OSG + the demo data in data/town
    add_subdirectory(src/loaders/pfb2osg)
    add_subdirectory(apps/townview)

    # the pfosg shim builds on both desktop and web.  The sample pguide
    # programs and perfly's GUI panel use immediate-mode GL that GLES2/WebGL
    # lacks; on the web perfly runs GUI-less (webfly), so the pguide samples
    # stay desktop-only.
    if(PF_HAVE_HEADERS)
        add_subdirectory(src/pfosg)
        if(NOT EMSCRIPTEN)
            add_subdirectory(apps/simple)
            add_subdirectory(apps/pguide)
            add_subdirectory(apps/shim_tests)
            add_subdirectory(apps/perfly)
        else()
            add_subdirectory(apps/webfly)
        endif()
    elseif(NOT EMSCRIPTEN)
        message(STATUS "pfosg shim + perfly skipped (no Performer headers); "
                       "townview still builds against data/town")
    endif()
endif()
