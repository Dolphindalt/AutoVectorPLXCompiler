#ifndef AST_H__
#define AST_H__

#include <stdint.h>
#include <string.h>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <symbol_table.h>

#define NUMBER_SIZE_BYTES 8

class ExprAST;
class ExprListAST;

using AST = std::shared_ptr<ExprAST>;
using ASTHead = std::shared_ptr<ExprListAST>;
using EASTPtr = std::shared_ptr<ExprAST>;

typedef enum operation {
    BOOL_E,
    BOOL_NE,
    BOOL_L,
    BOOL_G,
    BOOL_LE,
    BOOL_GE,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    ASSIGNMENT,
    ODD,
    WRITE,
    READ
} operation_t;

static std::map<std::string, operation_t> cmpOpMap = {
    {"=", BOOL_E},
    {"<=", BOOL_LE},
    {">=", BOOL_GE},
    {"<", BOOL_L},
    {">", BOOL_G},
    {"#", BOOL_NE},
    {"+", ADDITION},
    {"-", SUBTRACTION},
    {"*", MULTIPLICATION},
    {"/", DIVISION}
};

class ExprAST {
public:
    std::shared_ptr<SymbolTable> symTable;
    ExprAST(std::shared_ptr<SymbolTable> symTable)
    : symTable(symTable) {}
    virtual ~ExprAST() {};

    virtual void typeChecker() = 0;

    // Children should be inorder of desired traversal.
    virtual std::vector<EASTPtr> getChildren() = 0;

    static void treeTraversal(
        EASTPtr parent, std::function<void(EASTPtr)> action
    );
};

class ExprListAST : public ExprAST {
public:
    std::vector<EASTPtr> expressions;

    ExprListAST(std::shared_ptr<SymbolTable> symTable) 
    : ExprAST(symTable), expressions(std::vector<EASTPtr>()) {}

    ExprListAST(
        std::vector<EASTPtr> &expressions, 
        std::shared_ptr<SymbolTable> symTable
    )
    : ExprAST(symTable), expressions(expressions) {}

    ExprListAST(ExprListAST &copy, std::shared_ptr<SymbolTable> symTable) 
    : ExprAST(symTable), expressions(copy.expressions) {}

    void addChild(EASTPtr node) { 
        this->expressions.push_back(node); 
    };

    std::vector<EASTPtr> getChildren() { return this->expressions; };

    void typeChecker();
};

class NumberAST : public ExprAST {
public:
    std::string name;
    uint8_t value[NUMBER_SIZE_BYTES];

    NumberAST(
        std::string &name, 
        void *value, 
        std::shared_ptr<SymbolTable> symTable
    ) 
    : ExprAST(symTable), name(name) {
        memcpy(this->value, value, NUMBER_SIZE_BYTES);
    }

    std::vector<EASTPtr> getChildren() { return {}; };

    void typeChecker();
};

class VariableAST : public ExprAST {
public:
    std::string name;

    VariableAST(std::string &name, std::shared_ptr<SymbolTable> symTable)
    : ExprAST(symTable), name(name) {}

    ~VariableAST() {
        this->symTable = nullptr;
    }

    std::vector<EASTPtr> getChildren() { return {}; };

    void typeChecker();
};

class BinaryExprAST : public ExprAST {
public:
    operation_t operation;
    EASTPtr lhs, rhs;

    BinaryExprAST(
        operation_t operation, 
        EASTPtr lhs, 
        EASTPtr rhs, 
        std::shared_ptr<SymbolTable> symTable
    ) 
    : ExprAST(symTable), operation(operation), lhs(lhs), rhs(rhs) {}
    
    std::vector<EASTPtr> getChildren() { return { this->rhs, this->lhs }; };

    void typeChecker();
};

class UnaryExprAST : public ExprAST {
public:
    operation_t operation;
    EASTPtr operand;

    UnaryExprAST(
        operation_t operation, 
        EASTPtr operand, 
        std::shared_ptr<SymbolTable> symTable
    )
    : ExprAST(symTable), operation(operation), operand(operand) {}

    std::vector<EASTPtr> getChildren() { return { operand }; };

    void typeChecker();
};

class CallExprAST : public ExprAST {
public:
    std::string callee;
    std::vector<EASTPtr> arguments;

    CallExprAST(
        std::string &callee, 
        std::vector<EASTPtr> arguments,
        std::shared_ptr<SymbolTable> symTable
    )
    : ExprAST(symTable), callee(callee), arguments(arguments) {}

    std::vector<EASTPtr> getChildren() { return arguments; };

    void typeChecker();
};

class Prototype {
public:
    std::string name;
    std::vector<std::shared_ptr<VariableAST>> arguments;
    std::shared_ptr<VariableAST> returnVariable = nullptr;

    Prototype(
        std::string name, 
        std::vector<std::shared_ptr<VariableAST>> arguments,
        std::shared_ptr<VariableAST> returnVariable
    ) : name(name), arguments(arguments), 
        returnVariable(returnVariable) {}
};

class ProcedureAST : public ExprAST {
public:
    std::shared_ptr<Prototype> proto;
    std::shared_ptr<ExprListAST> body;

    ProcedureAST(
        std::shared_ptr<Prototype> proto, 
        std::shared_ptr<ExprListAST> body,
        std::shared_ptr<SymbolTable> symTable
    ) : ExprAST(symTable), proto(proto), body(body) {}

    std::vector<EASTPtr> getChildren() { return { body }; };

    void typeChecker();
};

class IfStatementAST : public ExprAST {
public:
    EASTPtr condition;
    EASTPtr body;

    IfStatementAST(
        EASTPtr condition, 
        EASTPtr body, 
        std::shared_ptr<SymbolTable> symTable
    )
    : ExprAST(symTable), condition(condition), 
        body(body) {}
    
    std::vector<EASTPtr> getChildren() { return { condition, body }; };

    void typeChecker();
};

class WhileStatementAST : public ExprAST {
public:
    EASTPtr condition;
    EASTPtr body;

    WhileStatementAST(
        EASTPtr condition, 
        EASTPtr body, 
        std::shared_ptr<SymbolTable> symTable
    )
    : ExprAST(symTable), condition(condition), 
        body(body) {}

    std::vector<EASTPtr> getChildren() { return { condition, body }; };

    void typeChecker();
};

class ArrayIndexAST : public ExprAST {
public:
    std::shared_ptr<VariableAST> array;
    EASTPtr index;

    ArrayIndexAST(
        std::shared_ptr<VariableAST> array, 
        EASTPtr index,
        std::shared_ptr<SymbolTable> symTable
    ) : ExprAST(symTable), array(array), index(index) {}

    std::vector<EASTPtr> getChildren() { return { array, index }; };

    void typeChecker();
};

#endif