/* ============================================================================
 *  pfCore.cpp - pragmatic implementation of the libpr object-system core:
 *  pfType, pfMemory, pfObject, and the pfNotify family.
 *
 *  This is NOT a reconstruction.  The originals (pfMemory.C, pfObject.C,
 *  pfNotify.c) implement shared-memory arenas, multiprocess ref counting and
 *  the type-bit registry; this port is single-process (PFMP_APPCULLDRAW), so
 *  arenas collapse to the C heap and ref counting is plain integers.  The
 *  public API contracts are honored:
 *    - pfMalloc'd blocks carry a pfMemory header directly before the data
 *      pointer, so pfGetSize/pfFree/pfMemory::getMemory work as documented.
 *    - objects created with `new` are their own header; pfRef/pfUnref/
 *      pfDelete operate on the object pointer itself.
 *    - pfObject::parentCopy returns 2 when the source type matches and the
 *      caller should copy its own fields (semantics inferred from the
 *      decompiled call sites in the reconstructed slices).
 * ==========================================================================*/

/* Deliberate preservation shim: lets this TU define/initialize private
 * statics (classType) and touch pfType's private registry fields. */
#define protected public
#define private public

#include <Performer/pr.h>
#include <Performer/pr/pfType.h>
#include <Performer/pr/pfMemory.h>
#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfLinMath.h>   /* pfMatStack::classType lives here */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>

/* ---- pfNotify ------------------------------------------------------------ */

int _pf_readonly_notifyLevel = PFNFY_NOTICE;

static pfNotifyFuncType pf_notifyHandler = pfDefaultNotifyHandler;

void pfNotifyLevel(int _severity)
{
    if (_severity >= PFNFY_ALWAYS && _severity <= PFNFY_LEVEL_MAX)
        _pf_readonly_notifyLevel = _severity;
}

int pfGetNotifyLevel(void) { return _pf_readonly_notifyLevel; }

void pfNotifyHandler(pfNotifyFuncType _handler)
{
    pf_notifyHandler = _handler ? _handler : pfDefaultNotifyHandler;
}

pfNotifyFuncType pfGetNotifyHandler(void) { return pf_notifyHandler; }

void pfDefaultNotifyHandler(pfNotifyData* notice)
{
    static const char* sev[] = { "ALWAYS", "FATAL", "WARN", "NOTICE",
                                 "INFO", "DEBUG", "FP DEBUG", "INT DEBUG" };
    int s = notice->severity;
    fprintf(stderr, "PF %s: %s\n",
            (s >= 0 && s <= PFNFY_LEVEL_MAX) ? sev[s] : "?", notice->emsg);
    if (s == PFNFY_FATAL)
        exit(1);
}

void pfNotifyLock(void)   {}   /* single process: no lock needed */
void pfNotifyUnlock(void) {}

void pfNotify(int _severity, int _error, char* _format, ...)
{
    if (_severity > _pf_readonly_notifyLevel)
        return;
    char msg[2048];
    va_list ap;
    va_start(ap, _format);
    vsnprintf(msg, sizeof msg, _format, ap);
    va_end(ap);
    pfNotifyData notice;
    notice.severity = _severity;
    notice.pferrno  = _error;
    notice.emsg     = msg;
    pf_notifyHandler(&notice);
}

/* ---- pfType ---------------------------------------------------------------
 * The original keeps a bit-registry for O(1) isDerivedFrom on the first 32
 * builtin types; with typeBit == 0 the header inline falls back to
 * pr_isAncestor(), so a plain parent-chain walk is sufficient here.        */

pfType* pfType::firstMemoryType = NULL;
pfType* pfType::lastMemoryType  = NULL;

void* pfType::operator new(size_t s) { return ::calloc(1, s); }

pfType::pfType(pfType* _parent, char* _name)
{
    magic        = 0x70665479;              /* 'pfTy' */
    typeBit      = 0;                       /* force pr_isAncestor path */
    ancestorMask = 0;
    numAncestors = 0;
    ancestorList = NULL;
    parent       = _parent;
    name         = _name ? ::strdup(_name) : NULL;
    if (!firstMemoryType)
        firstMemoryType = this;
    lastMemoryType = this;
}

pfType::~pfType()
{
    ::free(name);
}

int pfType::pr_isAncestor(pfType* ancestor)
{
    for (pfType* t = this; t != NULL; t = t->parent)
        if (t == ancestor)
            return 1;
    return 0;
}

void pfType::setMaxTypes(int) {}

/* ---- pfMemory -------------------------------------------------------------
 * Raw allocations (pfMalloc/pfCalloc/pfStrdup) get a pfMemory header placed
 * immediately before the returned data pointer.                            */

pfType* pfMemory::classType = NULL;

void pfMemory::init()
{
    if (!classType)
        classType = new pfType(NULL, (char*)"pfMemory");
}

pfMemory::pfMemory()
{
    init();
    type       = classType;
    arenaIndex = 0;
    refCount   = 0;
    size       = 0;
}

pfMemory::~pfMemory() {}

void* pfMemory::operator new(size_t s)               { return ::calloc(1, s); }
void* pfMemory::operator new(size_t s, size_t nbytes)
                        { return ::calloc(1, nbytes > s ? nbytes : s); }
void* pfMemory::operator new(size_t s, size_t nbytes, void* /*arena*/)
                        { return ::calloc(1, nbytes > s ? nbytes : s); }
void  pfMemory::operator delete(void* p)             { ::free(p); }

int pfMemory::getArenaBytesUsed() { return 0; }

/* virtuals */
int pfMemory::compare(const pfMemory* _mem) const
{
    if (this == _mem) return 0;
    return (this < _mem) ? -1 : 1;
}

int pfMemory::copy(const pfMemory*) { return 0; }

int pfMemory::print(uint, uint, char* _prefix, FILE* _file)
{
    if (!_file) _file = stderr;
    fprintf(_file, "%s%s 0x%p\n", _prefix ? _prefix : "",
            getTypeName(), (const void*)this);
    return 1;
}

int  pfMemory::ref()         { return ++refCount; }
int  pfMemory::unref()       { return --refCount; }
int  pfMemory::unrefGetRef() { return --refCount; }
int  pfMemory::getRef()      { return refCount; }

int pfMemory::checkDelete()
{
    if (refCount <= 0) {
        destroy();
        delete this;
        return 1;
    }
    return 0;
}

int pfMemory::unrefDelete()
{
    --refCount;
    return checkDelete();
}

void pfMemory::destroy() {}

void* pfMemory::getArena() const { return NULL; }

/* raw-block allocation: header + data */
static pfMemory* pf_newHeader(size_t nbytes)
{
    void* raw = ::malloc(sizeof(pfMemory) + nbytes);
    pfMemory* hdr = ::new (raw) pfMemory();   /* global placement new */
    hdr->size = nbytes;
    return hdr;
}

void* pfMemory::malloc(size_t nbytes, void* /*arena*/)
{
    return pf_newHeader(nbytes)->getData();
}
void* pfMemory::malloc(size_t nbytes) { return malloc(nbytes, NULL); }

void* pfMemory::calloc(size_t numelem, size_t elsize, void* arena)
{
    void* data = malloc(numelem * elsize, arena);
    memset(data, 0, numelem * elsize);
    return data;
}
void* pfMemory::calloc(size_t numelem, size_t elsize)
                        { return calloc(numelem, elsize, NULL); }

char* pfMemory::strdup(const char* str, void* arena)
{
    if (!str) return NULL;
    size_t n = strlen(str) + 1;
    char* dst = (char*)malloc(n, arena);
    memcpy(dst, str, n);
    return dst;
}
char* pfMemory::strdup(const char* str) { return strdup(str, NULL); }

pfMemory* pfMemory::realloc(size_t nbytes)
{
    pfMemory* hdr = (pfMemory*)::realloc(this, sizeof(pfMemory) + nbytes);
    hdr->size = nbytes;
    return hdr;
}

void* pfMemory::realloc(void* data, size_t nbytes)
{
    if (!data)
        return malloc(nbytes, NULL);
    return getMemory(data)->realloc(nbytes)->getData();
}

pfMemory* pfMemory::getMemory(const void* data)
{
    return data ? (pfMemory*)((char*)data - sizeof(pfMemory)) : NULL;
}

size_t pfMemory::getSize(void* data)  { return data ? getMemory(data)->getSize() : 0; }
void*  pfMemory::getArena(void*)      { return NULL; }
void*  pfMemory::getData(const void* data) { return (void*)data; }

void pfMemory::free(void* data)
{
    if (!data) return;
    ::free(getMemory(data));
}

const char* pfMemory::getTypeName(const void* data)
                        { return getMemory(data)->getTypeName(); }
pfType* pfMemory::getType(const void* data)
                        { return getMemory(data)->getType(); }
int pfMemory::isOfType(const void* data, pfType* t)
                        { return getMemory(data)->isOfType(t); }
int pfMemory::isExactType(const void* data, pfType* t)
                        { return getMemory(data)->isExactType(t); }
int pfMemory::isFluxed(const void* data)
                        { return getMemory(data)->isFluxed(); }

/* the void* forms of the object operations take the object pointer itself
 * (this is how the C API pfRef/pfDelete/... are generated) */
int pfMemory::ref(void* _mem)           { return ((pfMemory*)_mem)->ref(); }
int pfMemory::unref(void* _mem)         { return ((pfMemory*)_mem)->unref(); }
int pfMemory::getRef(const void* _mem)  { return ((pfMemory*)_mem)->getRef(); }
int pfMemory::checkDelete(void* _mem)   { return ((pfMemory*)_mem)->checkDelete(); }
int pfMemory::unrefGetRef(void* _mem)   { return ((pfMemory*)_mem)->unrefGetRef(); }
int pfMemory::unrefDelete(void* _mem)   { return ((pfMemory*)_mem)->unrefDelete(); }
int pfMemory::compare(const void* _mem1, const void* _mem2)
                        { return ((const pfMemory*)_mem1)->compare((const pfMemory*)_mem2); }
int pfMemory::copy(void* dst, const void* src)
                        { return ((pfMemory*)dst)->copy((const pfMemory*)src); }
int pfMemory::print(const void* _mem, uint _travMode, uint _verbose, FILE* file)
                        { return ((pfMemory*)_mem)->print(_travMode, _verbose, (char*)"", file); }

void pfMemory::pr_initHeader(int _size, void* /*_arena*/)
{
    size = (size_t)_size;
}

/* ---- pfObject -------------------------------------------------------------
 * userData: original keeps a pfList of slots; pfList is not implemented yet,
 * so only slot 0 is stored (in the userDataList pointer itself).  XXX widen
 * to real slots when pfList.C lands.                                        */

pfType* pfObject::classType = NULL;

void pfObject::init()
{
    if (!classType) {
        pfMemory::init();
        classType = new pfType(pfMemory::getClassType(), (char*)"pfObject");
    }
}

pfObject::pfObject()
{
    init();
    setType(classType);
    userDataList = NULL;
}

pfObject::~pfObject() {}

void* pfObject::operator new(size_t s)          { return pfMemory::operator new(s); }
void* pfObject::operator new(size_t s, void* a) { return pfMemory::operator new(s, s, a); }

int pfObject::compare(const pfMemory* _obj) const { return parentCompare(_obj); }
int pfObject::copy(const pfMemory* _src)          { return parentCopy(_src, 0) == 2 ? 1 : 0; }

int pfObject::print(uint, uint, char* prefix, FILE* file)
{
    if (!file) file = stderr;
    fprintf(file, "%s%s 0x%p\n", prefix ? prefix : "",
            getTypeName(), (const void*)this);
    return 1;
}

void pfObject::destroy() {}

void pfObject::setUserData(int _slot, void* data)
{
    if (_slot != 0) {
        pfNotify(PFNFY_WARN, PFNFY_USAGE,
                 (char*)"pfObject::setUserData - only slot 0 is implemented");
        return;
    }
    userDataList = (pfList*)data;
}

void* pfObject::getUserData(int _slot) const
{
    return _slot == 0 ? (void*)userDataList : NULL;
}

int pfObject::getNumUserData() const { return userDataList ? 1 : 0; }

int pfObject::parentCompare(const pfMemory* mem) const
{
    if (!mem) return 1;
    if (getType() != mem->getType())
        return (getType() < mem->getType()) ? -1 : 1;
    return 0;
}

/* 2 = "types match, derived class copies its own fields now"
 * (inferred from the decompiled call sites in the reconstructed slices) */
int pfObject::parentCopy(const pfMemory* src, int /*doCompare*/)
{
    if (!src) return 0;
    if (getType() && src->getType() == getType())
        return 2;
    return 0;
}

/* ---- static classType definitions owned by not-yet-reconstructed TUs ---- */

pfType* pfMatStack::classType = NULL;   /* original home: pfLinMath.C */
