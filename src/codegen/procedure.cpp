#include <codegen/procedure.h>

#include <logging.h>
#include <assertions.h>

#define PUSH(reg) this->asmFile->insertTextInstruction("\tpushq " reg)
#define POP(reg) this->asmFile->insertTextInstruction("\tpopq " reg)

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

        if (this->isMain) {
            this->insertDataSection();
        }

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
        case TAC_WRITE: {
            Address thingToWrite = this->forceAddressIntoRegister(
                instruction.argument1,
                instruction.table,
                NORMAL,
                instruction.bid
            );

            // 8 pushes of 8 bytes = 64 bytes added to stack.
            PUSH("\%rax");
            PUSH("\%rcx");
            PUSH("\%rdx");
            PUSH("\%r9");
            PUSH("\%r10");
            PUSH("\%rsi");
            PUSH("\%rdi");
            this->asmFile->insertTextInstruction(
                "\tmovq " + thingToWrite.getAddressModeString(this->regTable) +
                    ", \%rdi"
            );
            this->asmFile->insertTextInstruction("\tcall write_pl_0");
            POP("\%rdi");
            POP("\%rsi");
            POP("\%r10");
            POP("\%r9");
            POP("\%rdx");
            POP("\%rcx");
            POP("\%rax");
            break;
        }
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
            // TAC assign has 2 forms
            ASSERT(instruction.argument2 == "");

            // Just the result
            if (instruction.argument1 == "") {
                this->getLocation(
                    instruction.result, instruction.table, instruction.bid
                );
            } else {
                // result = arg1.
                Address op1 = this->getLocation(
                    instruction.argument1, 
                    instruction.table,
                    instruction.bid
                );
                Address result = this->getLocation(
                    instruction.result,
                    instruction.table,
                    instruction.bid
                );
                // Produce the instruction.
                this->asmFile->insertTextInstruction(
                    "\tmovq " + op1.getAddressModeString(this->regTable) + ", " 
                        + result.getAddressModeString(this->regTable) 
                );
            }
            break;
        }
        case TAC_ADD: {
            Address op1 = this->getLocation(
                instruction.argument1, 
                instruction.table,
                instruction.bid
            );
            const Address op2 = this->getLocation(
                instruction.argument2,
                instruction.table,
                instruction.bid
            );

            // Add is in the form dest = dest + src.
            // This means that operand1 must be a register.
            reg_t reg = op1.getName();

            if (!op1.isRegister()) {
                reg = this->getRegister(
                    instruction.argument1, 
                    instruction.table, 
                    NORMAL, 
                    instruction.bid
                );
                this->asmFile->insertTextInstruction(
                    "\tmovq " + op1.getAddressModeString(this->regTable) + 
                        ", \%" + reg
                );
            }

            // Operand1 is also the result. We will say the result is 
            // in the selected register as a result.
            INFO_LOG("op1 name %s", reg.c_str());
            this->regTable->updateRegisterValue(reg, instruction.result);

            this->asmFile->insertTextInstruction(
                "\taddq " + op2.getAddressModeString(this->regTable) + 
                    ", \%" + reg
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
            Address operand1 = this->forceAddressIntoRegister(
                instruction.argument1,
                instruction.table,
                NORMAL,
                instruction.bid
            );
            const Address operand2 = this->getLocation(
                instruction.argument2,
                instruction.table,
                instruction.bid
            );

            this->asmFile->insertTextInstruction(
                "\tcmpq " + operand2.getAddressModeString(this->regTable) + 
                    ", " + operand1.getAddressModeString(this->regTable)
            );
            if (instruction.result != "") {
                reg_t resReg = this->getRegister(
                    instruction.result, 
                    instruction.table, 
                    NORMAL, 
                    instruction.bid
                );
                this->asmFile->insertTextInstruction("setq \%" + resReg);
            }
            break;
        }
        case TAC_ARRAY_INDEX: {
            Address array = this->getLocation(
                instruction.argument1,
                instruction.table,
                instruction.bid
            );
            Address index = this->forceAddressIntoRegister(
                instruction.argument2,
                instruction.table,
                NORMAL,
                instruction.bid
            );
            reg_t result = this->getRegister(
                instruction.result, 
                instruction.table,
                NORMAL,
                instruction.bid
            );

            reg_t temp = this->getRegister(
                instruction.result,
                instruction.table,
                NORMAL,
                instruction.bid
            );

            // Load the address of array + offset into temp.
            this->asmFile->insertTextInstruction(
                "\tleaq " + instruction.argument1 + 
                    "(," + index.getAddressModeString(this->regTable) 
                    + ",8), \%" + temp
            );
            this->regTable->updateRegisterValue(temp, instruction.result);
            this->regTable->updateRegisterContentType(temp, true);

            break;
        }
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
        this->stack.isVaribleInStack(value);
}

Address Procedure::getLocation(
    const std::string &value,
    std::shared_ptr<SymbolTable> symTable,
    const TID tid
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

    // If the value is not in a register or stack memory, it must be in
    // global memory.
    if (Procedure::data.isVariableInDataSection(value)) {
        return Address(A_GLOBAL, value);
    }

    // User defined variables need to be put into registers and never stored
    // into the memory in this way. This is part of the assumption that all
    // user defined variables are in memory by default.
    if (!tac_line_t::is_user_defined_var(value)) {
        reg_t reg = this->getRegister(value, symTable, NORMAL, tid);
        this->regTable->updateRegisterValue(reg, value);
        return Address(A_REGISTER, reg);
    }

    return this->allocateMemory(value, symTable);
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
        return "$" + sym_entry.token.lexeme;
    }

    liveness_info_t liveness = this->liveness->get(instID);

    const bool isDead = liveness.result.liveness == CG_DEAD;
    const bool noNextUse = liveness.result.next_use == NO_NEXT_USE;
    const bool isValueInReg = this->regTable->isValueInRegister(value);

    INFO_LOG("Liveness: is_dead %d no next use %d", isDead, noNextUse);

    if (isValueInReg && noNextUse && isDead) {
        reg_t toReturn = this->regTable->getRegisterWithValue(value);
        this->regTable->updateRegisterContentType(toReturn, false);
        return toReturn;
    } else if (this->regTable->getUnusedRegister(type) != NO_REGISTER) {
        reg_t toReturn = this->regTable->getUnusedRegister(type);
        this->regTable->updateRegisterContentType(toReturn, false);
        return toReturn;
    } else {
        // Select an occupied register.
        reg_t unlucky = this->regTable->getUnusedRegister(type);
        // Generate an instruction to store its value on the stack.
        std::string variableHeld = this->regTable->getRegisterValue(unlucky);
        unsigned int offset = this->stack
            .insertVariableIntoStack(variableHeld, VARIABLE_SIZE_BYTES);
        this->generateStackStore(offset, unlucky);
        // Update the descriptors and return the register.
        this->regTable->updateRegisterContentType(unlucky, false);
        return unlucky;
    }

    ASSERT(false);
    return "";
}

Address Procedure::allocateMemory(
    const std::string &value, 
    std::shared_ptr<SymbolTable> symTable
) {
    st_entry_t sym_entry;
    unsigned int level;
    symTable->lookup(value, &level, &sym_entry);

    if (!this->isMain) {
        // Arrays are all passed by reference and every other data type
        unsigned int offset = 
            this->stack.insertVariableIntoStack(value, VARIABLE_SIZE_BYTES);
        // is 8 bytes.
        return Address(A_STACK, value, offset);
    } else {
        if (sym_entry.variable.isArray) {
            Procedure::data.insert(value, sym_entry.variable.arraySize);
        } else {
            Procedure::data.insert(value, VARIABLE_SIZE_BYTES);
        }
        return Address(A_GLOBAL, value);
    }
}

void Procedure::generateStackStore(unsigned int offset, reg_t reg) {
    std::string inst = "\tmovq " + reg + ", " + int_to_hex(offset) + "(\%rbp)";
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
        this->asmFile->insertTextInstruction(".global _start", offset);
        this->asmFile->insertTextInstruction("_start:", offset + 1);
        offset += 2;
    }

    this->asmFile->insertTextInstruction("\tpushq \%rbp", offset);
    this->asmFile->insertTextInstruction("\tmovq \%rsp, \%rbp", offset + 1);
    if (this->stack.getStackSize() != 0) {
        this->asmFile->insertTextInstruction(
            "\tsubq " + int_to_hex(this->stack.getStackSize()) + ", \%rsp",
            offset + 2
        );
    }
}

void Procedure::instertEpilogue() {
    this->asmFile->insertTextInstruction("\tmovq \%rbp, \%rsp");
    this->asmFile->insertTextInstruction("\tpopq \%rbp");
    if (!this->isMain)
        this->asmFile->insertTextInstruction("\tret");
    else {
        // We got to exit the program.
        this->asmFile->insertTextInstruction("\tmovq $60, \%rax");
        this->asmFile->insertTextInstruction("\tmovq $0, \%rbx");
        this->asmFile->insertTextInstruction("\tsyscall");
    }
}

void Procedure::insertDataSection() {
    for (auto kv : Procedure::data.getDataObjects()) {
        const std::string &value = kv.first;
        const unsigned int size = kv.second;
        if (size == VARIABLE_SIZE_BYTES) {
            this->asmFile->insertDataInstruction(value + ": .quad 0");
        } else {
            this->asmFile->insertDataInstruction(
                value + ": .zero " + std::to_string(size)
            );
        }
    }
}

Address Procedure::forceAddressIntoRegister(
    const std::string &value,
    std::shared_ptr<SymbolTable> symTable,
    register_type_t type,
    const TID instID
) {
    Address operand = this->getLocation(value, symTable, instID);
    if (!operand.isRegister()) {
        reg_t reg = this->getRegister(value, symTable, type, instID);
        this->asmFile->insertTextInstruction(
            "\tmovq " + operand.getAddressModeString(this->regTable) + 
            ", \%" + reg
        );
        this->regTable->updateRegisterContentType(
            reg, this->regTable->isRegisterHoldingAddress(reg)
        );
        this->regTable->updateRegisterValue(reg, value);
        operand = this->getLocation(value, symTable, instID);
    }
    return operand;
}
