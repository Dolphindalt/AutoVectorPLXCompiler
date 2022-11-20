#ifndef PROCEDURE_H__
#define PROCEDURE_H__

#include <codegen/descriptor_table.h>
#include <codegen/liveness_info.h>
#include <codegen/asm_file.h>

class Procedure {
public:
    static DataSection data;

    Procedure(
        AsmFile *asmFile,
        std::vector<tac_line_t> &instructions,
        SymbolTable *symTable,
        LivenessInfoTable *liveness, 
        RegisterTable *regTable,
        bool isMain
    );
private:
    void generateAssemblyFromTAC(const tac_line_t &instruction);
    reg_t getRegister(const std::string &value);

    AsmFile *asmFile;
    SymbolTable *symTable;
    LivenessInfoTable *liveness;
    RegisterTable *regTable;
    AddressTable stack;
    bool isMain;
    unsigned int functionPrelog;
};

#endif