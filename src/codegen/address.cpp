#include <codegen/address.h>

#include <codegen/util.h>
#include <logging.h>

Address::Address(address_type_t type, const std::string &name) 
: type(type), name(name), offset(0) {}

Address::Address(
    address_type_t type, const std::string &name, unsigned int offset
) : type(type), name(name), offset(offset) {}

const std::string Address::getAddressModeString() const {
    switch (this->type) {
        case A_REGISTER:
            return "\%" + name;
        case A_LITERAL:
            return name;
        case A_STACK:
            if (this->offset == 0) {
                return "(\%rbp)";
            } else {
                return int_to_hex(this->offset) + "(\%rbp)";
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

const std::string Address::getName() const {
    return this->name;
}