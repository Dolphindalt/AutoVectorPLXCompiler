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
#include <past.h>
#include <symbol_table.h>

/**
 * Semantic checks to be performed:
 * - Duplicate procedures
 * - Duplicate variables
 * - Undefined procedures
 * - Undefined variables
 * - Number of arguments mismatch
 * - Argument type error
 * - Return type error
 * - Procedure call assignment type error
 * - If statement condition type error
 * - While loop condition type error
 * - Assignment type mismatch
 * - Expression type error
 */
class Parser {
public:
    Parser(const token_stream_t &tokens);
    virtual ~Parser();

    AST parse();
private:
    token_t getNextToken();
    token_t peekNextToken() const;

    ASTHead parseProgram();
    ASTHead parseRootBlock();
    ASTHead parseBlock();
    void parseConstDeclarations(std::shared_ptr<ExprListAST> parent);
    void parseConstDeclarationList(std::shared_ptr<ExprListAST> parent);
    void parseVarDeclarations(std::shared_ptr<ExprListAST> parent);
    void parseVarDeclarationList(std::shared_ptr<ExprListAST> parent);
    void parseProcedure(std::shared_ptr<ExprListAST> parent);
    std::vector<std::shared_ptr<VariableAST>> parseArguments();
    AST parseStatement();
    AST parseCondition();
    AST parseExpression();
    AST parseExpressionTail();
    AST parseTerm();
    AST parseTermTail();
    AST parseFactor();
    std::shared_ptr<NumberAST> parseNumber();
    AST parseVariable();
    void parseType(type_t *type, bool *is_array, unsigned int *array_size);

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

    std::shared_ptr<SymbolTable> currentScope();
    void enterNewScope();
    void exitOldScope();
    
    std::stack<std::shared_ptr<SymbolTable>> scopes;

    const token_stream_t tokens;
    unsigned int current_token = 0;
};

#endif