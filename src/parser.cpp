/**
 *  CPSC 323 Compilers and Languages
 * 
 *  Dalton Caron, Teaching Associate
 *  dcaron@fullerton.edu, +1 949-616-2699
 *  Department of Computer Science
 */
#include <parser.h>

Parser::Parser(const token_stream_t &tokens) 
: tokens(tokens) {}

Parser::~Parser() {}

ParseTree<std::string> Parser::parse() {
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

PTPtr<std::string> Parser::parseProgram() {
    PTPtr<std::string> programNode = 
        std::make_shared<PTNode<std::string>>("program");
    
    programNode->addChild(this->parseBlock());

    this->tryMatchTerminal(this->getNextToken(), PERIOD, programNode);
    return programNode;
}

PTPtr<std::string> Parser::parseBlock() {
    PTPtr<std::string> blockNode = 
        std::make_shared<PTNode<std::string>>("block");

    token_t token = this->peekNextToken();

    if (token.type == CONST_KEYWORD) {
        blockNode->addChild(this->parseConstDeclarations());
        token = this->peekNextToken();    
    }

    if (token.type == VAR_KEYWORD) {
        blockNode->addChild(this->parseVarDeclarations());
        token = this->peekNextToken();
    }

    if (token.type == PROCEDURE_KEYWORD) {
        // Procedures can be parsed 0 or more times.
        while (this->peekNextToken().type == PROCEDURE_KEYWORD) {
            blockNode->addChild(this->parseProcedure());
        }
    }

    blockNode->addChild(this->parseStatement());
    return blockNode;
}

PTPtr<std::string> Parser::parseConstDeclarations() {
    PTPtr<std::string> constDeclNode = 
        std::make_shared<PTNode<std::string>>("const_declaration");

    this->tryMatchTerminal(this->getNextToken(), CONST_KEYWORD, constDeclNode);

    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER, constDeclNode);

    this->tryMatchTerminal(this->getNextToken(), EQUALS, constDeclNode);

    token_t number = this->getNextToken();
    this->tryMatchTerminal(number, NUMBER_LITERAL, constDeclNode);
    
    // There can be zero or more constant items separated by a comma.
    while (this->peekNextToken().type == COMMA) {
        constDeclNode->addChild(this->parseConstDeclarationList());
    }

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON, constDeclNode);

    return constDeclNode;
}

PTPtr<std::string> Parser::parseConstDeclarationList() {
    PTPtr<std::string> constDeclListNode = 
        std::make_shared<PTNode<std::string>>("const_declaration_list");

    this->tryMatchTerminal(this->getNextToken(), COMMA, constDeclListNode);

    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER, constDeclListNode);
    this->tryMatchTerminal(this->getNextToken(), EQUALS, constDeclListNode);

    token_t number = this->getNextToken();
    this->tryMatchTerminal(number, NUMBER_LITERAL, constDeclListNode);

    return constDeclListNode;
}

PTPtr<std::string> Parser::parseVarDeclarations() {
    PTPtr<std::string> varDeclNode = 
        std::make_shared<PTNode<std::string>>("var_declaration");

    this->tryMatchTerminal(this->getNextToken(), VAR_KEYWORD, varDeclNode);
    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER, varDeclNode);

    // There can be zero or more variable names separated by a comma.
    while (this->peekNextToken().type == COMMA) {
        varDeclNode->addChild(this->parseVarDeclarationList());
    }

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON, varDeclNode);

    return varDeclNode;
}

PTPtr<std::string> Parser::parseVarDeclarationList() {
    PTPtr<std::string> varDeclNodeList = 
        std::make_shared<PTNode<std::string>>("var_declaration_list");
    
    this->tryMatchTerminal(this->getNextToken(), COMMA, varDeclNodeList);

    const token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER, varDeclNodeList);

    return varDeclNodeList;
}

PTPtr<std::string> Parser::parseProcedure() {
    PTPtr<std::string> procedure = 
        std::make_shared<PTNode<std::string>>("procedure");
    
    this->tryMatchTerminal(this->getNextToken(), PROCEDURE_KEYWORD, procedure);

    const token_t procedure_id = this->getNextToken();
    this->tryMatchTerminal(procedure_id, IDENTIFIER, procedure);
    this->tryMatchTerminal(this->getNextToken(), SEMICOLON, procedure);

    procedure->addChild(this->parseBlock());

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON, procedure);

    return procedure;
}

PTPtr<std::string> Parser::parseStatement() {
    PTPtr<std::string> statementNode = 
        std::make_shared<PTNode<std::string>>("statement");

    const token_t token = this->peekNextToken();
    switch (token.type) {
        case IDENTIFIER: {
            const token_t identifier = this->getNextToken();
            this->tryMatchTerminal(identifier, IDENTIFIER, statementNode);
            this->tryMatchTerminal(this->getNextToken(), DEFINE_EQUALS, statementNode);
            statementNode->addChild(this->parseExpression());
            break;
        }
        case CALL_KEYWORD: {
            this->tryMatchTerminal(this->getNextToken(), CALL_KEYWORD, statementNode);
            const token_t procedureID = this->getNextToken();
            this->tryMatchTerminal(procedureID, IDENTIFIER, statementNode);
            break;
        }
        case WRITE_OP: {
            this->tryMatchTerminal(this->getNextToken(), WRITE_OP, statementNode);
            statementNode->addChild(this->parseExpression());

            break;
        }
        case READ_OP: {
            this->tryMatchTerminal(this->getNextToken(), READ_OP, statementNode);
            const token_t identifier = this->getNextToken();
            this->tryMatchTerminal(identifier, IDENTIFIER, statementNode);

            break;
        }
        case BEGIN_KEYWORD: {
            this->tryMatchTerminal(this->getNextToken(), BEGIN_KEYWORD, statementNode);

            statementNode->addChild(this->parseStatement());
            while (this->peekNextToken().type == SEMICOLON) {
                this->tryMatchTerminal(this->getNextToken(), SEMICOLON, statementNode);
                statementNode->addChild(this->parseStatement());
            }
            this->tryMatchTerminal(this->getNextToken(), END_KEYWORD, statementNode);

            break;
        }
        case IF_KEYWORD: {
            this->tryMatchTerminal(this->getNextToken(), IF_KEYWORD, statementNode);
            statementNode->addChild(this->parseCondition());

            this->tryMatchTerminal(this->getNextToken(), THEN_KEYWORD, statementNode);
            statementNode->addChild(this->parseStatement());

            break;
        }
        case WHILE_KEYWORD: {
            this->tryMatchTerminal(this->getNextToken(), WHILE_KEYWORD, statementNode);
            statementNode->addChild(this->parseCondition());

            this->tryMatchTerminal(this->getNextToken(), DO_KEYWORD, statementNode);
            statementNode->addChild(this->parseStatement());

            break;
        }
        default:
            // Statements can be nothing (epsilon).
            statementNode->addChild("\x03\xB5");
            break;
    }

    return statementNode;
}

PTPtr<std::string> Parser::parseCondition() {
    PTPtr<std::string> conditionNode = 
        std::make_shared<PTNode<std::string>>("condition");

    const token_t token = this->peekNextToken();
    switch (token.type) {
        case ODD_OP: {
            this->tryMatchTerminal(this->getNextToken(), ODD_OP, conditionNode);
            conditionNode->addChild(this->parseExpression());
        }
        default: {
            conditionNode->addChild(this->parseExpression());
            const token_t cmpOp = this->getNextToken();
            this->tryMatchTerminal(cmpOp, {COMPARE_OP, EQUALS}, conditionNode);
            conditionNode->addChild(this->parseExpression());
        }
    }

    return conditionNode;
}

PTPtr<std::string> Parser::parseExpression() {
    PTPtr<std::string> expressionNode = 
        std::make_shared<PTNode<std::string>>("expression");

    // Optional plus and minus.
    const token_t unaryOp = this->peekNextToken();
    if (unaryOp.type == ADD_OP) {
        this->tryMatchTerminal(this->getNextToken(), ADD_OP, expressionNode);
    }

    expressionNode->addChild(this->parseTerm());

    // Read zero or more operator and term pairs.
    while (this->peekNextToken().type == ADD_OP) {
        const token_t op = this->getNextToken(); 
        this->tryMatchTerminal(op, ADD_OP, expressionNode);
        expressionNode->addChild(this->parseTerm());
    }

    return expressionNode;
}

PTPtr<std::string> Parser::parseTerm() {
    PTPtr<std::string> termNode = std::make_shared<PTNode<std::string>>("term");

    termNode->addChild(this->parseFactor());

    while (this->peekNextToken().type == MUL_OP) {
        const token_t op = this->getNextToken();
        this->tryMatchTerminal(op, MUL_OP, termNode);
        termNode->addChild(this->parseFactor());
    }

    return termNode;
}

PTPtr<std::string> Parser::parseFactor() {
    PTPtr<std::string> factorNode = 
        std::make_shared<PTNode<std::string>>("factor");

    const token_t token = this->peekNextToken();
    switch (token.type) {
        case IDENTIFIER:
            this->tryMatchTerminal(this->getNextToken(), IDENTIFIER, factorNode);
            break;
        case NUMBER_LITERAL:
            this->tryMatchTerminal(this->getNextToken(), NUMBER_LITERAL, factorNode);
            break;
        case LEFT_PAREN: {
            this->tryMatchTerminal(this->getNextToken(), LEFT_PAREN, factorNode);
            factorNode->addChild(this->parseExpression());
            this->tryMatchTerminal(this->getNextToken(), RIGHT_PAREN, factorNode);
            break;
        }
        default:
            raiseMismatchError(token, {
                IDENTIFIER,
                NUMBER_LITERAL,
                LEFT_PAREN
            });
    }

    return factorNode;
}

void Parser::tryMatchTerminal(
    const token_t &actual,
    const token_class_t expected,
    PTPtr<std::string> node
) const {
    if (actual.type != expected) {
        raiseMismatchError(actual, expected);
    }
    node->addChild(actual.lexeme);
}

void Parser::tryMatchTerminal(
    const token_t &actual, 
    const std::initializer_list<token_class_t> expected,
    PTPtr<std::string> node
) const {
    for (const token_class_t type : expected) {
        if (type == actual.type) {
            node->addChild(actual.lexeme);
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
