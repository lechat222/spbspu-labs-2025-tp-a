#include "DataStruct.hpp"
#include <vector>
#include <algorithm>
#include <iterator>
#include <limits>
bool comparator(const hismatova::DataStruct& a, const hismatova::DataStruct& b)
{
  if (a.key1 != b.key1)
  {
    return a.key1 < b.key1;
  }
  if (std::abs(a.key2) != std::abs(b.key2))
  {
    return std::abs(a.key2) < std::abs(b.key2);
  }
  return a.key3.length() < b.key3.length();
}
int main()
{
  using namespace hismatova;
  using InIter = std::istream_iterator< DataStruct >;
  using OutIter = std::ostream_iterator< DataStruct >;
  std::vector< DataStruct > dataVec;
  while (!std::cin.eof())
  {
    std::copy(InIter(std::cin), InIter(), std::back_inserter(dataVec));
    if (std::cin.fail() && !std::cin.eof())
    {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    }
  }
  std::sort(dataVec.begin(), dataVec.end(), comparator);
  std::copy(dataVec.begin(), dataVec.end(), OutIter(std::cout, "\n"));
  return 0;
}
