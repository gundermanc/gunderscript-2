// Gunderscript-2 Common Resources API
// (C) 2016 Christian Gunderman

// Private module implementation details via PIMPL pattern.
#include "common_resourcesimpl.h"

namespace gunderscript {

// Public implementation constructor.
CommonResources::CommonResources() : pimpl_(new CommonResourcesImpl()) {

}

#ifdef NJ_VERBOSE
bool CommonResources::verbose_asm() {
    return this->pimpl().verbose_asm();
}

void CommonResources::set_verbose_asm(bool verbose_asm) {
    this->pimpl().set_verbose_asm(verbose_asm);
}
#endif // NJ_VERBOSE

} // namespace gunderscript
