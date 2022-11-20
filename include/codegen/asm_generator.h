#ifndef ASM_GENERATOR_H__
#define ASM_GENERATOR_H__

#include <symbol_table.h>
#include <3ac.h>
#include <basic_block.h>
#include <codegen/descriptor_table.h>
#include <codegen/liveness_info.h>

class AssemblyGenerator {
public:
    AssemblyGenerator();
    virtual ~AssemblyGenerator();

    void generateAssembly(std::vector<tac_line_t> &instructions);
private:
    void reorderProcedureDeclarationsFirst(std::vector<tac_line_t> &instructions); 
    void populateSymbolTableDefaults(std::vector<tac_line_t> &instructions);
    void computeLiveness(std::vector<tac_line_t> &instructions);

    void initVariableInTable(const std::string &variable);
    bool variableIsDefinedAndNotLabel(const std::string &variable) const;
    bool isUserDefinedVariable(const std::string &variable) const;
    bool isLabel(const std::string &variable) const;

    SymbolTable symTable;
    RegisterTable regTable;
    LivenessInfoTable liveness;
};

#endif