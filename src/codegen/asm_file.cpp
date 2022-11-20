#include <codegen/asm_file.h>

#include <cstdio>
#include <logging.h>

AsmFile::AsmFile() {}

void AsmFile::insertDataInstruction(const std::string &inst) {
    this->dataSectionLines.push_back(inst);
}

void AsmFile::insertTextInstruction(const std::string &inst) {
    this->textSectionLines.push_back(inst);
}

void AsmFile::insertTextInstruction(
    const std::string &inst, unsigned int offset
) {
    this->textSectionLines.insert(this->textSectionLines.begin()+offset, inst);
}

void AsmFile::replaceTextInstruction(
    const std::string &inst, unsigned int offset
) {
    this->textSectionLines[offset] = inst;
}

const unsigned int AsmFile::getLine() const {
    return this->textSectionLines.size() - 1;
}

const void AsmFile::to_file(const char *fileName) const {
    FILE *file = NULL;

    file = fopen(fileName, "w");

    if (file == NULL) {
        ERROR_LOG("Failed to write to assembly file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, ".data\n");
    for (const std::string &line : this->dataSectionLines) {
        fprintf(file, "%s\n", line);
    }

    fprintf(file, ".text\n");
    for (const std::string &line : this->textSectionLines) {
        fprintf(file, "%s\n", line);
    }

    fclose(file);
}