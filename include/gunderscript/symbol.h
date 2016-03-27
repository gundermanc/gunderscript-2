// Gunderscript-2 Symbol Table Symbol
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_SYMBOL__H__
#define GUNDERSCRIPT_SYMBOL__H__

#include <cassert>
#include <string>
#include <vector>

#include "lexer_resources.h"

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
    TYPE,
    TYPE_TEMPLATE,
    GENERIC_TYPE_TEMPLATE,
    GENERIC_TYPE,
    PARAM
};

enum class TypeFormat {
    FVOID,
    BOOL,
    INT,
    FLOAT,
    POINTER
};

class TypeSymbol;

class SymbolBase {
public:
    SymbolBase(
        SymbolType symbol_type,
        LexerSymbol access_modifier,
        const std::string& spec_name,
        const std::string& symbol_name)
        : symbol_type_(symbol_type), access_modifier_(access_modifier),
        spec_name_(spec_name), symbol_name_(symbol_name) { }

    // No copy constructor, use clone() to copy so that children may override.
    SymbolBase(const SymbolBase&) = delete;
    virtual ~SymbolBase() { }
    virtual const SymbolBase* Clone() const { return new SymbolBase(symbol_type_, access_modifier_, spec_name_, symbol_name_); }
    virtual const TypeSymbol* type_symbol() const { assert(false); return (TypeSymbol*)(NULL); }

    bool operator==(const SymbolBase& other) const { return this->symbol_name() == other.symbol_name(); }
    bool operator!=(const SymbolBase& other) const { return !(*this == other); }

    SymbolType symbol_type() const { return symbol_type_; }
    LexerSymbol access_modifier() const { return access_modifier_; }
    const std::string& spec_name() const { return spec_name_; }
    const std::string& symbol_name() const { return symbol_name_; }

protected:
    const SymbolType symbol_type_;
    const LexerSymbol access_modifier_;
    const std::string spec_name_;
    const std::string symbol_name_;
};

class FunctionSymbol : public SymbolBase {
public:
    FunctionSymbol(
        SymbolType symbol_type,
        LexerSymbol access_modifier,
        const std::string& spec_name,
        const std::string& symbol_name,
        const SymbolBase* return_symbol)
        : SymbolBase(symbol_type, access_modifier, spec_name, symbol_name),
        return_symbol_(return_symbol) { }
    ~FunctionSymbol() { }
    const SymbolBase* Clone() const { return new FunctionSymbol(symbol_type(), access_modifier(), spec_name(), symbol_name(), return_symbol_); }
    const TypeSymbol* type_symbol() const { return (TypeSymbol*)(return_symbol_); }

private:
    const SymbolBase* return_symbol_;
};

class TypeSymbol : public SymbolBase {
public:
    TypeSymbol(
        SymbolType symbol_type,
        LexerSymbol access_modifier,
        const std::string& symbol_name,
        TypeFormat type_format,
        int size)
        : SymbolBase(symbol_type, access_modifier, "", symbol_name),
        type_format_(type_format), size_(size) { }
    TypeSymbol(
        LexerSymbol access_modifier,
        const std::string& symbol_name) 
        : TypeSymbol(SymbolType::TYPE, access_modifier, symbol_name, TypeFormat::POINTER, sizeof(void*)) { }
    virtual ~TypeSymbol() { }
    virtual const SymbolBase* Clone() const { return new TypeSymbol(symbol_type(), access_modifier(),
        symbol_name(), type_format(), size()); }
    const TypeSymbol* type_symbol() const { return (TypeSymbol*)this; }

    TypeFormat type_format() const { return type_format_; }
    int size() const { return size_; }

private:
    const TypeFormat type_format_;
    const int size_;
};

class GenericTypeSymbol : public TypeSymbol {
public:
    GenericTypeSymbol(
        SymbolType symbol_type,
        LexerSymbol access_modifier,
        const std::string& symbol_name,
        const std::vector<const SymbolBase*> type_params)
        : TypeSymbol(symbol_type, access_modifier, symbol_name, TypeFormat::POINTER, sizeof(void*)),
        type_params_(type_params) { }
    ~GenericTypeSymbol() { }
    const SymbolBase* Clone() const { return new GenericTypeSymbol(symbol_type(), access_modifier(), symbol_name(), type_params()); }
    TypeSymbol* type_symbol() const { return (GenericTypeSymbol*)this; }
    const std::vector<const SymbolBase*> type_params() const { return type_params_; }

private:
    const std::vector<const SymbolBase*> type_params_;
};

// Built in types.
const TypeSymbol TYPE_NONE(LexerSymbol::PUBLIC, "none"); // TODO: kill this.
const TypeSymbol TYPE_VOID(SymbolType::TYPE, LexerSymbol::PUBLIC, "void", TypeFormat::FVOID, 0);
const TypeSymbol TYPE_FUNCTION(LexerSymbol::PUBLIC, "function");
const TypeSymbol TYPE_BOOL(SymbolType::TYPE, LexerSymbol::PUBLIC, "bool", TypeFormat::BOOL, 4);
const TypeSymbol TYPE_INT(SymbolType::TYPE, LexerSymbol::PUBLIC, "int32", TypeFormat::INT, 4);
const TypeSymbol TYPE_FLOAT(SymbolType::TYPE, LexerSymbol::PUBLIC, "float32", TypeFormat::FLOAT, 4);
const TypeSymbol TYPE_STRING(LexerSymbol::PUBLIC, "string");
const TypeSymbol TYPE_INT8(SymbolType::TYPE, LexerSymbol::PUBLIC, "int8", TypeFormat::INT, 1);

// Vector of all builtin types.
const std::vector<const TypeSymbol*> BUILTIN_TYPES = {
    &TYPE_VOID,
    &TYPE_BOOL,
    &TYPE_INT,
    &TYPE_FLOAT,
    &TYPE_STRING,
    &TYPE_INT8
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_SYMBOL__H__
