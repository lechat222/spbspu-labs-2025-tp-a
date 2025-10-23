#include "geom.hpp"
#include <algorithm>
#include <iterator>
#include "iofmtguard.hpp"
#include "DelimiterIO.hpp"

std::istream& kurbyko::operator>>(std::istream& in, Point& point)
{
  std::istream::sentry sentry(in);
  if (!sentry)
  {
    return in;
  }
  using sep = DelimiterIO;
  return in >> sep{ '(' } >> point.x >> sep{ ';' } >> point.y >> sep{ ')' };
}

std::ostream& kurbyko::operator<<(std::ostream& out, const Point& point)
{
  std::ostream::sentry sentry(out);
  if (!sentry)
  {
    return out;
  }
  iofmtguard guard(out);
  return out << '(' << point.x << ';' << point.y << ')';
}

bool kurbyko::operator==(const Point& point1, const Point& point2)
{
  return point1.x == point2.x && point1.y == point2.y;
}
