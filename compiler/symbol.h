// Gunderscript-2 Symbol Table Symbol
// (C) 2015 Christian Gunderman

#ifndef GUNDERSCRIPT_SYMBOL__H__
#define GUNDERSCRIPT_SYMBOL__H__

#include <string>

#include "lexer.h"
#include "type.h"

namespace gunderscript {
namespace compiler {

class Symbol {
public:
    Symbol(
        LexerSymbol access_modifier,
        bool native,
        const Type& type,
        const std::string& spec_name,
        const std::string& name) 
        : access_modifier_(access_modifier),
        native_(native),
        type_(type),
        spec_name_(spec_name),
        name_(name) { }

    LexerSymbol access_modifier() const { return access_modifier_; }
    bool native() const { return native_; }
    const Type& type() const { return type_; }
    const std::string name() const { return name_; }
    const std::string spec_name() const { return spec_name_; }

private:
    LexerSymbol access_modifier_;
    const bool native_;
    const Type type_;
    const std::string name_;
    const std::string spec_name_;
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_SYMBOL__H__
