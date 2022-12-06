#ifndef ASM_FILE_H__
#define ASM_FILE_H__

#include <vector>
#include <string>

class AsmFile {
public:
    AsmFile();

    void insertDataInstruction(const std::string &inst);
    void insertRODataInstruction(const std::string &inst);
    void insertTextInstruction(const std::string &inst);
    void insertTextInstruction(const std::string &inst, unsigned int offset);
    void replaceTextInstruction(const std::string &inst, unsigned int offset);

    const unsigned int getLine() const;
    const void to_file(const char *fileName) const;
private:
    std::vector<std::string> dataSectionLines;
    std::vector<std::string> roDataSectionLines;
    std::vector<std::string> textSectionLines;
};

#endif