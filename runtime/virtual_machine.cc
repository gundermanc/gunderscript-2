// Gunderscript-2 Virtual Machine API
// (C) 2016 Christian Gunderman

#include <unordered_map>

#include "gunderscript/exceptions.h"
#include "gunderscript/virtual_machine.h"

#include "gs_assert.h"
#include "common_resourcesimpl.h"
#include "moduleimpl.h"

#include "nanojit.h"

// Platform specific:
// TODO: abstract this.
#ifdef _MSC_VER
// Windows MSVC++ only:
#define GS_CDECL __cdecl
#elif __GNUG__
// GCC only:
#define GS_CDECL __attribute__((cdecl))
#endif

using namespace nanojit;

namespace gunderscript {

// Virtual Machine Private Implementation.
class VirtualMachineImpl {
public:
    VirtualMachineImpl(CommonResourcesImpl& common_resources) 
        : code_alloc_(&common_resources.config()), common_resources_(common_resources) {
        // Setup Nanojit Log control logging if in NJ_VERBOSE (Debug configuration).
#ifdef NJ_VERBOSE
        this->log_control_.lcbits = common_resources.verbose_asm() ? LC_ReadLIR | LC_Native : 0;
#else
        this->log_control_.lcbits = 0;
#endif
    }

    void AssembleModule(Module& module);
    int HackyRunScriptMainInt(Module& module);
    float HackyRunScriptMainFloat(Module& module);
    char HackyRunScriptMainChar(Module& module);
    bool HackyRunScriptMainBool(Module& module);

private:
    LogControl* log_control() { return &log_control_; }
    CodeAlloc& code_alloc() { return code_alloc_; }

    CommonResourcesImpl& common_resources_;
    LogControl log_control_;
    CodeAlloc code_alloc_;
};

// Public constructor.
VirtualMachine::VirtualMachine(CommonResources& common_resources) 
    : pimpl_(new VirtualMachineImpl(
        common_resources.pimpl())) {

}

int VirtualMachine::HackyRunScriptMainInt(Module& module) {
    return this->pimpl_->HackyRunScriptMainInt(module);
}

float VirtualMachine::HackyRunScriptMainFloat(Module& module) {
    return this->pimpl_->HackyRunScriptMainFloat(module);
}

char VirtualMachine::HackyRunScriptMainChar(Module& module) {
    return this->pimpl_->HackyRunScriptMainChar(module);
}

bool VirtualMachine::HackyRunScriptMainBool(Module& module) {
    return this->pimpl_->HackyRunScriptMainBool(module);
}

void VirtualMachine::AssembleModule(Module& module) {
    this->pimpl_->AssembleModule(module);
}

void VirtualMachineImpl::AssembleModule(Module& module) {

    // No need to assemble a module multiple times.
    if (module.assembled()) {
        return;
    }

    // If assembly succeeds or fails, we don't want to allow trying again.
    module.pimpl()->set_assembled(true);

    // Allow unoptimized builds in the future??
    const bool optimize = true;

    // Allocate an assembler.
    Assembler assm(
        this->code_alloc_,
        this->common_resources_.alloc(),
        this->common_resources_.alloc(),
        &this->log_control_,
        this->common_resources_.config());

    // Assemble all fragments in the module.
    for (size_t i = 0; i < module.pimpl()->symbols_vector().size(); i++) {
        Fragment* f = module.pimpl()->symbols_vector().at(i).fragment();

        GS_ASSERT_FALSE(f == NULL, "NULL fragment in assembler");

        // Set the ABI now to be sure that it matches the expected value.
        f->lirbuf->abi = AbiKind::ABI_CDECL;

        // Create an instruction printer if in NJ_VERBOSE (Debug Configuration).
#ifdef NJ_VERBOSE
        if (common_resources_.verbose_asm()) {
            std::cout << "Symbol: "
                << module.pimpl()->symbols_vector().at(i).symbol_name()
                << std::endl;
        }

        LInsPrinter p(this->common_resources_.alloc(), 1024);
        f->lirbuf->printer = &p;
#endif

        // Assemble LIR to native code.
        assm.compile(f, this->common_resources_.alloc(), optimize verbose_only(, &p));

        // Store a reference to this function in the module's function lookup table.
        // This mechanism gives the generated code a place to lookup function addresses
        // to prevent the need to back patch between functions.
        module.pimpl()->func_table()[i] = reinterpret_cast<ModuleFunc>(f->code());

        // Handle assembler errors.
        if (assm.error() != AssmError::None) {

            // Prints exact value on failure.
            GS_ASSERT_TRUE(assm.error() != AssmError::None, "Error performing assemble operation");

            // Probably a bug if this happens.
            THROW_EXCEPTION(1, 1, STATUS_ASSEMBLER_DIED);
        }
    }
}

// TODO: replace this with a legit function call mechanism.
int VirtualMachineImpl::HackyRunScriptMainInt(Module& module) {

    AssembleModule(module);

    for (size_t i = 0; i < module.pimpl()->symbols_vector().size(); i++) {
        ModuleImplSymbol& symbol = module.pimpl()->symbols_vector().at(i);

        // Is this the right function?
        if (symbol.symbol_name() == "main") {

            // TODO: exception for this:
            GS_ASSERT_TRUE(symbol.symbol()->type() == TYPE_INT, "Invalid type in main function");

            // Call the hacky main function.
            typedef int(GS_CDECL *MainFunction)();
            return reinterpret_cast<MainFunction>(symbol.fragment()->code())();
        }
    }

    // TODO: we didn't find the function, fail.
    // Actually handle this error.
    GS_ASSERT_FAIL("No main function");
}

// TODO: replace this with a legit function call mechanism.
float VirtualMachineImpl::HackyRunScriptMainFloat(Module& module) {

    AssembleModule(module);

    for (size_t i = 0; i < module.pimpl()->symbols_vector().size(); i++) {
        ModuleImplSymbol& symbol = module.pimpl()->symbols_vector().at(i);

        // Is this the right function?
        if (symbol.symbol_name() == "main") {

            // TODO: exception for this:
            GS_ASSERT_TRUE(symbol.symbol()->type() == TYPE_FLOAT, "Invalid type in main function");

            // Call the hacky main function.
            typedef float(GS_CDECL *MainFunction)();
            return reinterpret_cast<MainFunction>(symbol.fragment()->code())();
    }
}

    // TODO: we didn't find the function, fail.
    // Actually handle this error.
    GS_ASSERT_FAIL("No main function");
}

// TODO: replace this with a legit function call mechanism.
char VirtualMachineImpl::HackyRunScriptMainChar(Module& module) {

    AssembleModule(module);

    for (size_t i = 0; i < module.pimpl()->symbols_vector().size(); i++) {
        ModuleImplSymbol& symbol = module.pimpl()->symbols_vector().at(i);

        // Is this the right function?
        if (symbol.symbol_name() == "main") {

            // TODO: exception for this:
            GS_ASSERT_TRUE(symbol.symbol()->type() == TYPE_INT8, "Invalid type in main function");

            // Call the hacky main function.
            typedef int(GS_CDECL *MainFunction)();
            return (char)reinterpret_cast<MainFunction>(symbol.fragment()->code())();
    }
}

    // TODO: we didn't find the function, fail.
    // Actually handle this error.
    GS_ASSERT_FAIL("No main function");
}

// TODO: replace this with a legit function call mechanism.
bool VirtualMachineImpl::HackyRunScriptMainBool(Module& module) {

    AssembleModule(module);

    for (size_t i = 0; i < module.pimpl()->symbols_vector().size(); i++) {
        ModuleImplSymbol& symbol = module.pimpl()->symbols_vector().at(i);

        // Is this the right function?
        if (symbol.symbol_name() == "main") {

            // TODO: exception for this:
            GS_ASSERT_TRUE(symbol.symbol()->type() == TYPE_BOOL, "Invalid type in main function");

            // Call the hacky main function.
            typedef int(GS_CDECL *MainFunction)();
            return reinterpret_cast<MainFunction>(symbol.fragment()->code())();
        }
    }

    // TODO: we didn't find the function, fail.
    // Actually handle this error.
    GS_ASSERT_FAIL("No main function");
}
} // namespace gunderscript
