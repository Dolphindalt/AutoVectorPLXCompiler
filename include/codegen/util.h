#ifndef UTIL_H__
#define UTIL_H__

#include <iomanip>
#include <iostream>
#include <sstream>

template< typename T >
std::string int_to_hex( T i )
{
  std::stringstream stream;
  stream << "0x" 
         << std::setfill ('0') << std::setw(sizeof(T)) 
         << std::hex << i;
  return stream.str();
}

#endif