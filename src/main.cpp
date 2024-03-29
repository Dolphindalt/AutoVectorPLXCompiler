/**
 *  CPSC 323 Compilers and Languages
 * 
 *  Dalton Caron, Teaching Associate
 *  dcaron@fullerton.edu, +1 949-616-2699
 *  Department of Computer Science
 */
#include <cstdio>
#include <argp.h>
#include <lexer.h>
#include <parser.h>
#include <past.h>
#include <3ac.h>
#include <optimizer/optimizer.h>
//#include <codegen/asm_generator.h>
#include <codegen2/code_generator.h>

const char *argp_program_version = "Dalton\'s Toy Compiler";
const char *argp_program_bug_address = "dpcaron@csu.fullerton.edu";
static char doc[] = "A compiler program for demonstrating an optimizer.";
static char args_doc[] = "<source code file> [-v]";
static struct argp_option options[] = {
    {"vectorize", 'v', 0, 0, 
        "Boolean flag for enabling automatic vectorization"},
    { 0 }
};

/** Program input arguments structure. */
struct arguments {
    char *args[1];
    bool vectorize;
};

struct arguments arguments;

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = (struct arguments *) state->input;
    switch (key) {
        case 'v':
            arguments->vectorize = true;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 1) {
                // To many arguments.
                argp_usage(state);
            }
            arguments->args[state->arg_num] = arg;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 1) {
                // Not enough arguments.
                argp_usage(state);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

bool AUTOMATIC_VECTORIZATION_ENABLED = false;

int main(int argc, char *argv[]) {

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    char *source_file = arguments.args[0];
    AUTOMATIC_VECTORIZATION_ENABLED = arguments.vectorize;

    if (source_file == NULL) {
        (void) printf("Please provide a source file.\n");
        return EXIT_SUCCESS;
    }

    Lexer lexer;
    const token_stream_t tokens = lexer.lex(source_file);

    Parser parser(tokens);

    AST ast = parser.parse();

    ExprAST::treeTraversal(ast, [](EASTPtr parent) {
        parent->typeChecker();
    });

    TACGenerator tacGenerator;
    std::vector<tac_line_t> tacCode;
    ast->generateCode(tacGenerator, tacCode);
    ast = nullptr;

    INFO_LOG("TAC Code before optimizer");
    for (const tac_line_t &inst : tacCode) {
        INFO_LOG("%s", TACGenerator::tacLineToString(inst).c_str());
    }

    Optimizer optimizer(tacCode);

    //AssemblyGenerator generator;
    //generator.generateAssembly(optimizer.getBlocks());
    CodeGenerator generator;
    generator.generate(optimizer.getBlocks());

    return EXIT_SUCCESS;
}