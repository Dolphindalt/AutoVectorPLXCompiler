/**
 * The lexer.h file is responsible for classes and functions related to the 
 * lexical analysis phase of the compiler. Lexical analysis is the process in 
 * which the input source code is tokenized to be used as the input to 
 * syntax analysis. 
 * 
 * @file lexer.h
 * @author Dalton Caron
 */
#ifndef LEXER_H__
#define LEXER_H__

#include <string>
#include <vector>
#include <map>
#include <utilities.h>

// A state is represented as an integer, aliased to this state type.
typedef int state_t;

/**
 * The token class represents the various classes of lexemes that can be 
 * encountered in the source code. Each lexeme will be tagged with one of 
 * these classes. These classes will be important later during parsing, but 
 * the classes also let us know what kind of lexeme we have read.
 */
typedef enum token_class {
    ERROR = 0,
    IDENTIFIER = 1,
    CONST_KEYWORD = 2,
    INT_NUMBER_LITERAL = 3,
    COMMA = 4,
    DEFINE_EQUALS = 5,
    EQUALS = 6,
    LEFT_PAREN = 7,
    RIGHT_PAREN = 8,
    ODD_OP = 9,
    END_KEYWORD = 10,
    BEGIN_KEYWORD = 11,
    CALL_KEYWORD = 12,
    SEMICOLON = 13,
    PROCEDURE_KEYWORD = 14,
    IF_KEYWORD = 15,
    THEN_KEYWORD = 16,
    WHILE_KEYWORD = 17,
    DO_KEYWORD = 18,
    ADD_OP = 19,
    MUL_OP = 20,
    COMPARE_OP = 21,
    VAR_KEYWORD = 22,
    WRITE_OP = 23,
    READ_OP = 24,
    PERIOD = 25,
    END_OF_FILE = 26,
    WHITESPACE = 27,
    FLOAT_NUMBER_LITERAL = 28,
    LEFT_SQUARE_BRACKET = 29,
    RIGHT_SQUARE_BRACKET = 30,
    INT_TYPE_KEYWORD = 31,
    FLOAT_TYPE_KEYWORD = 32
} token_class_t;

#define LAZY_TYPE_MAP(type) { type, #type }

static std::map<token_class_t, std::string> type_to_string = {
    LAZY_TYPE_MAP(ERROR),
    LAZY_TYPE_MAP(IDENTIFIER),
    LAZY_TYPE_MAP(CONST_KEYWORD),
    LAZY_TYPE_MAP(INT_NUMBER_LITERAL),
    LAZY_TYPE_MAP(COMMA),
    LAZY_TYPE_MAP(DEFINE_EQUALS),
    LAZY_TYPE_MAP(EQUALS),
    LAZY_TYPE_MAP(LEFT_PAREN),
    LAZY_TYPE_MAP(RIGHT_PAREN),
    LAZY_TYPE_MAP(ODD_OP),
    LAZY_TYPE_MAP(END_KEYWORD),
    LAZY_TYPE_MAP(BEGIN_KEYWORD),
    LAZY_TYPE_MAP(CALL_KEYWORD),
    LAZY_TYPE_MAP(SEMICOLON),
    LAZY_TYPE_MAP(PROCEDURE_KEYWORD),
    LAZY_TYPE_MAP(IF_KEYWORD),
    LAZY_TYPE_MAP(THEN_KEYWORD),
    LAZY_TYPE_MAP(WHILE_KEYWORD),
    LAZY_TYPE_MAP(DO_KEYWORD),
    LAZY_TYPE_MAP(ADD_OP),
    LAZY_TYPE_MAP(MUL_OP),
    LAZY_TYPE_MAP(COMPARE_OP),
    LAZY_TYPE_MAP(VAR_KEYWORD),
    LAZY_TYPE_MAP(WRITE_OP),
    LAZY_TYPE_MAP(READ_OP),
    LAZY_TYPE_MAP(PERIOD),
    LAZY_TYPE_MAP(END_OF_FILE),
    LAZY_TYPE_MAP(WHITESPACE),
    LAZY_TYPE_MAP(FLOAT_NUMBER_LITERAL),
    LAZY_TYPE_MAP(LEFT_SQUARE_BRACKET),
    LAZY_TYPE_MAP(RIGHT_SQUARE_BRACKET),
    LAZY_TYPE_MAP(INT_TYPE_KEYWORD),
    LAZY_TYPE_MAP(FLOAT_TYPE_KEYWORD)
};

/**
 * Converts a token class enum into a string counterpart for printing purposes.
 * I do not like this function either, but you should want to print.
 * 
 * @param type The token class to convert to a string.
 * @return String of the token class.
 */
std::string tokenTypeToString(const token_class_t type);

// Reserved words for the language. 
static std::map<std::string, token_class_t> reserved_words = {
    {"const", CONST_KEYWORD},
    {"end", END_KEYWORD},
    {"begin", BEGIN_KEYWORD},
    {"call", CALL_KEYWORD},
    {"procedure", PROCEDURE_KEYWORD},
    {"if", IF_KEYWORD},
    {"then", THEN_KEYWORD},
    {"while", WHILE_KEYWORD},
    {"do", DO_KEYWORD},
    {"var", VAR_KEYWORD},
    {"int", INT_TYPE_KEYWORD},
    {"float", FLOAT_TYPE_KEYWORD}
};

/** 
 * Stores a lexeme scanned and the information associated with the lexeme.
 * The file, line, and column are for printing error messages with the 
 * location of the lexeme.
 */
typedef struct token {
public:
    token_class_t type;
    std::string lexeme;
    std::string file;
    int line;
    int column;
} token_t;

/** The Scanner takes an input file and reads it character by character. */
class Scanner {
public:
    /**
     * Initializes a Scanner with the input being the contents of the provided
     * file.
     * 
     * @param file_path The file path of the file to scan.
     */
    Scanner(const std::string file_path);

    /**
     * Returns the next character from the input file and advances the character marker.
     * Also handles moving the current line and current column file markers.
     * 
     * @return The next character.
     */
    char next();

    /**
     * Returns the next character from the input file without advancing the
     * character marker.
     * 
     * @return The next character.
     */
    char peek() const;

    /**
     * @return The current column within the source file.
     */
    int getColumn() const { return this->current_column; };

    /**
     * @return The current line within the source file.
     */
    int getLine() const { return this->current_line; };

    /**
     * @return The path of the current file.
     */
    std::string getFilePath() const { return this->file_path; };

private:
    // The path of the file being scanned.
    std::string file_path;
    // The column the current character is located.
    int current_column;
    // The line where the current character is located.
    int current_line;
    // The location of the current character in the file.
    unsigned int current_character;
    // The contents of the input file as a string.
    std::string source;
};

/**
 * The scanning table represents a deterministic finite state machine and is 
 * used to recognize tokens from the input source code. This is associated with 
 * a partner TokenTable.
 */
class ScanningTable {
public:
    /**
     * Builds the scanning table from a CSV Reader. The CSV table should 
     * have a first row that contains single characters from the source code.
     * The first column must contain integer states. The cells correspond to 
     * which state to transition to from the row state and column character.
     * See the ../test/scanning_table.csv for an example table.
     * 
     * @param table The CSV Reader that will be converted into a 
     * scanning table.
     */
    ScanningTable(const CsvReader &table);

    /**
     * Returns the state transition that corresponds with given state and 
     * symbol. 
     * 
     * @param state The state to transition from.
     * @param symbol The symbol to transition with.
     * @return The state to transition to.
     */
    char getNextState(const state_t state, const char symbol);

    /**
     * Checks if a transition exists for a given state and symbol.
     * 
     * @param state The state to transition from.
     * @param symbol The symbol to attempt to transition with.
     * @return True if a transition exists; else false.
     */
    bool containsNextState(const state_t state, const char symbol);
private:
    /**
     * The table assumes that every string in the first row is a single 
     * character. There are some exceptions. This function will convert 
     * special string codes into whitespace characters.
     * 0x0A -> newline \n
     * 0x0D -> carrige return \r
     * 0x20 -> space
     * 0x09 -> tab \t
     * comma -> comma ,
     * Any other string -> Into the first character
     * 
     * @param str String to convert into the first character of the string.
     * @return Converted character.
     */
    char map_string_to_char(const std::string &str) const;

    // A map that models a FSM state transition.
    // Maps a state and symbol to another state.
    std::map<std::pair<state_t, char>, state_t> state_symbol_to_state;
};

/** 
 * Represents a table that denotes what final states are associated with 
 * a token class. This is associated with a partner ScanningTable.
 */
class TokenTable {
public:
    /**
     * Builds the token table from the CSV Reader. The CSV Reader must contain
     * a two columns: the first column contains the final states of the 
     * associated DFSM, the second column contains a number that corresponds 
     * to a token_class_t enumeration.
     * 
     * @param table The CSV Reader that will be converted into a TokenTable.
     */
    TokenTable(const CsvReader &table);

    /**
     * Returns the token class that is recognized by the given final state.
     * 
     * @param final_state A final state from the associated DFSM.
     */
    token_class_t getTokenTypeFromFinalState(const state_t final_state);

    /**
     * Looks up in the token table to check if the provided state final or 
     * not.
     * 
     * @param state_t A state from the associated DFSM.
     * @return True if the provided state is final; false otherwise.
     */
    bool isStateFinal(const state_t state) const;
private:
    // Maps a final state to a token class.
    std::map<state_t, token_class_t> state_to_token_type;
};

// Aliases a vector of token types into a token stream.
typedef std::vector<token_t> token_stream_t;

/**
 * The Lexer performs lexical analysis upon an input source code.
 */
class Lexer {
public:
    /** Initializes the Lexer. */
    Lexer();

    virtual ~Lexer();

    /**
     * Performs a lexical analysis of the provided source code. The result is 
     * a sequence of lexemes denoted as a token stream.
     * 
     * @param source_code Path to the source code file.
     * @return A vector of token_t representing all of the tokens lexed from 
     * the source code.
     */
    token_stream_t lex(const std::string &file_path);

    /**
     * Scans a single token from the source code.
     * 
     * @param scanner Scanner of the source code.
     * @return A single lexed token.
     */
    token_t scan_token(Scanner &scanner);
private:
    // Represents the 3 choices to perform in a DFSM state.
    typedef enum lexer_action { MOVE, RECOGNIZE, ERROR } lexer_action_t;

    /**
     * Given a state and symbol, returns the DFSM action.
     * 
     * @param current_state The current state of the DFSM.
     * @param current_character The latest character read from the scanner.
     * @return What action the DFSM should perform. 
     */
    Lexer::lexer_action_t choose_action(
        const int current_state, 
        const char current_character
    );

    // Path of the scanning table to use.
    const std::string scanning_table_path = "../tables/scanning_table.csv";
    // Path of the token table to use.
    const std::string token_table_path = "../tables/token_table.csv";

    ScanningTable scanning_table;

    TokenTable token_table;
};

#endif