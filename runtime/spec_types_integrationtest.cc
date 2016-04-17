// Gunderscript 2 Spec Types and Operations Integration Test
// (C) 2016 Christian Gunderman

#include "gtest/gtest.h"

#include "testing_macros.h"

#include "gunderscript/compiler.h"
#include "gunderscript/virtual_machine.h"

TEST(SpecTypesIntegration, MemberFunctionCalls) {
    EXPECT_EQ(5, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() {"
        "    return MathPassThru(new Math()).add(3, 2);"
        "}"
        "public Math MathPassThru(Math math) {"
        "    math2 <- math;"
        "    return math2;"
        "}"
        "public spec Math {"
        "    public construct() { }"
        "    public int32 add(int32 a, int32 b) {"
        "        return a + b;"
        "    }"
        "}"));
}