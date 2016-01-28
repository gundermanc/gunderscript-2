// Gunderscript-2 Private Module Implementation
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_MODULEIMPL__H__
#define GUNDERSCRIPT_MODULEIMPL__H__

#include <memory>
#include <vector>

#include "nanojit.h"

#include "gunderscript/module.h"
#include "gunderscript/symbol.h"

using namespace nanojit;

namespace gunderscript {

// Exported Module Symbol.
// Contains the definitions for the symbol, its mangled name,
// and the NanoJIT code fragment implementing it.
class ModuleImplSymbol {
public:
    ModuleImplSymbol(const std::string& symbol_name, const Symbol& symbol, Fragment* fragment)
        : symbol_name_(symbol_name), symbol_(symbol), fragment_(std::shared_ptr<Fragment>(fragment)) { }

    const std::string& symbol_name() const { return symbol_name_; }
    const Symbol& symbol() const { return symbol_; }
    Fragment* fragment() { return fragment_.get(); }

private:
    const std::string symbol_name_;
    const Symbol& symbol_;
    std::shared_ptr<Fragment> fragment_;
};

// Module Private Implementation.
class ModuleImpl {
public:
    ModuleImpl();
    ~ModuleImpl();

    bool compiled() { return compiled_; }
    const std::string& module_name() const { return module_name_; }
    void set_module_name(const std::string& module_name) { module_name_ = module_name; }
    std::vector<ModuleImplSymbol>& symbols_vector() const { return *symbols_vector_; }

private:
    bool compiled_;
    std::string module_name_;
    std::vector<ModuleImplSymbol>* symbols_vector_;
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_MODULEIMPL__H__