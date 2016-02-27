// Gunderscript-2 Private Module Implementation
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_MODULEIMPL__H__
#define GUNDERSCRIPT_MODULEIMPL__H__

#include <memory>
#include <string>
#include <vector>

#include "nanojit.h"

#include "gunderscript/module.h"
#include "gunderscript/symbol.h"

using namespace nanojit;

namespace gunderscript {

typedef void(*ModuleFunc)();

// Exported Module Symbol.
// Contains the definitions for the symbol, its mangled name,
// and the NanoJIT code fragment implementing it.
class ModuleImplSymbol {
public:
    ModuleImplSymbol(const std::string& symbol_name, const Symbol* symbol, Fragment* fragment)
        : symbol_name_(symbol_name), symbol_(symbol), fragment_(fragment) { }

    const std::string& symbol_name() const { return symbol_name_; }
    const Symbol* symbol() const { return symbol_.get(); }
    Fragment* fragment() { return fragment_.get(); }

private:
    const std::string symbol_name_;
    std::unique_ptr<const Symbol> symbol_;
    std::unique_ptr<Fragment> fragment_;
};

// Module Private Implementation.
class ModuleImpl {
public:
    ModuleImpl();

    bool compiled() { return compiled_; }
    void set_compiled(bool compiled) { compiled_ = compiled; }
    bool assembled() { return assembled_; }
    void set_assembled(bool assembled) { assembled_ = assembled; }
    const std::string& module_name() const { return module_name_; }
    void set_module_name(const std::string& module_name) { module_name_ = module_name; }
    std::vector<ModuleImplSymbol>& symbols_vector() const { return *symbols_vector_.get(); }
    ModuleFunc* func_table() { return func_table_.get(); }
    void set_func_table(ModuleFunc* func_table) { func_table_ = std::unique_ptr<ModuleFunc>(func_table); }

private:
    bool compiled_;
    bool assembled_;
    std::string module_name_;
    std::unique_ptr<std::vector<ModuleImplSymbol>> symbols_vector_;
    std::unique_ptr<ModuleFunc> func_table_;
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_MODULEIMPL__H__