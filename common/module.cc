// Gunderscript-2 Module API
// (C) 2016 Christian Gunderman

#include <unordered_map>
#include <vector>

#include "nanojit.h"

// Private module implementation details via PIMPL pattern.
#include "moduleimpl.h"

using namespace nanojit;

namespace gunderscript {

// Public constructor.
Module::Module() : pimpl_(new ModuleImpl()) {
}

// Public destructor.
Module::~Module() {
    delete this->pimpl_;
}

// Tells whether or not a module has been compiled into this object.
bool Module::compiled() {
    return pimpl_->compiled();
}

// Module Name getter implementation.
const std::string& Module::module_name() const {
    return this->pimpl_->module_name();
}

// Creates a new instance of Module Private implementation.
ModuleImpl::ModuleImpl()
    : compiled_(false),
    module_name_(""), 
    symbols_vector_(new std::vector<ModuleImplSymbol>()) {
}

// Destroys Module Private Implementation details resources.
ModuleImpl::~ModuleImpl() {
    delete this->symbols_vector_;
}

} // namespace gunderscript
