#ifndef CODE_GEN_CONTEXT_H__
#define CODE_GEN_CONTEXT_H__

#include <vector>
#include <string>

class CodeGenContext {
public:
    CodeGenContext();

    void insertEntry();

    void insertExit();

    void comment(const std::string &content);

    void insertText(const std::string &inst);
    
    void insertGlobalArray(const std::string &name, const unsigned int size);

    void insertGlobalVariable(
        const std::string &name,
        const unsigned int size,
        const unsigned int value
    );

    void to_file(const char *fileName) const;
private:
    std::vector<std::string> &selectSection();

    bool procedureMode;
    std::vector<std::string> textSection;
    std::vector<std::string> procedureSection;
    std::vector<std::string> dataSection;
};

#endif