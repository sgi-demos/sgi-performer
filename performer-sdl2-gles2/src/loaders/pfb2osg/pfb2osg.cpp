/* ============================================================================
 *  pfb2osg - direct OpenGL Performer .pfb reader building an OSG scene graph.
 *
 *  Stage-A (OSG backend) bring-up loader.  The binary format was transcribed
 *  from the loader source SGI shipped with Performer 3.0
 *  (external/Src/lib/libpfdb/libpfpfb/pfpfb.c); this file implements the
 *  subset of list types present in the town database (pfb version 18):
 *      LLIST VLIST CLIST NLIST TLIST ILIST MTL TEX TENV TGEN LPSTATE
 *      GSTATE GSET NODE
 *  Unknown lists are skipped with a warning (their byte size is in the list
 *  header).  Texture images are external .pfi files resolved relative to the
 *  .pfb's directory.
 *
 *  Simplifications (documented, revisit for the native backend which will
 *  port pfpfb.c verbatim):
 *   - every geoset is flattened to non-indexed per-vertex arrays (Performer
 *     allows an independent index list per attribute; OSG does not),
 *   - PER_PRIM bindings are expanded to per-vertex,
 *   - FLAT_* strip variants replicate the first color/normal for the two
 *     (one, for lines) leading vertices of each strip,
 *   - TEXGEN and LPOINTSTATE state elements are parsed but ignored.
 * ==========================================================================*/

#include "pfb2osg.h"

#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/AlphaFunc>
#include <osg/CullFace>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/LOD>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/PolygonOffset>
#include <osg/Sequence>
#include <osg/ShadeModel>
#include <osg/Switch>
#include <osg/TexEnv>
#include <osg/Texture2D>
#include <osgDB/ReadFile>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace pfb2osg {

/* ---- tokens from pfpfb.c ------------------------------------------------- */

static const uint32_t PFB_MAGIC    = 0xdb0ace00u;
static const uint32_t PFB_MAGIC_LE = 0x00ce0adbu;

enum {
    L_MTL = 0, L_TEX, L_TENV, L_GSTATE, L_LLIST, L_VLIST, L_CLIST, L_NLIST,
    L_TLIST, L_ILIST, L_GSET, L_UDATA, L_NODE, L_FLIST, L_MORPH, L_LODSTATE,
    L_FOG, L_TGEN, L_LMODEL, L_LIGHT, L_CTAB, L_LPSTATE, L_HLIGHT, L_LSOURCE,
    L_FRUST, L_FONT, L_STRING, L_IMAGE, L_CUSTOM, L_TLOD, L_ASDDATA, L_QUEUE,
    L_ITILE, L_ICACHE, L_FLUX, L_ENGINE, L_UFUNC, L_UDATA_NAME, L_UDATA_LIST,
    L_SG_NAME, L_SHADER, L_IBR_TEX, L_APPEARANCE, L_COUNT
};

enum {
    STATE_TRANSPARENCY = 1, STATE_ANTIALIAS, STATE_DECAL, STATE_ALPHAFUNC,
    STATE_ENLIGHTING, STATE_ENTEXTURE, STATE_ENFOG, STATE_CULLFACE,
    STATE_ENWIREFRAME, STATE_ENCOLORTABLE, STATE_ENHIGHLIGHTING,
    STATE_ENLPOINTSTATE, STATE_ENTEXGEN, STATE_ALPHAREF, STATE_FRONTMTL,
    STATE_BACKMTL, STATE_TEXTURE, STATE_TEXENV, STATE_FOG, STATE_LIGHTMODEL,
    STATE_LIGHTS, STATE_COLORTABLE, STATE_HIGHLIGHT, STATE_LPOINTSTATE,
    STATE_TEXGEN, STATE_ENTEXMAT, STATE_TEXMAT, STATE_ENTEXLOD, STATE_TEXLOD
};

/* NOTE: every *_table[] in pfpfb.c stores its size at index 0, so all values
 * stored in the file are 1-BASED indices into those tables. */

/* gspt_table order (1-based) */
enum {
    PT_TRISTRIPS = 1, PT_TRIS, PT_POINTS, PT_LINES, PT_LINESTRIPS,
    PT_FLAT_LINESTRIPS, PT_QUADS, PT_FLAT_TRISTRIPS, PT_POLYS, PT_TRIFANS,
    PT_FLAT_TRIFANS
};

/* gsb_table order (1-based) */
enum { B_OFF = 1, B_PER_VERTEX, B_PER_PRIM, B_OVERALL };

/* oo_table: 1 = PF_OFF, 2 = PF_ON */
static inline bool onoff(int v) { return v == 2; }

enum {
    N_LIGHTPOINT = 0, N_TEXT, N_GEODE, N_BILLBOARD, N_LIGHTSOURCE, N_GROUP,
    N_SCS, N_DCS, N_PARTITION, N_SCENE, N_SWITCH, N_LOD, N_SEQUENCE, N_LAYER,
    N_MORPH, N_ASD, N_FCS, N_DOUBLE_DCS, N_DOUBLE_FCS, N_DOUBLE_SCS,
    N_IBR_NODE
};
static const int N_NOT_CUSTOM_MASK = 0x0000ffff;
static const int PFB_SWITCH_ON = -1, PFB_SWITCH_OFF = -2;

static const int PFBV_NODE_BSPHERE = 5, PFBV_GSET_DO_DP = 5,
                 PFBV_GSET_BBOX_FLUX = 8, PFBV_SWITCH_VAL_FLUX = 11,
                 PFBV_UFUNC = 12, PFBV_UDATA_SLOT_FUNCS = 16,
                 PFBV_MULTITEXTURE = 19, PFBV_ANISOTROPY = 20,
                 PFBV_CLIPTEXTURE = 6;

/* ---- byte-swapping reader ------------------------------------------------ */

class Reader {
public:
    FILE* fp = nullptr;
    bool swap = false;
    bool error = false;

    ~Reader() { if (fp) fclose(fp); }

    void readRaw(void* dst, size_t n)
    {
        if (fread(dst, 1, n, fp) != n) error = true;
    }
    int32_t i32()
    {
        uint32_t v = 0;
        readRaw(&v, 4);
        if (swap) v = __builtin_bswap32(v);
        return (int32_t)v;
    }
    uint32_t u32() { return (uint32_t)i32(); }
    float f32()
    {
        uint32_t v = u32();
        float f;
        memcpy(&f, &v, 4);
        return f;
    }
    void i32v(int32_t* dst, size_t n)  { for (size_t i = 0; i < n; i++) dst[i] = i32(); }
    void f32v(float* dst, size_t n)    { for (size_t i = 0; i < n; i++) dst[i] = f32(); }
    void u16v(uint16_t* dst, size_t n)
    {
        readRaw(dst, n * 2);
        if (swap)
            for (size_t i = 0; i < n; i++)
                dst[i] = (uint16_t)((dst[i] >> 8) | (dst[i] << 8));
    }
    std::string pstring()          /* int length (-1 = none) + chars */
    {
        int32_t n = i32();
        if (n < 0) return std::string();
        std::string s((size_t)n, '\0');
        readRaw(&s[0], (size_t)n);
        return s;
    }
};

/* an int/float word buffer, mirroring pfpfb.c's buf[buf_pos++] idiom */
struct WordBuf {
    std::vector<int32_t> w;
    size_t pos = 0;
    int32_t i()  { return pos < w.size() ? w[pos++] : 0; }
    uint32_t u() { return (uint32_t)i(); }
    float f()
    {
        float v = 0;
        if (pos < w.size()) memcpy(&v, &w[pos], 4);
        pos++;
        return v;
    }
    void read(Reader& r, size_t n)
    {
        w.resize(n);
        r.i32v(w.data(), n);
        pos = 0;
    }
};

/* ---- parsed-list storage -------------------------------------------------- */

struct GState {
    osg::ref_ptr<osg::StateSet> ss;
};

struct NodeRec {
    WordBuf buf;               /* the node's word payload, cursor after type */
    int type = -1;
    std::string name;
};

struct Loader {
    Reader r;
    int version = 0;
    std::string dir;           /* directory of the .pfb, for .pfi lookup */

    std::vector<std::vector<int32_t>>            llist;
    std::vector<osg::ref_ptr<osg::Vec3Array>>    vlist;
    std::vector<osg::ref_ptr<osg::Vec4Array>>    clist;
    std::vector<osg::ref_ptr<osg::Vec3Array>>    nlist;
    std::vector<osg::ref_ptr<osg::Vec2Array>>    tlist;
    std::vector<std::vector<uint16_t>>           ilist;
    std::vector<osg::ref_ptr<osg::Material>>     mtl;
    std::vector<osg::ref_ptr<osg::Texture2D>>    tex;
    std::vector<osg::ref_ptr<osg::TexEnv>>       tenv;
    std::vector<GState>                          gstate;
    std::vector<osg::ref_ptr<osg::Geometry>>     gset;
    std::vector<NodeRec>                         nodes;
    std::vector<osg::ref_ptr<osg::Node>>         osgNodes;

    int warnedTexgen = 0;

    bool load(const std::string& path);
    void skipListHeaderPayload(int32_t bytes) { fseek(r.fp, bytes, SEEK_CUR); }

    template <class ArrayT, class EltReader>
    void readSlist(std::vector<osg::ref_ptr<ArrayT>>& out, EltReader elt);
    void readLlist();
    void readIlist();
    void readMtl();
    void readTex();
    void readTenv();
    void readGState();
    void readGSet();
    void readNode(int index);
    osg::ref_ptr<osg::Image> loadPfi(const std::string& name);
    void buildNodes();
    osg::ref_ptr<osg::Node> root;
};

/* slist framing: {count, memtype, udata} then count elements */
template <class ArrayT, class EltReader>
void Loader::readSlist(std::vector<osg::ref_ptr<ArrayT>>& out, EltReader elt)
{
    int32_t hdr[3];
    r.i32v(hdr, 3);
    osg::ref_ptr<ArrayT> a = new ArrayT();
    a->reserve(hdr[0] > 0 ? hdr[0] : 0);
    for (int32_t i = 0; i < hdr[0]; i++)
        a->push_back(elt(r));
    out.push_back(a);
}

void Loader::readLlist()
{
    int32_t hdr[3];
    r.i32v(hdr, 3);
    std::vector<int32_t> v((size_t)(hdr[0] > 0 ? hdr[0] : 0));
    r.i32v(v.data(), v.size());
    llist.push_back(std::move(v));
}

void Loader::readIlist()
{
    int32_t hdr[3];
    r.i32v(hdr, 3);
    std::vector<uint16_t> v((size_t)(hdr[0] > 0 ? hdr[0] : 0));
    r.u16v(v.data(), v.size());
    ilist.push_back(std::move(v));
}

void Loader::readMtl()
{
    /* mtl_t: side alpha shininess ambient3 diffuse3 specular3 emission3
     *        cmode[2] udata  (18 words) */
    WordBuf b;
    b.read(r, 18);
    int side = b.i();
    (void)side;
    float alpha = b.f(), shininess = b.f();
    osg::Vec3 amb(b.f(), b.f(), b.f());
    osg::Vec3 dif(b.f(), b.f(), b.f());
    osg::Vec3 spe(b.f(), b.f(), b.f());
    osg::Vec3 emi(b.f(), b.f(), b.f());
    int cmode_side = b.i();
    int cmode = b.i();
    (void)cmode_side;

    osg::ref_ptr<osg::Material> m = new osg::Material;
    m->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(amb, alpha));
    m->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(dif, alpha));
    m->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(spe, alpha));
    m->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(emi, alpha));
    m->setShininess(osg::Material::FRONT_AND_BACK, shininess);
    /* cmode_table (1-based): 1 off, 2 AMBIENT_AND_DIFFUSE, 3 AMBIENT,
     *              4 DIFFUSE, 5 SPECULAR, 6 EMISSION, 7 COLOR(=A&D) */
    static const osg::Material::ColorMode cm[] = {
        osg::Material::OFF, osg::Material::OFF,
        osg::Material::AMBIENT_AND_DIFFUSE,
        osg::Material::AMBIENT, osg::Material::DIFFUSE,
        osg::Material::SPECULAR, osg::Material::EMISSION,
        osg::Material::AMBIENT_AND_DIFFUSE };
    if (cmode >= 1 && cmode <= 7) m->setColorMode(cm[cmode]);
    mtl.push_back(m);
}

osg::ref_ptr<osg::Image> Loader::loadPfi(const std::string& name)
{
    /* strip any path, resolve against the .pfb directory */
    std::string base = name;
    size_t slash = base.find_last_of("/\\");
    if (slash != std::string::npos) base = base.substr(slash + 1);
    std::string path = dir + "/" + base;
    size_t dot = base.find_last_of('.');
    if (dot != std::string::npos && base.substr(dot) != ".pfi") {
        /* .rgb and friends go through OSG's image plugins; static OSG
         * builds have none, so fall back to the built-in SGI reader */
        osg::ref_ptr<osg::Image> img = osgDB::readImageFile(path);
        if (!img)
            img = pfb2osgLoadRgbImage(path);
        if (!img)
            fprintf(stderr, "pfb2osg: cannot read texture image \"%s\"\n",
                    path.c_str());
        return img;
    }
    return pfb2osgLoadPfiImage(path);
}

}   /* namespace pfb2osg — reopened below for load() etc. */

/* SGI image library format (.rgb/.rgba/.bw/.la): 512-byte big-endian
 * header, channel-planar scanlines stored bottom-to-top (GL orientation),
 * verbatim or RLE, 1 byte per component.  Needed because the static OSG
 * builds (GLES2 web/native) carry no image plugins. */
osg::ref_ptr<osg::Image> pfb2osgLoadRgbImage(const std::string& path)
{
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp) return nullptr;
    std::vector<unsigned char> file;
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz < 512) { fclose(fp); return nullptr; }
    file.resize((size_t)sz);
    size_t got = fread(file.data(), 1, (size_t)sz, fp);
    fclose(fp);
    if (got != (size_t)sz) return nullptr;

    const unsigned char* h = file.data();
    auto be16 = [&](size_t o) { return (h[o] << 8) | h[o + 1]; };
    auto be32 = [&](size_t o) -> uint32_t {
        return ((uint32_t)h[o] << 24) | ((uint32_t)h[o+1] << 16) |
               ((uint32_t)h[o+2] << 8) | h[o+3];
    };
    if (be16(0) != 474) {
        fprintf(stderr, "pfb2osg: \"%s\" is not an SGI image\n", path.c_str());
        return nullptr;
    }
    int storage = h[2], bpc = h[3];
    int xsize = be16(6), ysize = be16(8), zsize = be16(10);
    if (bpc != 1 || xsize <= 0 || ysize <= 0 || zsize < 1 || zsize > 4) {
        fprintf(stderr, "pfb2osg: \"%s\": unsupported SGI image "
                "(bpc=%d z=%d)\n", path.c_str(), bpc, zsize);
        return nullptr;
    }

    static const GLenum glfmt[] = { 0, GL_LUMINANCE, GL_LUMINANCE_ALPHA,
                                    GL_RGB, GL_RGBA };
    size_t npix = (size_t)xsize * ysize;
    unsigned char* out = new unsigned char[npix * zsize];

    bool ok = true;
    if (storage == 0) {                       /* verbatim, channel-planar */
        if (file.size() < 512 + npix * zsize) ok = false;
        else
            for (int z = 0; z < zsize; z++) {
                const unsigned char* plane = h + 512 + (size_t)z * npix;
                for (size_t p = 0; p < npix; p++)
                    out[p * zsize + z] = plane[p];
            }
    } else {                                  /* RLE */
        size_t ntab = (size_t)ysize * zsize;
        if (file.size() < 512 + ntab * 8) ok = false;
        for (int z = 0; ok && z < zsize; z++)
            for (int y = 0; ok && y < ysize; y++) {
                size_t ti = 512 + ((size_t)z * ysize + y) * 4;
                uint32_t off = be32(ti), len = be32(ti + ntab * 4);
                if (off + len > file.size()) { ok = false; break; }
                const unsigned char* p = h + off;
                const unsigned char* pend = p + len;
                unsigned char* row = out + (size_t)y * xsize * zsize + z;
                int x = 0;
                while (p < pend) {
                    int c = *p++, count = c & 0x7f;
                    if (!count) break;
                    if (count > xsize - x || (c & 0x80 ? p + count : p + 1) > pend)
                        { ok = false; break; }
                    if (c & 0x80)
                        while (count--) { row[(size_t)x++ * zsize] = *p++; }
                    else {
                        unsigned char v = *p++;
                        while (count--) row[(size_t)x++ * zsize] = v;
                    }
                }
            }
    }
    if (!ok) {
        fprintf(stderr, "pfb2osg: corrupt SGI image \"%s\"\n", path.c_str());
        delete[] out;
        return nullptr;
    }

    osg::ref_ptr<osg::Image> img = new osg::Image;
    img->setImage(xsize, ysize, 1, glfmt[zsize], glfmt[zsize],
                  GL_UNSIGNED_BYTE, out, osg::Image::USE_NEW_DELETE);
    img->setFileName(path);
    return img;
}

osg::ref_ptr<osg::Image> pfb2osgLoadPfiImage(const std::string& path)
{
    using namespace pfb2osg;
    Reader ir;
    ir.fp = fopen(path.c_str(), "rb");
    if (!ir.fp) {
        fprintf(stderr, "pfb2osg: cannot open texture image \"%s\"\n", path.c_str());
        return nullptr;
    }
    uint32_t magic = 0;
    ir.readRaw(&magic, 4);
    if (magic == 0x00db0fdbu)      ir.swap = true;   /* byte-reversed */
    else if (magic != 0xdb0fdb00u) {
        fprintf(stderr, "pfb2osg: \"%s\" is not a .pfi image\n", path.c_str());
        return nullptr;
    }
    /* pfi_header_t after magic: version data_start data_size size[3]
     *                           num_comp format packing gl mipmaps num_images */
    int32_t h[11];
    ir.i32v(h, 11);
    int dataStart = h[1], w = h[3], hgt = h[4], comp = h[6], fmt = h[7];
    if (fmt != 1 /* PFI_FORMAT_UINT_8 */) {
        fprintf(stderr, "pfb2osg: \"%s\": unsupported pfi format %d\n", path.c_str(), fmt);
        return nullptr;
    }
    static const GLenum glfmt[] = { 0, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };
    if (comp < 1 || comp > 4) return nullptr;

    size_t bytes = (size_t)w * hgt * comp;
    unsigned char* data = new unsigned char[bytes];
    fseek(ir.fp, dataStart, SEEK_SET);
    ir.readRaw(data, bytes);
    if (ir.error) {
        fprintf(stderr, "pfb2osg: short read on \"%s\"\n", path.c_str());
        delete[] data;
        return nullptr;
    }
    osg::ref_ptr<osg::Image> img = new osg::Image;
    img->setImage(w, hgt, 1, glfmt[comp], glfmt[comp], GL_UNSIGNED_BYTE,
                  data, osg::Image::USE_NEW_DELETE);
    img->setFileName(path);
    return img;
}

namespace pfb2osg {

void Loader::readTex()
{
    std::string name = r.pstring();

    /* tex_1_t for 6 <= version < 20: 57 words */
    size_t nwords = 57;
    if (version >= PFBV_ANISOTROPY) nwords = 58;
    else if (version < PFBV_CLIPTEXTURE) nwords = 56;
    WordBuf b;
    b.read(r, nwords);

    /* offsets within tex_1_t (see pfpfb.c TEX_0_DATA) */
    int32_t* t = b.w.data();
    int wrapS = t[10], wrapT = t[11];       /* wrap[3]: {both, s, t} at 9,10,11 */
    int listSize = t[53];
    int numLevels = t[55];
    int type = (version >= PFBV_CLIPTEXTURE) ? t[56] : 0;

    if (listSize > 0) {                      /* itlist: skip */
        std::vector<int32_t> skip((size_t)listSize);
        r.i32v(skip.data(), skip.size());
    }
    if (type == 0) {                         /* TEXTYPE_TEXTURE: levels {level,obj} */
        for (int i = 0; i < numLevels; i++) { r.i32(); r.i32(); }
    } else {                                 /* cliptexture: cliptex_t + cliplevels */
        fprintf(stderr, "pfb2osg: cliptexture \"%s\" not supported\n", name.c_str());
        for (int i = 0; i < 14; i++) r.i32();
        for (int i = 0; i < numLevels; i++) { r.i32(); r.i32(); r.i32(); r.i32(); r.i32(); }
    }

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setName(name);
    osg::ref_ptr<osg::Image> img = loadPfi(name);
    if (img) texture->setImage(img);
    /* txr_table (1-based): 1 REPEAT, 2 CLAMP, 3 SELECT */
    texture->setWrap(osg::Texture::WRAP_S,
                     wrapS == 2 ? osg::Texture::CLAMP_TO_EDGE : osg::Texture::REPEAT);
    texture->setWrap(osg::Texture::WRAP_T,
                     wrapT == 2 ? osg::Texture::CLAMP_TO_EDGE : osg::Texture::REPEAT);
    texture->setFilter(osg::Texture::MIN_FILTER,
                       getenv("PFOSG_NO_MIPMAP") ? osg::Texture::LINEAR
                                                 : osg::Texture::LINEAR_MIPMAP_LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    tex.push_back(texture);
}

void Loader::readTenv()
{
    /* tenv_t: mode component color4 udata (7 words) */
    WordBuf b;
    b.read(r, 7);
    int mode = b.i();
    osg::ref_ptr<osg::TexEnv> e = new osg::TexEnv;
    /* tem_table (1-based): 1 MODULATE, 2 BLEND, 3 DECAL, 4 ALPHA */
    switch (mode) {
        case 2:  e->setMode(osg::TexEnv::BLEND);    break;
        case 3:  e->setMode(osg::TexEnv::DECAL);    break;
        default: e->setMode(osg::TexEnv::MODULATE); break;
    }
    tenv.push_back(e);
}

void Loader::readGState()
{
    int32_t bufSize = r.i32();
    WordBuf b;
    b.read(r, (size_t)bufSize);

    GState g;
    g.ss = new osg::StateSet;
    osg::StateSet* ss = g.ss.get();

    int texIndex = -1, tenvIndex = -1;
    bool texOn = false;

    size_t end = bufSize > 0 ? (size_t)bufSize - 1 : 0;   /* last word = udata */
    while (b.pos < end) {
        int s = b.i();
        switch (s) {
        case STATE_TRANSPARENCY: {
            int v = b.i();     /* tr_table (1-based); 1 and 8 are OFF variants */
            if (v != 1 && v != 8) {
                ss->setMode(GL_BLEND, osg::StateAttribute::ON);
                ss->setAttributeAndModes(
                    new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
                /* blended surfaces must not write depth: the bin re-sorts by
                 * bound-center distance every frame, and a decal (crosswalk/
                 * stop-line) sorting ahead of its base road tile would
                 * otherwise punch clear-color holes through it */
                ss->setAttributeAndModes(
                    new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));
            }
            break;
        }
        case STATE_ANTIALIAS: b.i(); break;
        case STATE_DECAL: {
            int d = b.i();
            b.i(); b.i();      /* layer, offset flag */
            /* decal_table (1-based): 1 off, 2 LAYER, 8..12 LAYER_* variants */
            if (d == 2 || d >= 8)
                ss->setAttributeAndModes(new osg::PolygonOffset(-1.0f, -1.0f));
            break;
        }
        case STATE_ALPHAFUNC: {
            int v = b.i();     /* af_table (1-based): 1 OFF, 2 NEVER..9 ALWAYS */
            if (v > 1)
                ss->setAttributeAndModes(
                    new osg::AlphaFunc((osg::AlphaFunc::ComparisonFunction)(GL_NEVER + v - 2), 0.0f));
            break;
        }
        case STATE_CULLFACE: {
            int v = b.i();     /* cf_table (1-based): 1 OFF, 2 BACK, 3 FRONT, 4 BOTH */
            if (v == 1)
                ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
            else if (v >= 2 && v <= 4) {
                static const osg::CullFace::Mode m[] = {
                    osg::CullFace::BACK, osg::CullFace::FRONT,
                    osg::CullFace::FRONT_AND_BACK };
                ss->setAttributeAndModes(new osg::CullFace(m[v - 2]));
            }
            break;
        }
        case STATE_ENTEXTURE:  texOn = onoff(b.i()); break;
        case STATE_ENLIGHTING:
            ss->setMode(GL_LIGHTING, onoff(b.i()) ? osg::StateAttribute::ON
                                                  : osg::StateAttribute::OFF);
            break;
        case STATE_ENTEXGEN:
            if (onoff(b.i()) && !warnedTexgen++)
                fprintf(stderr, "pfb2osg: texgen enabled in gstate - ignored\n");
            break;
        case STATE_ENFOG: case STATE_ENWIREFRAME: case STATE_ENCOLORTABLE:
        case STATE_ENHIGHLIGHTING: case STATE_ENLPOINTSTATE:
        case STATE_ENTEXMAT: case STATE_ENTEXLOD:
            b.i();
            break;
        case STATE_ALPHAREF: {
            float ref = b.f();
            const osg::AlphaFunc* af = dynamic_cast<const osg::AlphaFunc*>(
                ss->getAttribute(osg::StateAttribute::ALPHAFUNC));
            if (af)
                ss->setAttributeAndModes(new osg::AlphaFunc(af->getFunction(), ref));
            break;
        }
        case STATE_FRONTMTL: {
            int i = b.i();
            if (i >= 0 && i < (int)mtl.size())
                ss->setAttributeAndModes(mtl[i]);
            break;
        }
        case STATE_BACKMTL: b.i(); break;
        case STATE_TEXTURE: texIndex = b.i(); break;
        case STATE_TEXENV:  tenvIndex = b.i(); break;
        case STATE_FOG: case STATE_LIGHTMODEL: case STATE_COLORTABLE:
        case STATE_HIGHLIGHT: case STATE_LPOINTSTATE: case STATE_TEXGEN:
        case STATE_TEXLOD:
            b.i();
            break;
        case STATE_LIGHTS: {
            int n = b.i();
            for (int i = 0; i < n; i++) b.i();
            break;
        }
        case STATE_TEXMAT:
            for (int i = 0; i < 16; i++) b.f();
            break;
        default:
            fprintf(stderr, "pfb2osg: unknown gstate element %d\n", s);
            b.pos = end;   /* cursor unreliable now; bail on this gstate */
            break;
        }
    }

    if (texOn && texIndex >= 0 && texIndex < (int)tex.size()) {
        ss->setTextureAttributeAndModes(0, tex[texIndex]);
        if (tenvIndex >= 0 && tenvIndex < (int)tenv.size())
            ss->setTextureAttributeAndModes(0, tenv[tenvIndex]);
    }
    gstate.push_back(g);
}

void Loader::readGSet()
{
    /* gset_2_t: 40 words for 8 <= version < 19 */
    size_t nwords = 40;
    if (version < PFBV_GSET_DO_DP) nwords = 33;
    else if (version < PFBV_GSET_BBOX_FLUX) nwords = 39;
    WordBuf b;
    b.read(r, nwords);
    const int32_t* g = b.w.data();

    int ptype = g[0], pcount = g[1], llistIdx = g[2];   /* ptype is 1-based */
    const int32_t* vl = g + 3;    /* {bind, list, ilist} */
    const int32_t* cl = g + 6;
    const int32_t* nl = g + 9;
    const int32_t* tl = g + 12;
    bool flatShade = onoff(g[15]);      /* draw_mode[0] via oo_table */
    int gstateIdx = g[18];

    /* vertex counts per primitive */
    std::vector<int> lens;
    int nstrips = 0, vtotal = 0;
    bool strip = false;
    GLenum glmode = GL_TRIANGLES;
    int fixed = 0, flatSkip = 0;
    switch (ptype) {
        case PT_TRIS:            glmode = GL_TRIANGLES;      fixed = 3; break;
        case PT_QUADS:           glmode = GL_QUADS;          fixed = 4; break;
        case PT_POINTS:          glmode = GL_POINTS;         fixed = 1; break;
        case PT_LINES:           glmode = GL_LINES;          fixed = 2; break;
        case PT_TRISTRIPS:       glmode = GL_TRIANGLE_STRIP; strip = true; break;
        case PT_FLAT_TRISTRIPS:  glmode = GL_TRIANGLE_STRIP; strip = true; flatSkip = 2; break;
        case PT_TRIFANS:         glmode = GL_TRIANGLE_FAN;   strip = true; break;
        case PT_FLAT_TRIFANS:    glmode = GL_TRIANGLE_FAN;   strip = true; flatSkip = 2; break;
        case PT_LINESTRIPS:      glmode = GL_LINE_STRIP;     strip = true; break;
        case PT_FLAT_LINESTRIPS: glmode = GL_LINE_STRIP;     strip = true; flatSkip = 1; break;
        case PT_POLYS:           glmode = GL_POLYGON;        strip = true; break;
        default:
            fprintf(stderr, "pfb2osg: unknown prim type %d\n", ptype);
            gset.push_back(new osg::Geometry);
            return;
    }
    if (strip) {
        if (llistIdx < 0 || llistIdx >= (int)llist.size()) {
            fprintf(stderr, "pfb2osg: strip geoset without lengths list\n");
            gset.push_back(new osg::Geometry);
            return;
        }
        nstrips = pcount;
        for (int i = 0; i < nstrips; i++) {
            lens.push_back(llist[llistIdx][i]);
            vtotal += llist[llistIdx][i];
        }
    } else {
        vtotal = pcount * fixed;
    }

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setUseDisplayList(getenv("PFOSG_NO_DLIST") == nullptr);

    /* attribute fetch: flatten to per-vertex (see file header) */
    auto vertexOf = [&](const int32_t* a, int vert, int prim,
                        int vertInPrim, bool flatAttr) -> int {
        /* returns source element index for attribute spec a={bind,list,ilist},
         * or -1 for none.  vert = global vertex number.  flatAttr marks the
         * attributes affected by FLAT_* primitives (colors and normals only;
         * coords and texcoords always carry the full vertex count). */
        switch (a[0]) {
        case B_OFF:     return -1;
        case B_OVERALL: return 0;
        case B_PER_PRIM: return prim;
        case B_PER_VERTEX: {
            int idx = vert;
            if (flatSkip && flatAttr) {
                /* FLAT_*: source arrays omit the first flatSkip verts of each
                 * strip; clamp those to the strip's first stored value */
                idx = 0;
                for (int s2 = 0; s2 < prim; s2++) idx += lens[s2] - flatSkip;
                idx += vertInPrim < flatSkip ? 0 : vertInPrim - flatSkip;
            }
            if (a[2] >= 0 && a[2] < (int)ilist.size())
                return ilist[a[2]][idx];
            return idx;
        }
        }
        return -1;
    };

    osg::ref_ptr<osg::Vec3Array> vout = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> cout_;
    osg::ref_ptr<osg::Vec3Array> nout;
    osg::ref_ptr<osg::Vec2Array> tout;
    if (cl[0] != B_OFF && cl[1] >= 0) cout_ = new osg::Vec4Array;
    if (nl[0] != B_OFF && nl[1] >= 0) nout = new osg::Vec3Array;
    if (tl[0] != B_OFF && tl[1] >= 0) tout = new osg::Vec2Array;

    int vert = 0;
    int nprims = strip ? nstrips : pcount;
    for (int p = 0; p < nprims; p++) {
        int plen = strip ? lens[p] : fixed;
        for (int k = 0; k < plen; k++, vert++) {
            int vi = vertexOf(vl, vert, p, k, false);
            vout->push_back(vi >= 0 && vl[1] < (int)vlist.size()
                                ? (*vlist[vl[1]])[vi] : osg::Vec3());
            if (cout_) {
                int ci = vertexOf(cl, vert, p, k, true);
                cout_->push_back(ci >= 0 ? (*clist[cl[1]])[ci]
                                         : osg::Vec4(1, 1, 1, 1));
            }
            if (nout) {
                int ni = vertexOf(nl, vert, p, k, true);
                nout->push_back(ni >= 0 ? (*nlist[nl[1]])[ni]
                                        : osg::Vec3(0, 0, 1));
            }
            if (tout) {
                int ti = vertexOf(tl, vert, p, k, false);
                tout->push_back(ti >= 0 ? (*tlist[tl[1]])[ti] : osg::Vec2());
            }
        }
    }

    /* Surface primitives are re-emitted as independent triangles, and any
     * triangle with an edge longer than PFOSG_SUBDIV (default 25 units) is
     * recursively split.  Two reasons: Apple's GL-on-Metal drops whole
     * triangles that straddle the eye plane (w-sign-mixed "external"
     * triangles — the missing-road-triangle wedge under the vehicle), which
     * only bites for very long slivers; and GLES2 has no QUADS/POLYGON, so
     * this also keeps the draw modes web-safe.  Winding parity of strips is
     * preserved; per-vertex attributes are interpolated at split points. */
    bool surface = glmode == GL_TRIANGLES || glmode == GL_QUADS ||
                   glmode == GL_TRIANGLE_STRIP ||
                   glmode == GL_TRIANGLE_FAN || glmode == GL_POLYGON;
    static const float subdivMax = [] {
        const char* e = getenv("PFOSG_SUBDIV");
        return e ? (float)atof(e) : 25.0f;
    }();
    bool retriangulated = false;
    if (surface && subdivMax > 0.0f) {
        struct VR { osg::Vec3 p; osg::Vec4 c; osg::Vec3 n; osg::Vec2 t; };
        auto rec = [&](int i) {
            VR r;
            r.p = (*vout)[i];
            if (cout_) r.c = (*cout_)[i];
            if (nout)  r.n = (*nout)[i];
            if (tout)  r.t = (*tout)[i];
            return r;
        };
        auto mid = [&](const VR& a, const VR& b) {
            VR m;
            m.p = (a.p + b.p) * 0.5f;
            m.c = (a.c + b.c) * 0.5f;
            m.n = a.n + b.n;
            m.n.normalize();
            m.t = (a.t + b.t) * 0.5f;
            return m;
        };

        struct Item { VR a, b, c; int d; };
        std::vector<Item> work;
        int first = 0;
        for (int p = 0; p < nprims; p++) {
            int plen = strip ? lens[p] : fixed;
            if (glmode == GL_TRIANGLES) {
                for (int k = 0; k + 2 < plen; k += 3)
                    work.push_back({rec(first + k), rec(first + k + 1),
                                    rec(first + k + 2), 0});
            } else if (glmode == GL_QUADS) {
                for (int k = 0; k + 3 < plen; k += 4) {
                    work.push_back({rec(first + k), rec(first + k + 1),
                                    rec(first + k + 2), 0});
                    work.push_back({rec(first + k), rec(first + k + 2),
                                    rec(first + k + 3), 0});
                }
            } else if (glmode == GL_TRIANGLE_STRIP) {
                for (int t = 0; t + 2 < plen; t++) {
                    int a = first + t, b = first + t + 1, c = first + t + 2;
                    if (t & 1) std::swap(a, b);
                    work.push_back({rec(a), rec(b), rec(c), 0});
                }
            } else {                     /* TRIANGLE_FAN / POLYGON */
                for (int t = 0; t + 2 < plen; t++)
                    work.push_back({rec(first), rec(first + t + 1),
                                    rec(first + t + 2), 0});
            }
            first += plen;
        }

        osg::ref_ptr<osg::Vec3Array> nv = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec4Array> nc = cout_ ? new osg::Vec4Array : nullptr;
        osg::ref_ptr<osg::Vec3Array> nn = nout ? new osg::Vec3Array : nullptr;
        osg::ref_ptr<osg::Vec2Array> nt = tout ? new osg::Vec2Array : nullptr;
        auto emit = [&](const VR& r) {
            nv->push_back(r.p);
            if (nc) nc->push_back(r.c);
            if (nn) nn->push_back(r.n);
            if (nt) nt->push_back(r.t);
        };
        const float mx2 = subdivMax * subdivMax;
        while (!work.empty()) {
            Item it = work.back();
            work.pop_back();
            float e0 = (it.b.p - it.a.p).length2();
            float e1 = (it.c.p - it.b.p).length2();
            float e2 = (it.a.p - it.c.p).length2();
            float longest = std::max(e0, std::max(e1, e2));
            /* leave degenerate slivers alone: they never rasterize */
            float area2 = ((it.b.p - it.a.p) ^ (it.c.p - it.a.p)).length2();
            if (it.d >= 8 || longest <= mx2 || area2 < 1e-8f) {
                emit(it.a); emit(it.b); emit(it.c);
                continue;
            }
            if (e0 >= e1 && e0 >= e2) {
                VR m = mid(it.a, it.b);
                work.push_back({it.a, m, it.c, it.d + 1});
                work.push_back({m, it.b, it.c, it.d + 1});
            } else if (e1 >= e2) {
                VR m = mid(it.b, it.c);
                work.push_back({it.a, it.b, m, it.d + 1});
                work.push_back({it.a, m, it.c, it.d + 1});
            } else {
                VR m = mid(it.c, it.a);
                work.push_back({it.a, it.b, m, it.d + 1});
                work.push_back({m, it.b, it.c, it.d + 1});
            }
        }
        vout = nv;
        if (cout_) cout_ = nc;
        if (nout)  nout = nn;
        if (tout)  tout = nt;
        retriangulated = true;
    }

    geom->setVertexArray(vout);
    if (cout_) geom->setColorArray(cout_, osg::Array::BIND_PER_VERTEX);
    if (nout)  geom->setNormalArray(nout, osg::Array::BIND_PER_VERTEX);
    if (tout)  geom->setTexCoordArray(0, tout);

    if (retriangulated) {
        geom->addPrimitiveSet(
            new osg::DrawArrays(GL_TRIANGLES, 0, (int)vout->size()));
    } else if (strip) {
        int first = 0;
        for (int p = 0; p < nstrips; p++) {
            geom->addPrimitiveSet(new osg::DrawArrays(glmode, first, lens[p]));
            first += lens[p];
        }
    } else {
        geom->addPrimitiveSet(new osg::DrawArrays(glmode, 0, vtotal));
    }

    osg::StateSet* ss = nullptr;
    if (gstateIdx >= 0 && gstateIdx < (int)gstate.size())
        ss = gstate[gstateIdx].ss.get();
    if (ss) geom->setStateSet(ss);
    if (flatShade) {
        if (!ss) { geom->getOrCreateStateSet(); ss = geom->getStateSet(); }
        else {
            /* gstates are shared; clone-on-write for the flat-shade variant */
            osg::ref_ptr<osg::StateSet> own =
                new osg::StateSet(*ss, osg::CopyOp::SHALLOW_COPY);
            geom->setStateSet(own);
            ss = own.get();
        }
        ss->setAttributeAndModes(new osg::ShadeModel(osg::ShadeModel::FLAT));
    }

    geom->setName("gset" + std::to_string(gset.size()));
    gset.push_back(geom);
}

void Loader::readNode(int /*index*/)
{
    NodeRec rec;
    int32_t bufSize = r.i32();
    rec.buf.read(r, (size_t)bufSize);
    rec.type = rec.buf.i() & N_NOT_CUSTOM_MASK;
    nodes.push_back(std::move(rec));
    /* trailing name (outside the word buffer) */
    nodes.back().name = r.pstring();
}

void Loader::buildNodes()
{
    size_t n = nodes.size();
    osgNodes.resize(n);
    std::vector<std::vector<int>> children(n);

    for (size_t i = 0; i < n; i++) {
        NodeRec& rec = nodes[i];
        WordBuf& b = rec.buf;
        osg::ref_ptr<osg::Node> node;
        bool isGroup = false, isGeode = false;
        int count = 0;

        switch (rec.type) {
        case N_GROUP: {
            node = new osg::Group;
            isGroup = true;
            break;
        }
        case N_SCENE: {
            osg::ref_ptr<osg::Group> grp = new osg::Group;
            int gs = b.i(), gsi = b.i();
            (void)gsi;
            if (gs >= 0 && gs < (int)gstate.size())
                grp->setStateSet(gstate[gs].ss.get());
            node = grp;
            isGroup = true;
            break;
        }
        case N_SCS: case N_DCS: {
            uint32_t mtype = 0;
            if (rec.type == N_DCS) mtype = b.u();
            (void)mtype;
            osg::Matrixf m;             /* Performer row-vector convention
                                           matches OSG's row-major Matrix */
            for (int r2 = 0; r2 < 4; r2++)
                for (int c = 0; c < 4; c++)
                    m(r2, c) = b.f();
            osg::ref_ptr<osg::MatrixTransform> xf = new osg::MatrixTransform(m);
            if (rec.type == N_SCS)
                xf->setDataVariance(osg::Object::STATIC);
            node = xf;
            isGroup = true;
            break;
        }
        case N_SWITCH: {
            osg::ref_ptr<osg::Switch> sw = new osg::Switch;
            int val = b.i();
            if (version >= PFBV_SWITCH_VAL_FLUX) b.i();   /* val flux */
            sw->setUserValue("pfb_switch_val", val);      /* applied post-connect */
            node = sw;
            isGroup = true;
            break;
        }
        case N_LOD: {
            osg::ref_ptr<osg::LOD> lod = new osg::LOD;
            count = b.i();
            std::vector<float> ranges((size_t)count + 1);
            for (int k = 0; k <= count; k++) ranges[k] = b.f();
            for (int k = 0; k <= count; k++) b.f();       /* transitions */
            osg::Vec3 center(b.f(), b.f(), b.f());
            b.i(); b.i();                                  /* lodstate, index */
            lod->setCenter(center);
            lod->setCenterMode(osg::LOD::USER_DEFINED_CENTER);
            for (int k = 0; k < count; k++)
                lod->setRange(k, ranges[k], ranges[k + 1]);
            node = lod;
            isGroup = true;
            break;
        }
        case N_SEQUENCE: {
            osg::ref_ptr<osg::Sequence> seq = new osg::Sequence;
            count = b.i();
            std::vector<float> times((size_t)count);
            for (int k = 0; k < count; k++) times[k] = b.f();
            int ival = b.i(), beg = b.i(), end2 = b.i();
            float speed = b.f();
            int nreps = b.i();
            int smode = b.i();
            /* sqi (1-based): 1 CYCLE, 2 SWING; sqm: 1 STOP, 2 START, ... */
            seq->setInterval(ival == 2 ? osg::Sequence::SWING
                                       : osg::Sequence::LOOP, beg, end2);
            seq->setDuration(speed, nreps);
            for (int k = 0; k < count; k++) seq->setTime(k, times[k]);
            seq->setMode(smode == 2 ? osg::Sequence::START : osg::Sequence::STOP);
            node = seq;
            isGroup = true;
            break;
        }
        case N_LAYER: {
            /* pfLayer: child 0 = base geometry, children 1.. = coplanar decal
             * layers.  Approximate the decal with a polygon offset applied to
             * the layer children (post-connect). */
            b.i(); b.i(); b.i();          /* layer mode triple */
            osg::ref_ptr<osg::Group> grp = new osg::Group;
            grp->setUserValue("pfb_layer", 1);
            node = grp;
            isGroup = true;
            break;
        }
        case N_GEODE: {
            node = new osg::Geode;
            count = b.i();
            isGeode = true;
            break;
        }
        case N_BILLBOARD: {
            osg::ref_ptr<osg::Billboard> bb = new osg::Billboard;
            count = b.i();                    /* one position per gset */
            std::vector<osg::Vec3> pos((size_t)count);
            for (int k = 0; k < count; k++)
                pos[k].set(b.f(), b.f(), b.f());
            int rot = b.i();                  /* bbr (1-based): 1 axial, 2 eye, 3 world */
            osg::Vec3 axis(b.f(), b.f(), b.f());
            if (rot == 1) {
                bb->setMode(osg::Billboard::AXIAL_ROT);
                bb->setAxis(axis);
                /* Performer billboards face +Y before rotation */
                bb->setNormal(osg::Vec3(0.0f, -1.0f, 0.0f));
            } else {
                bb->setMode(rot == 2 ? osg::Billboard::POINT_ROT_EYE
                                     : osg::Billboard::POINT_ROT_WORLD);
            }
            for (int k = 0; k < count; k++) {
                int gs = b.i();
                if (gs >= 0 && gs < (int)gset.size())
                    bb->addDrawable(gset[gs].get(), pos[k]);
            }
            count = 0;                        /* gsets consumed here */
            node = bb;
            break;
        }
        default:
            fprintf(stderr, "pfb2osg: node type %d not supported, using Group "
                            "(children preserved)\n", rec.type);
            node = new osg::Group;
            isGroup = true;
            break;
        }

        if (isGroup) {
            int cc = b.i();
            children[i].resize((size_t)cc);
            for (int k = 0; k < cc; k++) children[i][k] = b.i();
        } else if (isGeode) {
            osg::Geode* gd = node->asGeode();
            for (int k = 0; k < count; k++) {
                int gs = b.i();
                if (gs >= 0 && gs < (int)gset.size())
                    gd->addDrawable(gset[gs].get());
            }
        }

        /* common tail: trav masks, ufuncs, udata, bsphere - all ignorable */
        b.u(); b.u(); b.u(); b.u();
        if (version >= PFBV_UFUNC && b.i() == 1)
            for (int k = 0; k < 12; k++) b.i();
        b.i();                                            /* udata */
        /* bound_table (1-based): 1 = STATIC (sphere follows), 2 = DYNAMIC */
        if (version >= PFBV_NODE_BSPHERE && b.i() == 1)
            { b.f(); b.f(); b.f(); b.f(); }

        if (!rec.name.empty()) node->setName(rec.name);
        osgNodes[i] = node;
    }

    /* connect children */
    for (size_t i = 0; i < n; i++) {
        if (children[i].empty()) continue;
        osg::Group* grp = osgNodes[i]->asGroup();
        for (int c : children[i])
            if (c >= 0 && c < (int)n && osgNodes[c].valid())
                grp->addChild(osgNodes[c].get());
    }

    /* decal layers: push layer children (1..) slightly toward the eye */
    for (size_t i = 0; i < n; i++) {
        int isLayer = 0;
        if (!osgNodes[i] || !osgNodes[i]->getUserValue("pfb_layer", isLayer) || !isLayer)
            continue;
        osg::Group* grp = osgNodes[i]->asGroup();
        for (unsigned c = 1; c < grp->getNumChildren(); c++) {
            osg::StateSet* ss = grp->getChild(c)->getOrCreateStateSet();
            ss->setAttributeAndModes(
                new osg::PolygonOffset(-1.0f * (float)c, -1.0f * (float)c));
            ss->setRenderBinDetails(1 + (int)c, "RenderBin");
        }
    }

    /* apply deferred switch values now that children exist */
    for (size_t i = 0; i < n; i++) {
        osg::Switch* sw = dynamic_cast<osg::Switch*>(osgNodes[i].get());
        if (!sw) continue;
        int val = 0;
        sw->getUserValue("pfb_switch_val", val);
        if (val == PFB_SWITCH_ON)       sw->setAllChildrenOn();
        else if (val == PFB_SWITCH_OFF) sw->setAllChildrenOff();
        else                            sw->setSingleChildOn((unsigned)val);
    }

    if (!osgNodes.empty()) root = osgNodes[0];
}

bool Loader::load(const std::string& path)
{
    size_t slash = path.find_last_of("/\\");
    dir = (slash == std::string::npos) ? "." : path.substr(0, slash);

    r.fp = fopen(path.c_str(), "rb");
    if (!r.fp) {
        fprintf(stderr, "pfb2osg: cannot open \"%s\"\n", path.c_str());
        return false;
    }
    uint32_t magic = 0;
    r.readRaw(&magic, 4);
    if (magic == PFB_MAGIC) r.swap = false;
    else if (magic == PFB_MAGIC_LE) r.swap = true;
    else {
        fprintf(stderr, "pfb2osg: \"%s\" is not a .pfb file (magic %08x)\n",
                path.c_str(), magic);
        return false;
    }
    version = r.i32();
    r.i32();                      /* header word 2 (unused by the loader) */
    int32_t listOffset = r.i32();
    if (version > 18)
        fprintf(stderr, "pfb2osg: pfb version %d is newer than the tested "
                        "subset (18); proceeding anyway\n", version);
    fseek(r.fp, listOffset, SEEK_SET);

    while (!r.error) {
        int32_t hdr[3];
        if (fread(hdr, 4, 3, r.fp) != 3) break;
        if (r.swap)
            for (int i = 0; i < 3; i++) hdr[i] = (int32_t)__builtin_bswap32((uint32_t)hdr[i]);
        int type = hdr[0], cnt = hdr[1], bytes = hdr[2];
        if (type < 0 || type >= L_COUNT) {
            fprintf(stderr, "pfb2osg: unknown list type %d, skipping\n", type);
            fseek(r.fp, bytes, SEEK_CUR);
            continue;
        }
        switch (type) {
        case L_LLIST: for (int i = 0; i < cnt; i++) readLlist(); break;
        case L_VLIST:
            for (int i = 0; i < cnt; i++)
                readSlist(vlist, [](Reader& rr) {
                    float x = rr.f32(), y = rr.f32(), z = rr.f32();
                    return osg::Vec3(x, y, z); });
            break;
        case L_CLIST:
            for (int i = 0; i < cnt; i++)
                readSlist(clist, [](Reader& rr) {
                    float x = rr.f32(), y = rr.f32(), z = rr.f32(), w = rr.f32();
                    return osg::Vec4(x, y, z, w); });
            break;
        case L_NLIST:
            for (int i = 0; i < cnt; i++)
                readSlist(nlist, [](Reader& rr) {
                    float x = rr.f32(), y = rr.f32(), z = rr.f32();
                    return osg::Vec3(x, y, z); });
            break;
        case L_TLIST:
            for (int i = 0; i < cnt; i++)
                readSlist(tlist, [](Reader& rr) {
                    float x = rr.f32(), y = rr.f32();
                    return osg::Vec2(x, y); });
            break;
        case L_ILIST:  for (int i = 0; i < cnt; i++) readIlist();  break;
        case L_MTL:    for (int i = 0; i < cnt; i++) readMtl();    break;
        case L_TEX:    for (int i = 0; i < cnt; i++) readTex();    break;
        case L_TENV:   for (int i = 0; i < cnt; i++) readTenv();   break;
        case L_GSTATE: for (int i = 0; i < cnt; i++) readGState(); break;
        case L_GSET:   for (int i = 0; i < cnt; i++) readGSet();   break;
        case L_NODE:
            for (int i = 0; i < cnt; i++) readNode(i);
            break;
        default:
            fprintf(stderr, "pfb2osg: list L_%d (%d records) not needed for "
                            "rendering - skipped\n", type, cnt);
            fseek(r.fp, bytes, SEEK_CUR);
            break;
        }
        if (type == L_NODE) break;    /* L_NODE is the final list at v18 */
    }

    if (r.error) {
        fprintf(stderr, "pfb2osg: read error in \"%s\"\n", path.c_str());
        return false;
    }
    buildNodes();
    return root.valid();
}

}   /* namespace pfb2osg */

osg::ref_ptr<osg::Node> pfb2osgLoadFile(const std::string& path)
{
    pfb2osg::Loader loader;
    if (!loader.load(path)) return nullptr;
    return loader.root;
}
