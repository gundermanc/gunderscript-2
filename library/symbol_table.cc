// Gunderscript-2 Symbol Table
// (C) 2014 Christian Gunderman

#include <utility>

#include "symbol.h"
#include "symbol_table.h"

namespace gunderscript {
namespace library {

// Instantiate template so we can unit test and link from external modules.
template class SymbolTable<std::string>;
template class SymbolTable<Symbol>;

// Default Constructor, creates empty SymbolTable with
// automatic minimum depth of 1.
template <typename ValueType>
SymbolTable<ValueType>::SymbolTable() {
    this->Push();
}

// Pushes another level of scope onto the SymbolTable. This
// is equivalent to going inside of anther of '{ }' delimited
// block of code.
template <typename ValueType>
void SymbolTable<ValueType>::Push() {
    this->map_vector_.push_back(
        std::unordered_map<std::string, ValueType, std::hash<std::string> >());
}

// Pops a level of scope off of the SymbolTable. This is
// equivalent to exiting a '{ }' delimited block of code.
// Returns: true if success, false if depth() == 1. Symbol
// table minimum depth is 1.
template <typename ValueType>
void SymbolTable<ValueType>::Pop() {

    // Check that there are items left to pop.
    if (this->map_vector_.size() == 1) {
        throw SymbolTableBottomOfStackException();
    }

    this->map_vector_.pop_back();
}

// Puts the given key in the topmost level of the SymbolTable
// with the given value.
// key: the symbol to associate with value.
// value: the value to associate with symbol.
// Throws: subclass of SymbolTableException if symbol was already
// defined.
template <typename ValueType>
void SymbolTable<ValueType>::Put(const std::string& key, ValueType value) {

    std::pair<typename std::unordered_map<std::string, ValueType>::iterator, bool> result
        = this->map_vector_.back().insert(std::make_pair(key, value));

    // Result of operation is second tuple entry.
    if (!result.second) {
        throw SymbolTableDuplicateKeyException();
    }
}

// Puts the given key in the bottommost level of the SymbolTable
// with the given value.
// key: the symbol to associate with value.
// value: the value to associate with symbol.
// Throws: subclass of SymbolTableException if symbol was already
// defined.
template <typename ValueType>
void SymbolTable<ValueType>::PutBottom(const std::string& key, ValueType value) {

    std::pair<typename std::unordered_map<std::string, ValueType>::iterator, bool> result
        = this->map_vector_.front().insert(std::make_pair(key, value));

    // Result of operation is second tuple entry.
    if (!result.second) {
        throw SymbolTableDuplicateKeyException();
    }
}

// Gets the most recently declared value associated with the given
// key by selecting the definition of value from the top most table.
// key: the symbol to look up.
// Returns: the value most recently associated with key.
// Throws: SymbolTable exception subclass if symbol is undefined.
template <typename ValueType>
const ValueType& SymbolTable<ValueType>::Get(const std::string& key) const {

    for (int i = this->map_vector_.size() - 1; i >= 0; i--) {
        try {
            return this->map_vector_[i].at(key);
        }
        catch (const std::out_of_range ex) {
            // do nothing
        }
    }

    throw SymbolTableUndefinedSymbolException();
}

// Gets the value associated with the given symbol, if one was Put
// since lass call to Push().
// key: The symbol to look up.
// Returns: the value associated with key since last call to Push().
// Throws: SymbolTableException if key does not exist.
template <typename ValueType>
const ValueType& SymbolTable<ValueType>::GetTopOnly(const std::string& key) const {
    try {
        return this->map_vector_.back().at(key);
    }
    catch (const std::out_of_range ex) {
        throw SymbolTableUndefinedSymbolException();
    }
}

} // namespace library
} // namespace gunderscript
