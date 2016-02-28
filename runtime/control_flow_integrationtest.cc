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