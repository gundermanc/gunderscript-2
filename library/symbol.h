// Gunderscript-2 Symbol Table Symbol
// (C) 2015 Christian Gunderman

#ifndef GUNDERSCRIPT_SYMBOL__H__
#define GUNDERSCRIPT_SYMBOL__H__

#include <string>

#include "lexer.h"
#include "symbol_table.h"

namespace gunderscript {
namespace library {

class Symbol {
public:
    Symbol(LexerSymbol access_modifier, const std::string& name)
        : access_modifier_(access_modifier), name_(name) { }

    LexerSymbol access_modifier() const { return access_modifier_; }
    const std::string& name() const { return name_; }

private:
    const LexerSymbol access_modifier_;
    const std::string name_;
};

class FunctionSymbol : public Symbol {
public:
    FunctionSymbol(
        LexerSymbol access_modifier,
        const std::string& class_name,
        const std::string& name,
        bool native,
        LexerSymbol type) : Symbol(access_modifier, name), native_(native),
        type_(type), class_name_(class_name) { }

    const std::string& class_name() const { return class_name_; }
    bool native() const { return native_; }
    LexerSymbol type() const { return type_; }

private:
    const bool native_;
    const LexerSymbol type_;
    const std::string class_name_;
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_SYMBOL__H__
