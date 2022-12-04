#ifndef ASM_GENERATOR_H__
#define ASM_GENERATOR_H__

#include <symbol_table.h>
#include <3ac.h>
#include <basic_block.h>
#include <codegen/descriptor_table.h>
#include <codegen/liveness_info.h>
#include <codegen/asm_file.h>
#include <codegen/address.h>
#include <codegen/util.h>
#include <optimizer/block_types.h>

typedef struct array_index {
    Address array;
    Address index;
} array_index_t;

class AssemblyGenerator {
public:
    AssemblyGenerator();
    virtual ~AssemblyGenerator();

    void generateAssembly(const BlockSet &blockSet);
private:
    void generateAssemblyFromTAC(const tac_line_t &);

    Address getLocation(
        const std::string &value,
        const tac_line_t &inst,
        const register_type_t type
    );

    reg_t getRegister(
        const std::string &value, 
        const tac_line_t &inst,
        register_type_t type
    );

    Address allocateMemory(
        const std::string &value,
        std::shared_ptr<SymbolTable> symTable
    );

    void generateStackStore(unsigned int offset, const reg_t &reg);

    void generateDataStore(const std::string &name, const reg_t &reg);

    void allocateProcedureArguments(const tac_line_t &procedureLabel);

    void insertPrologue();

    void instertEpilogue();

    void insertDataSection();

    Address forceAddressIntoRegister(
        const std::string &value,
        const tac_line_t &inst,
        register_type_t type,
        bool isAddress
    );

    void freeRegisters();

    void insertArrayVariable(
        const std::string &name, 
        const Address &array, 
        const Address &index
    );
    void removeArrayVariable(const std::string &name);
    bool isVariableInArray(const std::string &name) const; 
    array_index_t getArrayIndex(const std::string &name) const;

    SymbolTable symTable;
    RegisterTable regTable;
    AddressTable stack;
    DataSection data;
    AsmFile asmFile;
    LivenessInfoTable liveness;

    bool isMain;
    unsigned int basePointer;
    unsigned int functionPrelog;
    unsigned int lastFunctionExitMarker;

    std::map<std::string, array_index_t> arrayVariable;
};

#endif