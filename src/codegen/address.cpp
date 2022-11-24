#include <codegen/address.h>

#include <codegen/util.h>
#include <logging.h>

Address::Address(address_type_t type, const std::string &name) 
: type(type), name(name), offset(0) {}

Address::Address(
    address_type_t type, const std::string &name, unsigned int offset
) : type(type), name(name), offset(offset) {}

const std::string Address::getAddressModeString(RegisterTable *regt) const {
    switch (this->type) {
        case A_REGISTER:
            if (regt->isRegisterHoldingAddress(this->name)) {
                return "(\%" + this->name + ")";
            }
            return "\%" + this->name;
        case A_LITERAL:
            return "$" + this->name;
        case A_STACK:
            if (this->offset == 0) {
                return "(\%rbp)";
            } else {
                return int_to_hex(this->offset) + "(\%rbp)";
            }
        case A_GLOBAL:
            if (this->offset == 0) {
                return this->name + "(\%rip)";
            } else {
                return this->name + "(," + std::to_string(this->offset) + ", 8)";
            }
        default:
            break;
    }

    ERROR_LOG("invalid address type passed");
    exit(EXIT_FAILURE);
}

const bool Address::isRegister() const {
    return this->type == A_REGISTER;
}

const bool Address::isMemoryAddress() const {
    return this->type == A_GLOBAL;
}

const std::string Address::getName() const {
    return this->name;
}