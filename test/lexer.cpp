/**
 *  CPSC 323 Compilers and Languages
 * 
 *  Dalton Caron, Teaching Associate
 *  dcaron@fullerton.edu, +1 949-616-2699
 *  Department of Computer Science
 */
#include <catch2/catch.hpp>

#include <lexer.h>

TEST_CASE("ScanningTable", "[ScanningTable]") {

    const char *scanning_table_path = "../test/scanning_table.csv";
    ScanningTable scanning_table(CsvReader(scanning_table_path, ','));

    REQUIRE(scanning_table.getNextState(0, 'a') == 1);
    REQUIRE(scanning_table.containsNextState(3, '\t') == false);
    REQUIRE(scanning_table.getNextState(2, '\n') == 3);
    REQUIRE(scanning_table.getNextState(1, 'a') == 2);
}

TEST_CASE("TokenTable", "[TokenTable]") {

    const char *token_table_path = "../test/token_table.csv";
    TokenTable token_table(CsvReader(token_table_path, ','));

    REQUIRE(token_table.isStateFinal(2) == true);
    REQUIRE(token_table.isStateFinal(100) == false);
    REQUIRE(token_table.getTokenTypeFromFinalState(2) == (token_class_t) 2);
}

TEST_CASE("Test Case 1", "[Lexer]") {

    const char *file_path = "../test/test_code/test1.p0";
    const int token_count = 44;
    token_class_t expected_types[token_count] = {
        VAR_KEYWORD, INT_TYPE_KEYWORD, IDENTIFIER, COMMA, INT_TYPE_KEYWORD, 
        IDENTIFIER, SEMICOLON, PROCEDURE_KEYWORD, IDENTIFIER, SEMICOLON, 
        BEGIN_KEYWORD, IDENTIFIER, DEFINE_EQUALS, IDENTIFIER, MUL_OP, IDENTIFIER,
        END_KEYWORD, SEMICOLON, BEGIN_KEYWORD, IDENTIFIER, DEFINE_EQUALS,
        INT_NUMBER_LITERAL, SEMICOLON, WHILE_KEYWORD, IDENTIFIER,
        COMPARE_OP, INT_NUMBER_LITERAL, DO_KEYWORD, BEGIN_KEYWORD, CALL_KEYWORD,
        IDENTIFIER, SEMICOLON, WRITE_OP, IDENTIFIER, SEMICOLON, IDENTIFIER,
        DEFINE_EQUALS, IDENTIFIER, ADD_OP, INT_NUMBER_LITERAL, END_KEYWORD,
        END_KEYWORD, PERIOD, END_OF_FILE
    };

    Lexer lexer;
    token_stream_t token_stream = lexer.lex(file_path);
    
    for (int i = 0; i < token_count; i++) {
        REQUIRE(
            tokenTypeToString(token_stream.at(i).type) == 
            tokenTypeToString(expected_types[i])
        );
    }

}

TEST_CASE("Test Case 2", "[Lexer]") {
    
    const char *file_path = "../test/test_code/test2.p0";
    const int token_count = 99;
    token_class_t expected_types[token_count] = {
        CONST_KEYWORD, INT_TYPE_KEYWORD, IDENTIFIER, EQUALS, INT_NUMBER_LITERAL, 
        SEMICOLON,
        VAR_KEYWORD, INT_TYPE_KEYWORD, IDENTIFIER, COMMA, INT_TYPE_KEYWORD, 
        IDENTIFIER, SEMICOLON,

        PROCEDURE_KEYWORD, IDENTIFIER, SEMICOLON,
        VAR_KEYWORD, INT_TYPE_KEYWORD, IDENTIFIER, SEMICOLON,
        BEGIN_KEYWORD,
        IDENTIFIER, DEFINE_EQUALS, INT_NUMBER_LITERAL, SEMICOLON,
        IDENTIFIER, DEFINE_EQUALS, INT_NUMBER_LITERAL, SEMICOLON,
        WHILE_KEYWORD, IDENTIFIER, COMPARE_OP, IDENTIFIER, DO_KEYWORD,
        BEGIN_KEYWORD,
        IF_KEYWORD, IDENTIFIER, MUL_OP, IDENTIFIER, MUL_OP, IDENTIFIER, EQUALS,
        IDENTIFIER, THEN_KEYWORD,
        BEGIN_KEYWORD,
        IDENTIFIER, DEFINE_EQUALS, INT_NUMBER_LITERAL, SEMICOLON,
        IDENTIFIER, DEFINE_EQUALS, IDENTIFIER,
        END_KEYWORD, SEMICOLON,
        IDENTIFIER, DEFINE_EQUALS, IDENTIFIER, ADD_OP, INT_NUMBER_LITERAL,
        END_KEYWORD,
        END_KEYWORD, SEMICOLON,

        PROCEDURE_KEYWORD, IDENTIFIER, SEMICOLON,
        BEGIN_KEYWORD,
        IDENTIFIER, DEFINE_EQUALS, INT_NUMBER_LITERAL, SEMICOLON,
        WHILE_KEYWORD, IDENTIFIER, COMPARE_OP, IDENTIFIER, DO_KEYWORD,
        BEGIN_KEYWORD,
        CALL_KEYWORD, IDENTIFIER, SEMICOLON,
        IF_KEYWORD, IDENTIFIER, EQUALS, INT_NUMBER_LITERAL, THEN_KEYWORD,
        WRITE_OP, IDENTIFIER, SEMICOLON,
        IDENTIFIER, DEFINE_EQUALS, IDENTIFIER, ADD_OP, INT_NUMBER_LITERAL,
        END_KEYWORD,
        END_KEYWORD, SEMICOLON,

        CALL_KEYWORD, IDENTIFIER,
        PERIOD,
        END_OF_FILE
    };

    Lexer lexer;
    token_stream_t token_stream = lexer.lex(file_path);

    for (int i = 0; i < token_count; i++) {
        REQUIRE(
            tokenTypeToString(token_stream.at(i).type) == 
            tokenTypeToString(expected_types[i])
        );
    }
}

TEST_CASE("Floating Point Test", "[Lexer]") {
    const char *file_path = "../test/test_code/test3.p0";
    const int token_count = 44;
    token_class_t expected_types[token_count] = {
        VAR_KEYWORD, INT_TYPE_KEYWORD, LEFT_SQUARE_BRACKET, INT_NUMBER_LITERAL,
        RIGHT_SQUARE_BRACKET, IDENTIFIER, COMMA,
        INT_TYPE_KEYWORD, IDENTIFIER, COMMA,
        FLOAT_TYPE_KEYWORD, IDENTIFIER, SEMICOLON,

        BEGIN_KEYWORD,
        IDENTIFIER, DEFINE_EQUALS, FLOAT_NUMBER_LITERAL, SEMICOLON,
        IDENTIFIER, DEFINE_EQUALS, INT_NUMBER_LITERAL, SEMICOLON,

        WHILE_KEYWORD, IDENTIFIER, COMPARE_OP, INT_NUMBER_LITERAL, DO_KEYWORD,
        BEGIN_KEYWORD,
        IDENTIFIER, LEFT_SQUARE_BRACKET, IDENTIFIER, RIGHT_SQUARE_BRACKET,
        DEFINE_EQUALS, INT_NUMBER_LITERAL, SEMICOLON,
        IDENTIFIER, DEFINE_EQUALS, IDENTIFIER, ADD_OP, INT_NUMBER_LITERAL,
        END_KEYWORD,

        END_KEYWORD,
        PERIOD,
        END_OF_FILE
    };

    Lexer lexer;
    token_stream_t token_stream = lexer.lex(file_path);

    for (int i = 0; i < token_count; i++) {
        REQUIRE(
            tokenTypeToString(token_stream.at(i).type) == 
            tokenTypeToString(expected_types[i])
        );
    }
}