// Gunderscript-2 Asserts
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_GS_ASSERT__H__
#define GUNDERSCRIPT_GS_ASSERT__H__

#ifdef _DEBUG
#include <cassert>
#include <iostream>

#define GS_ASSERT_TRUE(cond, message)                         \
([](bool condition) {                                         \
    if (!(condition)) {                                       \
        std::cout << std::endl << "  >> " << __FILE__;        \
        std::cout << std::endl << "  >> Line: " << __LINE__;  \
        std::cout << std::endl << "  >> ";                    \
        std::cout << message << ": " << #cond << std::endl;   \
        abort();                                              \
    }                                                         \
}(cond))

#define GS_ASSERT_FALSE(cond, message) GS_ASSERT_TRUE(!(cond), message)

#define GS_ASSERT_FAIL(message) GS_ASSERT_TRUE(/* Always Fail */ false, message)

#else // _DEBUG

#define GS_ASSERT_TRUE(cond, message) ((void)0);

#define GS_ASSERT_FALSE(cond, message) ((void)0);

#define GS_ASSERT_FAIL(message) ((void)0);

#endif // _DEBUG
#endif // GUNDERSCRIPT_GS_ASSERT__H__