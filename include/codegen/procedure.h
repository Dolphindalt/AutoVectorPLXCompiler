#ifndef PROCEDURE_H__
#define PROCEDURE_H__

#include <memory>

#include <codegen/descriptor_table.h>
#include <codegen/liveness_info.h>
#include <codegen/asm_file.h>
#include <codegen/address.h>
#include <codegen/util.h>

class Procedure {
public:
    Procedure(
        AsmFile *asmFile,
        std::vector<tac_line_t>::iterator start,
        std::vector<tac_line_t>::iterator end,
        SymbolTable *cgTable,
        LivenessInfoTable *liveness, 
        RegisterTable *regTable,
        bool isMain
    );
private:
    void generateAssemblyFromTAC(const tac_line_t &instruction);

    bool isValueInMemory(
        const std::string &value,
        std::shared_ptr<SymbolTable> symTable
    ) const;

    Address getLocation(
        const std::string &value,
        std::shared_ptr<SymbolTable> symTable
    );

    reg_t getRegister(
        const std::string &value, 
        std::shared_ptr<SymbolTable> symTable,
        register_type_t type,
        const TID instID
    );

    void generateStackStore(unsigned int offset, reg_t reg);

    void allocateProcedureArguments(const tac_line_t &procedureLabel);

    void insertPrologue();

    void instertEpilogue();

    void insertDataSection();

    AsmFile *asmFile;
    SymbolTable *cgTable;
    LivenessInfoTable *liveness;
    RegisterTable *regTable;
    AddressTable stack;
    bool isMain;
    unsigned int functionPrelog;
    unsigned int stackSize;
};

#endif