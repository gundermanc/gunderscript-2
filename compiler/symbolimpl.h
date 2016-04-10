// Gunderscript-2 Symbol Table Symbol Private
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_SYMBOLIMPL__H__
#define GUNDERSCRIPT_SYMBOLIMPL__H__

#include "gunderscript/symbol.h"

#include "gs_assert.h"

namespace gunderscript {

// Defines SYMBOL_CAST macro for statically casting down to Symbol child classes.
// Subclass type is checked via assert in DEBUG configuration. Although down casting
// is normally indicative of bad OOP design, it is necesary in this case because
// we have several distinct types that have members and unused params waste memory.
#ifdef _DEBUG

#define SYMBOL_CAST(src_symbol, sym_type, cast_type)                                 \
([](const SymbolBase* sym) {                                                         \
    GS_ASSERT_TRUE((sym)->symbol_type() == (sym_type), "Invalid symbol cast");       \
    return static_cast< cast_type >((sym));                                          \
}(src_symbol))

#else // _DEBUG

#define SYMBOL_CAST(src_symbol, sym_type, cast_type) static_cast<cast_type>(src_symbol);

#endif // _DEBUG

#define SYMBOL_TO_FUNCTION(src_symbol) SYMBOL_CAST(src_symbol, SymbolType::FUNCTION, const FunctionSymbol*)

#define SYMBOL_TO_PROPERTY(src_symbol) SYMBOL_CAST(src_symbol, SymbolType::PROPERTY, const FunctionSymbol*)

#define SYMBOL_TO_VARIABLE(src_symbol) SYMBOL_CAST(src_symbol, SymbolType::VARIABLE, const FunctionSymbol*)

#define SYMBOL_TO_TYPE(src_symbol) SYMBOL_CAST(src_symbol, SymbolType::TYPE, const TypeSymbol*)

#define SYMBOL_TO_GENERIC_TYPE(src_symbol) SYMBOL_CAST(src_symbol, SymbolType::GENERIC_TYPE, const GenericTypeSymbol*)

#define SYMBOL_TO_GENERIC_TYPE_TEMPLATE(src_symbol) SYMBOL_CAST(src_symbol, SymbolType::GENERIC_TYPE_TEMPLATE, const GenericTypeSymbol*)

#define SYMBOL_TO_PARAM(src_symbol) SYMBOL_CAST(src_symbol, SymbolType::PARAM, const ValueSymbol*)

} // namespace gunderscript

#endif // GUNDERSCRIPT_SYMBOLIMPL__H__
