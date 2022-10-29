#ifndef AST_H__
#define AST_H__

#include <stdint.h>
#include <string.h>
#include <string>
#include <memory>
#include <vector>
#include <map>

#define NUMBER_SIZE_BYTES 8

class ExprAST;
class ExprListAST;

using AST = std::unique_ptr<ExprAST>;
using ASTHead = std::unique_ptr<ExprListAST>;
using EASTPtr = std::unique_ptr<ExprAST>;

typedef enum type { 
    UNKNOWN = 0,
    INT, 
    FLOAT,
    VOID,
    NO_TYPE
} type_t;

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
    type_t type;

    ExprAST() : type(UNKNOWN) {};
    ExprAST(type_t type) : type(type) {};

    virtual ~ExprAST() {};
};

class ExprListAST : public ExprAST {
public:
    std::vector<EASTPtr> expressions;

    ExprListAST() : ExprAST(NO_TYPE), expressions(std::vector<EASTPtr>()) {}

    ExprListAST(std::vector<EASTPtr> &expressions)
    : ExprAST(NO_TYPE), expressions(std::move(expressions)) {}

    ExprListAST(ExprListAST &copy) 
    : ExprAST(copy.type), expressions(std::move(copy.expressions)) {}

    void addChild(EASTPtr node) { 
        this->expressions.push_back(std::move(node)); 
    };
};

class NumberAST : public ExprAST {
public:
    uint8_t value[NUMBER_SIZE_BYTES];

    NumberAST(void *value, type_t type) 
    : ExprAST(type) {
        memcpy(this->value, value, NUMBER_SIZE_BYTES);
    }
};

class VariableAST : public ExprAST {
public:
    bool is_constant;
    std::string name;

    VariableAST(bool is_constant, std::string &name, type_t type)
    : ExprAST(type), is_constant(true), name(name) {}
};

class BinaryExprAST : public ExprAST {
public:
    operation_t operation;
    EASTPtr lhs, rhs;

    BinaryExprAST(
        operation_t operation, EASTPtr lhs, EASTPtr rhs) 
        : ExprAST(), operation(operation), lhs(std::move(lhs)), 
            rhs(std::move(rhs)) {}
};

class UnaryExprAST : public ExprAST {
public:
    operation_t operation;
    EASTPtr operand;

    UnaryExprAST(operation_t operation, EASTPtr operand)
    : ExprAST(), operation(operation), operand(std::move(operand)) {}
};

class CallExprAST : public ExprAST {
public:
    std::string callee;
    std::vector<EASTPtr> arguments;

    CallExprAST(std::string &callee, std::vector<EASTPtr> arguments)
    : ExprAST(UNKNOWN), callee(callee), arguments(std::move(arguments)) {}
};

class Prototype {
public:
    std::string name;
    std::vector<std::unique_ptr<VariableAST>> arguments;
    std::unique_ptr<VariableAST> returnVariable = nullptr;

    Prototype(
        std::string name, 
        std::vector<std::unique_ptr<VariableAST>> arguments,
        std::unique_ptr<VariableAST> returnVariable
    ) : name(name), arguments(std::move(arguments)), 
        returnVariable(std::move(returnVariable)) {}
};

class ProcedureAST : public ExprAST {
public:
    std::unique_ptr<Prototype> proto;
    std::unique_ptr<ExprListAST> body;

    ProcedureAST(
        std::unique_ptr<Prototype> proto, 
        std::unique_ptr<ExprListAST> body
    ) : ExprAST(VOID), proto(std::move(proto)), body(std::move(body)) {}
};

class IfStatementAST : public ExprAST {
public:
    EASTPtr condition;
    EASTPtr body;

    IfStatementAST(EASTPtr condition, EASTPtr body)
    : ExprAST(VOID), condition(std::move(condition)), body(std::move(body)) {}
};

class WhileStatementAST : public ExprAST {
public:
    EASTPtr condition;
    EASTPtr body;

    WhileStatementAST(EASTPtr condition, EASTPtr body)
    : ExprAST(VOID), condition(std::move(condition)), body(std::move(body)) {}
};

#endif