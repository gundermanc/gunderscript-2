// Gunderscript-2 Common Resources API
// (C) 2016 Christian Gunderman

// Private module implementation details via PIMPL pattern.
#include "common_resourcesimpl.h"

namespace gunderscript {

// Public implementation constructor.
CommonResources::CommonResources() : pimpl_(new CommonResourcesImpl()) {

}

} // namespace gunderscript
