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
private:
    address_type_t type;
    std::string name;
    unsigned int offset;
};

#endif