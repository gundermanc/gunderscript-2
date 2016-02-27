// Gunderscript-2 Virtual Machine API
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_VIRTUAL_MACHINE__H__
#define GUNDERSCRIPT_VIRTUAL_MACHINE__H__

#include "common_resources.h"
#include "module.h"

namespace gunderscript {

// Forward declaration of private implementation class.
class VirtualMachineImpl;

// Declaration of public class interface.
class VirtualMachine {
public:
    VirtualMachine(CommonResources& common_resources);

    // TODO: replace with universal function calling interface.
    int HackyRunScriptMainInt(Module& module);
    float HackyRunScriptMainFloat(Module& module);
    char HackyRunScriptMainChar(Module& module);
    bool HackyRunScriptMainBool(Module& module);

private:
    std::shared_ptr<VirtualMachineImpl> pimpl_;
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_VIRTUAL_MACHINE__H__
