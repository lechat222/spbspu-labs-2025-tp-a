#ifndef DATASTRUCT_HPP
#define DATASTRUCT_HPP

#include <iostream>
#include <complex>
#include <string>

namespace hismatova
{
  struct DataStruct
  {
    unsigned long long key1 = 0;
    std::complex< double > key2 = {0, 0};
    std::string key3 = "";
  };
  inline bool isEmpty(const DataStruct& data)
  {
    return data.key1 == 0 && data.key2.real() == 0 && data.key2.imag() == 0 && data.key3.empty();
  }
  std::istream& operator>>(std::istream& in, DataStruct& data);
  std::ostream& operator<<(std::ostream& out, const DataStruct& data);
  struct CharIO
  {
    char ch;
  };
  std::istream& operator>>(std::istream& in, CharIO&& data);
  struct ULLIO
  {
    unsigned long long& ref;
  };
  std::istream& operator>>(std::istream& in, ULLIO&& data);
  struct ComplexIO
  {
    std::complex< double >& ref;
  };
  std::istream& operator>>(std::istream& in, ComplexIO&& data);
  struct StringIO
  {
    std::string& ref;
  };
  std::istream& operator>>(std::istream& in, StringIO&& data);
  class StreamGuard
  {
  public:
    explicit StreamGuard(std::ostream& stream);
    ~StreamGuard();
  private:
    std::ostream& stream_;
    std::ios::fmtflags flags_;
  };
}

#endif
