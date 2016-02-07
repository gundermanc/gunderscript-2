// Gunderscript-2 Virtual Machine API
// (C) 2016 Christian Gunderman

#include <unordered_map>

#include "gunderscript/virtual_machine.h"

#include "common_resourcesimpl.h"
#include "moduleimpl.h"

#include "nanojit.h"

using namespace nanojit;

namespace gunderscript {

// Virtual Machine Private Implementation.
class VirtualMachineImpl {
public:
    VirtualMachineImpl(Allocator& alloc, Config& config) 
        : alloc_(alloc), config_(config) { }

    Allocator& alloc() { return alloc_; }
    Config& config() { return config_; }
    int HackyRunScriptMain(const Module& module);

private:
    Allocator& alloc_;
    Config& config_;
};

// Public constructor.
VirtualMachine::VirtualMachine(CommonResources& common_resources) 
    : pimpl_(new VirtualMachineImpl(
        common_resources.pimpl().alloc(),
        common_resources.pimpl().config())) {

}

// Public destructor.
VirtualMachine::~VirtualMachine() {
    delete this->pimpl_;
}

int VirtualMachine::HackyRunScriptMain(const Module& module) {
    return this->pimpl_->HackyRunScriptMain(module);
}

// TODO: replace this with a legit function call mechanism.
int VirtualMachineImpl::HackyRunScriptMain(const Module& module) {

    // Assemble module:
    // TODO: share these resources between compiler and runtime.
    bool optimize = true;
    LogControl lc;
    CodeAlloc codeAlloc(&config_);

#ifdef NJ_VERBOSE
    lc.lcbits = LC_ReadLIR | LC_Native;
#else
    lc.lcbits = 0;
#endif

    Assembler assm(codeAlloc, alloc_, alloc_, &lc, config_);

    Fragment* f = NULL;

    // TODO: store references instead???
    std::unordered_map<std::string, ModuleImplSymbol> symbols_map;

    // Add symbols to the table.
    for (size_t i = 0; i < module.pimpl()->symbols_vector().size(); i++) {
        // TODO: handle exception if symbol exists.

        printf(module.pimpl()->symbols_vector().at(i).symbol_name().c_str());
        symbols_map.insert(std::make_pair(
            module.pimpl()->symbols_vector().at(i).symbol_name(),
            module.pimpl()->symbols_vector().at(i)));
    }

    try {
        ModuleImplSymbol& main = symbols_map.at("Foo::main");

        f = main.fragment();

#ifdef NJ_VERBOSE
        LInsPrinter p(alloc_, 1024);
        f->lirbuf->printer = &p;
#endif

        assm.compile(f, alloc_, optimize verbose_only(, &p));
        printf("error: %d\n", assm.error());

        typedef int32_t(*MainFunction)();

        // Call the function.
        return reinterpret_cast<MainFunction>(f->code())();

        //return 0;
    }
    catch (const std::exception&) {
        // TODO
        printf("No symbols");
        return -1;
    }
}

} // namespace gunderscript
