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

TEST(SpecTypesIntegration, ExternalPropertyDefaultValues) {
    EXPECT_EQ(0, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() {"
        "    pt <- new Point();"
        "    return pt.X + pt.Y;"
        "}"
        "public spec Point{"
        "    int32 X{ public get; public set; }"
        "    int32 Y{ public get; public set; }"
        "    public construct() { }"
        "}"));
}

TEST(SpecTypesIntegration, ExternalPropertyGetSet) {
    EXPECT_EQ(-3, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() {"
        "    pt <- new Point();"
        "    pt.X <- 34;"
        "    pt.Y <- 31;"
        "    return pt.Y - pt.X;"
        "}"
        "public spec Point{"
        "    int32 X{ public get; public set; }"
        "    int32 Y{ public get; public set; }"
        "    public construct() { }"
        "}"));
}

TEST(SpecTypesIntegration, ConstructorCallArgsAndSetters) {
    EXPECT_EQ(6, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() {"
        "    pt <- new Point(66, 72);"
        "    return pt.Y - pt.X;"
        "}"
        "public spec Point{"
        "    int32 X{ public get; public set; }"
        "    int32 Y{ public get; public set; }"
        "    public construct(int32 x, int32 y) {"
        "        this.X <- x; this.Y <- y;"
        "    }"
        "}"));
}

TEST(SpecTypesIntegration, ConstructorCallArgsGettersAndSetters) {
    EXPECT_EQ(28, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() {"
        "    pt <- new Point(66, 72);"
        "    return pt.Y - pt.X;"
        "}"
        "public spec Point{"
        "    int32 X{ public get; public set; }"
        "    int32 Y{ public get; public set; }"
        "    public construct(int32 x, int32 y) {"
        "        this.X <- 22; this.Y <- 44;"
        "        this.X <- (this.X + x); this.Y <- (this.Y + y);"
        "    }"
        "}"));
}

TEST(SpecTypesIntegration, ConstructorCallMemberFunctionGettersAndSetters) {
    EXPECT_EQ(28, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() {"
        "    pt <- new Point(66, 72);"
        "    return pt.Y - pt.X;"
        "}"
        "public spec Point{"
        "    int32 X{ public get; public set; }"
        "    int32 Y{ public get; public set; }"
        "    public construct(int32 x, int32 y) {"
        "        this.Init(x, y);"
        "    }"
        "    public void Init(int32 x, int32 y) {"
        "        this.X <- 22; this.Y <- 44;"
        "        this.X <- (this.X + x); this.Y <- (this.Y + y);"
        "    }"
        "}"));
}

TEST(SpecTypesIntegration, NodesListFirst) {
    EXPECT_EQ(1, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() {"
        "    list <-new Node(1, new Node(2, new Node(3, default(Node))));"
        "    return list.Item;"
        "}"
        "public spec Node{"
        "    Node Next{ public get; public set; }"
        "    int32 Item{ public get; public set; }"
        "    public construct(int32 item, Node next) {"
        "        this.Next <-next;"
        "        this.Item <-item;"
        "    }"
        "}"));
}

TEST(SpecTypesIntegration, NodesListSecond) {
    EXPECT_EQ(2, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() {"
        "    list <-new Node(1, new Node(2, new Node(3, default(Node))));"
        "    if (true) { list <- list.Next; }"
        "    return list.Item;"
        "}"
        "public spec Node{"
        "    Node Next{ public get; public set; }"
        "    int32 Item{ public get; public set; }"
        "    public construct(int32 item, Node next) {"
        "        this.Next <-next;"
        "        this.Item <-item;"
        "    }"
        "}"));
}

TEST(SpecTypesIntegration, NodesListThirdChained) {
    EXPECT_EQ(3, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() {"
        "    list <-new Node(1, new Node(2, new Node(3, default(Node))));"
        "    return list.Next.Next.Item;"
        "}"
        "public spec Node{"
        "    Node Next{ public get; public set; }"
        "    int32 Item{ public get; public set; }"
        "    public construct(int32 item, Node next) {"
        "        this.Next <-next;"
        "        this.Item <-item;"
        "    }"
        "}"));
}

TEST(SpecTypesIntegration, NodesListFourthChainedIsNull) {
    EXPECT_EQ(321, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() {"
        "    list <-new Node(1, new Node(2, new Node(3, default(Node))));"
        "    if (list.Next.Next.Next = default(Node)) { return 321; }"
        "}"
        "public spec Node{"
        "    Node Next{ public get; public set; }"
        "    int32 Item{ public get; public set; }"
        "    public construct(int32 item, Node next) {"
        "        this.Next <-next;"
        "        this.Item <-item;"
        "    }"
        "}"));
}

TEST(SpecTypesIntegration, NodesListWhileLoopLast) {
    EXPECT_EQ(3, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() {"
        "    list <-new Node(1, new Node(2, new Node(3, default(Node))));"
        "    while (list.Next != default(Node)) { list <- list.Next; }"
        "    return list.Item;"
        "}"
        "public spec Node{"
        "    Node Next{ public get; public set; }"
        "    int32 Item{ public get; public set; }"
        "    public construct(int32 item, Node next) {"
        "        this.Next <-next;"
        "        this.Item <-item;"
        "    }"
        "}"));
}

TEST(SpecTypesIntegration, NodesListSum) {
    EXPECT_EQ(3, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() {"
        "    list <-new Node(1, new Node(2, new Node(3, default(Node))));"
        "    sum <- 0;"
        "    for (i <-list; i != default(Node); i <-i.Next) {"
        "        sum <-i.Item;"
        "    }"
        "    return sum;"
        "}"
        "public spec Node{"
        "    Node Next{ public get; public set; }"
        "    int32 Item{ public get; public set; }"
        "    public construct(int32 item, Node next) {"
        "        this.Next <-next;"
        "        this.Item <-item;"
        "    }"
        "}"));
}

TEST(SpecTypesIntegration, EncapsulatedListSum) {
    EXPECT_EQ(1338, COMPILE_AND_RUN_INT_MAIN_CLASS(
        "public int32 main() { "
        "    list <-new LinkedList();"
        "    list.Prepend(90);"
        "    list.Prepend(456);"
        "    list.Prepend(789);"
        "    return list.Sum() + list.Count;"
        "}"
        "public spec LinkedList{"
        "    int32 Count{ public get; concealed set; }"
        "    Node Head{ public get; concealed set; }"
        "    public construct() { }"
        "    public void Prepend(int32 value) {"
        "        this.Head <-new Node(value, this.Head);"
        "        this.Count <-(this.Count + 1);"
        "    }"
        "    public int32 Sum() {"
        "        sum <-0;"
        "        for (x <-this.Head; x != default(Node); x <-x.Next) {"
        "            sum <-(sum + x.Value);"
        "        }"
        "        return sum;"
        "    }"
        "}"
        "concealed spec Node{"
        "    Node Next{ public get; public set; }"
        "    int32 Value{ public get; public set; }"
        "    public construct(int32 value, Node next) {"
        "        this.Next <-next;"
        "        this.Value <-value;"
        "    }"
        "}"));
}