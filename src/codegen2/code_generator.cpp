#include <codegen2/code_generator.h>

#include <assertions.h>

CodeGenerator::CodeGenerator() {}

void CodeGenerator::generate(const BlockSet &blocks) {
    for (const BBP &bb : blocks) {
        this->generateFromBB(bb);
    }
    this->context.insertExit();
    this->context.to_file("output.s");
}

void CodeGenerator::generateFromBB(const BBP &bb) {
    const LivenessTable liveness(bb);
    for (
        auto i = bb->getInstructions().begin(); 
        i != bb->getInstructions().end() - 1; 
        i++
    ) {
        this->generateFrom3AC(*i, liveness);
    }

    if (bb->changesControlAtEnd()) {
        this->freeRegisters();
        this->generateFrom3AC(*(bb->getInstructions().end() - 1), liveness);
    } else {
        this->generateFrom3AC(*(bb->getInstructions().end() - 1), liveness);
        this->freeRegisters();
    }
}

void CodeGenerator::generateFrom3AC(
    const tac_line_t &inst,
    const LivenessTable &liveness
) {
    INFO_LOG("%s", this->addressTable.to_string().c_str());
    INFO_LOG("%s", this->regTable.to_string().c_str());
    INFO_LOG("%s", TACGenerator::tacLineToString(inst).c_str());

    this->context.comment(TACGenerator::tacLineToString(inst));

    // If there are any literals, they need to be stored in the address table.
    this->addressTable.insertIfLiteral(inst.argument1, inst.table);
    this->addressTable.insertIfLiteral(inst.argument2, inst.table);

    // There are two kinds of instructions: those that require registers and 
    // those that do not.
    switch (inst.operation) {
        case TAC_NOP:
            this->context
                .insertText("\t" + this->tacToInstruction(inst.operation));
            break;
        case TAC_ENTER_PROC:
            break;
        case TAC_EXIT_PROC:
            break;
        case TAC_UNCOND_JMP:
            this->generateLabelledInstruction(inst.operation, inst.argument1);
            break;
        case TAC_READ:
            break;
        case TAC_WRITE:
            this->generateWrite(inst.argument1);
            break;
        case TAC_LABEL:
            this->generateLabel(inst.argument1);
            break;
        case TAC_CALL:
        case TAC_JMP_E ... TAC_JMP_ZERO:
            this->generateLabelledInstruction(inst.operation, inst.argument1);
            break;
        case TAC_RETVAL:
            break;
        case TAC_PROC_PARAM:
            break;
        case TAC_ASSIGN:
            if (inst.result != "" && inst.argument1 != ""
                && inst.argument2 != "") {
                    this->convertGeneral3AC(inst, liveness);
            } 
            else {
                this->generateSpecialAssignment(inst, liveness);
            }
            break;
        case TAC_ADD ... TAC_MULT:
            this->convertGeneral3AC(inst, liveness);
            break;
        case TAC_LESS_THAN ... TAC_NOT_EQUALS:
            this->generateConditional(inst, liveness);
            break;
        case TAC_ARRAY_INDEX:
            this->generateArrayIndex(inst, liveness);
            break;
        case TAC_VADD:
        case TAC_VSUB:

            break;
        case TAC_VLOAD:
            break;
        case TAC_VSTORE:
            break;
        case TAC_VASSIGN:
            break;
        default:
            ERROR_LOG(
                "invalid 3AC instruction %s", 
                TACGenerator::tacLineToString(inst).c_str()
            );
            exit(EXIT_FAILURE);
    }
}

void CodeGenerator::generateWrite(const std::string &variable) {
    const Location toWrite = this->addressTable.getLocation(variable);

    auto registersSaved = this->pushRegisters();
    this->context.insertText("\tpushq \%rsi");
    this->context.insertText("\tpushq \%rdi");
    this->context.insertText("\tmovq " + toWrite.address() + ", \%rdi");
    this->context.insertText("\tcall write_pl_0");
    this->context.insertText("\tpopq \%rdi");
    this->context.insertText("\tpopq \%rsi");
    this->popRegisters(registersSaved);
}

void CodeGenerator::generateSpecialAssignment(
    const tac_line_t &inst,
    const LivenessTable &liveness
) {
    ASSERT(inst.operation == TAC_ASSIGN);
    ASSERT(inst.argument2 == "");

    const std::string instStr = this->tacToInstruction(inst.operation);

    // Case: variable declaration.
    if (inst.result != "" && inst.argument1 == "") {
            this->generateVaribleDeclaration(inst);
            return;
    }
    
    const Location &source = this->addressTable.getLocation(inst.argument1);

    std::string resultAddr;

    if (this->addressTable.contains(inst.result)) {
        Location &dest = this->addressTable.getLocation(inst.result);
        resultAddr = dest.address();
    } else {
        RegPtr reg = this->getRegister(liveness, inst.result, inst.bid, GPR);
        resultAddr = reg->getName();
    }

    const std::string instertion = "\t" + instStr + " " + 
        source.address() + ", " + resultAddr;

    this->context.insertText(instertion);
}

void CodeGenerator::convertGeneral3AC(
    const tac_line_t &inst,
    const LivenessTable &liveness
) {
    const std::string instStr = this->tacToInstruction(inst.operation);

    const RegPtr &reg = 
        this->forceRegister(liveness, inst.argument1, inst.bid, GPR);

    // Instruction is in the form a = b (op) c.
    const Location &other = this->addressTable.getLocation(inst.argument2);

    this->context.insertText(
        "\t" + instStr + "q " + other.address() + ", " + reg->getName()
    );

    this->addressTable.insert(inst.result, Location(LT_REGISTER).setReg(reg));
    this->regTable.setRegisterValue(reg, inst.result);
}

void CodeGenerator::generateConditional(
    const tac_line_t &inst,
    const LivenessTable &liveness
) {
    ASSERT(inst.operation >= TAC_LESS_THAN && inst.operation <= TAC_NOT_EQUALS);

    // Two cases in which the comparison is stored or not.
    const std::string instStr = this->tacToInstruction(inst.operation);

    const RegPtr &reg = 
        this->getRegister(liveness, inst.argument1, inst.bid, GPR);
    ASSERT(this->regTable.getVariableInRegister(reg) == inst.argument1);
    ASSERT(this->addressTable.getLocation(inst.argument1).inRegister());
    
    const Location &other = this->addressTable.getLocation(inst.argument2);

    this->context.insertText(
        "\t" + instStr + "q " + other.address() + ", " + reg->getName()
    );

    // Store the result.
    if (inst.result != "") {
        // TODO: Actually make this work properly.
        const RegPtr &regResult = 
            this->getRegister(liveness, inst.argument1, inst.bid, GPR);
        this->context.insertText("\ttest " + regResult->getName());
    }
}

void CodeGenerator::generateArrayIndex(
    const tac_line_t &inst,
    const LivenessTable &liveness
) {
    // Load the index and the array into registers. The index will be an 
    // immediate value stored in the register while the array is an address.
    const RegPtr idxReg = 
        this->forceRegister(liveness, inst.argument2, inst.bid, GPR);

    const RegPtr arrayReg = 
        this->forceRegister(liveness, inst.argument1, inst.bid, GPR, true);

    const RegPtr result = 
        this->getRegister(liveness, inst.result, inst.bid, GPR, true);

    this->context.insertText(
        "\tleaq (" + arrayReg->getName() + ", " + idxReg->getName() + 
            ", 8), " + result->getName()
    );

    this->addressTable
        .insert(inst.result, Location(LT_REGISTER)
            .setReg(result)
            .setIsRegAddress(true)
        );
    this->regTable.setRegisterValue(result, inst.result);
}

void CodeGenerator::generateVaribleDeclaration(
    const tac_line_t &inst
) {
    if (this->stackTable.inGlobalScope()) {
        this->storeVariableInGlobalMemoryInit(inst.result, inst.table);
    } else {
        this->storeVariableInStack(inst.result, 8);
    }
}

void CodeGenerator::generateLabelledInstruction(
    const tac_op_t operation,
    const std::string &labelName
) {
    const std::string instStr = this->tacToInstruction(operation);
    const std::string extLabel = tac_line_t::extract_label(labelName);
    this->context.insertText("\t" + instStr + " " + extLabel);
}

void CodeGenerator::generateLabel(const std::string &labelName) {
    this->context.insertText(
        tac_line_t::extract_label(labelName) + ": "
    );
}

RegPtr CodeGenerator::getRegister(
    const LivenessTable &liveness,
    const std::string &variable,
    const TID &instid,
    const register_type_t &type,
    const bool address
) {
    RegPtr reg;

    if (
        this->addressTable.isInRegister(variable)
        && !liveness.getLivenessAndNextUse(instid).isLive(variable)
    ) {
        reg = this->addressTable.getRegister(variable);
        if (this->globalTable.isGlobal(variable) 
            || this->stackTable.inStack(variable)) {
                this->storeContentFromRegister(reg);
        }
    } 
    else if (this->regTable.atLeastOneRegisterUnused(type)) {
        reg = this->regTable.getUnusedRegister(type);
    } 
    else {
        reg = this->regTable.getARegisterInUse(type);
        this->storeContentFromRegister(reg);
    }

    this->generateMovToRegisterIfInMemory(variable, reg, address);
    this->regTable.setRegisterValue(reg, variable);
    this->addressTable.insert(variable, Location(LT_REGISTER)
        .setReg(reg)
        .setIsRegAddress(address)
    );
    return reg;
}

RegPtr CodeGenerator::forceRegister(
    const LivenessTable &liveness,
    const std::string &variable,
    const TID&instid,
    const register_type_t &type,
    const bool address
) {
    const bool existsOutsideRegister = this->addressTable.contains(variable);

    if (existsOutsideRegister) {
        const Location &location = 
            this->addressTable.getLocation(variable);
        
        if (location.inRegister()) {

            if (location.isRegAddress() && !address) {
                // Allocate a new register to store the value of the address.
                RegPtr reg = 
                    this->getRegister(liveness, variable, instid, type, address);
                
                WARNING_LOG("Generating a movq with the address");
                this->context.insertText(
                    "\tmovq " + location.address(true) + ", " + reg->getName()
                );
                this->regTable.setRegisterValue(reg, variable);
                this->addressTable
                    .insert(variable, Location(LT_REGISTER).setReg(reg));

                return reg;
            }

            return this->addressTable.getLocation(variable).getRegister();
        }
    }

    return this->getRegister(liveness, variable, instid, type, address);
}

void CodeGenerator::generateMovToRegisterIfInMemory(
    const std::string &variable,
    const RegPtr &reg,
    const bool address
) {
    if (this->addressTable.contains(variable)) {
        const Location oldLocation = this->addressTable.getLocation(variable);
        if (!oldLocation.inRegister()) {
            const std::string instStr = (address) ? "leaq" : "movq";

            this->context.insertText(
                "\t" + instStr + " " + oldLocation.address() + ", " + 
                    reg->getName()
            );
        }
    }
}

void CodeGenerator::storeContentFromRegister(RegPtr &reg) {
    ASSERT(reg != nullptr);
    const std::string variable = this->regTable.getVariableInRegister(reg);
    this->storeVariable(variable, reg);
}

void CodeGenerator::storeVariable(
    const std::string &variable, 
    const RegPtr &reg
) {
    if (this->globalTable.isGlobal(variable)) {
        this->storeVariableInGlobalMemory(variable, reg);
    } else {
        this->storeVariableInStack(variable, reg);
    }
}

void CodeGenerator::storeVariableInGlobalMemory(
    const std::string &variable, 
    const RegPtr &reg
) {
    const std::string storeInst = 
        "\tmovq " + reg->getName() + ", " + variable + "(%rip)";
    this->context.insertText(storeInst);
    this->regTable.freeRegister(reg);
    this->addressTable
        .insert(variable, Location(LT_MEMORY_GLOBAL)
        .setImmValueOrGlobal(variable));
}

void CodeGenerator::storeVariableInGlobalMemoryInit(
    const std::string &variable,
    std::shared_ptr<SymbolTable> table
) {
    unsigned int level;
    st_entry_t entry;
    const bool success = table->lookup(variable, &level, &entry);

    ASSERT(success);
    ASSERT(entry.entry_type == ST_VARIABLE);

    if (entry.variable.isArray) {
        const size_t arrSize = entry.variable.arraySize * 8;
        this->context.insertGlobalArray(variable, arrSize);
        this->globalTable.insertGlobalArray(variable, arrSize);
    } else {
        this->context.insertGlobalVariable(variable, 8, 0);
        this->globalTable.insertGlobalVariable(variable, 8);
    }

    this->addressTable
        .insert(variable, Location(LT_MEMORY_GLOBAL)
        .setImmValueOrGlobal(variable));
}

void CodeGenerator::storeVariableInStack(
    const std::string &variable,
    const unsigned int size
) {
    unsigned int offset = this->stackTable.allocate(variable, size);
    this->addressTable
        .insert(variable, Location(LT_MEMORY_STACK).setStack(offset));
}

void CodeGenerator::storeVariableInStack(
    const std::string &variable, 
    const RegPtr &reg
) {
    unsigned int offset;

    if (this->stackTable.inStack(variable)) {
        offset = this->stackTable.getAddress(variable);
    } else {
        offset = this->stackTable.allocate(variable, 8);
    }

    const std::string storeInst = 
        "\tmovq " + reg->getName() + ", " + std::to_string(offset) + "(%rbp)";

    this->regTable.freeRegister(reg);

    this->addressTable
        .insert(variable, Location(LT_MEMORY_STACK).setStack(offset));
}

std::string CodeGenerator::tacToInstruction(const tac_op_t operation) const {
    switch (operation) {
        case TAC_NOP:
            return "nop";
        case TAC_UNCOND_JMP:
            return "jmp";
        case TAC_CALL:
            return "call";
        case TAC_JMP_E:
            return "je";
        case TAC_JMP_LE: // jmpl and jmpg are unused on purpose.
            return "jle";
        case TAC_JMP_GE:
            return "jge";
        case TAC_JMP_NE:
            return "jne";
        case TAC_JMP_ZERO:
            return "jz";
        case TAC_ASSIGN:
            return "movq";
        case TAC_ADD:
            return "add";
        case TAC_SUB:
            return "sub";
        case TAC_MULT:
            return "imul";
        case TAC_DIV:
            return "idiv";
        case TAC_LESS_THAN ... TAC_NOT_EQUALS:
            return "cmp";
        default:
            break;
    }
    ERROR_LOGV("failed to match 3AC to instruction");
    exit(EXIT_FAILURE);
}

std::stack<RegPtr> CodeGenerator::pushRegisters() {
    std::set<RegPtr> toPush = this->regTable.getAllRegistersInUse();
    std::stack<RegPtr> regStack;
    for (auto i = toPush.begin(); i != toPush.end(); i++) {
        regStack.push(*i);
        this->context.insertText("\tpushq " + (*i)->getName());
    }
    return regStack;
}

void CodeGenerator::popRegisters(std::stack<RegPtr> &toPop) {
    while (!toPop.empty()) {
        this->context.insertText("\tpopq " + toPop.top()->getName());
        toPop.pop();
    }
}

void CodeGenerator::freeRegisters() {
    const auto registerLocations = 
        this->addressTable.getValueAndLocationInRegisters();
    
    for (const std::pair<std::string, Location> &p : registerLocations) {
        if (this->globalTable.isGlobal(p.first)) {
            this->storeVariableInGlobalMemory(p.first, p.second.getRegister());
        } 
        else if (this->stackTable.inStack(p.first)) {
            this->storeVariableInStack(p.first, p.second.getRegister());
        }
    }

    this->addressTable.clearRegisters();
    this->regTable.clear();
}
