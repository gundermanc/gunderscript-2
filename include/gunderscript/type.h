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
    POINTER
};

class Type {
public:
    Type(const std::string& symbol_name, const TypeFormat type_format, int size = -1)
        : symbol_name_(symbol_name), type_format_(type_format), size_(size) { }
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
const Type TYPE_NONE("none", TypeFormat::POINTER, -1);
const Type TYPE_FUNCTION("function", TypeFormat::POINTER, sizeof(void*));
const Type TYPE_BOOL("bool", TypeFormat::BOOL, 1);
const Type TYPE_INT("int32", TypeFormat::INT, 4);
const Type TYPE_FLOAT("float32", TypeFormat::FLOAT, 4);
const Type TYPE_STRING("string", TypeFormat::POINTER);
const Type TYPE_INT8("int8", TypeFormat::INT, 1);

// Vector of all default types.
const std::vector<Type> TYPES = {
    TYPE_BOOL,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_INT8
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_TYPE__H__
