#!/usr/bin/env python3
"""Harvest the Performer API surface pfpfb.c needs from the vendored SGI
headers into shim-consumable files.

Reads /tmp/missing_ids.txt, /tmp/missing_types.txt, /tmp/missing_funcs.txt
(produced by the clang error sweep) and the vendored headers, and writes:

  src/pfosg/include/Performer/pf_pfb_api.h   tokens + typedefs + extern decls
  src/pfosg/pfosg_pfb_stubs.cpp              accept-and-ignore definitions

Functions listed in HAND_IMPLEMENTED are declared but not stub-generated;
their real implementations live in pfosg_pfb.cpp.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
INC = ROOT / "vendor/Performer/Include/Performer"
HEADERS = [INC / "pf.h", INC / "pr.h", INC / "pfstats.h", INC / "prstats.h",
           INC / "opengl.h"]

# implemented for real in pfosg_pfb.cpp (or already in the shim)
HAND_IMPLEMENTED = {
    "pfGetGSetPrimLengths", "pfNewBboard", "pfBboardMode", "pfBboardAxis",
    "pfBboardPos", "pfGetBboardClassType", "pfNewSeq", "pfSeqDuration",
    "pfSeqInterval", "pfSeqMode", "pfSeqTime", "pfGetSeqClassType",
    "pfNewLayer", "pfLayerMode", "pfGetLayerClassType", "pfSwitchVal",
    "pfGetSwitchValFlux", "pfRef", "pfUnref", "pfGetRef", "pfGetArena",
    "pfRealloc", "pfGetNodeTravMask", "pfTexRepeat", "pfGetTexRepeat",
    "pfTexFormat", "pfGetTexFormat", "pfTexFilter", "pfGetTexFilter",
    "pfTexImage", "pfGetTexImage", "pfGSetDrawMode", "pfGetGSetDrawMode",
    "pfGSetLineWidth", "pfGetGSetLineWidth", "pfGSetPntSize",
    "pfGetGSetPntSize", "pfGSetIsectMask", "pfGetGSetIsectMask",
    "pfGetGSetAttrBind", "pfGetGSetClassType", "pfMtlShininess",
    "pfGetMtlShininess", "pfMtlAlpha", "pfGetMtlAlpha", "pfMtlSide",
    "pfLModelTwoSide", "pfGetLightPos", "pfLightAtten",
    "pfGetLODCenter", "pfLODCenter", "pfGetLODRange",
}

ids = [l.strip() for l in open("/tmp/missing_ids.txt") if l.strip()]
types = [l.strip() for l in open("/tmp/missing_types.txt") if l.strip()]
funcs = [l.strip() for l in open("/tmp/missing_funcs.txt") if l.strip()]

hdr_text = {h: h.read_text(errors="replace") for h in HEADERS if h.exists()}

def find_define(name):
    for h, text in hdr_text.items():
        m = re.search(rf"^[ \t]*#define[ \t]+{re.escape(name)}\b[^\n]*",
                      text, re.M)
        if m:
            line = m.group(0)
            # follow line continuations
            pos = m.end()
            while line.rstrip().endswith("\\"):
                nl = text.find("\n", pos)
                line = line.rstrip()[:-1] + "\n" + text[pos:nl]
                pos = nl + 1
            return line
    return None

def find_typedef(name):
    for h, text in hdr_text.items():
        m = re.search(rf"^typedef[^\n;]*\b{re.escape(name)}\s*;", text, re.M)
        if m:
            return m.group(0)
        m = re.search(
            rf"^typedef[^;{{]*\(\s*\*\s*{re.escape(name)}\s*\)[^;]*;",
            text, re.M | re.S)
        if m:
            return " ".join(m.group(0).split())
    return None

def find_decl(name):
    for h, text in hdr_text.items():
        m = re.search(
            rf"^extern[ \t]+(?:DLLEXPORT[ \t]+)?[^;]*?\b{re.escape(name)}\s*\([^;]*?\)\s*;",
            text, re.M | re.S)
        if m:
            return " ".join(m.group(0).split())
    return None

tokens_out, missing_tokens = [], []
skip_ids = set(types) | {
    "True", "False", "RTLD_LAZY",  # X / dlfcn, handled elsewhere
}
for name in ids:
    if name in skip_ids or not name[0].isupper() and not name.startswith("pf"):
        continue
    if name.startswith("pf"):    # type name, not a token
        continue
    d = find_define(name)
    if d:
        # a harvested line may open a comment that closed on the next line
        # of the original header; balance it or it swallows what follows
        if d.count("/*") > d.count("*/"):
            d += " */"
        if d.count("*/") > d.count("/*"):
            d = d[: d.rindex("*/")]
        tokens_out.append(d)
    else:
        missing_tokens.append(name)

# opaque struct types referenced by harvested decls/typedefs but never
# defined in what we consume (calligraphics, raster, ISL shader lib)
FORCED_OPAQUE = {"pfCalligData", "pfRasterData", "islAppearance"}

# structs pfpfb.c allocates/subscripts: extracted with full definitions
REAL_STRUCTS = ["pfASDFace", "pfASDVert", "pfASDLODRange"]

def find_struct(name):
    for h, text in hdr_text.items():
        m = re.search(
            rf"^typedef struct _?{re.escape(name)}\b.*?^\}}\s*{re.escape(name)}\s*;",
            text, re.M | re.S)
        if m:
            return m.group(0)
    return None

# transitive closure: harvested defines may expand to further SGI tokens
# (TX_*, FG_*, PFTEX_* chains in opengl.h); pull those in too, guarded
emitted = {re.match(r"[ \t]*#define[ \t]+(\w+)", t).group(1)
           for t in tokens_out}
changed = True
while changed:
    changed = False
    refs = set()
    for t in tokens_out:
        body = t.split(None, 2)[2] if len(t.split(None, 2)) > 2 else ""
        refs |= set(re.findall(r"\b[A-Z][A-Z0-9]*_[A-Z0-9_]+\b", body))
    for name in sorted(refs - emitted):
        if name.startswith("GL_"):
            continue                    # provided by GL headers / compat
        d = find_define(name)
        if d:
            if d.count("/*") > d.count("*/"):
                d += " */"
            tokens_out.append(f"#ifndef {name}\n{d}\n#endif")
            emitted.add(name)
            changed = True

type_names = sorted(set(types) | FORCED_OPAQUE | {
    n for n in ids
    if n.startswith("pf") and not find_define(n)
})
typedefs_out, functype_out, structs_out = [], [], []
for name in REAL_STRUCTS:
    sd = find_struct(name)
    if sd:
        structs_out.append(sd)
    else:
        print("  MISSING STRUCT:", name)
for name in type_names:
    if name in REAL_STRUCTS:
        continue
    if name.endswith("FuncType") or name.endswith("Type"):
        td = find_typedef(name)
        functype_out.append(td if td else
                            f"typedef void (*{name})(void*);")
    else:
        typedefs_out.append(f"typedef struct {name} {name};")

decls_out, missing_decls, stub_bodies = [], [], []
def default_return(ret):
    ret = ret.replace("extern", "").replace("DLLEXPORT", "").strip()
    if ret == "void":
        return ""
    if "*" in ret:
        return " return 0;"
    if ret in ("float", "double"):
        return " return 0.0f;"
    return " return 0;"

for name in funcs:
    if name in ("dlopen", "dlsym", "fcos", "fsin", "PFASSERTDEBUG"):
        continue
    d = find_decl(name)
    if not d:
        missing_decls.append(name)
        continue
    decls_out.append(d)
    if name in HAND_IMPLEMENTED:
        continue
    # build a stub definition: strip extern/DLLEXPORT, keep signature
    sig = d[: d.rindex(";")]
    sig = re.sub(r"^extern\s+(DLLEXPORT\s+)?", "", sig)
    m = re.match(r"(.*?)\b" + re.escape(name) + r"\s*\(", sig)
    ret = m.group(1).strip() if m else "int"
    stub_bodies.append(f"extern \"C\" {sig}\n{{{default_return(ret)} }}")

GL_COMPAT = """\
/* SGI's token defines expand to GL and IRIS-GL identifiers; pull in the
 * platform GL header and fill the EXT_texture-era gaps it no longer has */
#include <GL/gl.h>
#ifndef TX_SELECT
#define TX_SELECT 0x303
#endif
#ifndef GL_UNSIGNED_SHORT_5_5_5_1_EXT
#define GL_UNSIGNED_SHORT_5_5_5_1_EXT 0x8034
#endif
#ifndef GL_LUMINANCE8_ALPHA8_EXT
#define GL_LUMINANCE8_ALPHA8_EXT 0x8045
#endif
#ifndef GL_LUMINANCE12_ALPHA4_EXT
#define GL_LUMINANCE12_ALPHA4_EXT 0x8046
#endif
#ifndef GL_LUMINANCE12_ALPHA12_EXT
#define GL_LUMINANCE12_ALPHA12_EXT 0x8047
#endif
#ifndef GL_INTENSITY8_EXT
#define GL_INTENSITY8_EXT 0x804B
#endif
#ifndef GL_INTENSITY16_EXT
#define GL_INTENSITY16_EXT 0x804D
#endif
#ifndef GL_RGB4_EXT
#define GL_RGB4_EXT 0x804F
#endif
#ifndef GL_RGB5_EXT
#define GL_RGB5_EXT 0x8050
#endif
#ifndef GL_RGB12_EXT
#define GL_RGB12_EXT 0x8053
#endif
#ifndef GL_RGBA4_EXT
#define GL_RGBA4_EXT 0x8056
#endif
#ifndef GL_RGB5_A1_EXT
#define GL_RGB5_A1_EXT 0x8057
#endif
#ifndef GL_RGBA8_EXT
#define GL_RGBA8_EXT 0x8058
#endif
#ifndef GL_RGBA12_EXT
#define GL_RGBA12_EXT 0x805A
#endif
#ifndef GL_TEXTURE_WRAP_R_EXT
#define GL_TEXTURE_WRAP_R_EXT 0x8072
#endif
"""

api = ROOT / "src/pfosg/include/Performer/pf_pfb_api.h"
api.write_text(
    "/* pf_pfb_api.h - GENERATED by tools/harvest_pfb_api.py: the API\n"
    " * surface SGI's pfpfb.c needs beyond the base shim.  Tokens, typedefs\n"
    " * and declarations are extracted verbatim from the vendored SGI\n"
    " * headers.  Included from the shim's pf.h; do not hand-edit. */\n"
    "#ifndef PFOSG_PF_PFB_API_H\n#define PFOSG_PF_PFB_API_H\n\n"
    + GL_COMPAT + "\n"
    "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"
    "/* ---- opaque types ---- */\n" + "\n".join(typedefs_out) + "\n\n"
    "/* ---- real structs (verbatim from SGI headers) ---- */\n" +
    "\n\n".join(structs_out) + "\n\n"
    "/* ---- callback types ---- */\n" + "\n".join(functype_out) + "\n\n"
    "/* ---- tokens (verbatim from SGI headers) ---- */\n" +
    "\n".join(tokens_out) + "\n\n"
    "/* ---- declarations (verbatim from SGI headers) ---- */\n" +
    "\n".join(decls_out) + "\n\n"
    "#ifdef __cplusplus\n}\n#endif\n#endif\n")

stubs = ROOT / "src/pfosg/pfosg_pfb_stubs.cpp"
stubs.write_text(
    "/* pfosg_pfb_stubs.cpp - GENERATED by tools/harvest_pfb_api.py:\n"
    " * accept-and-ignore implementations of the Performer API surface\n"
    " * referenced by SGI's pfpfb.c but not needed by the demo databases\n"
    " * (fluxes, engines, image caches, IBR, ASD, clip textures, fonts...).\n"
    " * Real implementations live in pfosg_pfb.cpp; do not hand-edit. */\n"
    "#include <Performer/pf.h>\n\n" + "\n".join(stub_bodies) + "\n")

print(f"tokens: {len(tokens_out)}  (missing: {len(missing_tokens)})")
for t in missing_tokens:
    print("  MISSING TOKEN:", t)
print(f"typedefs: {len(typedefs_out)}  functypes: {len(functype_out)}")
print(f"decls: {len(decls_out)}  (missing: {len(missing_decls)})")
for f in missing_decls:
    print("  MISSING DECL:", f)
print(f"stubs: {len(stub_bodies)}")
