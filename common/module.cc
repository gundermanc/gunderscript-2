// Gunderscript-2 Module API
// (C) 2016 Christian Gunderman

#include <unordered_map>
#include <vector>

#include "nanojit.h"

#include "symbol.h"

// Private module implementation details via PIMPL pattern.
#include "moduleimpl.h"

using namespace nanojit;

namespace gunderscript {

// Module Name getter implementation.
const std::string& Module::module_name() const {
    return this->pimpl_->module_name();
}

// Public destructor.
Module::~Module() {
    delete this->pimpl_;
}

} // namespace gunderscript
