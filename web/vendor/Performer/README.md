# Vendored OpenGL Performer open-source components

Copied verbatim from SGI's OpenGL Performer 3.0 distribution (Windows
release, 2002-12-10; installer OpenGLPerformer300.exe, SHA-256
1f9c8310c92ed8d51b1bc902f8aeea9f50deb83d15cb4277933730d422da522f):

- `Include/` — the Performer header files
- `Src/sample/` — sample applications (perfly, the common viewer framework)
- `Src/pguide/` — programming-guide examples
- `Src/lib/` — utility libraries shipped as source (libpfutil, libpfui,
  libpfdu, and the libpfdb database loaders, including the .pfb loader)
- `Src/tools`, `Src/conv` — converters and tools

SGI released the Performer headers, utility libraries, loaders, samples and
demo databases as open/community-shared content (see the archived
`oss.sgi.com/projects/performer` project and its CVS repository, linked from
[data/town/README.md](../../data/town/README.md)); only the implementation
of the core libraries (libpf/libpr) remained closed. The core library is
reimplemented on OpenSceneGraph by the pfosg shim in src/pfosg.

These files are intentionally unmodified. The port compiles perfly and the
samples from these sources byte-for-byte, against the pfosg shim.
