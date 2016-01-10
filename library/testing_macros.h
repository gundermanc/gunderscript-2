// Gunderscript-2 Testing Macros
// (C) 2016 Christian Gunderman

#ifndef GUNDERSCRIPT_TESTING_MACROS__H__
#define GUNDERSCRIPT_TESTING_MACROS__H__

#include "gtest/gtest.h"

#include "gunderscript/exceptions.h"

using namespace gunderscript;

// Pretty basic macro for checking that the exception thrown by stmt
// has the expected exception.
#define EXPECT_STATUS(stmt, exception_status)                   \
try {                                                           \
    stmt;                                                       \
}                                                               \
catch (const Exception2& ex) {                                  \
    EXPECT_EQ(ex.status().code(), (exception_status).code());   \
    return;                                                     \
}                                                               \
FAIL();

#endif // GUNDERSCRIPT_TESTING_MACROS__H__
