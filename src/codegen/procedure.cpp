#include <codegen/procedure.h>

#include <logging.h>
#include <assertions.h>

DataSection Procedure::data;

Procedure::Procedure(
    AsmFile *asmFile,
    std::vector<tac_line_t>::iterator start,
    std::vector<tac_line_t>::iterator end,
    SymbolTable *cgTable, 
    LivenessInfoTable *liveness, 
    RegisterTable *regTable,
    bool isMain=false
) : asmFile(asmFile), cgTable(cgTable), liveness(liveness), 
    regTable(regTable), isMain(isMain), functionPrelog(asmFile->getLine()),
    stackSize(0) {

        if (!this->isMain) {
            // We are in some procedure, so we expect the first instruction
            // to be a label with the procedure name.
            this->allocateProcedureArguments(*start);
        }

        for (; start != end; start++) {
            const tac_line_t &inst = *start;

            if (inst.operation == TAC_ENTER_PROC) {
                auto proc_start = ++start; // Skip the enter proc.
                while ((*start).operation != TAC_EXIT_PROC) {
                    start++;
                }
                auto proc_end = start;
                // Generate ASM for this particular procedure.
                Procedure(
                    asmFile, proc_start, proc_end, cgTable, 
                    liveness, regTable, false
                );

                // Skip the exit proc.
                continue;
            }

            this->generateAssemblyFromTAC(inst);
        }

        this->insertPrologue();
        this->instertEpilogue();

    }

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
            this->asmFile->insertTextInstruction(
                "\tjmp " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_READ:
            break;
        case TAC_WRITE:
            break;
        case TAC_LABEL:
            this->asmFile->insertTextInstruction(
                tac_line_t::extract_label(instruction.argument1) + ":");
            break;
        case TAC_CALL:
            this->asmFile->insertTextInstruction(
                "call " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_JMP_E:
            this->asmFile->insertTextInstruction(
                "\tje " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_JMP_L:
            this->asmFile->insertTextInstruction(
                "\tjl " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_JMP_G:
            this->asmFile->insertTextInstruction(
                "\tjg " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_JMP_LE:
            this->asmFile->insertTextInstruction(
                "\tjle " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_JMP_GE:
            this->asmFile->insertTextInstruction(
                "\tjge " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_JMP_NE:
            this->asmFile->insertTextInstruction(
                "\tjne " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_JMP_ZERO:
            this->asmFile->insertTextInstruction(
                "\tjz " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_RETVAL:
            break;
        case TAC_PROC_PARAM:
            break;
        case TAC_ASSIGN: {
            // This is a declaration.
            st_entry_t entry;
            unsigned int level;
            instruction.table->lookup(instruction.result, &level, &entry);
            // TAC assign has 1 form
            ASSERT(instruction.argument2 == "");
            // result = arg1.
            Address op1 = this->getOperandLocation(
                instruction.argument1, 
                instruction.table
            );
            // Get a register to store the result.
            reg_t res_reg = this->getRegister(
                instruction.result, 
                instruction.table,
                NORMAL,
                instruction.bid
            );
            // Produce the instruction.
            this->asmFile->insertTextInstruction(
                "\tmov " + op1.getAddressModeString() + ", \%" + res_reg 
            );
            this->regTable->updateRegisterValue(res_reg, instruction.result);
            break;
        }
        case TAC_ADD: {
            Address op1 = this->getOperandLocation(
                instruction.argument1, 
                instruction.table
            );
            const Address op2 = this->getOperandLocation(
                instruction.argument2,
                instruction.table
            );

            // Add is in the form dest = dest + src.
            // This means that operand1 must be a register.
            if (!op1.isRegister()) {
                reg_t reg = this->getRegister(
                    instruction.argument1, 
                    instruction.table, 
                    NORMAL, 
                    instruction.bid
                );
                this->asmFile->insertTextInstruction(
                    "\tmov " + op1.getAddressModeString() + ", " + reg
                );
            }

            // Op1 now must be a register.
            op1 = this->getOperandLocation(
                instruction.argument1, 
                instruction.table
            );
            
            ASSERT(op1.isRegister() == true);

            // Operand1 is also the result. We will say the result is 
            // in the selected register as a result.
            this->regTable->updateRegisterValue(
                op1.getName(), instruction.result
            );

            this->asmFile->insertTextInstruction(
                "\tadd " + op2.getAddressModeString() + ", " + 
                    op1.getAddressModeString()
            );

            break;
        }
        case TAC_SUB:
            break;
        case TAC_DIV:
            break;
        case TAC_MULT:
            break;
        case TAC_LESS_THAN:
        case TAC_GREATER_THAN:
        case TAC_GE_THAN:
        case TAC_LE_THAN:
        case TAC_EQUALS:
        case TAC_NOT_EQUALS: {
            // Two cases. Comparing for a control flow or storing the result.
            // For the control flow.
            const Address operand1 = this->getOperandLocation(
                instruction.argument1,
                instruction.table
            );
            const Address operand2 = this->getOperandLocation(
                instruction.argument2,
                instruction.table
            );
            this->asmFile->insertTextInstruction(
                "\tcmp " + operand2.getAddressModeString() + 
                    ", " + operand1.getAddressModeString()
            );
            if (instruction.result != "") {
                reg_t resReg = this->getRegister(
                    instruction.result, 
                    instruction.table, 
                    NORMAL, 
                    instruction.bid
                );
                this->asmFile->insertTextInstruction("set \%" + resReg);
            }
            break;
        }
        case TAC_ARRAY_INDEX:
            break;
        default:
            ERROR_LOG("invalid 3AC operation %d", instruction.operation);
            exit(EXIT_FAILURE);
    }
}

bool Procedure::isValueInMemory(
    const std::string &value,
    std::shared_ptr<SymbolTable> symTable
) const {
    return this->regTable->isValueInRegister(value) ||
        this->stack.isVaribleInStack(value) ||
        this->data.isVariableInDataSection(value);
}

Address Procedure::getOperandLocation(
    const std::string &value,
    std::shared_ptr<SymbolTable> symTable
) {
    st_entry_t sym_entry;
    unsigned int level;
    symTable->lookup(value, &level, &sym_entry);

    // First, see if the value is a literal, which is in the text section.
    if (sym_entry.entry_type == ST_LITERAL) {
        // This is a literal.
        return Address(A_LITERAL, value);
    }

    // Maybe the value is in a register.
    bool isValueInReg = this->regTable->isValueInRegister(value);
    if (isValueInReg) {
        std::string registerName = this->regTable->getRegisterWithValue(value);
        return Address(A_REGISTER, registerName);
    }

    // If the value is not in a register, it may be in stack memory.
    if (this->stack.isVaribleInStack(value)) {
        unsigned int stackOffset = this->stack.findVariableInStack(value);
        return Address(A_STACK, value, stackOffset);
    }

    // The value must be in global memory if it is nowhere else.
    if (this->data.isVariableInDataSection(value)) {
        return Address(A_GLOBAL, value);
    }

    ASSERT(sym_entry.entry_type == ST_VARIABLE);
    // The operand is not found in memory, so it needs to be allocated 
    // to the stack if in a procedure or in global data if not in a procedure.
    if (!this->isMain) {
        // In a procedure.
        // Arrays are all passed by reference and every other data type
        // is 8 bytes.
        unsigned int stackOffset = 
            this->stack.insertVariableIntoStack(value, VARIABLE_SIZE_BYTES);
        return Address(A_STACK, value, stackOffset);
    } else {
        // In the entry point.
        // Must consider array structures.
        if (sym_entry.variable.isArray) {
            this->data.insert(value, sym_entry.variable.arraySize);
        } else {
            this->data.insert(value, VARIABLE_SIZE_BYTES);
        }
        return Address(A_GLOBAL, value);
    }

    ERROR_LOG(
        "failed to find or create operand %s in the memory", value.c_str()
    );
    exit(EXIT_FAILURE);
}

reg_t Procedure::getRegister(
    const std::string &value,
    std::shared_ptr<SymbolTable> symTable,
    register_type_t type,
    const TID instID
) {
    st_entry_t sym_entry;
    unsigned int _level;
    symTable->lookup(value, &_level, &sym_entry);
    ASSERT(sym_entry.entry_type != ST_CODE_GEN);

    if (sym_entry.entry_type == ST_LITERAL) {
        return sym_entry.token.lexeme;
    }

    liveness_info_t liveness = this->liveness->get(instID);

    const bool isDead = liveness.result.liveness == CG_DEAD;
    const bool noNextUse = liveness.result.next_use == NO_NEXT_USE;
    const bool isValueInReg = this->regTable->isValueInRegister(value);

    INFO_LOG("Liveness: is_dead %d no next use %d", isDead, noNextUse);

    if (isValueInReg && noNextUse && isDead) {
        return this->regTable->getRegisterWithValue(value);
    } else if (this->regTable->getUnusedRegister(type) != NO_REGISTER) {
        return this->regTable->getUnusedRegister(type);
    } else {
        // Select an occupied register.
        reg_t unlucky = this->regTable->getUnusedRegister(type);
        // Generate an instruction to store its value on the stack.
        std::string variableHeld = this->regTable->getRegisterValue(unlucky);
        unsigned int offset = this->stack
            .insertVariableIntoStack(variableHeld, VARIABLE_SIZE_BYTES);
        this->generateStackStore(offset, unlucky);
        // Update the descriptors and return the register.
        return unlucky;
    }

    ASSERT(false);
    return "";
}

void Procedure::generateStackStore(unsigned int offset, reg_t reg) {
    std::string inst = "\tmov " + reg + ", " + int_to_hex(offset) + "(\%rbp)";
    this->asmFile->insertTextInstruction(inst);
}

void Procedure::allocateProcedureArguments(const tac_line_t &procedureLabel) {
    const std::string procName = 
        tac_line_t::extract_label(procedureLabel.argument1);
    
    st_entry_t procEntry;
    unsigned int level;
    bool result = procedureLabel.table->lookup(procName, &level, &procEntry);
    ASSERT(result);
    ASSERT(procEntry.entry_type == ST_FUNCTION);

    uint8_t argsLen = procEntry.procedure.argumentsLength;
    for (uint8_t i = 0; i < argsLen; i++) {
        this->stack.insertVariableIntoStack(
            std::string(procEntry.procedure.argumentNames[i]), 
            VARIABLE_SIZE_BYTES
        );
    }

    if (procEntry.procedure.returnType != VOID) {
        this->stack.insertVariableIntoStack(
            std::string(procEntry.procedure.returnTypeName),
            VARIABLE_SIZE_BYTES
        );
    }
    
}

void Procedure::insertPrologue() {
    unsigned int offset = this->functionPrelog;

    // We need to specify the entry point.
    if (this->isMain) {
        this->asmFile->insertTextInstruction(".global main", offset);
        this->asmFile->insertTextInstruction("main:", offset + 1);
        offset += 2;
    }

    this->asmFile->insertTextInstruction("\tpushq \%rbp", offset);
    this->asmFile->insertTextInstruction("\tmovq \%rsp, \%rbp", offset + 1);
    this->asmFile->insertTextInstruction(
        "\tsub " + int_to_hex(this->stack.getStackSize()) + ", \%rsp",
        offset + 2
    );
}

void Procedure::instertEpilogue() {
    this->asmFile->insertTextInstruction("\tmovq \%rbp, \%rsp");
    this->asmFile->insertTextInstruction("\tpopq \%rbp");
    this->asmFile->insertTextInstruction("\tret");
}
