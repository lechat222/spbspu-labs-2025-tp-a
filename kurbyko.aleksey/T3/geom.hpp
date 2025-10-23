#ifndef GEOM_HPP
#define GEOM_HPP

#include <vector>
#include <iostream>

namespace kurbyko
{
  struct Point
  {
    int x;
    int y;
  };

  std::istream& operator>>(std::istream&, Point&);
  std::ostream& operator<<(std::ostream&, const Point&);
  bool operator==(const Point&, const Point&);
}
#endif
