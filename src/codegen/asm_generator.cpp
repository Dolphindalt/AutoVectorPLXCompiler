#include <codegen/asm_generator.h>

#include <codegen/asm_file.h>

#include <assertions.h>
#include <logging.h>

AssemblyGenerator::AssemblyGenerator() : symTable(SymbolTable()) {}

AssemblyGenerator::~AssemblyGenerator() {}

void AssemblyGenerator::generateAssembly(
    const BlockSet &blockSet
) {
    INFO_LOG("Starting code generation...");

    this->basePointer = 0;
    this->functionPrelog = 0;
    this->isMain = true;

    // Procedures need to be generated first, so the blocks will be reordered 
    // such that procedures are generated first, then the main entry point
    // will be generated.
    std::vector<BBP> entryBlocks;
    std::vector<BBP> blocks;
    bool inProcedure = false;

    for (const BBP &bb : blockSet) {
        if (bb->getHasEnterProcedure()) {
            inProcedure = true;
        }

        if (!inProcedure) {
            entryBlocks.push_back(bb);
        } else {
            blocks.push_back(bb);
        }

        if (bb->getHasExitProcedure()) {
            inProcedure = false;
        }
    }

    blocks.insert(blocks.end(), entryBlocks.begin(), entryBlocks.end());

    for (const BBP &bb : blocks) {

        // Compute liveness for the current basic block.
        this->liveness = LivenessInfoTable(bb);

        if (bb->getHasEnterProcedure()) {
            this->functionPrelog = this->asmFile.getLine();
            this->isMain = false;
            this->allocateProcedureArguments(bb->getFirstLabel());
        }

        auto iter = bb->getInstructions().begin();
        for (; iter != bb->getInstructions().end()-1; iter++) {
            this->generateAssemblyFromTAC(*iter);
        }

        ASSERT(iter != bb->getInstructions().end());

        if (bb->changesControlAtEnd()) {
            this->freeRegisters();
            this->generateAssemblyFromTAC(*iter);
        } else {
            this->generateAssemblyFromTAC(*iter);
            this->freeRegisters();
        }

        if (bb->getHasExitProcedure()) {
            this->insertPrologue();
            this->instertEpilogue();

            this->functionPrelog = this->asmFile.getLine();
        }

    }

    // This is for the entry point and program exit.
    this->insertPrologue();
    this->instertEpilogue();

    this->insertDataSection();

    this->asmFile.to_file("output.s");
}

void AssemblyGenerator::generateAssemblyFromTAC(const tac_line_t &instruction) {
    switch (instruction.operation) {
        // No data operations.
        case TAC_NOP:
            asmFile.insertTextInstruction("nop");
            break;
        // Unary operations.
        case TAC_NEGATE:
            
            break;
        case TAC_UNCOND_JMP:
            this->asmFile.insertTextInstruction(
                "\tjmp " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_READ:
            break;
        case TAC_WRITE: {
            Address thingToWrite = this->getLocation(
                instruction.argument1,
                instruction
            );

            // Push registers in use that are not perserved.
            for (const reg_t &reg : registers) {
                if (!this->regTable.isRegisterUnused(reg)) {
                    this->asmFile.insertTextInstruction("\tpushq " + reg);
                }
            }

            this->asmFile.insertTextInstruction(
                "\tmovq " + thingToWrite.address() + ", \%rdi"
            );
            this->asmFile.insertTextInstruction("\tcall write_pl_0");

            // Pop registers that are not perserved.
            for (auto i = registers.rbegin(); i != registers.rend(); i++) {
                const reg_t &reg = *i;
                if (!this->regTable.isRegisterUnused(reg)) {
                    this->asmFile.insertTextInstruction("\tpopq " + reg);
                }
            }

            break;
        }
        case TAC_LABEL:
            this->asmFile.insertTextInstruction(
                tac_line_t::extract_label(instruction.argument1) + ":");
            break;
        case TAC_CALL:
            this->asmFile.insertTextInstruction(
                "call " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_JMP_E:
            this->asmFile.insertTextInstruction(
                "\tje " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_JMP_LE:
            this->asmFile.insertTextInstruction(
                "\tjle " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_JMP_GE:
            this->asmFile.insertTextInstruction(
                "\tjge " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_JMP_NE:
            this->asmFile.insertTextInstruction(
                "\tjne " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_JMP_ZERO:
            this->asmFile.insertTextInstruction(
                "\tjz " + tac_line_t::extract_label(instruction.argument1)
            );
            break;
        case TAC_RETVAL:
            break;
        case TAC_PROC_PARAM:
            break;
        case TAC_ASSIGN: {
            // TAC assign has 2 forms
            ASSERT(instruction.argument2 == "");

            // Just the result
            if (instruction.argument1 == "") {
                this->getLocation(instruction.result, instruction);
            } else {
                // result = arg1.
                Address op1 = this->getLocation(
                    instruction.argument1, instruction
                );
                Address result = this->getLocation(
                    instruction.result, instruction
                );

                // Use a stratch memory for two addresses.
                if (result.isMemoryAddress() && op1.isMemoryAddress()) {
                    Address newOp1 = this->forceAddressIntoRegister(
                        instruction.result, instruction, NORMAL, false
                    );

                    this->asmFile.insertTextInstruction(
                        "\tmovq " + op1.address() + ", " + newOp1.address()
                    );
                    this->regTable.updateRegisterValue(
                        newOp1.address(), instruction.argument1, false);
                    op1 = newOp1;
                }

                // Produce the instruction.
                this->asmFile.insertTextInstruction(
                    "\tmovq " + op1.address() + ", " + result.address() 
                );

                if (result.isRegister()) {
                    this->regTable.setRegisterWasUpdated(result.address());
                }
            }
            break;
        }
        case TAC_SUB:
        case TAC_ADD: {
            const Address op1 = this->getLocation(
                instruction.argument1, instruction
            );
            const Address op2 = this->getLocation(
                instruction.argument2, instruction
            );

            // To avoid changing the operand register, we allocate a new 
            // register instead. This could be improved by only doing this 
            // if the op1 register is dead.
            reg_t resultRegister = getRegister(
                instruction.result, instruction, NORMAL
            );

            this->asmFile.insertTextInstruction(
                "\tmovq " + op1.address() + ", " + resultRegister
            );

            std::string num = "\taddq ";
            if (instruction.operation == TAC_SUB) {
                num = "\tsubq ";
            }

            this->asmFile.insertTextInstruction(
                num + op2.address() + ", " + resultRegister
            );

            this->regTable.updateRegisterValue(
                resultRegister, instruction.result, false
            );
            this->regTable.setRegisterWasUpdated(resultRegister);
            break;
        }
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
                instruction.argument1, instruction, NORMAL, false
            );
            const Address operand2 = this->getLocation(
                instruction.argument2,
                instruction);

            this->asmFile.insertTextInstruction(
                "\tcmpq " + operand2.address() + ", " + operand1.address()
            );
            if (instruction.result != "") {
                reg_t resReg = this->getRegister(
                    instruction.result, 
                    instruction,
                    NORMAL
                );
                this->asmFile.insertTextInstruction("setq " + resReg);
            }
            break;
        }
        case TAC_ARRAY_INDEX: {
            Address array = this->forceAddressIntoRegister(
                instruction.argument1,
                instruction,
                NORMAL,
                true
            );
            Address index = this->forceAddressIntoRegister(
                instruction.argument2,
                instruction,
                NORMAL,
                false
            );

            this->insertArrayVariable(instruction.result, array, index);

            break;
        }
        case TAC_ENTER_PROC:
        case TAC_EXIT_PROC:
            break;
        default:
            ERROR_LOG("invalid 3AC operation %d", instruction.operation);
            exit(EXIT_FAILURE);
    }
}

Address AssemblyGenerator::getLocation(
    const std::string &value,
    const tac_line_t &inst
) {
    st_entry_t sym_entry;
    unsigned int level;
    inst.table->lookup(value, &level, &sym_entry);

    // First, see if the value is a literal, which is in the text section.
    if (sym_entry.entry_type == ST_LITERAL) {
        // This is a literal.
        return Address(A_IMM64, value);
    }

    // Maybe the value is in a register.
    bool isValueInReg = this->regTable.isValueInRegister(value);
    if (isValueInReg) {
        std::string registerName = this->regTable.getRegisterWithValue(value);

        // Registers containing addresses must be indexed properly.
        if (this->regTable.containsAddress(registerName)) {
            return Address(A_RM64, registerName);
        }

        return Address(A_R64, registerName);
    }

    // If the value is not in a register, it may be in stack memory.
    if (this->stack.isVaribleInStack(value)) {
        unsigned int stackOffset = this->stack.findVariableInStack(value);
        return Address(std::to_string(stackOffset));
    }

    // If the value is not in a register or stack memory, it must be in
    // global memory.
    if (AssemblyGenerator::data.isVariableInDataSection(value)) {
        return Address(A_M64, value);
    }

    if (this->isVariableInArray(value)) {
        array_index_t av = this->getArrayIndex(value);
        this->removeArrayVariable(value);
        return Address(A_M64, av.array.address(), av.index.address());
    }

    // User defined variables need to be put into registers and never stored
    // into the memory in this way. This is part of the assumption that all
    // user defined variables are in memory by default.
    if (!tac_line_t::is_user_defined_var(value)) {
        reg_t reg = this->getRegister(value, inst, NORMAL);
        this->regTable.updateRegisterValue(reg, value, false);
        return Address(A_R64, reg);
    }

    return this->allocateMemory(value, inst.table);
}

reg_t AssemblyGenerator::getRegister(
    const std::string &value,
    const tac_line_t &inst,
    register_type_t type
) {
    st_entry_t sym_entry;
    unsigned int _level;
    bool found = inst.table->lookup(value, &_level, &sym_entry);
    ASSERT(sym_entry.entry_type != ST_CODE_GEN);

    if (found && sym_entry.entry_type == ST_LITERAL) {
        return "$" + sym_entry.token.lexeme;
    }

    liveness_info_t liveness = this->liveness.get(inst.bid);

    const bool isDead = liveness.result.liveness == CG_DEAD;
    const bool noNextUse = liveness.result.next_use == NO_NEXT_USE;
    const bool isValueInReg = this->regTable.isValueInRegister(value);

    if (isValueInReg && noNextUse && isDead) {
        return this->regTable.getRegisterWithValue(value);
    } else if (this->regTable.getUnusedRegister(type) != NO_REGISTER) {
        return this->regTable.getUnusedRegister(type);
    } else {
        // Select an occupied register.
        reg_t unlucky = this->regTable.getUnusedRegister(type);

        // Generate an instruction to store its value on the stack.
        std::string variableHeld = this->regTable.getRegisterValue(unlucky);
        unsigned int offset = this->stack
            .insertVariableIntoStack(variableHeld, VARIABLE_SIZE_BYTES);
        this->generateStackStore(offset, unlucky);

        return unlucky;
    }

    ASSERT(false);
    return "";
}

Address AssemblyGenerator::allocateMemory(
    const std::string &value, 
    std::shared_ptr<SymbolTable> symTable
) {
    st_entry_t sym_entry;
    unsigned int level;
    symTable->lookup(value, &level, &sym_entry);

    if (!isMain) {
        // Arrays are all passed by reference and every other data type
        unsigned int offset = 
            this->stack.insertVariableIntoStack(value, VARIABLE_SIZE_BYTES);
        // is 8 bytes.
        return Address(std::to_string(offset));
    } else {
        if (sym_entry.variable.isArray) {
            this->data.insert(value, sym_entry.variable.arraySize);
        } else {
            this->data.insert(value, VARIABLE_SIZE_BYTES);
        }
        return Address(A_M64, value);
    }
}

void AssemblyGenerator::generateStackStore(
    unsigned int offset, const reg_t &reg
) {
    const std::string inst = "\tmovq " + reg + ", " + int_to_hex(offset) + "(\%rbp)";
    this->asmFile.insertTextInstruction(inst);
}

void AssemblyGenerator::generateDataStore(
    const std::string &name, const reg_t &reg
) {
    const std::string inst = "\tmovq " + reg + ", " + name;
    this->asmFile.insertTextInstruction(inst);
}

void AssemblyGenerator::allocateProcedureArguments(
    const tac_line_t &AssemblyGeneratorLabel
) {
    const std::string procName = 
        tac_line_t::extract_label(AssemblyGeneratorLabel.argument1);
    
    st_entry_t procEntry;
    unsigned int level;
    bool result = AssemblyGeneratorLabel.table->lookup(procName, &level, &procEntry);
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

void AssemblyGenerator::insertPrologue() {
    unsigned int offset = this->functionPrelog;

    // We need to specify the entry point.
    if (this->isMain) {
        this->asmFile.insertTextInstruction(".global _start", offset);
        this->asmFile.insertTextInstruction("_start:", offset + 1);
        offset += 2;
    }

    this->asmFile.insertTextInstruction("\tpushq \%rbp", offset);
    this->asmFile.insertTextInstruction("\tmovq \%rsp, \%rbp", offset + 1);
    if (this->stack.getStackSize() != 0) {
        this->asmFile.insertTextInstruction(
            "\tsubq " + int_to_hex(this->stack.getStackSize()) + ", \%rsp",
            offset + 2
        );
    }
}

void AssemblyGenerator::instertEpilogue() {
    this->asmFile.insertTextInstruction("\tmovq \%rbp, \%rsp");
    this->asmFile.insertTextInstruction("\tpopq \%rbp");
    if (!this->isMain)
        this->asmFile.insertTextInstruction("\tret");
    else {
        // We got to exit the program.
        this->asmFile.insertTextInstruction("\tmovq $60, \%rax");
        this->asmFile.insertTextInstruction("\tmovq $0, \%rbx");
        this->asmFile.insertTextInstruction("\tsyscall");
    }
}

void AssemblyGenerator::insertDataSection() {
    for (auto kv : AssemblyGenerator::data.getDataObjects()) {
        const std::string &value = kv.first;
        const unsigned int size = kv.second;

        if (size == VARIABLE_SIZE_BYTES) {
            this->asmFile.insertDataInstruction(value + ": .quad 0");
        } else {
            this->asmFile.insertDataInstruction(
                value + ": .zero " + std::to_string(size)
            );
        }
    }
}

Address AssemblyGenerator::forceAddressIntoRegister(
    const std::string &value,
    const tac_line_t &inst,
    register_type_t type,
    bool isAddress=false
) {
    Address operand = this->getLocation(value, inst);
    if (!operand.isRegister()) {
        reg_t reg = this->getRegister(value, inst, type);

        if (!isAddress) {
            this->asmFile.insertTextInstruction(
                "\tmovq " + operand.address() + ", " + reg
            );
        } else {
            this->asmFile.insertTextInstruction(
                "\tleaq " + operand.address() + ", " + reg
            );
        }

        this->regTable.updateRegisterValue(reg, value, false);
        operand = this->getLocation(value, inst);
    }
    return operand;
}

void AssemblyGenerator::freeRegisters() {
    for (reg_t reg : this->regTable.getRegistersInUse(NORMAL)) {

        // If a register is updated and it is memory, we need to update the
        // memory location and not just simply discard the register value.
        if (this->regTable.isRegisterUpdated(reg)) {

            const std::string &value = this->regTable.getRegisterValue(reg);
            if (this->stack.isVaribleInStack(value)) {
                unsigned int offset = 
                    this->stack.findVariableInStack(value, basePointer);
                this->generateStackStore(offset, reg);
            } else if (this->data.isVariableInDataSection(value)) {
                this->generateDataStore(value, reg);
            }

        }

        this->regTable.freeRegister(reg);
    }
}

void AssemblyGenerator::insertArrayVariable(
    const std::string &name, 
    const Address &array, 
    const Address &index
) {
    const array_index_t vardiac = { array, index };
    this->arrayVariable.insert(std::make_pair(name, vardiac));
}

void AssemblyGenerator::removeArrayVariable(const std::string &name) {
    this->arrayVariable.erase(name);
}

bool AssemblyGenerator::isVariableInArray(const std::string &name) const {
    return this->arrayVariable.count(name) > 0;
}

array_index_t AssemblyGenerator::getArrayIndex(const std::string &name) const {
    return this->arrayVariable.at(name);
}
