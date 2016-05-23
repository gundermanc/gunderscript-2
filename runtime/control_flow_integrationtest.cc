// Gunderscript 2 Control Flow Integration Tests
// (C) 2016 Christian Gunderman

#include "gtest/gtest.h"

#include "testing_macros.h"

#include "gunderscript/compiler.h"
#include "gunderscript/virtual_machine.h"

// All of these implicitly test return int.

TEST(ControlFlowIntegration, If) {
    EXPECT_EQ(10, COMPILE_AND_RUN_INT_MAIN_LINES("if (true) { return 10; }"));
    EXPECT_EQ(0, COMPILE_AND_RUN_INT_MAIN_LINES("if (false) { return 10; }"));
}

TEST(ControlFlowIntegration, IfElse) {
    EXPECT_EQ(10, COMPILE_AND_RUN_INT_MAIN_LINES("if (true) { return 10; } else { return 9; }"));
    EXPECT_EQ(9, COMPILE_AND_RUN_INT_MAIN_LINES("if (false) { return 10; } else { return 9; }"));
}

TEST(ControlFlowIntegration, NestedIfElse) {
    EXPECT_EQ(10, COMPILE_AND_RUN_INT_MAIN_LINES(
        "if (true) {"
        "    if (true) {"
        "        return 10;"
        "    } else {"
        "        return 9;"
        "    }"
        "} else {"
        "    if (true) {"
        "        return 8;"
        "    } else {"
        "        return 7;"
        "    }"
        "}"));
    EXPECT_EQ(9, COMPILE_AND_RUN_INT_MAIN_LINES(
        "if (true) {"
        "    if (false) {"
        "        return 10;"
        "    } else {"
        "        return 9;"
        "    }"
        "} else {"
        "    if (true) {"
        "        return 8;"
        "    } else {"
        "        return 7;"
        "    }"
        "}"));
    EXPECT_EQ(8, COMPILE_AND_RUN_INT_MAIN_LINES(
        "if (false) {"
        "    if (true) {"
        "        return 10;"
        "    } else {"
        "        return 9;"
        "    }"
        "} else {"
        "    if (true) {"
        "        return 8;"
        "    } else {"
        "        return 7;"
        "    }"
        "}"));
    EXPECT_EQ(7, COMPILE_AND_RUN_INT_MAIN_LINES(
        "if (false) {"
        "    if (true) {"
        "        return 10;"
        "    } else {"
        "        return 9;"
        "    }"
        "} else {"
        "    if (false) {"
        "        return 8;"
        "    } else {"
        "        return 7;"
        "    }"
        "}"));
}

// Iterative factorial.
TEST(ControlFlowIntegration, For) {
    EXPECT_EQ(120, COMPILE_AND_RUN_INT_MAIN_LINES(
        "number <- 5;"
        "result <-1;"
        "for (i <- number; i > 0; i <- i - 1) {"
        "    result <-result * i;"
        "}"
        "return result;"));
}

TEST(ControlFlowIntegration, ForNoCondition) {
    EXPECT_EQ(120, COMPILE_AND_RUN_INT_MAIN_LINES(
        "number <- 5;"
        "result <-1;"
        "for (i <- number;; i <- i - 1) {"
        "    if (i > 0) {"
        "        result <-result * i;"
        "    } else {"
        "        return result;"
        "    }"
        "}"
        "return result;"));
}

TEST(ControlFlowIntegration, ForNoInitialization) {
    EXPECT_EQ(120, COMPILE_AND_RUN_INT_MAIN_LINES(
        "i <- 5;"
        "result <-1;"
        "for (; i > 0; i <- i - 1) {"
        "    result <-result * i;"
        "}"
        "return result;"));
}

TEST(ControlFlowIntegration, ForNoUpdate) {
    EXPECT_EQ(120, COMPILE_AND_RUN_INT_MAIN_LINES(
        "number <- 5;"
        "result <-1;"
        "for (i <- number; i > 0;) {"
        "    result <-result * i;"
        "    i <-i - 1;"
        "}"
        "return result;"));
}

TEST(ControlFlowIntegration, ForNoParams) {
    EXPECT_EQ(120, COMPILE_AND_RUN_INT_MAIN_LINES(
        "i <- 5;"
        "result <-1;"
        "for (;;) {"
        "    if (i > 0) {"
        "        result <-result * i;"
        "    } else {"
        "        return result;"
        "    }"
        "    i <- i - 1;"
        "}"
        "return result;"));
}

// Newton's method for square root.
TEST(ControlFlowIntegration, ForSquareroot) {
    EXPECT_FLOAT_EQ(2.236067977499978, COMPILE_AND_RUN_FLOAT_MAIN_LINES(
        "iterations <-5;"
        "value <-5.0;"
        "old <-value;"
        "new_val <-value;"
        "for (i <-0; i < iterations; i <-i + 1) {"
        "    new_val <-old - ((old * old) - value) / (2.0 * old);"
        "   old <-new_val;"
        "}"
        "return new_val;"));
}