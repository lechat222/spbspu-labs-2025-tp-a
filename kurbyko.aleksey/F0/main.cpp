#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include "commands.hpp"

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cerr << "Wrong arguments\n";
    return 1;
  }

  using namespace kurbyko;

  dictionaries dictOfDicts;
  std::string inputCmd(argv[1]);

  if (inputCmd == "--help")
  {
    printHelp();
  }

  if (inputCmd == "--check")
  {
    if (argc < 3)
    {
      std::cerr << "Usage: dictionary --check <filename>\n";
      return 1;
    }

    Dictionary testDict;
    if (testDict.loadFromFile(argv[2]))
    {
      std::cout << "File structure is correct\n";
    }
    else
    {
      std::cout << "Invalid file structure\n";
    }
    return 0;
  }

  if (argc >= 2)
  {
    Dictionary initialDict;
    if (initialDict.loadFromFile(argv[1]))
    {
      dictOfDicts["main"] = initialDict;
    }
  }

  std::map< std::string, std::function< void(dictionaries&, std::istream&, std::ostream&) > > commands;

  commands["create"] = create;
  commands["delete_dict"] = delete_dict;
  commands["load"] = load;
  commands["save"] = save;
  commands["merge"] = merge;
  commands["diff"] = diff;
  commands["intersect"] = intersect;
  commands["search"] = search;
  commands["add"] = add;
  commands["remove"] = removeWord;
  commands["translate"] = translate;
  commands["replace"] = replace;
  commands["mask_search"] = mask_search;
  commands["quiz"] = quiz;

  // Use loop for command input (allowed in main)
  std::string cmd;
  while (std::cin >> cmd)
  {
    try
    {
      if (cmd == "exit" || cmd == "quit")
      {
        break;
      }

      auto it = commands.find(cmd);
      if (it != commands.end())
      {
        it->second(dictOfDicts, std::cin, std::cout);
      }
      else
      {
        std::cout << "<INVALID COMMAND>\n";
        std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
      }
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << '\n';
      std::cin.clear();
      std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    }
  }

  return 0;
}
