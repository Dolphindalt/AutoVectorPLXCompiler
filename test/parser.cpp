/**
 *  CPSC 323 Compilers and Languages
 * 
 *  Dalton Caron, Teaching Associate
 *  dcaron@fullerton.edu, +1 949-616-2699
 *  Department of Computer Science
 */
#include <catch2/catch.hpp>

#include <parser.h>

/**
 * This test case checks if the following parse tree is created after parsing 
 * the ../test/test_code/test1.p0 file. Use the PTNode::printTree function to 
 * manully debug this test case. PTNode::printTree will produce the following 
 * tree if the implementation is correct.
 * 
 *   program
 *   +--- block
 *   |    +--- var_declaration
 *   |    |    +--- var
 *   |    |    +--- x
 *   |    |    +--- var_declaration_list
 *   |    |    |    +--- ,
 *   |    |    |    +--- squ
 *   |    |    +--- ;
 *   |    +--- procedure
 *   |    |    +--- procedure
 *   |    |    +--- square
 *   |    |    +--- ;
 *   |    |    +--- block
 *   |    |    |    +--- statement
 *   |    |    |        +--- begin
 *   |    |    |        +--- statement
 *   |    |    |        |    +--- squ
 *   |    |    |        |    +--- :=
 *   |    |    |        |    +--- expression
 *   |    |    |        |        +--- term
 *   |    |    |        |            +--- factor
 *   |    |    |        |            |    +--- x
 *   |    |    |        |            +--- *
 *   |    |    |        |            +--- factor
 *   |    |    |        |                +--- x
 *   |    |    |        +--- end
 *   |    |    +--- ;
 *   |    +--- statement
 *   |        +--- begin
 *   |        +--- statement
 *   |        |    +--- x
 *   |        |    +--- :=
 *   |        |    +--- expression
 *   |        |        +--- term
 *   |        |            +--- factor
 *   |        |                +--- 1
 *   |        +--- ;
 *   |        +--- statement
 *   |        |    +--- while
 *   |        |    +--- condition
 *   |        |    |    +--- expression
 *   |        |    |    |    +--- term
 *   |        |    |    |        +--- factor
 *   |        |    |    |            +--- x
 *   |        |    |    +--- <=
 *   |        |    |    +--- expression
 *   |        |    |        +--- term
 *   |        |    |            +--- factor
 *   |        |    |                +--- 10
 *   |        |    +--- do
 *   |        |    +--- statement
 *   |        |        +--- begin
 *   |        |        +--- statement
 *   |        |        |    +--- call
 *   |        |        |    +--- square
 *   |        |        +--- ;
 *   |        |        +--- statement
 *   |        |        |    +--- !
 *   |        |        |    +--- expression
 *   |        |        |        +--- term
 *   |        |        |            +--- factor
 *   |        |        |                +--- squ
 *   |        |        +--- ;
 *   |        |        +--- statement
 *   |        |        |    +--- x
 *   |        |        |    +--- :=
 *   |        |        |    +--- expression
 *   |        |        |        +--- term
 *   |        |        |        |    +--- factor
 *   |        |        |        |        +--- x
 *   |        |        |        +--- +
 *   |        |        |        +--- term
 *   |        |        |            +--- factor
 *   |        |        |                +--- 1
 *   |        |        +--- end
 *   |        +--- end
 *   +--- .
 */
TEST_CASE("Smoke Test 1", "[Parser]") {

    const char *source_file = "../test/test_code/test1.p0";
    Lexer lexer;
    const token_stream_t tokens = lexer.lex(source_file);

    Parser parser(tokens);

    ParseTree<std::string> parse_tree = parser.parse();

    std::vector<std::string> expected = {
        "program", ".", "block", "statement", "end", "statement", "statement",
        "end", "statement", "expression", "term", "factor", "1", "+", "term",
        "factor", "x", ":=", "x", ";", "statement", "expression", "term",
        "factor", "squ", "!", ";", "statement", "square", "call", "begin",
        "do", "condition", "expression", "term", "factor", "10", "<=",
        "expression", "term", "factor", "x", "while", ";", "statement",
        "expression", "term", "factor", "1", ":=", "x", "begin", "procedure",
        ";", "block", "statement", "end", "statement", "expression", "term",
        "factor", "x", "*", "factor", "x", ":=", "squ", "begin", ";", "square",
        "procedure", "var_declaration", ";", "var_declaration_list", "squ", ",",
        "x", "var"
    };
    size_t index = 0;

    PTNode<std::string>::dfsTraversal(
        parse_tree, 
        [&index, expected](std::string value) {
            if (index < expected.size()) {
                REQUIRE(expected[index] == value);
            }
            index++;
        }
    );

    
}

/**
 * This test runs parsing on ../test/test_code/test2.p0. The test case has no 
 * checks. It is up to you to ensure the proper tree is produced. Here is what 
 * the tree should look like when printed with PTNode::treePrint.
 * 
 *   program
 *   +--- block
 *   |    +--- CONST_DECLARATION
 *   |    |    +--- const
 *   |    |    +--- max
 *   |    |    +--- =
 *   |    |    +--- 100
 *   |    |    +--- ;
 *   |    +--- var_declaration
 *   |    |    +--- var
 *   |    |    +--- arg
 *   |    |    +--- var_declaration_list
 *   |    |    |    +--- ,
 *   |    |    |    +--- ret
 *   |    |    +--- ;
 *   |    +--- procedure
 *   |    |    +--- procedure
 *   |    |    +--- isprime
 *   |    |    +--- ;
 *   |    |    +--- block
 *   |    |    |    +--- var_declaration
 *   |    |    |    |    +--- var
 *   |    |    |    |    +--- i
 *   |    |    |    |    +--- ;
 *   |    |    |    +--- statement
 *   |    |    |        +--- begin
 *   |    |    |        +--- statement
 *   |    |    |        |    +--- ret
 *   |    |    |        |    +--- :=
 *   |    |    |        |    +--- expression
 *   |    |    |        |        +--- term
 *   |    |    |        |            +--- factor
 *   |    |    |        |                +--- 1
 *   |    |    |        +--- ;
 *   |    |    |        +--- statement
 *   |    |    |        |    +--- i
 *   |    |    |        |    +--- :=
 *   |    |    |        |    +--- expression
 *   |    |    |        |        +--- term
 *   |    |    |        |            +--- factor
 *   |    |    |        |                +--- 2
 *   |    |    |        +--- ;
 *   |    |    |        +--- statement
 *   |    |    |        |    +--- while
 *   |    |    |        |    +--- condition
 *   |    |    |        |    |    +--- expression
 *   |    |    |        |    |    |    +--- term
 *   |    |    |        |    |    |        +--- factor
 *   |    |    |        |    |    |            +--- i
 *   |    |    |        |    |    +--- <
 *   |    |    |        |    |    +--- expression
 *   |    |    |        |    |        +--- term
 *   |    |    |        |    |            +--- factor
 *   |    |    |        |    |                +--- arg
 *   |    |    |        |    +--- do
 *   |    |    |        |    +--- statement
 *   |    |    |        |        +--- begin
 *   |    |    |        |        +--- statement
 *   |    |    |        |        |    +--- if
 *   |    |    |        |        |    +--- condition
 *   |    |    |        |        |    |    +--- expression
 *   |    |    |        |        |    |    |    +--- term
 *   |    |    |        |        |    |    |        +--- factor
 *   |    |    |        |        |    |    |        |    +--- arg
 *   |    |    |        |        |    |    |        +--- /
 *   |    |    |        |        |    |    |        +--- factor
 *   |    |    |        |        |    |    |        |    +--- i
 *   |    |    |        |        |    |    |        +--- *
 *   |    |    |        |        |    |    |        +--- factor
 *   |    |    |        |        |    |    |            +--- i
 *   |    |    |        |        |    |    +--- =
 *   |    |    |        |        |    |    +--- expression
 *   |    |    |        |        |    |        +--- term
 *   |    |    |        |        |    |            +--- factor
 *   |    |    |        |        |    |                +--- arg
 *   |    |    |        |        |    +--- then
 *   |    |    |        |        |    +--- statement
 *   |    |    |        |        |        +--- begin
 *   |    |    |        |        |        +--- statement
 *   |    |    |        |        |        |    +--- ret
 *   |    |    |        |        |        |    +--- :=
 *   |    |    |        |        |        |    +--- expression
 *   |    |    |        |        |        |        +--- term
 *   |    |    |        |        |        |            +--- factor
 *   |    |    |        |        |        |                +--- 0
 *   |    |    |        |        |        +--- ;
 *   |    |    |        |        |        +--- statement
 *   |    |    |        |        |        |    +--- i
 *   |    |    |        |        |        |    +--- :=
 *   |    |    |        |        |        |    +--- expression
 *   |    |    |        |        |        |        +--- term
 *   |    |    |        |        |        |            +--- factor
 *   |    |    |        |        |        |                +--- arg
 *   |    |    |        |        |        +--- end
 *   |    |    |        |        +--- ;
 *   |    |    |        |        +--- statement
 *   |    |    |        |        |    +--- i
 *   |    |    |        |        |    +--- :=
 *   |    |    |        |        |    +--- expression
 *   |    |    |        |        |        +--- term
 *   |    |    |        |        |        |    +--- factor
 *   |    |    |        |        |        |        +--- i
 *   |    |    |        |        |        +--- +
 *   |    |    |        |        |        +--- term
 *   |    |    |        |        |            +--- factor
 *   |    |    |        |        |                +--- 1
 *   |    |    |        |        +--- end
 *   |    |    |        +--- end
 *   |    |    +--- ;
 *   |    +--- procedure
 *   |    |    +--- procedure
 *   |    |    +--- primes
 *   |    |    +--- ;
 *   |    |    +--- block
 *   |    |    |    +--- statement
 *   |    |    |        +--- begin
 *   |    |    |        +--- statement
 *   |    |    |        |    +--- arg
 *   |    |    |        |    +--- :=
 *   |    |    |        |    +--- expression
 *   |    |    |        |        +--- term
 *   |    |    |        |            +--- factor
 *   |    |    |        |                +--- 2
 *   |    |    |        +--- ;
 *   |    |    |        +--- statement
 *   |    |    |        |    +--- while
 *   |    |    |        |    +--- condition
 *   |    |    |        |    |    +--- expression
 *   |    |    |        |    |    |    +--- term
 *   |    |    |        |    |    |        +--- factor
 *   |    |    |        |    |    |            +--- arg
 *   |    |    |        |    |    +--- <
 *   |    |    |        |    |    +--- expression
 *   |    |    |        |    |        +--- term
 *   |    |    |        |    |            +--- factor
 *   |    |    |        |    |                +--- max
 *   |    |    |        |    +--- do
 *   |    |    |        |    +--- statement
 *   |    |    |        |        +--- begin
 *   |    |    |        |        +--- statement
 *   |    |    |        |        |    +--- call
 *   |    |    |        |        |    +--- isprime
 *   |    |    |        |        +--- ;
 *   |    |    |        |        +--- statement
 *   |    |    |        |        |    +--- if
 *   |    |    |        |        |    +--- condition
 *   |    |    |        |        |    |    +--- expression
 *   |    |    |        |        |    |    |    +--- term
 *   |    |    |        |        |    |    |        +--- factor
 *   |    |    |        |        |    |    |            +--- ret
 *   |    |    |        |        |    |    +--- =
 *   |    |    |        |        |    |    +--- expression
 *   |    |    |        |        |    |        +--- term
 *   |    |    |        |        |    |            +--- factor
 *   |    |    |        |        |    |                +--- 1
 *   |    |    |        |        |    +--- then
 *   |    |    |        |        |    +--- statement
 *   |    |    |        |        |        +--- !
 *   |    |    |        |        |        +--- expression
 *   |    |    |        |        |            +--- term
 *   |    |    |        |        |                +--- factor
 *   |    |    |        |        |                    +--- arg
 *   |    |    |        |        +--- ;
 *   |    |    |        |        +--- statement
 *   |    |    |        |        |    +--- arg
 *   |    |    |        |        |    +--- :=
 *   |    |    |        |        |    +--- expression
 *   |    |    |        |        |        +--- term
 *   |    |    |        |        |        |    +--- factor
 *   |    |    |        |        |        |        +--- arg
 *   |    |    |        |        |        +--- +
 *   |    |    |        |        |        +--- term
 *   |    |    |        |        |            +--- factor
 *   |    |    |        |        |                +--- 1
 *   |    |    |        |        +--- end
 *   |    |    |        +--- end
 *   |    |    +--- ;
 *   |    +--- statement
 *   |        +--- call
 *   |        +--- primes
 *   +--- .
 */
TEST_CASE("Smoke Test 2", "[Parser]") {

    const char *source_file = "../test/test_code/test2.p0";
    Lexer lexer;
    const token_stream_t tokens = lexer.lex(source_file);

    Parser parser(tokens);

    parser.parse();
}