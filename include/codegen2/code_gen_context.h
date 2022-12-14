/**
 * The code generation context refers to all code generated within a single 
 * compilation unit.
 * 
 * @file code_gen_context.h
 * @file Dalton Caron
*/
#ifndef CODE_GEN_CONTEXT_H__
#define CODE_GEN_CONTEXT_H__

#include <vector>
#include <string>

/**
 * Stores the data that is to be written to an assembly file and performs the 
 * write to the file.
 */
class CodeGenContext {
public:
    CodeGenContext();

    /** Inserts the program entry point. */
    void insertEntry();

    /** Inserts the program exit system call. */
    void insertExit();

    /** 
     * Inserts a comment into the text section. 
     * @param content Text to insert as a comment.
     */
    void comment(const std::string &content);

    /** 
     * Insert a string into the text section. 
     * @param inst Text to instert into the text section.
     */
    void insertText(const std::string &inst);
    
    /** 
     * Insert an array into the data section. 
     * @param name Name of the array.
     * @param size The size of the array in bytes.
     */
    void insertGlobalArray(const std::string &name, const unsigned int size);

    /**
     * Insert a variable into the data section.
     * @param name Name of the variable.
     * @param size Size of the variable in bytes.
     * @param value The literal value of the varialbe.
     * @param alignment The alignment of the variable in memory.
     */
    void insertGlobalVariable(
        const std::string &name,
        const unsigned int size,
        const unsigned int value,
        const unsigned int alignment=8
    );

    /**
     * Writes the assembly context into an assembly file.
     * 
     * @param fileName The name of the file to write to.
     */
    void to_file(const char *fileName) const;
private:
    std::vector<std::string> &selectSection();

    bool procedureMode;
    std::vector<std::string> textSection;
    std::vector<std::string> procedureSection;
    std::vector<std::string> dataSection;
};

#endif