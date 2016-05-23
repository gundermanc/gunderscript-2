// Gunderscript-2 Garbage Collector Interface
// (C) 2016 Christian Gunderman

#include <cstring>

#include "garbage_collector.h"

namespace gunderscript {
namespace runtime {

void* GarbageCollectorAllocBuffer(size_t buf_size) {
    void* buf = GC_malloc(buf_size);
    
    // Since Gunderscript is a high level language, we initialize all memory
    // to NULL before returning.
    memset(buf, 0, buf_size);

    return buf;
}

} // namespace runtime
} // namespace gunderscript