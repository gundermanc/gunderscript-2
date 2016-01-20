// Gunderscript-2 Virtual Machine API
// (C) 2016 Christian Gunderman

#include "gunderscript/virtual_machine.h"

namespace gunderscript {

// Virtual Machine Private Implementation.
class VirtualMachineImpl {

};

// Public constructor.
VirtualMachine::VirtualMachine() : pimpl_(new VirtualMachineImpl()) {
}

// Public destructor.
VirtualMachine::~VirtualMachine() {
    delete this->pimpl_;
}

} // namespace gunderscript
