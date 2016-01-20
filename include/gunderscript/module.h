// Gunderscript-2 Module API
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_MODULE__H__
#define GUNDERSCRIPT_MODULE__H__

namespace gunderscript {

// Forward declaration of private implementation class.
class ModuleImpl;

// Declaration of public class interface.
class Module {
public:
    Module();
    ~Module();

private:
    ModuleImpl* pimpl_;
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_MODULE__H__
