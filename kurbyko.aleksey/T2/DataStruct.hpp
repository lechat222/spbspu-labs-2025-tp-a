#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include <iostream>
#include <vector>
#include <iterator>
#include <iomanip>
#include <string>

namespace dStruct
{
  struct DataStruct
  {
    unsigned long long key1;
    char key2;
    std::string key3;
  };


  std::istream& operator>>(std::istream& in, DataStruct& dest);
  std::ostream& operator<<(std::ostream& out, const DataStruct& dest);


}

#endif
