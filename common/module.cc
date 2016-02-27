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

// Tells whether or not a module has been compiled into this object.
bool Module::compiled() {
    return pimpl_->compiled();
}

// Tells whether or not a moduel has been assembled into native code.
bool Module::assembled() {
    return pimpl_->assembled();
}

// Module Name getter implementation.
const std::string& Module::module_name() const {
    return this->pimpl_->module_name();
}

// Creates a new instance of Module Private implementation.
ModuleImpl::ModuleImpl()
    : compiled_(false),
    assembled_(false),
    func_table_(),
    module_name_(""),
    symbols_vector_(new std::vector<ModuleImplSymbol>(), std::default_delete<std::vector<ModuleImplSymbol>>()) {
}

} // namespace gunderscript
