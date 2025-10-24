#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iostream>
#include <map>
#include <string>
#include <stdexcept>
#include <functional>
#include "dictionary.hpp"
#include "iofmtguard.hpp"

namespace kurbyko
{

  using dictionaries = std::map< std::string, Dictionary >;

  void create(dictionaries& dicts, std::istream& in, std::ostream& out);
  void delete_dict(dictionaries& dicts, std::istream& in, std::ostream& out);
  void load(dictionaries& dicts, std::istream& in, std::ostream& out);
  void save(dictionaries& dicts, std::istream& in, std::ostream& out);
  void merge(dictionaries& dicts, std::istream& in, std::ostream& out);
  void diff(dictionaries& dicts, std::istream& in, std::ostream& out);
  void intersect(dictionaries& dicts, std::istream& in, std::ostream& out);
  void search(dictionaries& dicts, std::istream& in, std::ostream& out);
  void add(dictionaries& dicts, std::istream& in, std::ostream& out);
  void removeWord(dictionaries& dicts, std::istream& in, std::ostream& out);
  void translate(dictionaries& dicts, std::istream& in, std::ostream& out);
  void replace(dictionaries& dicts, std::istream& in, std::ostream& out);
  void mask_search(dictionaries& dicts, std::istream& in, std::ostream& out);
  void quiz(dictionaries& dicts, std::istream& in, std::ostream& out);

  void printHelp();

}

#endif
