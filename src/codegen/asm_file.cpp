#include <codegen/asm_file.h>

#include <cstdio>
#include <logging.h>
#include <assertions.h>

AsmFile::AsmFile() {}

void AsmFile::insertDataInstruction(const std::string &inst) {
    // All variables are aligned to 8 bytes.
    this->dataSectionLines.push_back(".align 8");
    this->dataSectionLines.push_back(inst);
}

void AsmFile::insertRODataInstruction(const std::string &inst) {
    this->roDataSectionLines.push_back(inst);
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
    return this->textSectionLines.size();
}

const void AsmFile::to_file(const char *fileName) const {
    FILE *file = NULL;

    file = fopen(fileName, "w");

    if (file == NULL) {
        ERROR_LOG("Failed to write to assembly file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, ".section\t.rodata\n");
    for (const std::string &line : this->roDataSectionLines) {
        fprintf(file, "%s\n", line.c_str());
    }

    fprintf(file, ".data\n");
    for (const std::string &line : this->dataSectionLines) {
        fprintf(file, "%s\n", line.c_str());
    }

    fprintf(file, ".text\n");
    for (const std::string &line : this->textSectionLines) {
        fprintf(file, "%s\n", line.c_str());
    }

    fclose(file);
}