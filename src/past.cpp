#include <past.h>

#include <assertions.h>

void typeMismatchError();

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

st_entry_t fastSymLookup(const std::string &name, ExprAST *exprAST) {
    st_entry_t ret;
    unsigned int level;
    exprAST->symTable->lookup(name, &level, &ret);
    return ret;
}

void ExprListAST::typeChecker() {}

void NumberAST::typeChecker() {
    #ifdef ASSERTIONS_ENABLED
        st_entry_t sym_ent = fastSymLookup(this->name, this);
        ASSERT(sym_ent.isLiteral == true);
        ASSERT(sym_ent.type != UNKNOWN);
        ASSERT(sym_ent.type != NO_TYPE);
    #endif
}

void VariableAST::typeChecker() {
    #ifdef ASSERTIONS_ENABLED
        st_entry_t sym_ent = fastSymLookup(this->name, this);
        ASSERT(sym_ent.isLiteral == false);
        ASSERT(sym_ent.isAssigned == false);
    #endif
}

void BinaryExprAST::typeChecker() {

}

void UnaryExprAST::typeChecker() {

}

void CallExprAST::typeChecker() {

}

void ProcedureAST::typeChecker() {

}

void IfStatementAST::typeChecker() {

}

void WhileStatementAST::typeChecker() {

}

void ArrayIndexAST::typeChecker() {

}

void typeMismatchError() {

}