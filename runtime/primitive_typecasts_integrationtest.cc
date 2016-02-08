// Gunderscript 2 Primitive Typecasts Integration Tests
// (C) 2016 Christian Gunderman

#include "gtest/gtest.h"

#include "testing_macros.h"

#include "gunderscript/compiler.h"
#include "gunderscript/virtual_machine.h"

TEST(PrimitiveTypecastsIntegration, IntToInt) {
    EXPECT_EQ(3, COMPILE_AND_RUN_INT_MAIN_LINES("return int32(3);"));
}

TEST(PrimitiveTypecastsIntegration, IntToFloat) {
    EXPECT_FLOAT_EQ(3.0, COMPILE_AND_RUN_FLOAT_MAIN_LINES("return float32(3);"));
}

TEST(PrimitiveTypecastsIntegration, IntToBool) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return bool(1);"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return bool(0);"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return bool(-1);"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return bool(101);"));
}

TEST(PrimitiveTypecastsIntegration, IntToChar) {
    EXPECT_EQ(9, COMPILE_AND_RUN_INT8_MAIN_LINES("return int8(9);"));
}

TEST(PrimitiveTypecastsIntegration, FloatToFloat) {
    EXPECT_EQ(9, COMPILE_AND_RUN_FLOAT_MAIN_LINES("return float32(9.0);"));
}

TEST(PrimitiveTypecastsIntegration, FloatToInt) {
    EXPECT_EQ(9, COMPILE_AND_RUN_INT_MAIN_LINES("return int32(9.0);"));
}

TEST(PrimitiveTypecastsIntegration, FloatToBool) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return bool(1.0);"));
    EXPECT_FALSE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return bool(0.0);"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return bool(-1.0);"));
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return bool(101.0);"));
}

TEST(PrimitiveTypecastsIntegration, FloatToChar) {
    EXPECT_EQ(101, COMPILE_AND_RUN_INT8_MAIN_LINES("return int8(101.0);"));
}

TEST(PrimitiveTypecastsIntegration, BoolToBool) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return bool(true);"));
}

TEST(PrimitiveTypecastsIntegration, BoolToInt) {
    EXPECT_EQ(1, COMPILE_AND_RUN_INT_MAIN_LINES("return int32(true);"));
}

TEST(PrimitiveTypecastsIntegration, BoolToFloat) {
    EXPECT_FLOAT_EQ(1.0, COMPILE_AND_RUN_FLOAT_MAIN_LINES("return float32(true);"));
}

TEST(PrimitiveTypecastsIntegration, BoolToChar) {
    EXPECT_EQ(1, COMPILE_AND_RUN_INT8_MAIN_LINES("return int8(true);"));
}

TEST(PrimitiveTypecastsIntegration, CharToChar) {
    EXPECT_EQ('a', COMPILE_AND_RUN_INT8_MAIN_LINES("return int8('a');"));
}

TEST(PrimitiveTypecastsIntegration, CharToInt) {
    EXPECT_EQ('a', COMPILE_AND_RUN_INT_MAIN_LINES("return int32('a');"));
}

TEST(PrimitiveTypecastsIntegration, CharToFloat) {
    EXPECT_FLOAT_EQ('a', COMPILE_AND_RUN_FLOAT_MAIN_LINES("return float32('a');"));
}

TEST(PrimitiveTypecastsIntegration, CharToBool) {
    EXPECT_TRUE(COMPILE_AND_RUN_BOOL_MAIN_LINES("return bool('a');"));
}