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
    ~VirtualMachine();

    // TODO: replace with universal function calling interface.
    int HackyRunScriptMain(const Module& module);

private:
    VirtualMachineImpl* pimpl_;
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_VIRTUAL_MACHINE__H__
