#ifndef ADDRESS_H__
#define ADDRESS_H__

#include <string>
#include <codegen/descriptor_table.h>

typedef enum address_type {
    A_R64 = 0x01,
    A_M64 = 0x02,
    A_IMM64 = 0x04,
    A_RM64 = 0x08
} address_type_t;

class Address {
public:
    Address(std::string offset);
    Address(address_type_t type, const std::string &name);
    Address(
        address_type_t type, const std::string &name, const std::string &offset
    );

    const std::string address() const;
    const bool isRegister() const;
    const bool isMemoryAddress() const;
    const bool isGlobal() const;
    const std::string getName() const;
private:
    address_type_t type;
    std::string name;
    std::string offset;
};

#endif