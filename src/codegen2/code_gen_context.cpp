#include <codegen2/code_gen_context.h>

#include <cstdio>
#include <logging.h>
#include <assertions.h>

CodeGenContext::CodeGenContext() : procedureMode(false) {
    this->insertEntry();
}

void CodeGenContext::insertEntry() {
    this->textSection.push_back(".global _start");
    this->textSection.push_back("_start:");
}

void CodeGenContext::insertExit() {
    this->textSection.push_back("\tmovq $60, \%rax");
    this->textSection.push_back("\tmovq $0, \%rbx");
    this->textSection.push_back("\tsyscall");
}

void CodeGenContext::comment(const std::string &content) {
    this->selectSection().push_back("# " + content);
}

void CodeGenContext::insertText(const std::string &inst) {

    // This will be moved into the dedicated rewrite rule class later.
    if (inst.substr(1, 4) == "movq" 
        && inst.substr(6, 2) == "$0" 
        && inst.substr(10, 1) == "\%") {
            const std::string newInst = "\txorq " + 
                inst.substr(9, std::string::npos) + ", " +
                inst.substr(9, std::string::npos);
            this->selectSection().push_back(newInst);
            return;
    }

    this->selectSection().push_back(inst);
}

void CodeGenContext::insertGlobalArray(
    const std::string &name, 
    const unsigned int size
) {
    const std::string insertion = 
        ".align 8\n" + name + ":\n.zero " + std::to_string(size);
    this->dataSection.push_back(insertion);
}

void CodeGenContext::insertGlobalVariable(
    const std::string &name,
    const unsigned int size,
    const unsigned int value,
    const unsigned int alignment
) {
    ASSERT(size % 8 == 0);
    std::string insertion = ".align " + std::to_string(alignment) + 
        "\n" + name + ":";
    for (unsigned int i = 0; i < size; i += 8) {
        insertion += "\n.quad " + std::to_string(value);
    }
    this->dataSection.push_back(insertion);
}

void CodeGenContext::to_file(const char *fileName) const {
    FILE *file = NULL;

    file = fopen(fileName, "w");

    if (file == NULL) {
        ERROR_LOG("Failed to write to assembly file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, ".data\n");
    for (const std::string &line : this->dataSection) {
        fprintf(file, "%s\n", line.c_str());
    }

    fprintf(file, ".text\n");
    for (const std::string &line : this->textSection) {
        fprintf(file, "%s\n", line.c_str());
    }

    fclose(file);
}

std::vector<std::string> &CodeGenContext::selectSection() {
    if (procedureMode) {
        return this->procedureSection;
    }
    return this->textSection;
}
