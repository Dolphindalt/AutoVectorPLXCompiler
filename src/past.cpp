#include <past.h>

#include <assertions.h>

void typeMismatchError(const type_t type1, const type_t type2);

void unknownProcedureError(const std::string &name);

void assignmentToVoidError(const type_t type);

void undefinedVariable(const std::string &name);

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

void ExprListAST::typeChecker() {}

void NumberAST::typeChecker() {
    #ifdef ASSERTIONS_ENABLED
        st_entry_t sym_ent;
        unsigned int level;
        this->symTable->lookup(this->name, &level, &sym_ent);
        ASSERT(sym_ent.isLiteral == true);
        ASSERT(sym_ent.type != UNKNOWN);
        ASSERT(sym_ent.type != NO_TYPE);
    #endif
}

void VariableAST::typeChecker() {
        st_entry_t sym_ent;
        unsigned int level;
        if (!this->symTable->lookup(this->name, &level, &sym_ent)) {
            undefinedVariable(this->name);
        }
        this->type = sym_ent.type;
        ASSERT(sym_ent.isLiteral == false);
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
        typeMismatchError(this->lhs->type, this->rhs->type);
    }
    this->type = this->lhs->type;
}

void UnaryExprAST::typeChecker() {
    this->type = this->operand->type;
}

void CallExprAST::typeChecker() {
    st_entry_t procedureInfo;
    unsigned int level;
    if (
        !this->symTable->lookup(this->callee, &level, &procedureInfo)
        ||
        !procedureInfo.isProcedure
    ) {
        unknownProcedureError(this->callee);
    }

    ASSERT(procedureInfo.type != UNKNOWN);
    ASSERT(procedureInfo.type != NO_TYPE);

    this->type = procedureInfo.type;
}

void ProcedureAST::typeChecker() {
    this->type = NO_TYPE;
}

void IfStatementAST::typeChecker() {
    this->type = NO_TYPE;
}

void WhileStatementAST::typeChecker() {
    this->type = NO_TYPE;
}

void ArrayIndexAST::typeChecker() {
    this->type = this->array->type;
}

void typeMismatchError(const type_t type1, const type_t type2) {
    ERROR_LOG(
            "type mismatch error: %s and %s",
            typeToString(type1).c_str(),
            typeToString(type2).c_str()
    );
    exit(EXIT_FAILURE);
}

void unknownProcedureError(const std::string &name) {
    ERROR_LOG(
        "attempted to call undeclared procedure with name %s",
        name.c_str()
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

void undefinedVariable(const std::string &name) {
    ERROR_LOG(
        "use of undefined variable %s",
        name.c_str()
    );
    exit(EXIT_FAILURE);
}