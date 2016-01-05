// Gunderscript-2 Type Record Definition
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_TYPE__H__
#define GUNDERSCRIPT_TYPE__H__

#include <vector>

namespace gunderscript {

enum class TypeFormat {
    BOOL,
    INT,
    FLOAT,
    OBJECT
};

class Type {
public:
    Type(const std::string& symbol_name, const TypeFormat type_format, int size = -1)
        : symbol_name_(symbol_name_), type_format_(type_format), size_(size) { }
    const std::string& symbol_name() const { return symbol_name_; }
    TypeFormat type_format() const { return type_format_; }
    int size() const { return size_; }
    bool operator==(const Type& other) const { return this->symbol_name() == other.symbol_name(); }
    bool operator!=(const Type& other) const { return !(*this == other); }

private:
    const std::string symbol_name_;
    const TypeFormat type_format_;
    const int size_;
};

// Default types.
const Type NONE("none", TypeFormat::OBJECT, -1);
const Type BOOL("bool", TypeFormat::BOOL, 8);
const Type INT("int", TypeFormat::INT, 8);
const Type FLOAT("float", TypeFormat::FLOAT, 8);
const Type STRING("string", TypeFormat::OBJECT);
const Type CHAR("char", TypeFormat::INT, 1);

// Vector of all default types.
const std::vector<Type> TYPES = {
    BOOL,
    INT,
    FLOAT,
    STRING,
    CHAR
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_TYPE__H__
