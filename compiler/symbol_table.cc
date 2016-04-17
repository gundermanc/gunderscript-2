// Gunderscript-2 Symbol Table
// (C) 2014 Christian Gunderman

#include <tuple>
#include <utility>

#include "gunderscript/symbol.h"

#include "symbol_table.h"

// HACK: required for explicit class instanatiation linking across libraries.
#include "lirgen_ast_walker.h"

namespace gunderscript {
namespace compiler {

// Instantiate template so we can unit test and link from external modules.
template class SymbolTable<std::tuple<const SymbolBase*, LIns*, int>>;
template class SymbolTable<std::string>;
template class SymbolTable<const SymbolBase*>;

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
        // Never thrown in normal operation, the numbers are arbitrary.
        THROW_EXCEPTION(
            1,
            1,
            STATUS_SYMBOLTABLE_BOTTOM_OF_STACK);
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
        THROW_EXCEPTION(
            1,
            1,
            STATUS_SYMBOLTABLE_DUPLICATE_SYMBOL);
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
        // These are incorrect line numbers but this exception should ALWAYS be caught
        // and never bubble up so it doesn't matter.
        THROW_EXCEPTION(
            1,
            1,
            STATUS_SYMBOLTABLE_DUPLICATE_SYMBOL);
    }
}

// Gets the most recently declared value associated with the given
// key by selecting the definition of value from the top most table.
// key: the symbol to look up.
// Returns: the value most recently associated with key.
// Throws: SymbolTable exception subclass if symbol is undefined.
template <typename ValueType>
const ValueType& SymbolTable<ValueType>::Get(const std::string& key) const {

    // size_t is the correct type to use when indexing the map_vector_ since
    // we can't a have a negative index, however, it is unsigned and so i
    // less than zero is an invalid loop termination because if i is zero and
    // we subtract one, it wraps around. To combat this, i is equal to the
    // desired index + 1 and the termination condition is i == 0. i = 1 maps
    // maps to the zero-th index.
    for (size_t i = this->map_vector_.size(); i > 0; i--) {
        try {
            return this->map_vector_[i-1].at(key);
        }
        catch (const std::out_of_range&) {
            // do nothing
        }
    }

    // These are incorrect line numbers but this exception should ALWAYS be caught
    // and never bubble up so it doesn't matter.
    THROW_EXCEPTION(
        1,
        1,
        STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
}

// Gets the value associated with the given symbol, if one was Put
// since last call to Push().
// key: The symbol to look up.
// Returns: the value associated with key since last call to Push().
// Throws: SymbolTableException if key does not exist.
template <typename ValueType>
const ValueType& SymbolTable<ValueType>::GetTopOnly(const std::string& key) const {
    try {
        return this->map_vector_.back().at(key);
    }
    catch (const std::out_of_range&) {
        THROW_EXCEPTION(
            1,
            1,
            STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
    }
}

// Updates the value associated with the given symbol.
// key: The symbol to look up.
// Returns: the value associated with key since last call to Push().
// Throws: SymbolTableException if key does not exist.
template <typename ValueType>
void SymbolTable<ValueType>::UpdateExisting(const std::string& key, ValueType value) {

    // size_t is the correct type to use when indexing the map_vector_ since
    // we can't a have a negative index, however, it is unsigned and so i
    // less than zero is an invalid loop termination because if i is zero and
    // we subtract one, it wraps around. To combat this, i is equal to the
    // desired index + 1 and the termination condition is i == 0. i = 1 maps
    // maps to the zero-th index.
    for (size_t i = this->map_vector_.size(); i > 0; i--) {
        std::unordered_map<std::string, ValueType, std::hash<std::string> >& map =
            this->map_vector_[i - 1];

        // Try to find item.
        std::unordered_map<std::string, ValueType, std::hash<std::string> >::iterator iter
            = map.find(key);

        // If element already existed in the map, then update it.
        if (iter != map.end()) {
            iter->second = value;
            return;
        }
    }

    // These are incorrect line numbers but this exception should ALWAYS be caught
    // and never bubble up so it doesn't matter.
    THROW_EXCEPTION(
        1,
        1,
        STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
}

// Updates the value associated with the given symbol, if one was Put
// since last call to Push().
// key: The symbol to look up.
// Returns: the value associated with key since last call to Push().
// Throws: SymbolTableException if key does not exist.
template <typename ValueType>
void SymbolTable<ValueType>::UpdateExistingTopOnly(const std::string& key, ValueType value) {
    std::unordered_map<std::string, ValueType, std::hash<std::string> >& map =
        this->map_vector_.back();

    // Try to find item.
    std::unordered_map<std::string, ValueType, std::hash<std::string> >::iterator iter
        = map.find(key);

    // If element already existed in the map, then update it.
    if (iter != map.end()) {
        iter->second = value;
        return;
    }

    THROW_EXCEPTION(
        1,
        1,
        STATUS_SYMBOLTABLE_UNDEFINED_SYMBOL);
}

} // namespace compiler
} // namespace gunderscript
