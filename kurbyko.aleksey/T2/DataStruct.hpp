#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include <iostream>
#include <vector>
#include <iterator>
#include <iomanip>
#include <string>
#include <algorithm>
#include <cctype>
#include <bitset>

namespace dStruct
{
  struct DataStruct
  {
    unsigned long long key1;
    char key2;
    std::string key3;
  };

  struct DelimiterIO
  {
    char exp;
  };

  struct ULLBinary
  {
    unsigned long long& value;
  };

  struct CharI
  {
    char& value;
  };

  struct StrKeyI
  {
    std::string& value;
  };

  struct StringIO
  {
    const char* expected;
  };

  std::istream& operator>>(std::istream& in, DelimiterIO&& dest);
  std::istream& operator>>(std::istream& in, ULLBinary&& key);
  std::istream& operator>>(std::istream& in, CharI&& key);
  std::istream& operator>>(std::istream& in, StrKeyI&& key);
  std::istream& operator>>(std::istream& in, const StringIO&& dest);
  std::istream& operator>>(std::istream& in, DataStruct& dest);
  std::ostream& operator<<(std::ostream& out, const DataStruct& dest);

}

#endif
