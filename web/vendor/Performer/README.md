# Vendored OpenGL Performer open-source components

Copied verbatim from SGI's OpenGL Performer 3.0 distribution (Windows
release, 2002-12-10):

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
of the core libraries (libpf/libpr) remained closed. Nothing from the core
binaries or anything derived from them is included here.

These files are intentionally unmodified. The port compiles perfly and the
samples from these sources byte-for-byte, against the pfosg shim.
