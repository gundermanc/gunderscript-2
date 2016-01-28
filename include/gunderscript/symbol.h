// Gunderscript-2 Symbol Table Symbol
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_SYMBOL__H__
#define GUNDERSCRIPT_SYMBOL__H__

#include <string>
#include <vector>

#include "lexer_resources.h"
#include "type.h"

namespace gunderscript {

// Indicates which property function a statement or expression exists in.
// NONE indicates that we're being called by a function.
// GET is PROPERTY->child(0)
// SET is PROPERTY->child(1)
enum class PropertyFunction {
    NONE,
    GET,
    SET
};

enum class SymbolType {
    FUNCTION,
    PROPERTY,
    VARIABLE,
    TYPE
};

class Symbol {
public:
    Symbol(
        SymbolType symbol_type,
        LexerSymbol access_modifier,
        bool native,
        const Type& type,
        const std::string& spec_name,
        const std::string& name) 
        : symbol_type_(symbol_type),
        access_modifier_(access_modifier),
        native_(native),
        type_(type),
        spec_name_(spec_name),
        name_(name) { }
    Symbol(const Symbol* symbol) :
        symbol_type_(symbol->symbol_type()),
        access_modifier_(symbol->access_modifier_),
        native_(symbol->native_),
        type_(symbol->type_),
        spec_name_(symbol->spec_name_),
        name_(symbol->name_) { }

    SymbolType symbol_type() const { return symbol_type_; }
    LexerSymbol access_modifier() const { return access_modifier_; }
    bool native() const { return native_; }
    const Type& type() const { return type_; }
    const std::string& name() const { return name_; }
    const std::string& spec_name() const { return spec_name_; }

private:
    SymbolType symbol_type_;
    LexerSymbol access_modifier_;
    const bool native_;
    const Type type_;
    const std::string name_;
    const std::string spec_name_;
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_SYMBOL__H__
