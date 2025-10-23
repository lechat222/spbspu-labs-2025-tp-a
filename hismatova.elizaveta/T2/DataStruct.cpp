#include "DataStruct.hpp"
#include <iomanip>
#include <vector>
#include <ios>

hismatova::StreamGuard::StreamGuard(std::ostream& stream):
  stream_(stream),
  flags_(stream.flags())
{}
hismatova::StreamGuard::~StreamGuard()
{
  stream_.flags(flags_);
}

std::istream& hismatova::operator>>(std::istream& in, hismatova::CharIO&& data)
{
  std::istream::sentry sen(in);
  if (!sen)
  {
    return in;
  }
  char c = '0';
  in >> c;
  if (in && (c != data.ch))
  {
    in.setstate(std::ios::failbit);
  }
  return in;
}

std::istream& hismatova::operator>>(std::istream& in, hismatova::ULLIO&& data)
{
  std::istream::sentry sen(in);
  if (!sen)
  {
    return in;
  }
  in >> data.ref;
  if (!in)
  {
    return in;
  }
  char first, second, third;
  in >> first;
  if (!in || (first != 'u' && first != 'U'))
  {
    in.setstate(std::ios::failbit);
    return in;
  }
  in >> second;
  if (!in || (second != 'l' && second != 'L'))
  {
    in.setstate(std::ios::failbit);
    return in;
  }
  in >> third;
  if (!in || (third != 'l' && third != 'L'))
  {
    in.setstate(std::ios::failbit);
  }
  return in;
}

std::istream& hismatova::operator>>(std::istream& in, hismatova::ComplexIO&& data)
{
  std::istream::sentry sen(in);
  if (!sen)
  {
    return in;
  }
  double real;
  double img;
  using io = hismatova::CharIO;
  in >> io{'#'} >> io{'c'} >> io{'('} >> real >> img >> io{')'};
  if (in)
  {
    data.ref = std::complex< double >(real, img);
  }
  return in;
}

std::istream& hismatova::operator>>(std::istream& in, hismatova::StringIO&& data)
{
  std::istream::sentry sen(in);
  if (!sen)
  {
    return in;
  }
  return std::getline(in >> hismatova::CharIO{'"'}, data.ref, '"');
}

std::istream& hismatova::operator>>(std::istream& in, hismatova::DataStruct& data)
{
  std::istream::sentry sen(in);
  if (!sen)
  {
    return in;
  }
  hismatova::DataStruct inp;
  {
    in >> hismatova::CharIO{'('};
    std::vector< std::string > keys(3);
    for (int i = 0; i < 3; ++i)
    {
      in >> hismatova::CharIO{':'} >> keys[i];
      if (keys[i] == "key1")
      {
        in >> hismatova::ULLIO{inp.key1};
      }
      else if (keys[i] == "key2")
      {
        in >> hismatova::ComplexIO{inp.key2};
      }
      else if (keys[i] == "key3")
      {
        in >> hismatova::StringIO{inp.key3};
      }
      else
      {
        in.setstate(std::ios::failbit);
      }
    }
    in >> hismatova::CharIO{':'} >> hismatova::CharIO{')'};
  }
  if (in)
  {
    data = inp;
  }
  return in;
}

std::ostream& hismatova::operator<<(std::ostream& out, const hismatova::DataStruct& data)
{
  hismatova::StreamGuard guard(out);
  out << std::fixed << std::setprecision(1);
  auto& k1 = data.key1;
  auto r = data.key2.real();
  auto i = data.key2.imag();
  auto& k3 = data.key3;
  out << "(:key1 " << k1 << "ull:key2 #c(" << r << " " << i << "):key3 \"" << k3 << "\":)";
  return out;
}
