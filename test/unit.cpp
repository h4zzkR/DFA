#include "automata.h"
#include <gtest/gtest.h>

TEST(SimpleTests, ReadTransformOperations) {
    // SimpleTest1
    Automata<char> a('#');
    a.add_transition(a.begin(), '#', a.begin(), true);
    std::string expr = "#";
    EXPECT_TRUE(a.read_expression(expr.begin(), expr.end()));

    // SimpleTest2
    a.add_transition(a.begin(), 'a', a.begin());
    a.add_transition(a.begin(), 'b', a.begin());
    a.add_transition(a.begin(), 'c', a.begin());
    expr = "abcabcabcbcbcbcaaaabcbcbcbcbbbc";
    EXPECT_TRUE(a.read_expression(expr.begin(), expr.end()));

    // SimpleTest3
    Automata<char> b('#');
    b.add_transition(b.begin(), 'a', 1);
    b.add_transition(1, 'b', b.begin());
    b.add_transition(b.begin(), 'c', 2, true);
    b.thompson_nfa2dfa();
    expr = "ababab";
    EXPECT_FALSE(b.read_expression(expr.begin(), expr.end()));
}

TEST(NotSoSimpleTests, ReadTransformOperations) {
    // NotSoSimpleTest1
    Automata<char> a('#');
    a.add_transition(a.begin(), 'a', a.begin());
    a.add_transition(a.begin(), 'b', a.begin());
    a.add_transition(a.begin(), 'c', a.begin());

    a.add_transition(a.begin(), 'a', 1);
    std::string expr = "bbbcccbbba";
    a.thompson_nfa2dfa();
    EXPECT_FALSE(a.read_expression(expr.begin(), expr.end()));

    // NotSoSimpleTest1.5
    a.toggle_terminal(1);
    EXPECT_TRUE(a.read_expression(expr.begin(), expr.end()));

    // NotSoSimpleTest2
    Automata<char> b('#');
    b.add_transition(b.begin(), 'a', 1);
    b.add_transition(1, 'b', b.begin());
    b.add_transition(b.begin(), 'c', 2, true);
    b.add_transition(2, 'm', 3, true);
    b.add_transition(b.begin(), '#', b.begin());

    b.thompson_nfa2dfa();
    expr = "abababc";
    EXPECT_TRUE(b.read_expression(expr.begin(), expr.end()));
    expr = "abababababcm";
    EXPECT_TRUE(b.read_expression(expr.begin(), expr.end()));
    expr = "abababcmmmm";
    EXPECT_FALSE(b.read_expression(expr.begin(), expr.end()));
}

TEST(NotSimpleTests, ReadTransformOperations) {
    Automata<char> a('#');
    for (int i = 1; i <= 3; ++i) {
        a.add_transition(0, 'a', i, true);
        a.add_transition(i, 'a' + i - 1, i, true);
    }
    a.thompson_nfa2dfa();
    EXPECT_TRUE(a.read_string("aaaaaaaaaa"));
    EXPECT_TRUE(a.read_string(("abbbbbbbbbb")));
    EXPECT_FALSE(a.read_string("aab"));
    EXPECT_TRUE(a.read_string("accccccc"));
    EXPECT_FALSE(a.read_string(("cccccccc")));

    Automata<char> b('#');
    b.add_transition(b.begin(), '#', 1);
    b.add_transition(b.begin(), '#', 2);
    b.add_transition(b.begin(), '#', 3);
    b.add_transition(1, 'b', 3, true);
    b.add_transition(1, 'b', 4, false);
    b.add_transition(1, 'c', 5, true);
    b.epsilon_edges_elimination();

    EXPECT_FALSE(b.read_string("a"));
    EXPECT_TRUE(b.read_string("b"));
    EXPECT_TRUE(b.read_string("c"));

    b.thompson_nfa2dfa();
    EXPECT_FALSE(b.read_string("a"));
    EXPECT_TRUE(b.read_string("b"));
    EXPECT_TRUE(b.read_string("c"));

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

