#include <past.h>

#include <assertions.h>

#define RETURN_ADDRESS "$return"

void typeMismatchErrorProcParam(
    const std::string callee, 
    const type_t type1, 
    const type_t type2
);

void typeMismatchError(EASTPtr refn, const type_t type1, const type_t type2);

void unknownProcedureError(ExprAST *refn, const std::string &name);

void assignmentToVoidError(const type_t type);

void undefinedVariable(ExprAST *refn, const std::string &name);

void conditionalExpectedType(EASTPtr refn, const type_t actual);

void arrayIndexTypeError(const type_t actual);

void procedureArgumentsLengthError(
    const std::string callee, 
    unsigned int expected,
    unsigned int actual
);

void ExprAST::treeTraversal(
    EASTPtr parent, std::function<void(EASTPtr)> action
) {
    if (parent != nullptr) {
        std::vector<EASTPtr> children = parent->getChildren();
        for (
            auto i = children.begin(); i != children.end(); i++
        ) {
            EASTPtr child = *i;
            ExprAST::treeTraversal(child, action);
            child = nullptr;
        }

        action(parent);
    }

    parent = nullptr;
}

std::optional<std::string> ExprAST::generateCode(
        TACGenerator &generator, 
        std::vector<tac_line_t> &generated
    ) { 
        std::vector<EASTPtr> children = this->getChildren();
        for (
            auto i = children.begin(); i != children.end(); i++
        ) {
            EASTPtr child = *i;
            if (child != nullptr) {
                child->generateCode(generator, generated);
            }

            child = nullptr;
        }
        return std::nullopt;
    };

void ExprListAST::typeChecker() {
    this->type = NO_TYPE;
}

void NumberAST::typeChecker() {
    #ifdef ASSERTIONS_ENABLED
        st_entry_t sym_ent;
        unsigned int level;
        this->symTable->lookup(this->name, &level, &sym_ent);
        ASSERT(sym_ent.entry_type == ST_LITERAL);
        ASSERT(sym_ent.literal.type != UNKNOWN);
        ASSERT(sym_ent.literal.type != NO_TYPE);
    #endif
}

std::optional<std::string> NumberAST::generateCode(
    TACGenerator &generator,
    std::vector<tac_line_t> &generated
) {
    return this->name;
}

void VariableAST::typeChecker() {
        st_entry_t sym_ent;
        unsigned int level;
        if (
            (!this->symTable->lookup(this->name, &level, &sym_ent))
            ||
            (!(sym_ent.entry_type == ST_VARIABLE))
        ) {
            undefinedVariable(this, this->name);
        }
        this->type = sym_ent.variable.type;
}

std::optional<std::string> VariableAST::generateCode(
    TACGenerator &generator,
    std::vector<tac_line_t> &generated
) {
    return this->name;
}

void BinaryExprAST::typeChecker() {
    ASSERT(this->rhs->type != UNKNOWN);

    if (this->rhs->type == VOID || this->rhs->type == NO_TYPE) {
        assignmentToVoidError(this->lhs->type);
    }

    if (this->operation == ASSIGNMENT) {

        if (this->lhs->type == UNKNOWN) {
            this->lhs->type = this->rhs->type;
        }
    }

    if (this->lhs->type != this->rhs->type) {
        typeMismatchError(this->lhs, this->lhs->type, this->rhs->type);
    }
    this->type = this->lhs->type;
}

std::optional<std::string> BinaryExprAST::generateCode(
    TACGenerator &generator,
    std::vector<tac_line_t> &generated
) {
    std::string arg1 = this->lhs->generateCode(generator, generated).value();
    std::string arg2 = this->rhs->generateCode(generator, generated).value();
    tac_line_t code = generator.makeQuad(
        treeTo3acOpMap.at(this->operation), arg1, arg2
    );
    generated.push_back(code);
    return code.result;
}

void UnaryExprAST::typeChecker() {
    this->type = this->operand->type;
}

std::optional<std::string> UnaryExprAST::generateCode(
    TACGenerator &generator,
    std::vector<tac_line_t> &generated
) {
    std::string arg1 = this->operand->generateCode(generator, generated)
        .value();
    tac_line_t code = generator.makeQuad(
        treeTo3acOpMap.at(this->operation), arg1
    );
    generated.push_back(code);
    return code.result;
}

void CallExprAST::typeChecker() {
    st_entry_t procedureInfo;
    unsigned int level;
    if (
        (!this->symTable->lookup(this->callee, &level, &procedureInfo))
        ||
        (!(procedureInfo.entry_type == ST_FUNCTION))
    ) {
        unknownProcedureError(this, this->callee);
    }

    this->type = procedureInfo.procedure.returnType;
}

std::optional<std::string> CallExprAST::generateCode(
    TACGenerator &generator,
    std::vector<tac_line_t> &generated
) {
    st_entry_t func_ent;
    unsigned int level;
    // Semantic analysis guarentees that this function has an entry.
    if (
        (!this->symTable->lookup(this->callee, &level, &func_ent))
        ||
        (func_ent.entry_type != ST_FUNCTION)
    ) {
        unknownProcedureError(this, this->callee);
    }

    if (func_ent.procedure.argumentsLength != this->arguments.size()) {
        procedureArgumentsLengthError(
            this->callee, 
            func_ent.procedure.argumentsLength,
            this->arguments.size()
        );
    }

    for (unsigned int i = 0; i < this->arguments.size(); i++) {
        type_t providedType = this->arguments.at(i)->type;
        type_t expectedType = func_ent.procedure.argumentTypes[i];
        if (providedType != expectedType) {
            typeMismatchErrorProcParam(
                this->callee, providedType, expectedType
            );
        }

        std::string arg1 = this->arguments
            .at(i)->generateCode(generator, generated).value();
        tac_line_t param_code = generator.makeQuad(TAC_PROC_PARAM, arg1);
        generated.push_back(param_code);
    }

    tac_line_t call_code = generator.makeQuad(TAC_CALL, this->callee);
    generated.push_back(call_code);

    return RETURN_ADDRESS;
}

void ProcedureAST::typeChecker() {
    this->type = NO_TYPE;
}

std::optional<std::string> ProcedureAST::generateCode(
    TACGenerator &generator,
    std::vector<tac_line_t> &generated
) {
    generated.push_back(generator.makeQuad(TAC_ENTER_PROC));
    tac_line_t functionLabel = generator.makeQuad(
        TAC_LABEL, 
        generator.customLabel(this->proto->name)
    );
    generated.push_back(functionLabel);

    this->body->generateCode(generator, generated);

    // Return types are optional.
    if (this->proto->returnVariable != nullptr) {
        generator.makeQuad(
            TAC_ASSIGN, this->proto->returnVariable->name, RETURN_ADDRESS
        );
    }

    generated.push_back(generator.makeQuad(TAC_EXIT_PROC));

    return std::nullopt;
}

void IfStatementAST::typeChecker() {
    if (this->condition->type != INT) {
        conditionalExpectedType(this->condition, this->condition->type);
    }
    this->type = NO_TYPE;
}

std::optional<std::string> IfStatementAST::generateCode(
    TACGenerator &generator,
    std::vector<tac_line_t> &generated
) {
    std::string cmp_result = 
        this->condition->generateCode(generator, generated).value();

    tac_line_t skipBodyLabel = generator.makeQuad(TAC_LABEL);
    tac_line_t skipBodyJump = generator.makeQuad(
        TAC_JMP_ZERO, skipBodyLabel.argument1
    );

    generated.push_back(skipBodyJump);

    this->body->generateCode(generator, generated);

    generated.push_back(skipBodyLabel);

    return std::nullopt;
}

void WhileStatementAST::typeChecker() {
    if (this->condition->type != INT) {
        conditionalExpectedType(this->condition, this->condition->type);
    }
    this->type = NO_TYPE;
}

std::optional<std::string> WhileStatementAST::generateCode(
    TACGenerator &generator,
    std::vector<tac_line_t> &generated
) {
    tac_line_t header = generator.makeQuad(TAC_LABEL);
    generated.push_back(header);

    std::string cmp_result = 
        this->condition->generateCode(generator, generated).value();
    
    tac_line_t skipBodyLabel = generator.makeQuad(TAC_LABEL);

    tac_line_t skipBodyJump = generator.makeQuad(
        TAC_JMP_ZERO, skipBodyLabel.argument1
    );

    generated.push_back(skipBodyJump);

    this->body->generateCode(generator, generated);

    generated.push_back(skipBodyLabel);

    return std::nullopt;
}

void ArrayIndexAST::typeChecker() {
    if (this->index->type != INT) {
        conditionalExpectedType(this->index, this->index->type);
    }
    this->type = this->array->type;
}

std::optional<std::string> ArrayIndexAST::generateCode(
    TACGenerator &generator,
    std::vector<tac_line_t> &generated
) {
    std::string array_address = 
        this->array->generateCode(generator, generated).value();
    std::string index_address = 
        this->index->generateCode(generator, generated).value();
    tac_line_t index_line = generator.makeQuad(
        TAC_ARRAY_INDEX, array_address, index_address);
    generated.push_back(index_line);

    return index_line.result;
}

void typeMismatchErrorProcParam(
    const std::string callee, 
    const type_t type1, 
    const type_t type2
) {
    ERROR_LOG(
        "type mismatch error calling function %s: %s and %s",
        callee.c_str(),
        typeToString(type1).c_str(),
        typeToString(type2).c_str()
    );
    exit(EXIT_FAILURE);
}

void typeMismatchError(EASTPtr refn, const type_t type1, const type_t type2) {
    ERROR_LOG(
            "type mismatch error: %s and %s"
            " in %s at line %d column %d",
            typeToString(type1).c_str(),
            typeToString(type2).c_str(),
            refn->getFile().c_str(),
            refn->getLine(),
            refn->getColumn()
    );
    exit(EXIT_FAILURE);
}

void unknownProcedureError(ExprAST *refn, const std::string &name) {
    ERROR_LOG(
        "attempted to call undeclared procedure with name %s"
        " in %s at line %d column %d",
        name.c_str(),
        refn->getFile().c_str(),
        refn->getLine(),
        refn->getColumn()
    );
    exit(EXIT_FAILURE);
}

void assignmentToVoidError(const type_t type) {
    ERROR_LOG(
        "attempted an assignment of %s to void",
        typeToString(type).c_str()
    );
    exit(EXIT_FAILURE);
}

void undefinedVariable(ExprAST *refn, const std::string &name) {
    ERROR_LOG(
        "use of undefined variable %s"
        " in %s at line %d column %d",
        name.c_str(),
        refn->getFile().c_str(),
        refn->getLine(),
        refn->getColumn()
    );
    exit(EXIT_FAILURE);
}

void conditionalExpectedType(EASTPtr refn, const type_t actual) {
    ERROR_LOG(
        "conditional must result in type int, but got %s"
        " in %s at line %d column %d",
        typeToString(actual).c_str(),
        refn->getFile().c_str(),
        refn->getLine(),
        refn->getColumn()
    );
    exit(EXIT_FAILURE);
}

void arrayIndexTypeError(const type_t actual) {
    ERROR_LOG(
        "array index must result in type int, but got %s",
        typeToString(actual).c_str()
    );
    exit(EXIT_FAILURE);
}

void procedureArgumentsLengthError(
    const std::string callee, 
    unsigned int expected,
    unsigned int actual
) {
    ERROR_LOG(
        "procedure %s expected %d arguments, but got %d instead",
        callee.c_str(),
        expected,
        actual
    );
    exit(EXIT_FAILURE);
}