// Gunderscript-2 Symbol Table Symbol
// (C) 2014 Christian Gunderman

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

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_SYMBOL__H__
