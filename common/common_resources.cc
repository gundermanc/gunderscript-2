// Gunderscript-2 Common Resources API
// (C) 2016 Christian Gunderman

// Private module implementation details via PIMPL pattern.
#include "common_resourcesimpl.h"

using namespace nanojit;

namespace gunderscript {

// Public implementation constructor.
CommonResources::CommonResources() : pimpl_(new CommonResourcesImpl()) {
}

// Public implementation destructor.
CommonResources::~CommonResources() {
    delete this->pimpl_;
}

} // namespace gunderscript
