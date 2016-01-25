// Gunderscript-2 Private Module Implementation
// (C) 2016 Christian Gunderman

#include "moduleimpl.h"

namespace gunderscript {

// Creates a new instance of Module. Use this instead of constructor.
Module ModuleFactory(
    const std::string& module_name,
    std::vector<ModuleImplSymbol>& symbols) {
    ModuleImpl* pimpl = new ModuleImpl(module_name, symbols);

    return Module(pimpl);
}

// Creates a new instance of ModuleImplementation. Do not use.
// Use ModuleFactory.
ModuleImpl::ModuleImpl(const std::string& module_name, std::vector<ModuleImplSymbol>& symbols) 
    : module_name_(module_name) {
    // Put all symbols into the symbols vector.
    for (size_t i = 0; i < symbols.size(); i++) {
        this->symbols_vector_.push_back(symbols.at(i));
    }
}

} // namespace gunderscript
