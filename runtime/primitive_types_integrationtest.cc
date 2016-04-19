// Gunderscript 2 Primitive Types and Operations Integration Tests
// (C) 2016 Christian Gunderman

#include "gtest/gtest.h"

#include "testing_macros.h"

#include "gunderscript/compiler.h"
#include "gunderscript/virtual_machine.h"

// All of these implicitly test return int.

TEST(PrimitiveTypesIntegration, AddInt) {
    EXPECT_EQ(15, COMPILE_AND_RUN_INT_MAIN_LINES("return 10 + 5;"));
}

TEST(PrimitiveTypesIntegration, SubInt) {
    EXPECT_EQ(-3, COMPILE_AND_RUN_INT_MAIN_LINES("return 3 - 6;"));
}

TEST(PrimitiveTypesIntegration, MulInt) {
    EXPECT_EQ(11988, COMPILE_AND_RUN_INT_MAIN_LINES("return 999 * 12;"));
}

TEST(PrimitiveTypesIntegration, DivInt) {
    EXPECT_EQ(2, COMPILE_AND_RUN_INT_MAIN_LINES("return 9 / 4;"));
}

TEST(PrimitiveTypesIntegration, ModInt) {
    EXPECT_EQ(3, COMPILE_AND_RUN_INT_MAIN_LINES("return 95 % 4;"));
}

TEST(PrimitiveTypesIntegration, OrderOfOpsInt) {
    EXPECT_EQ(-4, COMPILE_AND_RUN_INT_MAIN_LINES("return 3 + 6 % 4 - 5 * 2 + 3 / 2;"));
}

// All of these implicitly test return float.

TEST(PrimitiveTypesIntegration, AddFloat) {
    EXPECT_FLOAT_EQ(16, COMPILE_AND_RUN_FLOAT_MAIN_LINES("return 10.3 + 5.7;"));
}

TEST(PrimitiveTypesIntegration, SubFloat) {
    EXPECT_FLOAT_EQ(-3, COMPILE_AND_RUN_FLOAT_MAIN_LINES("return 3.2 - 6.2;"));
}

TEST(PrimitiveTypesIntegration, MulFloat) {
    EXPECT_FLOAT_EQ(12.25, COMPILE_AND_RUN_FLOAT_MAIN_LINES("return 3.5 * 3.5;"));
}

TEST(PrimitiveTypesIntegration, DivFloat) {
    EXPECT_FLOAT_EQ(4.75, COMPILE_AND_RUN_FLOAT_MAIN_LINES("return 9.5 / 2.0;"));
}

TEST(PrimitiveTypesIntegration, ModFloat) {
    EXPECT_FLOAT_EQ(3.0, COMPILE_AND_RUN_FLOAT_MAIN_LINES("return 95.0 % 4.0;"));
}

TEST(PrimitiveTypesIntegration, OrderOfOpsFloat) {
    EXPECT_FLOAT_EQ(-3.5, COMPILE_AND_RUN_FLOAT_MAIN_LINES("return 3.0 + 6.0 % 4.0 - 5.0 * 2.0 + 3.0 / 2.0;"));
}

// All of these implicitly test return int8/char.

TEST(PrimitiveTypesIntegration, AddChar) {
    EXPECT_EQ(15, COMPILE_AND_RUN_INT8_MAIN_LINES("return int8(10) + int8(5);"));
}

TEST(PrimitiveTypesIntegration, SubChar) {
    EXPECT_EQ(-3, COMPILE_AND_RUN_INT8_MAIN_LINES("return int8(3) - int8(6);"));
}

TEST(PrimitiveTypesIntegration, MulChar) {
    EXPECT_EQ(10, COMPILE_AND_RUN_INT8_MAIN_LINES("return int8(5) * int8(2);"));
}

TEST(PrimitiveTypesIntegration, DivChar) {
    EXPECT_EQ(2, COMPILE_AND_RUN_INT8_MAIN_LINES("return int8(9) / int8(4);"));
}

TEST(PrimitiveTypesIntegration, ModChar) {
    EXPECT_EQ(3, COMPILE_AND_RUN_INT8_MAIN_LINES("return int8(95) % int8(4);"));
}

TEST(PrimitiveTypesIntegration, OrderOfOpsChar) {
    EXPECT_EQ(-4, COMPILE_AND_RUN_INT8_MAIN_LINES("return int8(3) + int8(6) % int8(4) - int8(5) * int8(2) + int8(3) / int8(2);"));
}

TEST(PrimitiveTypesIntegration, CharConstantsCombined) {
    EXPECT_EQ(35, COMPILE_AND_RUN_INT8_MAIN_LINES("return (('9' - '0') + '0' * int8(2)) / int8(3);"));
}

TEST(PrimitiveTypesIntegration, StoreLoadInt) {
    EXPECT_EQ(3, COMPILE_AND_RUN_INT_MAIN_LINES("x <- 3; return x;"));
}

TEST(PrimitiveTypesIntegration, StoreLoadChar) {
    EXPECT_EQ('a', COMPILE_AND_RUN_INT8_MAIN_LINES("foo <- 'a'; return foo;"));
}

TEST(PrimitiveTypesIntegration, StoreLoadFloat) {
    EXPECT_FLOAT_EQ(3.5, COMPILE_AND_RUN_FLOAT_MAIN_LINES("foo <- 3.5; return foo;"));
}

TEST(PrimitiveTypesIntegration, StoreLoadBool) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("bval <- true; return bval;"));
}

TEST(PrimitiveTypesIntegration, StoreUpdate) {
    EXPECT_EQ(2, COMPILE_AND_RUN_INT_MAIN_LINES("x <- 1; x <- x + 1; return x;"));
}

// Since first time assignment and variable declaration are the same thing in
// Gunderscript, the only real scoping provided by blocks is that of variable lifetime
// and masking of different types.
// In the second to last test, x is initially of value bool but is masked by an integer of
// the same name in the inner scope. When that scope is popped, x is an integer again.
// In the last test, x is of type true in the inner block which goes out of scope, allowing
// it to be reassigned to integer, which is not normally allowed.
TEST(PrimitiveTypesIntegration, EndToEndScoping) {
    EXPECT_EQ(3, COMPILE_AND_RUN_INT_MAIN_LINES("return 3; { }"));
    EXPECT_EQ(3, COMPILE_AND_RUN_INT_MAIN_LINES("{ } return 3;"));
    EXPECT_EQ(2, COMPILE_AND_RUN_INT_MAIN_LINES("x <- 1; { x <- 2; return x; } return x;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("x <- true; { x <- 0; } return x;"));
    EXPECT_EQ(1, COMPILE_AND_RUN_INT_MAIN_LINES("{ x <- true; } x <- 1; return x;"));
}

TEST(PrimitiveTypesIntegration, GreaterInt) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 1 > 0;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 0 > 0;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return -1 > 0;"));
}

TEST(PrimitiveTypesIntegration, LessInt) {
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 1 < 0;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 0 < 0;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return -1 < 0;"));
}

TEST(PrimitiveTypesIntegration, GreaterEqualsInt) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 1 >= 0;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 0 >= 0;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return -1 >= 0;"));
}

TEST(PrimitiveTypesIntegration, LessEqualsInt) {
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 1 <= 0;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 0 <= 0;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return -1 <= 0;"));
}

TEST(PrimitiveTypesIntegration, EqualsInt) {
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 1 = 0;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 0 = 0;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return -1 = 0;"));
}

TEST(PrimitiveTypesIntegration, NotEqualsInt) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 1 != 0;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 0 != 0;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return -1 != 0;"));
}

TEST(PrimitiveTypesIntegration, GreaterFloat) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 1.0 > 0.0;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 0.0 > 0.0;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return -1.0 > 0.0;"));
}

TEST(PrimitiveTypesIntegration, LessFloat) {
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 1.0 < 0.0;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 0.0 < 0.0;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return -1.0 < 0.0;"));
}

TEST(PrimitiveTypesIntegration, GreaterEqualsFloat) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 1.0 >= 0.0;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 0.0 >= 0.0;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return -1.0 >= 0.0;"));
}

TEST(PrimitiveTypesIntegration, LessEqualsFloat) {
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 1.0 <= 0.0;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 0.0 <= 0.0;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return -1.0 <= 0.0;"));
}

TEST(PrimitiveTypesIntegration, EqualsFloat) {
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 1.0 = 0.0;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 0.0 = 0.0;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return -1.0 = 0.0;"));
}

TEST(PrimitiveTypesIntegration, NotEqualsFloat) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 1.0 != 0.0;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 0.0 != 0.0;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return -1.0 != 0.0;"));
}

TEST(PrimitiveTypesIntegration, GreaterChar) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(1) > int8(0);"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(0) > int8(0);"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(-1) > int8(0);"));
}

TEST(PrimitiveTypesIntegration, LessChar) {
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(1) < int8(0);"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(0) < int8(0);"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(-1) < int8(0);"));
}

TEST(PrimitiveTypesIntegration, GreaterEqualsChar) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(1) >= int8(0);"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(0) >= int8(0);"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(-1) >= int8(0);"));
}

TEST(PrimitiveTypesIntegration, LessEqualsChar) {
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(1) <= int8(0);"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(0) <= int8(0);"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(-1) <= int8(0);"));
}

TEST(PrimitiveTypesIntegration, EqualsChar) {
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(1) = int8(0);"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(0) = int8(0);"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return int8(-1) = int8(0);"));
}

TEST(PrimitiveTypesIntegration, NotEqualsChar) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 1 != 0;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return 0 != 0;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return -1 != 0;"));
}

TEST(PrimitiveTypesIntegration, AndBool) {
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return false && false;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return true && false;"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return false && true;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return true && true;"));
}

TEST(PrimitiveTypesIntegration, OrBool) {
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return false || false;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return true || false;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return false || true;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return true || true;"));
}

TEST(PrimitiveTypesIntegration, NotBool) {
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return !true;"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return !false;"));
}

TEST(PrimitiveTypesIntegration, LotsOfArgs) {
    EXPECT_EQ(291, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() { return lots(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11); }"
        "public int32 lots(int32 a, int32 b, int32 c, int32 d, int32 e, int32 f, int32 g, int32 h, int32 i, int32 j, int32 k) { return a + b - c * d * e + f * g * h - i + j + k; }"));
}

TEST(PrimitiveTypesIntegration, ParamsAndVars) {
    EXPECT_EQ(20, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() { "
        "   x <- 3 + 2;"
        "   return y(x + 1) + 2;"
        "}"
        "public int32 y(int32 x) { y <- 3 * x; return y; }"));
}

TEST(PrimitiveTypesIntegration, ParamReassignment) {
    EXPECT_EQ(20, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() { "
        "   x <- 3 + 2;"
        "   return y(x + 1) + 2;"
        "}"
        "public int32 y(int32 x) { x <- 3 * x; return x; }"));
}

TEST(PrimitiveTypesIntegration, CallBoolParamsAndReturns) {
    EXPECT_EQ(0, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public bool x(bool x, bool z) { return !x && !z; }"
        "public int32 main() { return int32(x(1 != 1, 2 > 1)); }"));
}

TEST(PrimitiveTypesIntegration, CallIntParamsAndReturns) {
    EXPECT_EQ(8, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 x(int32 x, int32 z) { return 2 * z(x + z); }"
        "public int32 main() { return x(1 + 1, 2 - 2); }"
        "public int32 z(int32 z) { return z + z; }"));
}

TEST(PrimitiveTypesIntegration, CallFloatParamsAndReturns) {
    EXPECT_EQ(8, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public float32 x(float32 x, float32 z) { return 2.0 * z(x + z); }"
        "public int32 main() { return int32(x(1.0 + 1.0, 2.0 - 2.0)); }"
        "public float32 z(float32 z) { return z + z; }"));
}

TEST(PrimitiveTypesIntegration, CallCharParamsAndReturns) {
    EXPECT_EQ(8, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int8 x(int8 x, int8 z) { return int8(2) * z(x + z); }"
        "public int32 main() { return int32(x(int8(1 + 1), int8(2 - 2))); }"
        "public int8 z(int8 z) { return z + z; }"));
}

TEST(PrimitiveTypesIntegration, CallCombinedParams) {
    EXPECT_EQ(1, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public bool x(bool a, float32 b, bool c, int32 d, bool e, int8 f, bool g) { return a && b = 1.0 && !c &&  d = 2 && e && f = int8(122) && !g; }"
        "public int32 main() { return int32(x(true, 1.0, false, 2, true, int8(122), false)); }"));
}

TEST(PrimitiveTypesIntegration, VoidFunc) {
    EXPECT_NO_THROW(COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() { "
        "    foo();"
        "}"
        "public void foo() { return; }"));
}