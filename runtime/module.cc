// Gunderscript-2 Module API
// (C) 2016 Christian Gunderman

#include "gunderscript/module.h"

namespace gunderscript {

// Module Private Implementation.
class ModuleImpl {

};

// Public constructor.
Module::Module() : pimpl_(new ModuleImpl()) {
}

// Public destructor.
Module::~Module() {
    delete this->pimpl_;
}

} // namespace gunderscript
