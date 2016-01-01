// Gunderscript-2 Symbol Table
// (C) 2014-2015 Christian Gunderman

#ifndef GUNDERSCRIPT_SYMBOL_TABLE__H__
#define GUNDERSCRIPT_SYMBOL_TABLE__H__

#include <unordered_map>
#include <string>
#include <vector>

#include "exceptions.h"

namespace gunderscript {
namespace library {

template <typename ValueType>
class SymbolTable {
public:
    SymbolTable();
    void Push();
    void Pop();
    void Put(const std::string& key, ValueType value);
    void PutBottom(const std::string& key, ValueType value);
    const ValueType& Get(const std::string& key) const;
    const ValueType& GetTopOnly(const std::string& key) const;
    int depth() const { return this->map_vector_.size(); };

private:
    std::vector< std::unordered_map<std::string, ValueType, std::hash<std::string> > > map_vector_;
};

// SymbolTable Exceptions Parent Class
// All Parser exceptions descend from this class.
class SymbolTableException : public Exception {
public:
    SymbolTableException(const std::string& message)
        : Exception(message) { }
};

// SemanticAstWalker undefined symbol name exception.
class SymbolTableUndefinedSymbolException : public SymbolTableException {
public:
    SymbolTableUndefinedSymbolException() :
        SymbolTableException("Undefined symbol.") { }
};

// SemanticAstWalker bottom of stack exception.
class SymbolTableBottomOfStackException : public SymbolTableException {
public:
    SymbolTableBottomOfStackException() :
        SymbolTableException("Cannot pop symbol table, no more tables.") { }
};

// SemanticAstWalker duplicate key exception.
class SymbolTableDuplicateKeyException : public SymbolTableException {
public:
    SymbolTableDuplicateKeyException() :
        SymbolTableException("Cannot define symbol. Symbol already defined.") { }
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_SYMBOL_TABLE__H__
