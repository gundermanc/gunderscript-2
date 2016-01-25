// Gunderscript-2 Private Module Implementation
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_MODULEIMPL__H__
#define GUNDERSCRIPT_MODULEIMPL__H__

#include <vector>

#include "nanojit.h"

#include "gunderscript/module.h"

#include "symbol.h"

using namespace nanojit;

namespace gunderscript {

// Exported Module Symbol.
class ModuleImplSymbol {
public:
    ModuleImplSymbol(const std::string& symbol_name, const Symbol& symbol, const Fragment& fragment)
        : symbol_name_(symbol_name), symbol_(symbol), fragment_(fragment) { }
private:
    const std::string symbol_name_;
    const Symbol symbol_;
    Fragment fragment_;
};

// Module Private Implementation.
class ModuleImpl {
public:
    ModuleImpl(
        const std::string& module_name,
        std::vector<ModuleImplSymbol>& symbols);

    const std::string& module_name() const { return module_name_; }
private:
    const std::string module_name_;
    std::vector<ModuleImplSymbol> symbols_vector_;
};

// Constructs a module object. Use this instead of explictly calling constructor.
Module ModuleFactory(const std::string& module_name, std::vector<ModuleImplSymbol>& symbols);

} // namespace gunderscript

#endif // GUNDERSCRIPT_MODULEIMPL__H__