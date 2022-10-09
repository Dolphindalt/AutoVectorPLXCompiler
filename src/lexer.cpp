/**
 *  CPSC 323 Compilers and Languages
 * 
 *  Dalton Caron, Teaching Associate
 *  dcaron@fullerton.edu, +1 949-616-2699
 *  Department of Computer Science
 */
#include <lexer.h>

#include <fstream>
#include <sstream>
#include <algorithm>

std::string tokenTypeToString(const token_class_t type) {
    return type_to_string.at(type);
}

Scanner::Scanner(const std::string file_path) 
: file_path(file_path), current_column(1), current_line(1), 
    current_character(0) {
        std::ifstream infile(file_path);

        if (!infile.is_open()) {
            ERROR_LOG("Failed to open file %s", file_path.c_str());
            exit(EXIT_FAILURE);
        }

        std::stringstream buffer;
        buffer << infile.rdbuf();
        this->source = buffer.str();
    }

char Scanner::next() {
    if (this->current_character < this->source.length()) {
        const char next_character = this->source[this->current_character++];
        if (next_character == '\n') {
            this->current_line++;
            this->current_column = 1;
        } else {
            this->current_column++;
        }
        return next_character;
    } else {
        return 0;
    }
}

char Scanner::peek() const {
    if (this->current_character < this->source.length()) {
        return this->source[this->current_character];
    } else {
        return 0;
    }
}

ScanningTable::ScanningTable(const CsvReader &table) {
    // We read the table in column major order as to only call the 
    // map_string_to_char function once per column iteration.
    int current_column = 1;
    while (current_column < table.getColumns()) {

        const char current_symbol = 
            this->map_string_to_char(table.get(0, current_column));

        int current_row = 1;
        while (current_row < table.getRows()) {

            const state_t current_state = 
                atoi(table.get(current_row, 0).c_str());

            // This models the transition function ((state, symbol), state).
            const std::string next_state = table.get(current_row, current_column);

            // The map is only populated if there is a valid transition.
            if (!next_state.empty()) {

                this->state_symbol_to_state[std::make_pair(
                    current_state, current_symbol
                )] = atoi(next_state.c_str());

            }

            current_row++;
        }

        current_column++;
    }
} 

char ScanningTable::getNextState(const state_t state, const char symbol) {
    return this->state_symbol_to_state[std::make_pair(state, symbol)];
}

bool ScanningTable::containsNextState(const state_t state, const char symbol) {
    return this->state_symbol_to_state
        .count(std::make_pair(state, symbol)) > 0;
}

char ScanningTable::map_string_to_char(const std::string &str) const {
    // Map to special strings, otherwise take the first character.
    if (str == "0x0A" || str == "0x0a" || str == "\\n") {
        return '\n';
    } else if (str == "0x0D" || str == "0x0d" || str == "\\r") {
        return '\r';
    } else if (str == "0x20") {
        return ' ';
    } else if (str == "0x09" || str == "\\t") {
        return '\t';
    } else if (str == "comma" || str == "Comma") {
        return ',';
    } else {
        return str[0];
    }
}

TokenTable::TokenTable(const CsvReader &table) {
    // The token table has two columns:
    // 1. Accepting states and
    // 2. Token class numbers.

    // Read each row one by one.
    int row = 0;
    while (row < table.getRows()) {
        // Insert the state-token class pair into the token table map.
        const state_t final_state = atoi(table.get(row, 0).c_str());
        const token_class_t type_recognized = 
            (token_class_t) atoi(table.get(row, 1).c_str());
        this->state_to_token_type[final_state] = type_recognized;
        row++;
    }
}

token_class_t TokenTable::getTokenTypeFromFinalState(
    const state_t final_state
) {
    return this->state_to_token_type[final_state];
}

bool TokenTable::isStateFinal(const state_t state) const {
    return this->state_to_token_type.count(state) > 0;
}

Lexer::Lexer() 
: scanning_table(ScanningTable(CsvReader(this->scanning_table_path, ','))),
    token_table(TokenTable(CsvReader(this->token_table_path, ',')))
{}

Lexer::~Lexer() {}

token_stream_t Lexer::lex(const std::string &file_path) {
    token_stream_t token_stream;
    Scanner scanner = Scanner(file_path);
    token_t token;
    
    // While the scannr contains input, lex token by token.
    while (scanner.peek() != 0) {
        token = this->scan_token(scanner);
        if (token.type != WHITESPACE) {
            token_stream.push_back(token);
        }
    }

    // Add a token to denoate the end of the file.
    token.lexeme = "";
    token.type = END_OF_FILE;

    token_stream.push_back(token);

    return token_stream;
}

token_t Lexer::scan_token(Scanner &scanner) {

    std::string read_characters;
    char current_character;
    int current_state = 0;
    token_t to_return;

    to_return.line = scanner.getLine();
    to_return.column = scanner.getColumn();
    to_return.file = scanner.getFilePath();

    while (true) {
        current_character = scanner.peek();

        const Lexer::lexer_action_t action = 
            this->choose_action(current_state, current_character);

        switch (action) {

            case MOVE:
                current_state = 
                    this->scanning_table
                    .getNextState(current_state, current_character);
                read_characters += current_character;
                (void) scanner.next();
                break;

            case RECOGNIZE:
                to_return.lexeme = read_characters;
                to_return.type = 
                    this->token_table
                    .getTokenTypeFromFinalState(current_state);

                // If we recognize an identifier, check if it is a 
                // reserved word.
                if (to_return.type == IDENTIFIER) {
                    // Remember that reserved words is a set.
                    if (reserved_words.count(to_return.lexeme) > 0) {
                        to_return.type = reserved_words.at(to_return.lexeme);
                    }
                }

                return to_return;

            case ERROR:
                to_return.lexeme = read_characters;
                to_return.type = token_class_t::ERROR;
                (void) scanner.next();
                return to_return;

            default:
                ERROR_LOGV("Invalid lexer action.");
                exit(EXIT_FAILURE);
        }
    }
}

Lexer::lexer_action_t Lexer::choose_action(
    const state_t current_state, 
    const char current_character
) {
    // If it is possible to move to a next state, make the move.
    if (this->scanning_table
        .containsNextState(current_state, current_character)) {
            return MOVE;
    // Try to recognize the token.
    } else if (this->token_table.isStateFinal(current_state) || 
        current_character == '\0') {
            return RECOGNIZE;
    // The machine is in an error state.
    } else {
        return ERROR;
    }
}