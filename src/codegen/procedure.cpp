#include <codegen/procedure.h>

#include <logging.h>
#include <assertions.h>

DataSection Procedure::data;

Procedure::Procedure(
    AsmFile *asmFile,
    std::vector<tac_line_t> &instructions,
    SymbolTable *symTable, 
    LivenessInfoTable *liveness, 
    RegisterTable *regTable,
    bool isMain=false
) : asmFile(asmFile), symTable(symTable), liveness(liveness), 
    regTable(regTable), isMain(isMain), functionPrelog(asmFile->getLine()) {}

void Procedure::generateAssemblyFromTAC(const tac_line_t &instruction) {
    switch (instruction.operation) {
        // No data operations.
        case TAC_NOP:
            asmFile->insertTextInstruction("nop");
            break;
        // Unary operations.
        case TAC_NEGATE:
            
            break;
        case TAC_UNCOND_JMP:
            break;
        case TAC_READ:
            break;
        case TAC_WRITE:
            break;
        case TAC_LABEL:
            break;
        case TAC_CALL:
            break;
        case TAC_JMP_E:
            break;
        case TAC_JMP_L:
            break;
        case TAC_JMP_G:
            break;
        case TAC_JMP_NE:
            break;
        case TAC_JMP_ZERO:
            break;
        case TAC_RETVAL:
            break;
        case TAC_PROC_PARAM:
            break;
        case TAC_ASSIGN:
            // This is a declaration.
            if (instruction.argument1 == "" && instruction.argument2 == "") {
                // Insert data into the data section rather than in stack
                // memory.
                if (this->isMain) {
                    this->data.insert(instruction.result);
                } else { // Into stack memory.
                    this->stack.insertVariableIntoStack(instruction.result, 0x08);
                }
            }
            break;
        case TAC_ADD:
            break;
        case TAC_SUB:
            break;
        case TAC_DIV:
            break;
        case TAC_MULT:
            break;
        case TAC_LESS_THAN:
            break;
        case TAC_GREATER_THAN:
            break;
        case TAC_GE_THAN:
            break;
        case TAC_LE_THAN:
            break;
        case TAC_EQUALS:
            break;
        case TAC_NOT_EQUALS:
            break;
        case TAC_ARRAY_INDEX:
            break;
        default:
            ERROR_LOG("invalid 3AC operation %d", instruction.operation);
            exit(EXIT_FAILURE);
    }
}

reg_t Procedure::getRegister(const std::string &value) {
    st_entry_t cg_entry;
    unsigned int _level;
    this->symTable->lookup(value, &_level, &cg_entry);
    ASSERT(cg_entry.entry_type == ST_CODE_GEN);

    bool isDead = cg_entry.code_gen.liveness == CG_DEAD;
    bool isValueInReg = this->regTable->isValueInRegister(value);
    reg_t unused;

    if (isValueInReg && isDead) {
        return this->regTable->getRegisterWithValue(value);
    } else if ((unused = this->regTable->getUnusedRegister()) != NO_REGISTER) {
        return unused;
    } else {
        // We are out of registers and the value is not in the register.
        return NO_REGISTER;
    }
}
