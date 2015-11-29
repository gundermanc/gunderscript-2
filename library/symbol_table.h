// Gunderscript-2 Symbol Table
// (C) 2014 Christian Gunderman

#ifndef GUNDERSCRIPT_SYMBOL_TABLE__H__
#define GUNDERSCRIPT_SYMBOL_TABLE__H__

#include <unordered_map>
#include <string>
#include <vector>

namespace gunderscript {
namespace library {

template <typename ValueType>
class SymbolTable {
 public:
  SymbolTable();
  void Push();
  bool Pop();
  bool Put(std::string key, ValueType value);
  const ValueType* Get(const std::string& key);
  const ValueType* GetTopOnly(const std::string& key);
  int depth() const { return this->map_vector_.size(); };

 private:
  std::vector< std::unordered_map<std::string, ValueType, std::hash<std::string> > > map_vector_;
};

// instantiate template with strings so we can unit test from external module
// TODO: ifdef DEBUG here compatible with CMAKE.
template class SymbolTable<std::string>;

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_SYMBOL_TABLE__H__
