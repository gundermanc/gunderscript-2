// Gunderscript-2 Module API
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_MODULE__H__
#define GUNDERSCRIPT_MODULE__H__

#include <memory>
#include <string>

namespace gunderscript {

// Forward declaration of private implementation class.
class ModuleImpl;

// Declaration of public class interface.
class Module {
public:
    Module();

    bool compiled();
    bool assembled();
    const std::string& module_name() const;
    ModuleImpl* pimpl() const { return pimpl_.get(); }

private:
    std::shared_ptr<ModuleImpl> pimpl_;
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_MODULE__H__
