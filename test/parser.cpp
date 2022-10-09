/**
 *  CPSC 323 Compilers and Languages
 * 
 *  Dalton Caron, Teaching Associate
 *  dcaron@fullerton.edu, +1 949-616-2699
 *  Department of Computer Science
 */
#include <catch2/catch.hpp>

#include <parser.h>

TEST_CASE("Smoke Test 1", "[Parser]") {

    const char *source_file = "../test/test_code/test1.p0";
    Lexer lexer;
    const token_stream_t tokens = lexer.lex(source_file);

    Parser parser(tokens);

    parser.parse();
}

TEST_CASE("Smoke Test 2", "[Parser]") {

    const char *source_file = "../test/test_code/test2.p0";
    Lexer lexer;
    const token_stream_t tokens = lexer.lex(source_file);

    Parser parser(tokens);

    parser.parse();
}