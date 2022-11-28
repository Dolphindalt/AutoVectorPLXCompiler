#include <codegen/address.h>

#include <codegen/util.h>
#include <logging.h>

Address::Address(std::string offset) : type(A_M64), name("\%rbp"), 
offset("0") {}

Address::Address(address_type_t type, const std::string &name) 
: type(type), name(name), offset("0") {}

Address::Address(
    address_type_t type, const std::string &name, const std::string &offset
) : type(type), name(name), offset(offset) {}

const std::string Address::address() const {
    switch (this->type) {
        case A_R64:
            return this->name;
        case A_RM64:
            return "(" + this->name + ")";
        case A_IMM64:
            return "$" + this->name;
        case A_M64:
            // Dealing with the stack.
            if (this->name == "\%rbp") {
                if (this->offset == "0") {
                    return "(\%rbp)";
                } else {
                    return int_to_hex(this->offset) + "(\%rbp)";
                }
            }

            if (this->name.at(0) == '\%') {
                // A register.
                if (this->offset == "0") {
                    return "(" + this->name + ")";
                } else {
                    return "(" + this->name + ", " + this->offset + ",8)";
                }
            }

            // Dealing with global variables.
            if (this->offset == "0") {
                return this->name;
            } else {
                return this->name + "(," + this->offset + ", 8)";
            }
        default:
            break;
    }

    ERROR_LOG("invalid address type passed");
    exit(EXIT_FAILURE);
}

const bool Address::isRegister() const {
    return this->type == A_R64;
}

const bool Address::isMemoryAddress() const {
    return this->type == A_M64;
}

const bool Address::isGlobal() const {
    return this->type == A_M64 && this->name.size() > 0 
        && this->name.at(0) != '\%';
}

const std::string Address::getName() const {
    return this->name;
}