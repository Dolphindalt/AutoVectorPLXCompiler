#ifndef ASM_GENERATOR_H__
#define ASM_GENERATOR_H__

#include <symbol_table.h>
#include <3ac.h>

class AssemblyGenerator {
public:
    AssemblyGenerator();
    virtual ~AssemblyGenerator();

    void generateAssembly(const std::vector<tac_line_t> &instructions);
private:
    void populateSymbolTableDefaults(
        const std::vector<tac_line_t> &instructions
    );

    void initVariableInTable(const std::string &variable);
    bool variableIsDefinedAndNotLabel(const std::string &variable) const;
    bool isUserDefinedVariable(const std::string &variable) const;
    bool isLabel(const std::string &variable) const;

    SymbolTable symTable;
};

#endif