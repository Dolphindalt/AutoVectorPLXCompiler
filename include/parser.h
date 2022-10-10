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

    ParseTree<std::string> parse();
private:
    token_t getNextToken();
    token_t peekNextToken() const;

    PTPtr<std::string> parseProgram();
    PTPtr<std::string> parseBlock();
    PTPtr<std::string> parseConstDeclarations();
    PTPtr<std::string> parseConstDeclarationList();
    PTPtr<std::string> parseVarDeclarations();
    PTPtr<std::string> parseVarDeclarationList();
    PTPtr<std::string> parseProcedure();
    PTPtr<std::string> parseStatement();
    PTPtr<std::string> parseCondition();
    PTPtr<std::string> parseExpression();
    PTPtr<std::string> parseTerm();
    PTPtr<std::string> parseFactor();

    void tryMatchTerminal(
        const token_t &actual, 
        const token_class_t expected,
        PTPtr<std::string> node
    ) const;

    void tryMatchTerminal(
        const token_t &actual, 
        const std::initializer_list<token_class_t> expected,
        PTPtr<std::string> node
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