#include "DataStruct.hpp"

int main()
{
  using dStruct::DataStruct;

  std::vector<DataStruct> data;
  while (!std::cin.eof())
  {
    if (!std::cin)
    {
      std::cin.clear();
    }
    std::copy(
      std::istream_iterator<DataStruct>(std::cin),
      std::istream_iterator<DataStruct>(),
      std::back_inserter(data)
    );
    if (!std::cin)
    {
      std::cin.ignore(100, '\n');
    }

  }


  std::sort(data.begin(), data.end(), [](const DataStruct& a, const DataStruct& b) {
    if (a.key1 != b.key1) return a.key1 < b.key1;
    if (a.key2 != b.key2) return a.key2 < b.key2;
    return a.key3.size() < b.key3.size();
    });


  std::copy(
    data.begin(),
    data.end(),
    std::ostream_iterator<DataStruct>(std::cout, "\n")
  );

  return 0;
}
