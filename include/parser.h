/**
 *  CPSC 323 Compilers and Languages
 * 
 *  Dalton Caron, Teaching Associate
 *  dcaron@fullerton.edu, +1 949-616-2699
 *  Department of Computer Science
 */
#ifndef PARSER_H__
#define PARSER_H__

#include <lexer.h>
#include <stack>
#include <initializer_list>
#include <parse_tree.h>

class Parser {
public:
    Parser(const token_stream_t &tokens);
    virtual ~Parser();

    void parse();
private:
    token_t getNextToken();
    token_t peekNextToken() const;

    void parseProgram();
    void parseBlock();
    void parseConstDeclarations();
    void parseConstDeclarationList();
    void parseVarDeclarations();
    void parseVarDeclarationList();
    void parseProcedure();
    void parseStatement();
    void parseCondition();
    void parseExpression();
    void parseTerm();
    void parseFactor();

    void tryMatchTerminal(
        const token_t &actual, 
        const token_class_t expected
    ) const;

    void tryMatchTerminal(
        const token_t &actual, 
        const std::initializer_list<token_class_t> expected
    ) const;

    void raiseMismatchError(
        const token_t &actual, 
        const token_class_t expected
    ) const;

    void raiseMismatchError(
        const token_t &actual, 
        const std::initializer_list<token_class_t> expected
    ) const;

    const token_stream_t tokens;
    unsigned int current_token = 0;
};

#endif