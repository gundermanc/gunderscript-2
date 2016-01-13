// Gunderscript-2 Symbol Table
// (C) 2014-2015 Christian Gunderman

#ifndef GUNDERSCRIPT_SYMBOL_TABLE__H__
#define GUNDERSCRIPT_SYMBOL_TABLE__H__

#include <string>
#include <unordered_map>
#include <vector>

#include "gunderscript/exceptions.h"

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
    size_t depth() const { return this->map_vector_.size(); };

private:
    std::vector< std::unordered_map<std::string, ValueType, std::hash<std::string> > > map_vector_;
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_SYMBOL_TABLE__H__
