// Gunderscript-2 Common Resources API
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_COMMON_RESOURCES__H__
#define GUNDERSCRIPT_COMMON_RESOURCES__H__

#include <memory>

namespace gunderscript {

// Forward declaration of private implementation class.
class CommonResourcesImpl;

// Declaration of public class interface.
class CommonResources {
public:
    CommonResources();
    bool verbose_asm();
    void set_verbose_asm(bool verbose_asm);

    CommonResourcesImpl& pimpl() { return *(pimpl_.get()); }

private:
    std::shared_ptr<CommonResourcesImpl> pimpl_;
};

} // namespace gunderscript

#endif // GUNDERSCRIPT_COMMON_RESOURCES__H__
