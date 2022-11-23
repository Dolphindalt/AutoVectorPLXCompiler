#ifndef ADDRESS_H__
#define ADDRESS_H__

#include <string>

typedef enum address_type {
    A_REGISTER,
    A_LITERAL,
    A_STACK,
    A_GLOBAL
} address_type_t;

class Address {
public:
    Address(address_type_t type, const std::string &name);
    Address(address_type_t type, const std::string &name, unsigned int offset);

    const std::string getAddressModeString() const;
    const bool isRegister() const;

    /**
     * Returns the name field which acts differently depending on the contents 
     * of the address.
     * A_REGISTER: returns the register name.
     * A_LITERAL: returns the literal value.
     * A_STACK: returns the variable name on the stack.
     * A_GLOBAL: returns the variable name in the data section. 
     */
    const std::string getName() const;
private:
    address_type_t type;
    std::string name;
    unsigned int offset;
};

#endif