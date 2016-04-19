// Gunderscript-2 Garbage Collector Interface
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_GARBAGE_COLLECTOR__H__
#define GUNDERSCRIPT_GARBAGE_COLLECTOR__H__

// Include BohemGC in its own namespace.
#define GC_NAMESPACE
#include "gc_cpp.h"
#include "gc.h"

#include <nanojit.h>

using namespace nanojit;

namespace gunderscript {
namespace runtime {

// Must be called before any GarbageCollectible objects are created
// on all platforms. Uses platform specific features. Although GC
// may appear to work fine without this line on some platforms,
// it will fail on others. ALWAYS USE THIS LINE.
#define INIT_GARBAGE_COLLECTOR()  GC_INIT()

class GarbageCollectibleBase : public boehmgc::gc_cleanup {
public:
    GarbageCollectibleBase() : gc_cleanup() { }
    virtual ~GarbageCollectibleBase();
};

template <size_t BufSize>
class GarbageCollectibleBuffer : public GarbageCollectibleBase {
public:
    GarbageCollectibleBuffer() : GarbageCollectibleBase() { }
    ~GarbageCollectibleBuffer() { }

    char* buffer() { return buffer_; }

private:
    char buffer_[BufSize];
};

void* GarbageCollectorAllocBuffer(size_t buf_size);

// Garbage collection alloc NanoJIT mapping.
// TODO: correct calling convention?
const CallInfo CI_GC_ALLOC = {
    (uintptr_t)GarbageCollectorAllocBuffer,
    CallInfo::typeSig1(ARGTYPE_P, ARGTYPE_I),
    ABI_CDECL, 0, ACCSET_STORE_ANY verbose_only(, "GC_Alloc") };

} // namespace runtime
} // namespace gunderscript

#endif // GUNDERSCRIPT_GARBAGE_COLLECTOR__H__