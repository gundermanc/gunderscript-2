// Gunderscript-2 Private Common Resources Implementation
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_COMMON_RESOURCESIMPL__H__
#define GUNDERSCRIPT_COMMON_RESOURCESIMPL__H__

#include "nanojit.h"

#include "gunderscript/common_resources.h"

using namespace nanojit;

namespace gunderscript {

class CommonResourcesImpl {
public:
    CommonResourcesImpl() { }

    Allocator& alloc() { return alloc_; }
    Config& config() { return config_; }

#ifdef NJ_VERBOSE
    bool verbose_asm() { return verbose_asm_; }
    void set_verbose_asm(bool verbose_asm) { this->verbose_asm_ = verbose_asm; }
#endif // NJ_VERBOSE

private:
    Allocator alloc_;
    Config config_;

#ifdef NJ_VERBOSE
    bool verbose_asm_ = false;
#endif // NJ_VERBOSE

};

} // namespace gunderscript

#endif // GUNDERSCRIPT_COMMON_RESOURCESIMPL__H__