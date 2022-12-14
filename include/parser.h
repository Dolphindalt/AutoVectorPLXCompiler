/**
 * Performs the syntax directed translation phase of the compiler. This file 
 * contains aspects of syntax analysis and semantic analysis. The token stream 
 * that is produced from the lexical analyzer is taken as input and is 
 * transformed into an abstract syntax tree representation. 
 * 
 * @file parser.h
 * @author Dalton Caron
 */
#ifndef PARSER_H__
#define PARSER_H__

#include <lexer.h>
#include <stack>
#include <initializer_list>
#include <past.h>
#include <symbol_table.h>

/**
 * The Parser class facilitates the parsing of the token stream provided from 
 * the lexical analyzer. It has the dual purpose of performing syntax directed 
 * translation to produce an abstract syntax tree intermediate representation 
 * as output while also performing semantic checks and inserting semantic 
 * information into the symbol table and resulting abstract syntax tree.
 */
class Parser {
public:
    /**
     * Constructs a representation of the parser on the provided input tokens.
     * @param tokens Tokenized source code to parse.
     */
    Parser(const token_stream_t &tokens);
    virtual ~Parser();

    /**
     * Performs syntax directed translation to produce an abstract syntax tree. 
     * @return The root node of the generated abstract syntax tree.
     */
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