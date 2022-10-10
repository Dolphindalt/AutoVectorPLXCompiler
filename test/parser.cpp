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
 *   PROGRAM
 *   +--- BLOCK
 *   |    +--- VAR_DECLARATION
 *   |    |    +--- var
 *   |    |    +--- x
 *   |    |    +--- VAR_DECLARATION_LIST
 *   |    |    |    +--- ,
 *   |    |    |    +--- squ
 *   |    |    +--- ;
 *   |    +--- PROCEDURE
 *   |    |    +--- procedure
 *   |    |    +--- square
 *   |    |    +--- ;
 *   |    |    +--- BLOCK
 *   |    |    |    +--- STATEMENT
 *   |    |    |        +--- begin
 *   |    |    |        +--- STATEMENT
 *   |    |    |        |    +--- squ
 *   |    |    |        |    +--- :=
 *   |    |    |        |    +--- EXPRESSION
 *   |    |    |        |        +--- TERM
 *   |    |    |        |            +--- FACTOR
 *   |    |    |        |            |    +--- x
 *   |    |    |        |            +--- *
 *   |    |    |        |            +--- FACTOR
 *   |    |    |        |                +--- x
 *   |    |    |        +--- end
 *   |    |    +--- ;
 *   |    +--- STATEMENT
 *   |        +--- begin
 *   |        +--- STATEMENT
 *   |        |    +--- x
 *   |        |    +--- :=
 *   |        |    +--- EXPRESSION
 *   |        |        +--- TERM
 *   |        |            +--- FACTOR
 *   |        |                +--- 1
 *   |        +--- ;
 *   |        +--- STATEMENT
 *   |        |    +--- while
 *   |        |    +--- CONDITION
 *   |        |    |    +--- EXPRESSION
 *   |        |    |    |    +--- TERM
 *   |        |    |    |        +--- FACTOR
 *   |        |    |    |            +--- x
 *   |        |    |    +--- <=
 *   |        |    |    +--- EXPRESSION
 *   |        |    |        +--- TERM
 *   |        |    |            +--- FACTOR
 *   |        |    |                +--- 10
 *   |        |    +--- do
 *   |        |    +--- STATEMENT
 *   |        |        +--- begin
 *   |        |        +--- STATEMENT
 *   |        |        |    +--- call
 *   |        |        |    +--- square
 *   |        |        +--- ;
 *   |        |        +--- STATEMENT
 *   |        |        |    +--- !
 *   |        |        |    +--- EXPRESSION
 *   |        |        |        +--- TERM
 *   |        |        |            +--- FACTOR
 *   |        |        |                +--- squ
 *   |        |        +--- ;
 *   |        |        +--- STATEMENT
 *   |        |        |    +--- x
 *   |        |        |    +--- :=
 *   |        |        |    +--- EXPRESSION
 *   |        |        |        +--- TERM
 *   |        |        |        |    +--- FACTOR
 *   |        |        |        |        +--- x
 *   |        |        |        +--- +
 *   |        |        |        +--- TERM
 *   |        |        |            +--- FACTOR
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
        "PROGRAM", ".", "BLOCK", "STATEMENT", "end", "STATEMENT", "STATEMENT",
        "end", "STATEMENT", "EXPRESSION", "TERM", "FACTOR", "1", "+", "TERM",
        "FACTOR", "x", ":=", "x", ";", "STATEMENT", "EXPRESSION", "TERM",
        "FACTOR", "squ", "!", ";", "STATEMENT", "square", "call", "begin",
        "do", "CONDITION", "EXPRESSION", "TERM", "FACTOR", "10", "<=",
        "EXPRESSION", "TERM", "FACTOR", "x", "while", ";", "STATEMENT",
        "EXPRESSION", "TERM", "FACTOR", "1", ":=", "x", "begin", "PROCEDURE",
        ";", "BLOCK", "STATEMENT", "end", "STATEMENT", "EXPRESSION", "TERM",
        "FACTOR", "x", "*", "FACTOR", "x", ":=", "squ", "begin", ";", "square",
        "procedure", "VAR_DECLARATION", ";", "VAR_DECLARATION_LIST", "squ", ",",
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
 *   PROGRAM
 *   +--- BLOCK
 *   |    +--- CONST_DECLARATION
 *   |    |    +--- const
 *   |    |    +--- max
 *   |    |    +--- =
 *   |    |    +--- 100
 *   |    |    +--- ;
 *   |    +--- VAR_DECLARATION
 *   |    |    +--- var
 *   |    |    +--- arg
 *   |    |    +--- VAR_DECLARATION_LIST
 *   |    |    |    +--- ,
 *   |    |    |    +--- ret
 *   |    |    +--- ;
 *   |    +--- PROCEDURE
 *   |    |    +--- procedure
 *   |    |    +--- isprime
 *   |    |    +--- ;
 *   |    |    +--- BLOCK
 *   |    |    |    +--- VAR_DECLARATION
 *   |    |    |    |    +--- var
 *   |    |    |    |    +--- i
 *   |    |    |    |    +--- ;
 *   |    |    |    +--- STATEMENT
 *   |    |    |        +--- begin
 *   |    |    |        +--- STATEMENT
 *   |    |    |        |    +--- ret
 *   |    |    |        |    +--- :=
 *   |    |    |        |    +--- EXPRESSION
 *   |    |    |        |        +--- TERM
 *   |    |    |        |            +--- FACTOR
 *   |    |    |        |                +--- 1
 *   |    |    |        +--- ;
 *   |    |    |        +--- STATEMENT
 *   |    |    |        |    +--- i
 *   |    |    |        |    +--- :=
 *   |    |    |        |    +--- EXPRESSION
 *   |    |    |        |        +--- TERM
 *   |    |    |        |            +--- FACTOR
 *   |    |    |        |                +--- 2
 *   |    |    |        +--- ;
 *   |    |    |        +--- STATEMENT
 *   |    |    |        |    +--- while
 *   |    |    |        |    +--- CONDITION
 *   |    |    |        |    |    +--- EXPRESSION
 *   |    |    |        |    |    |    +--- TERM
 *   |    |    |        |    |    |        +--- FACTOR
 *   |    |    |        |    |    |            +--- i
 *   |    |    |        |    |    +--- <
 *   |    |    |        |    |    +--- EXPRESSION
 *   |    |    |        |    |        +--- TERM
 *   |    |    |        |    |            +--- FACTOR
 *   |    |    |        |    |                +--- arg
 *   |    |    |        |    +--- do
 *   |    |    |        |    +--- STATEMENT
 *   |    |    |        |        +--- begin
 *   |    |    |        |        +--- STATEMENT
 *   |    |    |        |        |    +--- if
 *   |    |    |        |        |    +--- CONDITION
 *   |    |    |        |        |    |    +--- EXPRESSION
 *   |    |    |        |        |    |    |    +--- TERM
 *   |    |    |        |        |    |    |        +--- FACTOR
 *   |    |    |        |        |    |    |        |    +--- arg
 *   |    |    |        |        |    |    |        +--- /
 *   |    |    |        |        |    |    |        +--- FACTOR
 *   |    |    |        |        |    |    |        |    +--- i
 *   |    |    |        |        |    |    |        +--- *
 *   |    |    |        |        |    |    |        +--- FACTOR
 *   |    |    |        |        |    |    |            +--- i
 *   |    |    |        |        |    |    +--- =
 *   |    |    |        |        |    |    +--- EXPRESSION
 *   |    |    |        |        |    |        +--- TERM
 *   |    |    |        |        |    |            +--- FACTOR
 *   |    |    |        |        |    |                +--- arg
 *   |    |    |        |        |    +--- then
 *   |    |    |        |        |    +--- STATEMENT
 *   |    |    |        |        |        +--- begin
 *   |    |    |        |        |        +--- STATEMENT
 *   |    |    |        |        |        |    +--- ret
 *   |    |    |        |        |        |    +--- :=
 *   |    |    |        |        |        |    +--- EXPRESSION
 *   |    |    |        |        |        |        +--- TERM
 *   |    |    |        |        |        |            +--- FACTOR
 *   |    |    |        |        |        |                +--- 0
 *   |    |    |        |        |        +--- ;
 *   |    |    |        |        |        +--- STATEMENT
 *   |    |    |        |        |        |    +--- i
 *   |    |    |        |        |        |    +--- :=
 *   |    |    |        |        |        |    +--- EXPRESSION
 *   |    |    |        |        |        |        +--- TERM
 *   |    |    |        |        |        |            +--- FACTOR
 *   |    |    |        |        |        |                +--- arg
 *   |    |    |        |        |        +--- end
 *   |    |    |        |        +--- ;
 *   |    |    |        |        +--- STATEMENT
 *   |    |    |        |        |    +--- i
 *   |    |    |        |        |    +--- :=
 *   |    |    |        |        |    +--- EXPRESSION
 *   |    |    |        |        |        +--- TERM
 *   |    |    |        |        |        |    +--- FACTOR
 *   |    |    |        |        |        |        +--- i
 *   |    |    |        |        |        +--- +
 *   |    |    |        |        |        +--- TERM
 *   |    |    |        |        |            +--- FACTOR
 *   |    |    |        |        |                +--- 1
 *   |    |    |        |        +--- end
 *   |    |    |        +--- end
 *   |    |    +--- ;
 *   |    +--- PROCEDURE
 *   |    |    +--- procedure
 *   |    |    +--- primes
 *   |    |    +--- ;
 *   |    |    +--- BLOCK
 *   |    |    |    +--- STATEMENT
 *   |    |    |        +--- begin
 *   |    |    |        +--- STATEMENT
 *   |    |    |        |    +--- arg
 *   |    |    |        |    +--- :=
 *   |    |    |        |    +--- EXPRESSION
 *   |    |    |        |        +--- TERM
 *   |    |    |        |            +--- FACTOR
 *   |    |    |        |                +--- 2
 *   |    |    |        +--- ;
 *   |    |    |        +--- STATEMENT
 *   |    |    |        |    +--- while
 *   |    |    |        |    +--- CONDITION
 *   |    |    |        |    |    +--- EXPRESSION
 *   |    |    |        |    |    |    +--- TERM
 *   |    |    |        |    |    |        +--- FACTOR
 *   |    |    |        |    |    |            +--- arg
 *   |    |    |        |    |    +--- <
 *   |    |    |        |    |    +--- EXPRESSION
 *   |    |    |        |    |        +--- TERM
 *   |    |    |        |    |            +--- FACTOR
 *   |    |    |        |    |                +--- max
 *   |    |    |        |    +--- do
 *   |    |    |        |    +--- STATEMENT
 *   |    |    |        |        +--- begin
 *   |    |    |        |        +--- STATEMENT
 *   |    |    |        |        |    +--- call
 *   |    |    |        |        |    +--- isprime
 *   |    |    |        |        +--- ;
 *   |    |    |        |        +--- STATEMENT
 *   |    |    |        |        |    +--- if
 *   |    |    |        |        |    +--- CONDITION
 *   |    |    |        |        |    |    +--- EXPRESSION
 *   |    |    |        |        |    |    |    +--- TERM
 *   |    |    |        |        |    |    |        +--- FACTOR
 *   |    |    |        |        |    |    |            +--- ret
 *   |    |    |        |        |    |    +--- =
 *   |    |    |        |        |    |    +--- EXPRESSION
 *   |    |    |        |        |    |        +--- TERM
 *   |    |    |        |        |    |            +--- FACTOR
 *   |    |    |        |        |    |                +--- 1
 *   |    |    |        |        |    +--- then
 *   |    |    |        |        |    +--- STATEMENT
 *   |    |    |        |        |        +--- !
 *   |    |    |        |        |        +--- EXPRESSION
 *   |    |    |        |        |            +--- TERM
 *   |    |    |        |        |                +--- FACTOR
 *   |    |    |        |        |                    +--- arg
 *   |    |    |        |        +--- ;
 *   |    |    |        |        +--- STATEMENT
 *   |    |    |        |        |    +--- arg
 *   |    |    |        |        |    +--- :=
 *   |    |    |        |        |    +--- EXPRESSION
 *   |    |    |        |        |        +--- TERM
 *   |    |    |        |        |        |    +--- FACTOR
 *   |    |    |        |        |        |        +--- arg
 *   |    |    |        |        |        +--- +
 *   |    |    |        |        |        +--- TERM
 *   |    |    |        |        |            +--- FACTOR
 *   |    |    |        |        |                +--- 1
 *   |    |    |        |        +--- end
 *   |    |    |        +--- end
 *   |    |    +--- ;
 *   |    +--- STATEMENT
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