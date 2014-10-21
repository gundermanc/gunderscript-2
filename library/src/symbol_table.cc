// Gunderscript-2 Symbol Table
// (C) 2014 Christian Gunderman

#include <utility>

#include "symbol_table.h"

namespace gunderscript {
namespace library {

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
    std::unordered_map<std::string, std::string, std::hash<std::string> >()
  );
}

// Pops a level of scope off of the SymbolTable. This is
// equivalent to exiting a '{ }' delimited block of code.
// Returns: true if success, false if depth() == 1. Symbol
// table minimum depth is 1.
template <typename ValueType>
bool SymbolTable<ValueType>::Pop() {

  // Check that there are items left to pop.
  if (this->map_vector_.size() == 1) {
    return false;
  }

  this->map_vector_.pop_back();
  return true;
}

// Puts the given key in the topmost level of the SymbolTable
// with the given value.
// key: the symbol to associate with value.
// value: the value to associate with symbol.
// Returns: true if the value was Put successfully, and false
// if the specified key already exists in the top-most level of
// the table.
template <typename ValueType>
bool SymbolTable<ValueType>::Put(std::string key, ValueType value) {

  std::pair<typename std::unordered_map<std::string, ValueType>::iterator, bool> result
    = this->map_vector_.back().insert(std::make_pair(key, value));

  return result.second;
}

// Gets the most recently declared value associated with the given
// key by selecting the definition of value from the top most table.
// key: the symbol to look up.
// Returns: the value most recently associated with key or NULL if
// there isn't one.
template <typename ValueType>
const ValueType*SymbolTable<ValueType>::Get(const std::string& key) {

  for (int i = this->map_vector_.size()-1; i >= 0; i--) {
    try {
      return &(this->map_vector_[i].at(key));
    } catch (const std::out_of_range ex) {
      // do nothing
    }
  }

  return NULL;
}

// Gets the value associated with the given symbol, if one was Put
// since lass call to Push().
// key: The symbol to look up.
// Returns: the value associated with key since last call to Push(),
// or NULL if there isn't one.
template <typename ValueType>
const ValueType* SymbolTable<ValueType>::GetTopOnly(const std::string& key) {
  try {
    return &(this->map_vector_.back().at(key));
  } catch (const std::out_of_range ex) {
    return NULL;
  }
}

} // namespace library
} // namespace gunderscript
