#include <iostream>
#include <sstream>
#include <string>
#include <gtest/gtest.h>

#include "Node.hpp"
#include "Parser.hpp"
#include "Error.hpp"
#include "ParseResult.hpp"

template <typename F>
void expectParse(
    const std::string &expr,
    const std::string &expected,
    F getString,
    bool isnull = false)
{
    ParseResult p(expr);

    if (isnull)
    {
        ASSERT_EQ(p.tree, nullptr) << "Error: expected null tree: " << expr;
        return;
    }

    ASSERT_NE(p.tree, nullptr) << "Error: unexpected parse error: " << expr;

    std::string got = getString(*p.tree);
    EXPECT_EQ(expected, got);
}

void expectParseUnfold(const std::string &expr, const std::string &expected, bool isnull = false)
{
    expectParse(expr, expected, [](const Node &n)
                { return n.unfold(); }, isnull);
}

void expectParseUnfoldWithPriority(const std::string &expr, const std::string &expected, bool isnull = false)
{
    expectParse(expr, expected, [](const Node &n)
                { return n.unfoldWithPriority(); }, isnull);
}

TEST(ParserTest, BaseLineTests)
{
    expectParseUnfold("", "");
    expectParseUnfold("a", "a");

    expectParseUnfold("not a", "not a");
    expectParseUnfold("a and b", "a and b");
    expectParseUnfold("a or b", "a or b");
    expectParseUnfold("a xor b", "a xor b");

    expectParseUnfold("(a and b) or not (c xor (a or not b))",
                      "(a and b) or not (c xor (a or not b))");
    expectParseUnfold("(a) or (b)", "(a) or (b)");
}

TEST(ParserTest, MixedTests)
{
    expectParseUnfold("a and b or c", "a and b or c");
    expectParseUnfold("a and (b or c)", "a and (b or c)");
    expectParseUnfold("a and b xor c", "a and b xor c");
    expectParseUnfold("a and (b xor c)", "a and (b xor c)");

    expectParseUnfold("not a and b or c", "not a and b or c");
    expectParseUnfold("not a and (b or c)", "not a and (b or c)");
    expectParseUnfold("not a and b xor c", "not a and b xor c");
    expectParseUnfold("not a and (b xor c)", "not a and (b xor c)");

    expectParseUnfold("a and not b or c", "a and not b or c");
    expectParseUnfold("a and (not b or c)", "a and (not b or c)");
    expectParseUnfold("a and not b xor c", "a and not b xor c");
    expectParseUnfold("a and (not b xor c)", "a and (not b xor c)");

    expectParseUnfold("a and b or not c", "a and b or not c");
    expectParseUnfold("a and (b or not c)", "a and (b or not c)");
    expectParseUnfold("a and b xor not c", "a and b xor not c");
    expectParseUnfold("a and (b xor not c)", "a and (b xor not c)");

    //--

    expectParseUnfold("a or b and c", "a or b and c");
    expectParseUnfold("(a or b) and c", "(a or b) and c");
    expectParseUnfold("a or b xor c", "a or b xor c");
    expectParseUnfold("a or (b xor c)", "a or (b xor c)");

    expectParseUnfold("not a or b and c", "not a or b and c");
    expectParseUnfold("not a or (b and c)", "not a or (b and c)");
    expectParseUnfold("not a or b xor c", "not a or b xor c");
    expectParseUnfold("not a or (b xor c)", "not a or (b xor c)");

    expectParseUnfold("a or not b and c", "a or not b and c");
    expectParseUnfold("a or (not b and c)", "a or (not b and c)");
    expectParseUnfold("a or not b xor c", "a or not b xor c");
    expectParseUnfold("a or (not b xor c)", "a or (not b xor c)");

    expectParseUnfold("a or b and not c", "a or b and not c");
    expectParseUnfold("a or (b and not c)", "a or (b and not c)");
    expectParseUnfold("a or b xor not c", "a or b xor not c");
    expectParseUnfold("a or (b xor not c)", "a or (b xor not c)");

    //--
    expectParseUnfold("a xor b and c", "a xor b and c");
    expectParseUnfold("a xor (b and c)", "a xor (b and c)");
    expectParseUnfold("a xor b or c", "a xor b or c");
    expectParseUnfold("a xor (b or c)", "a xor (b or c)");

    expectParseUnfold("not a xor b and c", "not a xor b and c");
    expectParseUnfold("not a xor (b and c)", "not a xor (b and c)");
    expectParseUnfold("not a xor b or c", "not a xor b or c");
    expectParseUnfold("not a xor (b or c)", "not a xor (b or c)");

    expectParseUnfold("a xor not b and c", "a xor not b and c");
    expectParseUnfold("a xor (not b and c)", "a xor (not b and c)");
    expectParseUnfold("a xor not b or c", "a xor not b or c");
    expectParseUnfold("a xor (not b or c)", "a xor (not b or c)");

    expectParseUnfold("a xor b and not c", "a xor b and not c");
    expectParseUnfold("a xor (b and not c)", "a xor (b and not c)");
    expectParseUnfold("a xor b or not c", "a xor b or not c");
    expectParseUnfold("a xor (b or not c)", "a xor (b or not c)");
}

TEST(ParserTest, BaselinePriorityTests)
{
    expectParseUnfoldWithPriority("(a)", "a");

    expectParseUnfoldWithPriority("not a", "(not a)");
    expectParseUnfoldWithPriority("a and b", "(a and b)");
    expectParseUnfoldWithPriority("a or b", "(a or b)");
    expectParseUnfoldWithPriority("a xor b", "(a xor b)");

    expectParseUnfoldWithPriority("not (a)", "(not a)");
    expectParseUnfoldWithPriority("(a) and (b)", "(a and b)");
    expectParseUnfoldWithPriority("(a) or (b)", "(a or b)");
    expectParseUnfoldWithPriority("(a) xor (b)", "(a xor b)");

    expectParseUnfoldWithPriority("not not not not a", "(not (not (not (not a))))");

    expectParseUnfoldWithPriority("(a and b) or not (c xor (a or not b))",
                                  "((a and b) or (not (c xor (a or (not b)))))");
}

TEST(ParserTest, MixedPriorityTests)
{
    expectParseUnfoldWithPriority("a and b or c", "((a and b) or c)");
    expectParseUnfoldWithPriority("a and (b or c)", "(a and (b or c))");
    expectParseUnfoldWithPriority("a and b xor c", "((a and b) xor c)");
    expectParseUnfoldWithPriority("a and (b xor c)", "(a and (b xor c))");

    expectParseUnfoldWithPriority("not a and b or c", "(((not a) and b) or c)");
    expectParseUnfoldWithPriority("not a and (b or c)", "((not a) and (b or c))");
    expectParseUnfoldWithPriority("not a and b xor c", "(((not a) and b) xor c)");
    expectParseUnfoldWithPriority("not a and (b xor c)", "((not a) and (b xor c))");

    expectParseUnfoldWithPriority("a and not b or c", "((a and (not b)) or c)");
    expectParseUnfoldWithPriority("a and (not b or c)", "(a and ((not b) or c))");
    expectParseUnfoldWithPriority("a and not b xor c", "((a and (not b)) xor c)");
    expectParseUnfoldWithPriority("a and (not b xor c)", "(a and ((not b) xor c))");

    expectParseUnfoldWithPriority("a and b or not c", "((a and b) or (not c))");
    expectParseUnfoldWithPriority("a and (b or not c)", "(a and (b or (not c)))");
    expectParseUnfoldWithPriority("a and b xor not c", "((a and b) xor (not c))");
    expectParseUnfoldWithPriority("a and (b xor not c)", "(a and (b xor (not c)))");

    //--

    expectParseUnfoldWithPriority("a or b and c", "(a or (b and c))");
    expectParseUnfoldWithPriority("(a or b) and c", "((a or b) and c)");
    expectParseUnfoldWithPriority("a or b xor c", "(a or (b xor c))");
    expectParseUnfoldWithPriority("(a or b) xor c", "((a or b) xor c)");

    expectParseUnfoldWithPriority("not a or b and c", "((not a) or (b and c))");
    expectParseUnfoldWithPriority("(not a or b) and c", "(((not a) or b) and c)");
    expectParseUnfoldWithPriority("not a or b xor c", "((not a) or (b xor c))");
    expectParseUnfoldWithPriority("(not a or b) xor c", "(((not a) or b) xor c)");

    expectParseUnfoldWithPriority("a or not b and c", "(a or ((not b) and c))");
    expectParseUnfoldWithPriority("(a or not b) and c", "((a or (not b)) and c)");
    expectParseUnfoldWithPriority("a or not b xor c", "(a or ((not b) xor c))");
    expectParseUnfoldWithPriority("(a or not b) xor c", "((a or (not b)) xor c)");

    expectParseUnfoldWithPriority("a or b and not c", "(a or (b and (not c)))");
    expectParseUnfoldWithPriority("(a or b) and not c", "((a or b) and (not c))");
    expectParseUnfoldWithPriority("a or b xor not c", "(a or (b xor (not c)))");
    expectParseUnfoldWithPriority("(a or b) xor not c", "((a or b) xor (not c))");

    //--

    expectParseUnfoldWithPriority("a xor b and c", "(a xor (b and c))");
    expectParseUnfoldWithPriority("(a xor b) and c", "((a xor b) and c)");
    expectParseUnfoldWithPriority("a xor b or c", "((a xor b) or c)");
    expectParseUnfoldWithPriority("a xor (b or c)", "(a xor (b or c))");

    expectParseUnfoldWithPriority("not a xor b and c", "((not a) xor (b and c))");
    expectParseUnfoldWithPriority("(not a xor b) and c", "(((not a) xor b) and c)");
    expectParseUnfoldWithPriority("not a xor b or c", "(((not a) xor b) or c)");
    expectParseUnfoldWithPriority("not a xor (b or c)", "((not a) xor (b or c))");

    expectParseUnfoldWithPriority("a xor not b and c", "(a xor ((not b) and c))");
    expectParseUnfoldWithPriority("(a xor not b) and c", "((a xor (not b)) and c)");
    expectParseUnfoldWithPriority("a xor not b or c", "((a xor (not b)) or c)");
    expectParseUnfoldWithPriority("a xor (not b or c)", "(a xor ((not b) or c))");

    expectParseUnfoldWithPriority("a xor b and not c", "(a xor (b and (not c)))");
    expectParseUnfoldWithPriority("a xor (b and not c)", "(a xor (b and (not c)))");
    expectParseUnfoldWithPriority("a xor b or not c", "((a xor b) or (not c))");
    expectParseUnfoldWithPriority("a xor (b or not c)", "(a xor (b or (not c)))");

    //--

    expectParseUnfoldWithPriority("not (a and b) xor c", "((not (a and b)) xor c)");
    expectParseUnfoldWithPriority("a and not (b xor c)", "(a and (not (b xor c)))");
    expectParseUnfoldWithPriority("(a and b) xor not c", "((a and b) xor (not c))");
}

TEST(ParserTest, EvilTests)
{
    expectParseUnfold("(", "", true);
    expectParseUnfold("(a and b))", "", true);
    expectParseUnfold("(a", "", true);
    expectParseUnfold("and or not xor", "", true);
    expectParseUnfold("not not", "", true);
    expectParseUnfold("()", "", true);
    expectParseUnfold("aaaaaaaa", "", true);
    expectParseUnfold("not (a and b) xor c000000", "", true);
    expectParseUnfold("-04034,mndjlgkdfdlnfl˙∆∆ß˚∂∆˚ß˙ß¥ƒ¨¨ç∫∆µ≈˜≤çµ≤¬≥…∂πˆ¨ˆ∂¨¥", "", true);
    expectParseUnfold("$", "", true);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(fail_fast) = true;
    return RUN_ALL_TESTS();
}