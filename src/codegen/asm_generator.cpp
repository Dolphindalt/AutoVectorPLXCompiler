#include <codegen/asm_generator.h>

#include <codegen/asm_file.h>
#include <codegen/procedure.h>

#include <assertions.h>
#include <logging.h>

AssemblyGenerator::AssemblyGenerator() : symTable(SymbolTable()) {}

AssemblyGenerator::~AssemblyGenerator() {}

void AssemblyGenerator::generateAssembly(
    std::vector<tac_line_t> &instructions
) {
    INFO_LOG("Starting code generation...");
    this->populateSymbolTableDefaults(instructions);
    this->computeLiveness(instructions);
    
    AsmFile asmFile;
    Procedure entryPoint(
        &asmFile, instructions.begin(), instructions.end(),
        &symTable, &liveness, &regTable, true
    );
    asmFile.to_file("output.s");
}

void AssemblyGenerator::populateSymbolTableDefaults(
    std::vector<tac_line_t> &instructions
) {
    for (
        auto i = instructions.begin(); 
         i != instructions.end(); 
         i++
    ) {
        const tac_line_t &instruction = *i;
        this->initVariableInTable(instruction.argument1);
        this->initVariableInTable(instruction.argument2);
        this->initVariableInTable(instruction.result);
    }
}

/**
 * For each instruction, in reverse order:
 * 1. Look at the symbol table and see wether the result and operands are dead
 * or alive, and see what the next use is. Attach this information to the 
 * instruction.
 * 2. Next, update the symbol table information for use earlier in the block.
 * a. Mark the symbol table entry for the target result dead and no next use.
 * b. Mark the symbol table entries for the operands live and set the next-use
 * values to the instruction ID.
 */
void AssemblyGenerator::computeLiveness(std::vector<tac_line_t> &instructions) {
    for (
        auto i = instructions.rbegin();
        i != instructions.rend();
        i++
    ) {
        const tac_line_t &instruction = *i;

        // Variables for storing lookup information.
        st_entry_t result_entry;
        st_entry_t operand1_entry;
        st_entry_t operand2_entry;
        unsigned int _level;

        this->symTable.lookup(instruction.result, &_level, &result_entry);

        bool hasOp1 = this->symTable.lookup(
            instruction.argument1, &_level, &result_entry
        );

        bool hasOp2 = this->symTable.lookup(
            instruction.argument2, &_level, &result_entry
        );

        // Perform step 1 from the above comment.
        liveness_info_t liveness;
        liveness.result.liveness = result_entry.code_gen.liveness;
        liveness.operand1.liveness = operand1_entry.code_gen.liveness;
        liveness.operand2.liveness = operand2_entry.code_gen.liveness;
        liveness.result.next_use = result_entry.code_gen.next_use;
        liveness.operand1.next_use = operand1_entry.code_gen.next_use;
        liveness.operand2.next_use = operand2_entry.code_gen.next_use;
        this->liveness.insert(instruction.bid, liveness);

        // Perform step 2, updating the symbol table.
        // Marking the target result as dead and no next use.
        result_entry.entry_type = ST_CODE_GEN;
        result_entry.code_gen.liveness = CG_DEAD;
        result_entry.code_gen.next_use = NO_NEXT_USE;
        this->symTable.insert(instruction.result, result_entry);

        // Marking operands as live and setting their next use to this 
        // instruction.
        if (hasOp1) {
            operand1_entry.code_gen.liveness = CG_LIVE;
            operand1_entry.code_gen.next_use = instruction.bid;
            this->symTable.insert(instruction.argument1, operand1_entry);
        }

        if (hasOp2) {
            operand2_entry.code_gen.liveness = CG_LIVE;
            operand2_entry.code_gen.next_use = instruction.bid;
            this->symTable.insert(instruction.argument2, operand2_entry);
        }
            
    }
}

void AssemblyGenerator::initVariableInTable(const std::string &variable) {
    // Labels and undefined variables are ignored.
    if (this->variableIsDefinedAndNotLabel(variable)) {
        st_entry_t to_enter;
        to_enter.entry_type = ST_CODE_GEN;
        to_enter.code_gen.next_use = NO_NEXT_USE;
        // User defined variables are initialized as live.
        if (tac_line_t::is_user_defined_var(variable)) {
            to_enter.code_gen.liveness = CG_LIVE; 
        } else { // While temporary variables are dead.
            to_enter.code_gen.liveness = CG_DEAD;
        }
        this->symTable.insert(variable, to_enter);
    }
}

bool AssemblyGenerator::variableIsDefinedAndNotLabel(
    const std::string &variable
) const {
    return variable != "" && !this->isLabel(variable);
}

bool AssemblyGenerator::isLabel(const std::string &variable) const {
    return variable.length() >= 3 && 
        variable.at(0) == '$' && 
        variable.at(1) == 'L';
}