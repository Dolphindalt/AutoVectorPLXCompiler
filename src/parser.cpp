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
    this->parseProgram();
    return this->parse_tree;
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

void Parser::parseProgram() {
    this->parseBlock();
    this->parseStatement();

    this->tryMatchTerminal(this->getNextToken(), PERIOD);
}

void Parser::parseBlock() {

    token_t token = this->peekNextToken();

    if (token.type == CONST_KEYWORD) {
        this->parseConstDeclarations();
        token = this->peekNextToken();    
    }

    if (token.type == VAR_KEYWORD) {
        this->parseVarDeclarations();
        token = this->peekNextToken();
    }

    if (token.type == PROCEDURE_KEYWORD) {
        // Procedures can be parsed 0 or more times.
        while (this->peekNextToken().type == PROCEDURE_KEYWORD) {
            this->parseProcedure();
        }
    }
}

void Parser::parseConstDeclarations() {
    this->tryMatchTerminal(this->getNextToken(), CONST_KEYWORD);

    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);

    this->tryMatchTerminal(this->getNextToken(), EQUALS);

    token_t number = this->getNextToken();
    this->tryMatchTerminal(number, NUMBER_LITERAL);
    
    // There can be zero or more constant items separated by a comma.
    while (this->peekNextToken().type == COMMA) {
        this->parseConstDeclarationList();
    }

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON);
}

void Parser::parseConstDeclarationList() {
    this->tryMatchTerminal(this->getNextToken(), COMMA);
    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);
    this->tryMatchTerminal(this->getNextToken(), EQUALS);
    token_t number = this->getNextToken();
    this->tryMatchTerminal(number, NUMBER_LITERAL);
}

void Parser::parseVarDeclarations() {
    this->tryMatchTerminal(this->getNextToken(), VAR_KEYWORD);
    token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);

    // There can be zero or more variable names separated by a comma.
    while (this->peekNextToken().type == COMMA) {
        this->parseVarDeclarationList();
    }

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON);
}

void Parser::parseVarDeclarationList() {
    this->tryMatchTerminal(this->getNextToken(), COMMA);
    const token_t identifier = this->getNextToken();
    this->tryMatchTerminal(identifier, IDENTIFIER);
}

void Parser::parseProcedure() {
    
    this->tryMatchTerminal(this->getNextToken(), PROCEDURE_KEYWORD);
    const token_t procedure_id = this->getNextToken();
    this->tryMatchTerminal(procedure_id, IDENTIFIER);
    this->tryMatchTerminal(this->getNextToken(), SEMICOLON);

    this->parseBlock();
    this->parseStatement();

    this->tryMatchTerminal(this->getNextToken(), SEMICOLON);
}

void Parser::parseStatement() {
    const token_t token = this->peekNextToken();
    switch (token.type) {
        case IDENTIFIER: {
            const token_t identifier = this->getNextToken();
            this->tryMatchTerminal(identifier, IDENTIFIER);
            this->tryMatchTerminal(this->getNextToken(), DEFINE_EQUALS);
            this->parseExpression();
            break;
        }
        case CALL_KEYWORD: {
            this->tryMatchTerminal(this->getNextToken(), CALL_KEYWORD);
            const token_t procedureID = this->getNextToken();
            this->tryMatchTerminal(procedureID, IDENTIFIER);
            break;
        }
        case WRITE_OP: {
            this->tryMatchTerminal(this->getNextToken(), WRITE_OP);
            const token_t identifier = this->getNextToken();
            this->tryMatchTerminal(identifier, IDENTIFIER);

            break;
        }
        case READ_OP: {
            this->tryMatchTerminal(this->getNextToken(), READ_OP);
            const token_t identifier = this->getNextToken();
            this->tryMatchTerminal(identifier, IDENTIFIER);

            break;
        }
        case BEGIN_KEYWORD:
            this->tryMatchTerminal(this->getNextToken(), BEGIN_KEYWORD);

            this->parseStatement();
            while (this->peekNextToken().type == SEMICOLON) {
                this->tryMatchTerminal(this->getNextToken(), SEMICOLON);
                this->parseStatement();
            }
            this->tryMatchTerminal(this->getNextToken(), END_KEYWORD);

            break;
        case IF_KEYWORD: {
            this->tryMatchTerminal(this->getNextToken(), IF_KEYWORD);
            this->parseCondition();
            this->tryMatchTerminal(this->getNextToken(), THEN_KEYWORD);
            this->parseStatement();
            break;
        }
        case WHILE_KEYWORD: {
            this->tryMatchTerminal(this->getNextToken(), WHILE_KEYWORD);
            this->parseCondition();
            this->tryMatchTerminal(this->getNextToken(), DO_KEYWORD);
            this->parseStatement();
            break;
        }
        default:
            // Statements can be nothing (epsilon).
            break;
    }
}

void Parser::parseCondition() {
    const token_t token = this->peekNextToken();
    switch (token.type) {
        case ODD_OP: {
            this->tryMatchTerminal(this->getNextToken(), ODD_OP);
            this->parseExpression();
        }
        default: {
            this->parseExpression();
            const token_t cmpOp = this->getNextToken();
            this->tryMatchTerminal(cmpOp, {COMPARE_OP, EQUALS});
            this->parseExpression();
        }
    }
}

void Parser::parseExpression() {
    // Optional plus and minus.
    const token_t unaryOp = this->peekNextToken();
    if (unaryOp.type == ADD_OP) {
        this->tryMatchTerminal(this->getNextToken(), ADD_OP);
    }

    this->parseTerm();

    // Read zero or more operator and term pairs.
    while (this->peekNextToken().type == ADD_OP) {
        const token_t op = this->getNextToken(); 
        this->tryMatchTerminal(op, ADD_OP);
        this->parseTerm();
    }
}

void Parser::parseTerm() {
    this->parseFactor();
    while (this->peekNextToken().type == MUL_OP) {
        const token_t op = this->getNextToken();
        this->tryMatchTerminal(op, MUL_OP);
        this->parseFactor();
    }
}

void Parser::parseFactor() {
    const token_t token = this->peekNextToken();
    switch (token.type) {
        case IDENTIFIER:
            this->tryMatchTerminal(this->getNextToken(), IDENTIFIER);
            break;
        case NUMBER_LITERAL:
            this->tryMatchTerminal(this->getNextToken(), NUMBER_LITERAL);
            break;
        case LEFT_PAREN: {
            this->tryMatchTerminal(this->getNextToken(), LEFT_PAREN);
            this->parseExpression();
            this->tryMatchTerminal(this->getNextToken(), RIGHT_PAREN);
            break;
        }
        default:
            raiseMismatchError(token, {
                IDENTIFIER,
                NUMBER_LITERAL,
                LEFT_PAREN
            });
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
