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