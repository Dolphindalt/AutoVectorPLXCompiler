#ifndef AST_H__
#define AST_H__

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <optional>

#include <symbol_table.h>
#include <3ac.h>

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

static std::map<operation_t, tac_op_t> treeTo3acOpMap = {
    {BOOL_E, TAC_EQUALS},
    {BOOL_LE, TAC_LE_THAN},
    {BOOL_GE, TAC_GE_THAN},
    {BOOL_L, TAC_LESS_THAN},
    {BOOL_G, TAC_GREATER_THAN},
    {BOOL_NE, TAC_NOT_EQUALS},
    {ADDITION, TAC_ADD},
    {SUBTRACTION, TAC_SUB},
    {MULTIPLICATION, TAC_MULT},
    {DIVISION, TAC_DIV},
    {ASSIGNMENT, TAC_ASSIGN},
    {ODD, TAC_NEGATE},
    {READ, TAC_READ},
    {WRITE, TAC_WRITE}
};

class ExprAST {
public:
    std::shared_ptr<SymbolTable> symTable;
    type_t type;
    bool is_array;

    ExprAST(
        std::shared_ptr<SymbolTable> symTable,
        std::string file,
        unsigned int line,
        unsigned int column
    ) : symTable(symTable), type(UNKNOWN), is_array(false), file(file), 
        line(line), column(column) {};

    ExprAST(
        std::shared_ptr<SymbolTable> symTable,
        type_t type, 
        std::string file,
        unsigned int line,
        unsigned int column
    ) : symTable(symTable), type(type), is_array(false), file(file), 
        line(line), column(column) {};

    ExprAST(
        std::shared_ptr<SymbolTable> symTable,
        type_t type, 
        bool is_array,
        std::string file,
        unsigned int line,
        unsigned int column
    ) : symTable(symTable), type(type), is_array(is_array), file(file), 
        line(line), column(column) {};

    virtual ~ExprAST() {};

    virtual void typeChecker() = 0;

    // Children should be inorder of desired traversal.
    virtual std::vector<EASTPtr> getChildren() = 0;

    virtual std::optional<std::string> generateCode(
        TACGenerator &generator, 
        std::vector<tac_line_t> &generated
    );

    static void treeTraversal(
        EASTPtr parent, std::function<void(EASTPtr)> action
    );

    const std::string getFile() const { return this->file; };
    const unsigned int getLine() const { return this->line; };
    const unsigned int getColumn() const { return this->column; };
private:
    std::string file;
    unsigned int line;
    unsigned int column;
};

class ExprListAST : public ExprAST {
public:
    std::vector<EASTPtr> expressions;

    ExprListAST(
        std::shared_ptr<SymbolTable> symTable, 
        std::string file, 
        unsigned int line, 
        unsigned int column
    ) 
    : ExprAST(symTable, file, line, column), expressions(std::vector<EASTPtr>())
    {}

    ExprListAST(
        std::shared_ptr<SymbolTable> symTable,
        std::vector<EASTPtr> &expressions, 
        std::string file, 
        unsigned int line, 
        unsigned int column
    )
    : ExprAST(symTable, file, line, column), expressions(expressions) {}

    ExprListAST(
        std::shared_ptr<SymbolTable> symTable,
        ExprListAST &copy, 
        std::string file, 
        unsigned int line, 
        unsigned int column
    ) 
    : ExprAST(symTable, file, line, column), expressions(copy.expressions) {}

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
        std::shared_ptr<SymbolTable> symTable,
        std::string &name, 
        type_t type,
        std::string file,
        unsigned int line,
        unsigned int column
    ) 
    : ExprAST(symTable, type, file, line, column), name(name) {}

    std::vector<EASTPtr> getChildren() { return {}; };

    void typeChecker();

    std::optional<std::string> generateCode(
        TACGenerator &, 
        std::vector<tac_line_t> &
    );
};

class VariableAST : public ExprAST {
public:
    std::string name;

    VariableAST(
        std::shared_ptr<SymbolTable> symTable, 
        std::string &name, 
        type_t type,
        bool is_array,
        std::string file,
        unsigned int line,
        unsigned int column
    )
    : ExprAST(symTable, type, is_array, file, line, column), name(name) {}

    ~VariableAST() {
        this->symTable = nullptr;
    }

    std::vector<EASTPtr> getChildren() { return {}; };

    void typeChecker();

    std::optional<std::string> generateCode(
        TACGenerator &, 
        std::vector<tac_line_t> &
    );
};

class BinaryExprAST : public ExprAST {
public:
    operation_t operation;
    EASTPtr lhs, rhs;

    BinaryExprAST(
        std::shared_ptr<SymbolTable> symTable, 
        operation_t operation, 
        EASTPtr lhs, 
        EASTPtr rhs,
        std::string file, 
        unsigned int line, 
        unsigned int column
    ) 
    : ExprAST(symTable, file, line, column), operation(operation), 
        lhs(lhs), rhs(rhs) {}
    
    std::vector<EASTPtr> getChildren() { return { this->rhs, this->lhs }; };

    void typeChecker();

    std::optional<std::string> generateCode(
        TACGenerator &, 
        std::vector<tac_line_t> &
    );
};

class UnaryExprAST : public ExprAST {
public:
    operation_t operation;
    EASTPtr operand;

    UnaryExprAST(
        std::shared_ptr<SymbolTable> symTable, 
        operation_t operation, 
        EASTPtr operand,
        std::string file, 
        unsigned int line, 
        unsigned int column 
    )
    : ExprAST(symTable, file, line, column), 
        operation(operation), operand(operand) {}

    std::vector<EASTPtr> getChildren() { return { operand }; };

    void typeChecker();

    std::optional<std::string> generateCode(
        TACGenerator &, 
        std::vector<tac_line_t> &
    );
};

class CallExprAST : public ExprAST {
public:
    std::string callee;
    std::vector<EASTPtr> arguments;

    CallExprAST(
        std::shared_ptr<SymbolTable> symTable,
        std::string &callee, 
        std::vector<EASTPtr> arguments,
        std::string file, 
        unsigned int line, 
        unsigned int column
    )
    : ExprAST(symTable, file, line, column), callee(callee), 
        arguments(arguments) {}

    std::vector<EASTPtr> getChildren() { return arguments; };

    void typeChecker();

    std::optional<std::string> generateCode(
        TACGenerator &, 
        std::vector<tac_line_t> &
    );
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
        std::shared_ptr<SymbolTable> symTable,
        std::shared_ptr<Prototype> proto, 
        std::shared_ptr<ExprListAST> body,
        std::string file, 
        unsigned int line, 
        unsigned int column
    ) : ExprAST(symTable, file, line, column), proto(proto), body(body) {}

    std::vector<EASTPtr> getChildren() { return { body }; };

    void typeChecker();

    std::optional<std::string> generateCode(
        TACGenerator &, 
        std::vector<tac_line_t> &
    );
};

class IfStatementAST : public ExprAST {
public:
    EASTPtr condition;
    EASTPtr body;

    IfStatementAST(
        std::shared_ptr<SymbolTable> symTable,
        EASTPtr condition, 
        EASTPtr body,
        std::string file, 
        unsigned int line, 
        unsigned int column
    )
    : ExprAST(symTable, file, line, column), condition(condition), 
        body(body) {}
    
    std::vector<EASTPtr> getChildren() { return { condition, body }; };

    void typeChecker();

    std::optional<std::string> generateCode(
        TACGenerator &, 
        std::vector<tac_line_t> &
    );
};

class WhileStatementAST : public ExprAST {
public:
    EASTPtr condition;
    EASTPtr body;

    WhileStatementAST(
        std::shared_ptr<SymbolTable> symTable,
        EASTPtr condition, 
        EASTPtr body,
        std::string file, 
        unsigned int line, 
        unsigned int column
    )
    : ExprAST(symTable, file, line, column), condition(condition), 
        body(body) {}

    std::vector<EASTPtr> getChildren() { return { condition, body }; };

    void typeChecker();

    std::optional<std::string> generateCode(
        TACGenerator &, 
        std::vector<tac_line_t> &
    );
};

class ArrayIndexAST : public ExprAST {
public:
    std::shared_ptr<VariableAST> array;
    EASTPtr index;

    ArrayIndexAST(
        std::shared_ptr<SymbolTable> symTable,
        std::shared_ptr<VariableAST> array, 
        EASTPtr index,
        type_t type,
        std::string file,
        unsigned int line,
        unsigned int column
    ) : ExprAST(symTable, type, file, line, column), array(array), 
        index(index) {}

    std::vector<EASTPtr> getChildren() { return { array, index }; };

    void typeChecker();

    std::optional<std::string> generateCode(
        TACGenerator &, 
        std::vector<tac_line_t> &
    );
};

#endif