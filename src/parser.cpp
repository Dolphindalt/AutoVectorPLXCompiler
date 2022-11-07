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
    
    ASTHead programBlock = this->parseRootBlock();

    this->tryMatchTerminal(this->getNextToken(), PERIOD);
    return programBlock;
}

ASTHead Parser::parseRootBlock() {
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

    EASTPtr statement = this->parseStatement();
    if (statement != nullptr) {
        blockNode->addChild(statement);
        statement = nullptr;
    }
    
    return blockNode;
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

    EASTPtr statement = this->parseStatement();
    if (statement != nullptr) {
        blockNode->addChild(statement);
        statement = nullptr;
    }
    return blockNode;
}

void Parser::parseConstDeclarations(std::shared_ptr<ExprListAST> parent) {
    this->tryMatchTerminal(this->getNextToken(), CONST_KEYWORD);

    type_t type;
    bool is_array;
    unsigned int array_size;
    this->parseType(&type, &is_array, &array_size);

    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);
    this->tryMatchTerminal(this->getNextToken(), EQUALS);

    EASTPtr number = this->parseNumber();

    st_entry_t type_info;
    type_info.entry_type = ST_VARIABLE;
    type_info.variable.isConstant = true;
    type_info.variable.isAssigned = true;
    type_info.variable.isArray = is_array;
    type_info.variable.arraySize = array_size;
    type_info.variable.type = type;
    type_info.token = identifier;

    EASTPtr variable = std::make_shared<VariableAST>(
        identifier.lexeme, 
        this->currentScope(), 
        type_info.variable.type, 
        type_info.variable.isArray
    );

    this->currentScope()->insert(identifier.lexeme, type_info);

    EASTPtr binAST 
        = std::make_shared<BinaryExprAST>(
            ASSIGNMENT, 
            variable, 
            number
        );
    parent->addChild(binAST);
    
    // There can be zero or more constant items separated by a comma.
    while (this->peekNextToken().type == COMMA) {
        this->parseConstDeclarationList(parent);
    }

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON);
}

void Parser::parseConstDeclarationList(std::shared_ptr<ExprListAST> parent) {
    this->tryMatchTerminal(this->getNextToken(), COMMA);

    type_t type;
    bool is_array;
    unsigned int array_size;
    this->parseType(&type, &is_array, &array_size);

    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);
    this->tryMatchTerminal(this->getNextToken(), EQUALS);

    EASTPtr number = this->parseNumber();

    st_entry_t type_info;
    type_info.entry_type = ST_VARIABLE;
    type_info.variable.isConstant = true;
    type_info.variable.isAssigned = true;
    type_info.variable.isArray = is_array;
    type_info.variable.arraySize = array_size;
    type_info.variable.type = type;
    type_info.token = identifier;

    EASTPtr variable = std::make_shared<VariableAST>(
        identifier.lexeme,
        this->currentScope(),
        type_info.variable.type,
        type_info.variable.isArray
    );

    this->currentScope()->insert(identifier.lexeme, type_info);

    EASTPtr binAST 
        = std::make_shared<BinaryExprAST>(
            ASSIGNMENT, variable, 
            number
        );
    parent->addChild(binAST);
}

void Parser::parseVarDeclarations(std::shared_ptr<ExprListAST> parent) {
    this->tryMatchTerminal(this->getNextToken(), VAR_KEYWORD);

    type_t type;
    bool is_array;
    unsigned int array_size;
    this->parseType(&type, &is_array, &array_size);

    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);

    st_entry_t type_info;
    type_info.entry_type = ST_VARIABLE;
    type_info.variable.isConstant = false;
    type_info.variable.isAssigned = false;
    type_info.variable.isArray = is_array;
    type_info.variable.arraySize = array_size;
    type_info.variable.type = type;
    type_info.token = identifier;

    EASTPtr variable = std::make_shared<VariableAST>(
        identifier.lexeme, 
        this->currentScope(),
        type_info.variable.type,
        type_info.variable.isArray
    );

    this->currentScope()->insert(identifier.lexeme, type_info);

    EASTPtr declaration = std::make_shared<UnaryExprAST>(
        ASSIGNMENT, variable
    );
    parent->addChild(declaration);

    // There can be zero or more variable names separated by a comma.
    while (this->peekNextToken().type == COMMA) {
        this->parseVarDeclarationList(parent);
    }

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON);

}

void Parser::parseVarDeclarationList(std::shared_ptr<ExprListAST> parent) {    
    this->tryMatchTerminal(this->getNextToken(), COMMA);

    type_t type;
    bool is_array;
    unsigned int array_size;
    this->parseType(&type, &is_array, &array_size);

    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);

    st_entry_t type_info;
    type_info.entry_type = ST_VARIABLE;
    type_info.variable.isConstant = false;
    type_info.variable.isAssigned = false;
    type_info.variable.isArray = is_array;
    type_info.variable.arraySize = array_size;
    type_info.variable.type = type;
    type_info.token = identifier;

    EASTPtr variable = std::make_shared<VariableAST>(
        identifier.lexeme, 
        this->currentScope(),
        type_info.variable.type,
        type_info.variable.isArray
    );

    this->currentScope()->insert(identifier.lexeme, type_info);

    EASTPtr declaration = std::make_shared<UnaryExprAST>(
        ASSIGNMENT, variable
    );
    parent->addChild(declaration);
}

void Parser::parseProcedure(std::shared_ptr<ExprListAST> parent) {
    this->enterNewScope();

    this->tryMatchTerminal(this->getNextToken(), PROCEDURE_KEYWORD);

    token_t procedure_id = this->getNextToken();
    this->tryMatchTerminal(procedure_id, IDENTIFIER);

    // Parse all arguments.
    std::vector<std::shared_ptr<VariableAST>> arguments;
    token_t peek = this->peekNextToken();
    if (peek.type == LEFT_PAREN) {
        this->tryMatchTerminal(this->getNextToken(), LEFT_PAREN);
        arguments = this->parseArguments();
        this->tryMatchTerminal(this->getNextToken(), RIGHT_PAREN);
    }

    // Parse return type.
    std::shared_ptr<VariableAST> returnId = nullptr;
    peek = this->peekNextToken();
    // First set of type.
    if (peek.type == INT_NUMBER_LITERAL || peek.type == FLOAT_NUMBER_LITERAL) {
        type_t type;
        bool is_array;
        unsigned int array_size;
        this->parseType(&type, &is_array, &array_size);

        st_entry_t type_info;
        type_info.entry_type = ST_VARIABLE;
        type_info.variable.type = type;
        type_info.variable.isArray = is_array;
        type_info.variable.arraySize = array_size;
        type_info.variable.isAssigned = false;
        type_info.variable.isConstant = false;

        token_t ident = this->getNextToken();
        this->tryMatchTerminal(ident, IDENTIFIER);

        type_info.token = ident;

        this->currentScope()->insert(ident.lexeme, type_info);

        returnId = std::make_shared<VariableAST>(
            ident.lexeme, this->currentScope(), 
            type_info.variable.type, 
            type_info.variable.isArray
        );
    }

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON);

    // Parse block.
    std::shared_ptr<ExprListAST> blockBody = this->parseBlock();

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON);

    std::shared_ptr<Prototype> prototype 
        = std::make_shared<Prototype>(
            procedure_id.lexeme, 
            arguments, 
            returnId
        );

    this->exitOldScope();

    if (prototype->arguments.size() >= MAXIMUM_ARGUMENTS) {
        ERROR_LOG(
            "the maxium number of arguments is %d violated by %s",
            MAXIMUM_ARGUMENTS,
            prototype->name.c_str()
        );
        exit(EXIT_FAILURE);
    }

    st_entry_t func_info;
    func_info.entry_type = ST_FUNCTION;

    func_info.procedure.argumentsLength = prototype->arguments.size();

    for (uint8_t i = 0; i < func_info.procedure.argumentsLength; i++) {
        func_info.procedure.argumentTypes[i] = prototype->arguments.at(i)->type;
    }

    func_info.procedure.returnType = 
        (returnId != nullptr) ? returnId->type : VOID;

    this->currentScope()->insert(prototype->name, func_info);

    EASTPtr procedure = std::make_shared<ProcedureAST>(
        prototype, blockBody, this->currentScope()
    );

    parent->addChild(procedure);
}

std::vector<std::shared_ptr<VariableAST>> Parser::parseArguments() {
    
    std::vector<std::shared_ptr<VariableAST>> arguments;

    type_t type;
    bool is_array;
    unsigned int array_size;
    this->parseType(&type, &is_array, &array_size);

    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);

    st_entry_t type_info;
    type_info.entry_type = ST_VARIABLE;
    type_info.variable.isConstant = false;
    type_info.variable.isAssigned = true;
    type_info.variable.isArray = is_array;
    type_info.variable.arraySize = array_size;
    type_info.token = identifier;

    arguments.push_back(
        std::make_shared<VariableAST>(
            identifier.lexeme, 
            this->currentScope(),
            type_info.variable.type,
            type_info.variable.isArray
        )
    );

    this->currentScope()->insert(identifier.lexeme, type_info);

    while (this->peekNextToken().type == COMMA) {
        this->tryMatchTerminal(this->getNextToken(), COMMA);

        this->parseType(&type, &is_array, &array_size);
        type_info.variable.type = type;
        type_info.variable.isArray = is_array;
        type_info.variable.arraySize = array_size;

        identifier = this->getNextToken();
        this->tryMatchTerminal(identifier, IDENTIFIER);

        type_info.token = identifier;

        arguments.push_back(
            std::make_shared<VariableAST>(
                identifier.lexeme, 
                this->currentScope(),
                type_info.variable.type,
                type_info.variable.isArray
            )
        );

        this->currentScope()->insert(identifier.lexeme, type_info);
    }
    return arguments;
}

AST Parser::parseStatement() {

    const token_t token = this->peekNextToken();
    switch (token.type) {
        case IDENTIFIER: {
            EASTPtr lhs = this->parseVariable();

            this->tryMatchTerminal(this->getNextToken(), DEFINE_EQUALS);

            EASTPtr rhs = this->parseExpression();

            EASTPtr binExpr = std::make_shared<BinaryExprAST>(
                ASSIGNMENT, 
                lhs,
                rhs
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

            EASTPtr call = std::make_shared<CallExprAST>(
                procedureID.lexeme, arguments, this->currentScope()
            );

            return call;
        }
        case WRITE_OP: {
            this->tryMatchTerminal(this->getNextToken(), WRITE_OP);
            EASTPtr toWrite = this->parseExpression();

            EASTPtr write = std::make_shared<UnaryExprAST>(
                WRITE, toWrite
            );

            return write;
        }
        case READ_OP: {
            this->tryMatchTerminal(this->getNextToken(), READ_OP);
            token_t identifier = this->getNextToken();
            this->tryMatchTerminal(identifier, IDENTIFIER);

            st_entry_t typeInfo;
            unsigned int level;
            this->currentScope()->lookup(token.lexeme, &level, &typeInfo);

            ASSERT(typeInfo.entry_type == ST_VARIABLE);

            EASTPtr toReadTo = std::make_shared<VariableAST>(
                identifier.lexeme, 
                this->currentScope(),
                typeInfo.variable.type,
                typeInfo.variable.isArray
            );

            EASTPtr read = std::make_shared<UnaryExprAST>(
                READ, toReadTo
            );

            return read;
        }
        case BEGIN_KEYWORD: {
            this->enterNewScope();

            this->tryMatchTerminal(this->getNextToken(), BEGIN_KEYWORD);

            std::shared_ptr<ExprListAST> exprList 
                = std::make_shared<ExprListAST>();

            EASTPtr statement = this->parseStatement();
            exprList->addChild(statement);

            while (this->peekNextToken().type == SEMICOLON) {
                this->tryMatchTerminal(this->getNextToken(), SEMICOLON);
                statement = this->parseStatement();
                if (statement != nullptr) {
                    exprList->addChild(statement);
                    statement = nullptr;
                }
            }
            this->tryMatchTerminal(this->getNextToken(), END_KEYWORD);

            this->exitOldScope();

            return exprList;
        }
        case IF_KEYWORD: {
            token_t if_token = this->getNextToken();
            this->tryMatchTerminal(if_token, IF_KEYWORD);
            EASTPtr condition = this->parseCondition();

            this->tryMatchTerminal(this->getNextToken(), THEN_KEYWORD);
            EASTPtr body = this->parseStatement();
            if (body == nullptr) {
                ERROR_LOG(
                    "if statement body cannot be empty "
                    "in %s at line %d column %d",
                    if_token.file.c_str(),
                    if_token.line,
                    if_token.column
                );
                exit(EXIT_FAILURE);
            }

            return std::make_shared<IfStatementAST>(
                condition, body
            );
        }
        case WHILE_KEYWORD: {
            token_t while_token = this->getNextToken();
            this->tryMatchTerminal(while_token, WHILE_KEYWORD);
            EASTPtr condition = this->parseCondition();

            this->tryMatchTerminal(this->getNextToken(), DO_KEYWORD);
            EASTPtr body = this->parseStatement();
            if (body == nullptr) {
                ERROR_LOG(
                    "while statement body cannot be empty "
                    "in %s at line %d column %d",
                    while_token.file.c_str(),
                    while_token.line,
                    while_token.column
                );
                exit(EXIT_FAILURE);
            }

            return std::make_shared<WhileStatementAST>(
                condition, body
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
            return std::make_shared<UnaryExprAST>(
                ODD, operand
            );
        }
        default: {
            EASTPtr lhs = this->parseExpression();
            const token_t cmpOp = this->getNextToken();
            this->tryMatchTerminal(cmpOp, {COMPARE_OP, EQUALS});
            operation_t op = cmpOpMap.at(cmpOp.lexeme);
            EASTPtr rhs = this->parseExpression();
            return std::make_shared<BinaryExprAST>(
                op, lhs, rhs
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
        lhs = std::make_shared<UnaryExprAST>(
            unary_op, lhs
        );
    }

    const token_t next = this->peekNextToken();
    if (next.type == ADD_OP) {
        operation_t op = cmpOpMap.at(next.lexeme);
        EASTPtr rhs = this->parseExpressionTail();
        return std::make_shared<BinaryExprAST>(
            op, lhs, rhs
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
        return std::make_shared<BinaryExprAST>(
            op, lhs, rhs
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
        return std::make_shared<BinaryExprAST>(
            op, lhs, rhs
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
        return std::make_shared<BinaryExprAST>(
            op, lhs, rhs
        );
    }

    return lhs;
}

AST Parser::parseFactor() {
    token_t token = this->peekNextToken();
    switch (token.type) {
        case IDENTIFIER: {
            return this->parseVariable();
        }
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
    typeInfo.entry_type = ST_LITERAL;
    typeInfo.token = next;

    switch (next.type) {
        case INT_NUMBER_LITERAL: {
            this->tryMatchTerminal(next, INT_NUMBER_LITERAL);
            uint64_t value = atoi(next.lexeme.c_str());
            typeInfo.literal.type = INT;
            memcpy(typeInfo.literal.value, &value, LITERAL_BYTE_LEN);
            number = std::make_shared<NumberAST>(
                next.lexeme,
                (void *)&value,
                this->currentScope(),
                INT
            );
            break;
        }
        case FLOAT_NUMBER_LITERAL: {
            this->tryMatchTerminal(next, FLOAT_NUMBER_LITERAL);
            double value = atof(next.lexeme.c_str());
            typeInfo.literal.type = FLOAT;
            memcpy(typeInfo.literal.value, &value, LITERAL_BYTE_LEN);
            number = std::make_shared<NumberAST>(
                next.lexeme,
                (void *)&value, 
                this->currentScope(),
                FLOAT
            );
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
        return std::make_shared<UnaryExprAST>(
            unary_op, 
            number
        );
    }

    return number;
}

AST Parser::parseVariable() {
    token_t ident = this->getNextToken(); 
    this->tryMatchTerminal(ident, IDENTIFIER);
    std::shared_ptr<VariableAST> var = std::make_unique<VariableAST>(
        ident.lexeme, this->currentScope(), UNKNOWN
    );

    if (this->peekNextToken().type == LEFT_SQUARE_BRACKET) {
        this->tryMatchTerminal(this->getNextToken(), LEFT_SQUARE_BRACKET);
        EASTPtr result = this->parseExpression();
        this->tryMatchTerminal(this->getNextToken(), RIGHT_SQUARE_BRACKET);
        return std::make_unique<ArrayIndexAST>(var, result, UNKNOWN);
    }

    return var;
}

void Parser::parseType(type_t *type, bool *is_array, unsigned int *array_size) {
    const token_t next = this->getNextToken();
    switch (next.type) {
        case INT_TYPE_KEYWORD:
            this->tryMatchTerminal(next, INT_TYPE_KEYWORD);
            *type = INT;
            break;
        case FLOAT_TYPE_KEYWORD:
            this->tryMatchTerminal(next, FLOAT_TYPE_KEYWORD);
            *type = FLOAT;
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
        *is_array = true;
        *array_size = atoi(int_literal.lexeme.c_str());
    }
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
        "expected %s got %s:%s "
        "at %s line %d column %d\n",
        tokenTypeToString(expected).c_str(),
        actual.lexeme.c_str(),
        tokenTypeToString(actual.type).c_str(),
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
