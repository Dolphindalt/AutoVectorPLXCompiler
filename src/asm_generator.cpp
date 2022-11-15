#include <asm_generator.h>

AssemblyGenerator::AssemblyGenerator() : symTable(SymbolTable()) {}

AssemblyGenerator::~AssemblyGenerator() {}

void AssemblyGenerator::generateAssembly(
    const std::vector<tac_line_t> &instructions
) {
    this->populateSymbolTableDefaults(instructions);
}

void AssemblyGenerator::populateSymbolTableDefaults(
    const std::vector<tac_line_t> &instructions
) {
    for (auto i = instructions.begin(); i != instructions.end(); i++) {
        const tac_line_t &instruction = *i;
        this->initVariableInTable(instruction.argument1);
        this->initVariableInTable(instruction.argument2);
        this->initVariableInTable(instruction.result);
    }
}

void AssemblyGenerator::initVariableInTable(const std::string &variable) {
    // Labels an undefined variables are ignored.
    if (this->variableIsDefinedAndNotLabel(variable)) {
        st_entry_t to_enter;
        to_enter.entry_type = ST_CODE_GEN;
        to_enter.code_gen.next_use = NO_NEXT_USE;
        // User defined variables are initialized as live.
        if (this->isUserDefinedVariable(variable)) {
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

bool AssemblyGenerator::isUserDefinedVariable(
    const std::string &variable
) const {
    return variable.length() >= 2 && 
        variable.at(0) == '$' && 
        variable.at(1) != 'L';
}

bool AssemblyGenerator::isLabel(const std::string &variable) const {
    return variable.length() >= 3 && 
        variable.at(0) == '$' && 
        variable.at(1) == 'L';
}