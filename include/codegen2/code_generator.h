/**
 * This file is the main entry point for the code generation compilation phase. 
 * The primary input is three address code that is converted into assembly.
 * The code that is generated is x86_64 assembly code.
 * 
 * @file code_generator.h
 * @author Dalton Caron
 */
#ifndef CODE_GENERATOR_H__
#define CODE_GENERATOR_H__

#include <map>
#include <string>
#include <codegen2/registers.h>
#include <codegen2/liveness.h>
#include <codegen2/address_table.h>
#include <codegen2/globals_table.h>
#include <codegen2/stack_table.h>
#include <codegen2/code_gen_context.h>
#include <optimizer/block_types.h>

/**
 * Generates assembly code from the three address code intermediate 
 * representation.
 */
class CodeGenerator {
public:
    /** Default constructor for stack initialization. */
    CodeGenerator();

    /** 
     * Generates assembly code to a file from the input three address code in 
     * the form of basic blocks. 
     * 
     * @param blocks Basic blocks to translate into assembly.
     */
    void generate(const BlockSet &blocks);
private:
    void generateFromBB(const BBP &bb);

    void generateFrom3AC(
        const tac_line_t &inst,
        const LivenessTable &liveness
    );

    void generateWrite(const std::string &variable);

    void generateSpecialAssignment(
        const tac_line_t &inst,
        const LivenessTable &liveness
    );

    void convertGeneral3AC(
        const tac_line_t &inst,
        const LivenessTable &liveness
    );

    void generateConditional(
        const tac_line_t &inst,
        const LivenessTable &liveness
    );

    void generateArrayIndex(
        const tac_line_t &inst,
        const LivenessTable &liveness
    );

    void generateVaribleDeclaration(
        const tac_line_t &inst
    );

    void generateLabelledInstruction(
        const tac_op_t operation,
        const std::string &labelName
    );

    void generateLabel(const std::string &labelName);

    void generateGeneralYmmOperation(
        const tac_line_t &inst,
        const LivenessTable &liveness
    );

    void generateYmmLoad(
        const LivenessTable &liveness,
        const tac_line_t &inst
    );

    void generateYmmStore(
        const LivenessTable &liveness,
        const tac_line_t &inst
    );

    void generateYmmAssign(
        const LivenessTable &liveness,
        const tac_line_t &inst
    );

    Location getLargeImmediate(const Location immediate);

    void convertImmediateInto256MemoryRegion(const Location immediate);

    RegPtr getRegister(
        const LivenessTable &liveness,
        const std::string &variable, 
        const TID &instid,
        const register_type_t &type,
        const bool address=false
    );

    RegPtr forceRegister(
        const LivenessTable &liveness,
        const std::string &variable,
        const TID&instid,
        const register_type_t &type,
        const bool address=false
    );

    void generateMovToRegisterIfInMemory(
        const std::string &varible,
        const RegPtr &reg,
        const bool address=false
    );

    void storeContentFromRegister(RegPtr &reg);

    void storeVariable(const std::string &variable, const RegPtr &reg);

    void storeVariableInGlobalMemory(
        const std::string &variable, 
        const RegPtr &,
        const bool updated=true
    );

    void storeVariableInGlobalMemoryInit(
        const std::string &variable,
        std::shared_ptr<SymbolTable> table
    );

    void storeVariableInStack(
        const std::string &variable,
        const unsigned int size
    );

    void storeVariableInStack(
        const std::string &variable, 
        const RegPtr &reg,
        const bool updated=true
    );

    void insertGlobal(const std::string &variable);
    bool isGlobal(const std::string &variable) const;

    std::string tacToInstruction(const tac_op_t operation) const;

    std::stack<RegPtr> pushRegisters();

    void popRegisters(std::stack<RegPtr> &toPop);

    void freeRegisters(const LivenessTable &liveness);

    RegisterAllocationTable regTable;
    AddressTable addressTable;
    GlobalTable globalTable;
    StackTable stackTable;
    CodeGenContext context;
};

#endif