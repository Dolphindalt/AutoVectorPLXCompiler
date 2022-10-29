/**
 *  CPSC 323 Compilers and Languages
 * 
 *  Dalton Caron, Teaching Associate
 *  dcaron@fullerton.edu, +1 949-616-2699
 *  Department of Computer Science
 */
#include <parser.h>

Parser::Parser(const token_stream_t &tokens) 
: tokens(tokens) {
    std::shared_ptr<SymbolTable> globalScope = std::make_shared<SymbolTable>();
    this->scopes.push(globalScope);
    globalScope = nullptr;
}

Parser::~Parser() {}

AST Parser::parse() {
    return this->parseProgram();
}

token_t Parser::getNextToken() {
    // Assumes that the end of the token stream always has 
    // the end of file enumeration.
    if (this->current_token == this->tokens.size()) {
        return this->tokens[this->current_token - 1];
    }
    
    return this->tokens[this->current_token++];
}

token_t Parser::peekNextToken() const {
    if (this->current_token == this->tokens.size()) {
        return this->tokens[this->current_token - 1];
    }
    return this->tokens[this->current_token];
}

ASTHead Parser::parseProgram() {
    
    ASTHead programBlock = this->parseBlock();

    this->tryMatchTerminal(this->getNextToken(), PERIOD);
    return programBlock;
}

ASTHead Parser::parseBlock() {
    std::shared_ptr<ExprListAST> blockNode = 
        std::make_shared<ExprListAST>();

    token_t token = this->peekNextToken();

    if (token.type == CONST_KEYWORD) {
        this->parseConstDeclarations(blockNode);
        token = this->peekNextToken();    
    }

    if (token.type == VAR_KEYWORD) {
        this->parseVarDeclarations(blockNode);
        token = this->peekNextToken();
    }

    if (token.type == PROCEDURE_KEYWORD) {
        // Procedures can be parsed 0 or more times.
        while (this->peekNextToken().type == PROCEDURE_KEYWORD) {
            this->parseProcedure(blockNode);
        }
    }

    blockNode->addChild(this->parseStatement());
    return std::make_unique<ExprListAST>(*blockNode.get());
}

void Parser::parseConstDeclarations(std::shared_ptr<ExprListAST> parent) {
    this->tryMatchTerminal(this->getNextToken(), CONST_KEYWORD);

    st_entry_t typeInfo = this->parseType();

    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);

    this->tryMatchTerminal(this->getNextToken(), EQUALS);

    EASTPtr number = this->parseNumber();

    EASTPtr variable = std::make_unique<VariableAST>(
        identifier.lexeme, this->currentScope()
    );

    typeInfo.isConstant = true;
    typeInfo.isAssigned = true;
    typeInfo.token = identifier;

    this->currentScope()->insert(identifier.lexeme, typeInfo);

    EASTPtr binAST 
        = std::make_unique<BinaryExprAST>(
            ASSIGNMENT, std::move(variable), std::move(number)
        );
    parent->addChild(std::move(binAST));
    
    // There can be zero or more constant items separated by a comma.
    while (this->peekNextToken().type == COMMA) {
        this->parseConstDeclarationList(parent);
    }

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON);
}

void Parser::parseConstDeclarationList(std::shared_ptr<ExprListAST> parent) {
    this->tryMatchTerminal(this->getNextToken(), COMMA);

    st_entry_t typeInfo = this->parseType();

    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);
    this->tryMatchTerminal(this->getNextToken(), EQUALS);

    EASTPtr number = this->parseNumber();
    EASTPtr variable = std::make_unique<VariableAST>(
        identifier.lexeme,
        this->currentScope()
    );

    typeInfo.isConstant = true;
    typeInfo.isAssigned = true;
    typeInfo.token = identifier;

    this->currentScope()->insert(identifier.lexeme, typeInfo);

    EASTPtr binAST 
        = std::make_unique<BinaryExprAST>(
            ASSIGNMENT, std::move(variable), std::move(number)
        );
    parent->addChild(std::move(binAST));
}

void Parser::parseVarDeclarations(std::shared_ptr<ExprListAST> parent) {
    this->tryMatchTerminal(this->getNextToken(), VAR_KEYWORD);

    st_entry_t typeInfo = this->parseType();

    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);

    EASTPtr variable = std::make_unique<VariableAST>(
        identifier.lexeme, 
        this->currentScope()
    );

    typeInfo.isConstant = false;
    typeInfo.isAssigned = false;
    typeInfo.token = identifier;

    this->currentScope()->insert(identifier.lexeme, typeInfo);

    EASTPtr declaration = std::make_unique<UnaryExprAST>(
        ASSIGNMENT, std::move(variable)
    );
    parent->addChild(std::move(declaration));

    // There can be zero or more variable names separated by a comma.
    while (this->peekNextToken().type == COMMA) {
        this->parseVarDeclarationList(parent);
    }

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON);

}

void Parser::parseVarDeclarationList(std::shared_ptr<ExprListAST> parent) {    
    this->tryMatchTerminal(this->getNextToken(), COMMA);

    st_entry_t typeInfo = this->parseType();

    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);

    EASTPtr variable = std::make_unique<VariableAST>(
        identifier.lexeme, 
        this->currentScope()
    );

    typeInfo.isConstant = false;
    typeInfo.isAssigned = false;
    typeInfo.token = identifier;

    this->currentScope()->insert(identifier.lexeme, typeInfo);

    EASTPtr declaration = std::make_unique<UnaryExprAST>(
        ASSIGNMENT, std::move(variable)
    );
    parent->addChild(std::move(declaration));
}

void Parser::parseProcedure(std::shared_ptr<ExprListAST> parent) {    
    this->tryMatchTerminal(this->getNextToken(), PROCEDURE_KEYWORD);

    token_t procedure_id = this->getNextToken();
    this->tryMatchTerminal(procedure_id, IDENTIFIER);

    std::vector<std::unique_ptr<VariableAST>> arguments;

    token_t peek = this->peekNextToken();
    if (peek.type == LEFT_PAREN) {
        this->tryMatchTerminal(this->getNextToken(), LEFT_PAREN);
        arguments = this->parseArguments();
        this->tryMatchTerminal(this->getNextToken(), RIGHT_PAREN);
    }

    std::unique_ptr<VariableAST> returnId = nullptr;

    peek = this->peekNextToken();
    // First set of type.
    if (peek.type == INT_NUMBER_LITERAL || peek.type == FLOAT_NUMBER_LITERAL) {
        st_entry_t typeInfo = this->parseType();
        typeInfo.isAssigned = false;

        token_t ident = this->getNextToken();
        this->tryMatchTerminal(ident, IDENTIFIER);

        typeInfo.token = ident;

        this->currentScope()->insert(ident.lexeme, typeInfo);

        returnId = std::make_unique<VariableAST>(
            ident.lexeme, this->currentScope()
        );
    }

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON);

    std::unique_ptr<ExprListAST> blockBody = this->parseBlock();

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON);

    std::unique_ptr<Prototype> prototype 
        = std::make_unique<Prototype>(
            procedure_id.lexeme, std::move(arguments), std::move(returnId)
        );

    EASTPtr procedure = std::make_unique<ProcedureAST>(
        std::move(prototype), std::move(blockBody)
    );

    parent->addChild(std::move(procedure));
}

std::vector<std::unique_ptr<VariableAST>> Parser::parseArguments() {
    
    std::vector<std::unique_ptr<VariableAST>> arguments;

    st_entry_t typeInfo = this->parseType();
    // All function parameters are considered to be assigned.
    typeInfo.isAssigned = true;

    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);

    typeInfo.token = identifier;

    arguments.push_back(
        std::make_unique<VariableAST>(
            identifier.lexeme, 
            this->currentScope()
        )
    );

    this->currentScope()->insert(identifier.lexeme, typeInfo);

    while (this->peekNextToken().type == COMMA) {
        this->tryMatchTerminal(this->getNextToken(), COMMA);

        typeInfo = this->parseType();
        typeInfo.isAssigned = true;

        identifier = this->getNextToken();
        this->tryMatchTerminal(identifier, IDENTIFIER);

        typeInfo.token = identifier;

        arguments.push_back(
            std::make_unique<VariableAST>(
                identifier.lexeme, 
                this->currentScope()
            )
        );

        this->currentScope()->insert(identifier.lexeme, typeInfo);
    }
    return arguments;
}

AST Parser::parseStatement() {

    const token_t token = this->peekNextToken();
    switch (token.type) {
        case IDENTIFIER: {
            token_t identifier = this->getNextToken();
            this->tryMatchTerminal(identifier, IDENTIFIER);
            EASTPtr lhs = std::make_unique<VariableAST>(
                identifier.lexeme, 
                this->currentScope()
            );

            this->tryMatchTerminal(this->getNextToken(), DEFINE_EQUALS);

            EASTPtr rhs = this->parseExpression();

            EASTPtr binExpr = std::make_unique<BinaryExprAST>(
                ASSIGNMENT, 
                std::move(lhs), 
                std::move(rhs)
            );

            return binExpr;
        }
        case CALL_KEYWORD: {
            this->tryMatchTerminal(this->getNextToken(), CALL_KEYWORD);
            token_t procedureID = this->getNextToken();
            this->tryMatchTerminal(procedureID, IDENTIFIER);

            std::vector<EASTPtr> arguments;

            if (this->peekNextToken().type == LEFT_PAREN) {
                this->tryMatchTerminal(this->getNextToken(), LEFT_PAREN);
                arguments.push_back(this->parseExpression());
                this->tryMatchTerminal(this->getNextToken(), RIGHT_PAREN);
            }

            EASTPtr call = std::make_unique<CallExprAST>(
                procedureID.lexeme, std::move(arguments)
            );

            return call;
        }
        case WRITE_OP: {
            this->tryMatchTerminal(this->getNextToken(), WRITE_OP);
            EASTPtr toWrite = this->parseExpression();

            EASTPtr write = std::make_unique<UnaryExprAST>(
                WRITE, std::move(toWrite)
            );

            return write;
        }
        case READ_OP: {
            this->tryMatchTerminal(this->getNextToken(), READ_OP);
            token_t identifier = this->getNextToken();
            this->tryMatchTerminal(identifier, IDENTIFIER);

            EASTPtr toReadTo = std::make_unique<VariableAST>(
                identifier.lexeme, 
                this->currentScope()
            );

            EASTPtr read = std::make_unique<UnaryExprAST>(
                READ, std::move(toReadTo)
            );

            return read;
        }
        case BEGIN_KEYWORD: {
            this->tryMatchTerminal(this->getNextToken(), BEGIN_KEYWORD);

            std::unique_ptr<ExprListAST> exprList 
                = std::make_unique<ExprListAST>();

            EASTPtr statement = this->parseStatement();
            exprList->addChild(std::move(statement));

            while (this->peekNextToken().type == SEMICOLON) {
                this->tryMatchTerminal(this->getNextToken(), SEMICOLON);
                statement = this->parseStatement();
                exprList->addChild(std::move(statement));
            }
            this->tryMatchTerminal(this->getNextToken(), END_KEYWORD);

            return exprList;
        }
        case IF_KEYWORD: {
            this->tryMatchTerminal(this->getNextToken(), IF_KEYWORD);
            EASTPtr condition = this->parseCondition();

            this->tryMatchTerminal(this->getNextToken(), THEN_KEYWORD);
            EASTPtr body = this->parseStatement();

            return std::make_unique<IfStatementAST>(
                std::move(condition), std::move(body)
            );
        }
        case WHILE_KEYWORD: {
            this->tryMatchTerminal(this->getNextToken(), WHILE_KEYWORD);
            EASTPtr condition = this->parseCondition();

            this->tryMatchTerminal(this->getNextToken(), DO_KEYWORD);
            EASTPtr body = this->parseStatement();

            return std::make_unique<WhileStatementAST>(
                std::move(condition), std::move(body)
            );
        }
        default:
            break;
    }

    return nullptr;
}

AST Parser::parseCondition() {
    const token_t token = this->peekNextToken();
    switch (token.type) {
        case ODD_OP: {
            this->tryMatchTerminal(this->getNextToken(), ODD_OP);
            EASTPtr operand = this->parseExpression();
            return std::make_unique<UnaryExprAST>(ODD, std::move(operand));
        }
        default: {
            EASTPtr lhs = this->parseExpression();
            const token_t cmpOp = this->getNextToken();
            this->tryMatchTerminal(cmpOp, {COMPARE_OP, EQUALS});
            operation_t op = cmpOpMap.at(cmpOp.lexeme);
            EASTPtr rhs = this->parseExpression();
            return std::make_unique<BinaryExprAST>(
                op, std::move(lhs), std::move(rhs)
            );
        }
    }

    return nullptr;
}

AST Parser::parseExpression() {
    // Optional plus and minus.
    bool is_unary = false;
    operation_t unary_op;
    const token_t unaryOp = this->peekNextToken();
    if (unaryOp.type == ADD_OP) {
        this->tryMatchTerminal(this->getNextToken(), ADD_OP);
    }

    EASTPtr lhs = this->parseTerm();

    if (is_unary) {
        lhs = std::make_unique<UnaryExprAST>(unary_op, std::move(lhs));
    }

    const token_t next = this->peekNextToken();
    if (next.type == ADD_OP) {
        operation_t op = cmpOpMap.at(next.lexeme);
        EASTPtr rhs = this->parseExpressionTail();
        return std::make_unique<BinaryExprAST>(
            op, std::move(lhs), std::move(rhs)
        );
    }

    return lhs;
}

AST Parser::parseExpressionTail() {
    this->tryMatchTerminal(this->getNextToken(), ADD_OP);
    EASTPtr lhs = this->parseTerm();

    const token_t next = this->peekNextToken();
    if (next.type == ADD_OP) {
        operation_t op = cmpOpMap.at(next.lexeme);
        EASTPtr rhs = this->parseExpressionTail();
        return std::make_unique<BinaryExprAST>(
            op, std::move(lhs), std::move(rhs)
        );
    }

    return lhs;
}

AST Parser::parseTerm() {
    EASTPtr lhs = this->parseFactor();

    const token_t next = this->peekNextToken();
    while (next.type == MUL_OP) {
        operation_t op = cmpOpMap.at(next.lexeme);        
        EASTPtr rhs = this->parseTermTail();
        return std::make_unique<BinaryExprAST>(
            op, std::move(lhs), std::move(rhs)
        );
    }

    return lhs;
}

AST Parser::parseTermTail() {
    this->tryMatchTerminal(this->getNextToken(), MUL_OP);
    EASTPtr lhs = this->parseFactor();
    const token_t next = this->peekNextToken();
    if (next.type == MUL_OP) {
        operation_t op = cmpOpMap.at(next.lexeme);
        EASTPtr rhs = this->parseTermTail();
        return std::make_unique<BinaryExprAST>(
            op, std::move(lhs), std::move(rhs)
        );
    }

    return lhs;
}

AST Parser::parseFactor() {
    token_t token = this->peekNextToken();
    switch (token.type) {
        case IDENTIFIER:
            this->tryMatchTerminal(this->getNextToken(), IDENTIFIER);
            return std::make_unique<VariableAST>(
                token.lexeme,
                this->currentScope()
            );
        case INT_NUMBER_LITERAL:
        case FLOAT_NUMBER_LITERAL:
            return this->parseNumber();
        case LEFT_PAREN: {
            this->tryMatchTerminal(this->getNextToken(), LEFT_PAREN);
            EASTPtr expr = this->parseExpression();
            this->tryMatchTerminal(this->getNextToken(), RIGHT_PAREN);
            return expr;
        }
        default:
            raiseMismatchError(token, {
                IDENTIFIER,
                INT_NUMBER_LITERAL,
                FLOAT_NUMBER_LITERAL,
                LEFT_PAREN
            });
    }

    return nullptr;
}

AST Parser::parseNumber() {
    token_t next = this->getNextToken();

    bool is_unary = false;
    operation_t unary_op;

    if (next.type == ADD_OP) {
        this->tryMatchTerminal(next, ADD_OP);
        unary_op = cmpOpMap.at(next.lexeme);
        next = this->getNextToken();
    }

    EASTPtr number;

    st_entry_t typeInfo;
    typeInfo.isConstant = true;
    typeInfo.isAssigned = true;
    typeInfo.isLiteral = true;
    typeInfo.token = next;

    switch (next.type) {
        case INT_NUMBER_LITERAL: {
            this->tryMatchTerminal(next, INT_NUMBER_LITERAL);
            uint64_t value = atoi(next.lexeme.c_str());
            number = std::make_unique<NumberAST>((void *)&value);
            break;
        }
        case FLOAT_NUMBER_LITERAL: {
            this->tryMatchTerminal(next, FLOAT_NUMBER_LITERAL);
            double value = atof(next.lexeme.c_str());
            number = std::make_unique<NumberAST>((void *)&value);
            break;
        }
        default:
            raiseMismatchError(next, {
                ADD_OP,
                INT_NUMBER_LITERAL,
                FLOAT_NUMBER_LITERAL
            });
    }

    this->currentScope()->insert(next.lexeme, typeInfo);

    if (is_unary) {
        return std::make_unique<UnaryExprAST>(unary_op, std::move(number));
    }

    return number;
}

st_entry_t Parser::parseType() {
    st_entry_t st_ent;
    st_ent.isArray = false;
    st_ent.isConstant = false;
    st_ent.isAssigned = false;

    const token_t next = this->getNextToken();
    switch (next.type) {
        case INT_TYPE_KEYWORD:
            this->tryMatchTerminal(next, INT_TYPE_KEYWORD);
            st_ent.type = INT;
            break;
        case FLOAT_TYPE_KEYWORD:
            this->tryMatchTerminal(next, FLOAT_TYPE_KEYWORD);
            st_ent.type = FLOAT;
            break;
        default:
            raiseMismatchError(next, { 
                INT_TYPE_KEYWORD, 
                FLOAT_TYPE_KEYWORD 
            });
    }

    const token_t peek = this->peekNextToken();
    if (peek.type == LEFT_SQUARE_BRACKET) {
        this->tryMatchTerminal(this->getNextToken(), LEFT_SQUARE_BRACKET);
        const token_t int_literal = this->getNextToken();
        this->tryMatchTerminal(int_literal, INT_NUMBER_LITERAL);
        this->tryMatchTerminal(this->getNextToken(), RIGHT_SQUARE_BRACKET);
        st_ent.isArray = true;
        st_ent.arraySize = atoi(int_literal.lexeme.c_str());
    }

    return st_ent;
}

void Parser::tryMatchTerminal(
    const token_t &actual,
    const token_class_t expected
) const {
    if (actual.type != expected) {
        raiseMismatchError(actual, expected);
    }
}

void Parser::tryMatchTerminal(
    const token_t &actual, 
    const std::initializer_list<token_class_t> expected
) const {
    for (const token_class_t type : expected) {
        if (type == actual.type) {
            return;
        }
    }
    raiseMismatchError(actual, expected);
}

void Parser::raiseMismatchError(
        const token_t &actual, 
        const token_class_t expected
) const {
    (void) fprintf(
        stderr,
        "expected %s got %s "
        "at %s line %d column %d\n",
        tokenTypeToString(expected).c_str(),
        actual.lexeme.c_str(),
        actual.file.c_str(),
        actual.line,
        actual.column
    );

    exit(EXIT_FAILURE);
}

void Parser::raiseMismatchError(
        const token_t &actual, 
        const std::initializer_list<token_class_t> expected
) const {
    (void) fprintf(stderr, "expected ");

    for (auto token : expected) {
        (void) fprintf(
            stderr,
            "%s ",
            tokenTypeToString(token).c_str()
        );
    }

    (void) fprintf(
        stderr, 
        "got %s at %s line %d column %d\n", 
        actual.lexeme.c_str(),
        actual.file.c_str(),
        actual.line,
        actual.column
    );

    exit(EXIT_FAILURE);
}

std::shared_ptr<SymbolTable> Parser::currentScope() {
    return this->scopes.top();
}

void Parser::enterNewScope() {
    std::shared_ptr<SymbolTable> newScope 
        = std::make_shared<SymbolTable>(this->scopes.top());
    this->scopes.push(newScope);
}

void Parser::exitOldScope() {
    ASSERT(this->scopes.size() != 1);
    this->scopes.pop();
}
