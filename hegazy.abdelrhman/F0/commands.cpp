#include "commands.hpp"
#include <iterator>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <functional>
#include <cctype>
#include <string>

namespace bob
{
  namespace
  {
    bool isAlpha(char c)
    {
      return std::isalpha(static_cast<unsigned char>(c));
    }

    char toLowercase(char c)
    {
      return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    bool comparatorAscending(const Word& a, const Word& b)
    {
      return a.second < b.second;
    }

    bool comparatorDescending(const Word& a, const Word& b)
    {
      return a.second > b.second;
    }

    void sortWords(std::vector< Word >& words, const std::string& order)
    {
      if (order == "ascending")
      {
        std::sort(words.begin(), words.end(), comparatorAscending);
      }
      else
      {
        std::sort(words.begin(), words.end(), comparatorDescending);
      }
    }

    bool comparatorFrequency(const Word& word, int low, int high)
    {
      return word.second >= low && word.second <= high;
    }

    struct PrintWord
    {
      std::ostream& out_;
      void operator()(const Word& word) const
      {
        out_ << word.first << ' ' << word.second << '\n';
      }
    };

    struct PrintDicts
    {
      std::ostream& out_;
      void operator()(const std::pair< std::string, Dict >& dict) const
      {
        out_ << dict.first << ' ' << dict.second.size() << '\n';
        std::for_each(dict.second.cbegin(), dict.second.cend(), PrintWord{ out_ });
      }
    };

    bool intersectWord(const Dict& dict, const Word& word)
    {
      return dict.find(word.first) != dict.end();
    }

    Word frequencyUpdate(const Dict& dict, const Word& word)
    {
      auto it = dict.find(word.first);
      return { word.first, std::min(word.second, it->second) };
    }

    void mergeWords(Dict& dict, const Word& word)
    {
      auto& freq = dict[word.first];
      freq += word.second;
    }

    struct PrintNameDicts
    {
      std::ostream& out_;
      void operator()(const std::pair< std::string, Dict >& dict) const
      {
        out_ << dict.first << '\n';
      }
    };

    void readRawWord(Dict& dict, const std::string& word)
    {
      std::string realWord;
      std::copy_if(word.cbegin(), word.cend(), std::back_inserter(realWord), isAlpha);
      std::transform(realWord.begin(), realWord.end(), realWord.begin(), toLowercase);
      if (!realWord.empty())
      {
        dict[realWord]++;
      }
    }

    Word readWordFreq(std::istream& in)
    {
      Word word;
      in >> word.first >> word.second;
      return word;
    }

    Dict readDict(std::istream& in)
    {
      size_t wordCount;
      in >> wordCount;
      Dict dict;
      for (size_t i = 0; i < wordCount; ++i)
      {
        auto word = readWordFreq(in);
        dict.insert(word);
      }
      return dict;
    }

    std::pair< std::string, Dict > readNamedDict(std::istream& in)
    {
      std::pair< std::string, Dict > namedDict;
      in >> namedDict.first;
      namedDict.second = readDict(in);
      return namedDict;
    }
  }

  void createDictionary(std::istream& in, Dicts& dicts)
  {
    std::string dictName;
    in >> dictName;
    if (dicts.find(dictName) != dicts.end())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    dicts[dictName] = Dict{};
  }

  void showDictionary(std::ostream& out, const Dicts& dicts)
  {
    if (dicts.empty())
    {
      out << '\n';
      return;
    }
    std::for_each(dicts.cbegin(), dicts.cend(), PrintNameDicts{ out });
  }

  void loadText(std::istream& in, Dicts& dicts)
  {
    std::string filename, dictName;
    in >> dictName >> filename;
    std::ifstream file(filename);
    if (!file.is_open())
    {
      throw std::runtime_error("<INVALID FILE>");
    }
    auto it = dicts.find(dictName);
    if (it == dicts.end())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    std::string word;
    while (file >> word)
    {
      readRawWord(it->second, word);
    }
  }

  void unionDictionary(std::istream& in, Dicts& dicts)
  {
    std::string dictName1, dictName2, resultName;
    in >> resultName >> dictName1 >> dictName2;
    auto it1 = dicts.find(dictName1);
    auto it2 = dicts.find(dictName2);
    if (it1 == dicts.end() || it2 == dicts.end())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    if (dicts.find(resultName) != dicts.end())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    auto& result = dicts[resultName];
    const auto& dict1 = it1->second;
    const auto& dict2 = it2->second;

    result = dict1;

    for (const auto& word : dict2)
    {
      result[word.first] += word.second;
    }
  }

  void intersectDictionary(std::istream& in, Dicts& dicts)
  {
    std::string dictName1, dictName2, resultName;
    in >> resultName >> dictName1 >> dictName2;
    auto it1 = dicts.find(dictName1);
    auto it2 = dicts.find(dictName2);
    if (it1 == dicts.end() || it2 == dicts.end())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    if (dicts.find(resultName) != dicts.end())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    auto& result = dicts[resultName];
    const auto& dict1 = it1->second;
    const auto& dict2 = it2->second;

    for (const auto& word : dict1)
    {
      auto it = dict2.find(word.first);
      if (it != dict2.end())
      {
        result[word.first] = std::min(word.second, it->second);
      }
    }
  }

  void copyDictionary(std::istream& in, Dicts& dicts)
  {
    std::string resultName, dictName;
    in >> resultName >> dictName;
    auto it = dicts.find(dictName);
    if (it == dicts.end() || dicts.find(resultName) != dicts.end())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    dicts[resultName] = it->second;
  }

  void addWord(std::istream& in, Dicts& dicts)
  {
    std::string dictName, wordName, frequency;
    in >> dictName >> wordName >> frequency;
    auto it = dicts.find(dictName);
    if (it == dicts.end())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    if (it->second.find(wordName) != it->second.end())
    {
      throw std::runtime_error("<INVALID WORD>");
    }
    int num = std::stoi(frequency);
    if (num <= 0)
    {
      throw std::runtime_error("<INVALID NUMBER>");
    }
    it->second[wordName] = num;
  }

  void printSize(std::istream& in, std::ostream& out, const Dicts& dicts)
  {
    std::string dictName;
    in >> dictName;
    auto it = dicts.find(dictName);
    if (it == dicts.cend())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    out << it->second.size() << '\n';
  }

  void cleanWord(std::istream& in, Dicts& dicts)
  {
    std::string dictName, wordName;
    in >> dictName >> wordName;
    auto it = dicts.find(dictName);
    if (it == dicts.end())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    if (it->second.find(wordName) == it->second.end())
    {
      throw std::runtime_error("<INVALID WORD>");
    }
    it->second.erase(wordName);
  }

  void cleanDictionary(std::istream& in, Dicts& dicts)
  {
    std::string dictName;
    in >> dictName;
    auto it = dicts.find(dictName);
    if (it == dicts.end())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    dicts.erase(dictName);
  }

  void printTopRare(std::istream& in, std::ostream& out, const Dicts& dicts, const std::string& order)
  {
    std::string dictName;
    size_t number;
    in >> dictName >> number;
    auto dictIt = dicts.find(dictName);
    if (dictIt == dicts.cend())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    if (number == 0 || number > dictIt->second.size())
    {
      throw std::runtime_error("<INVALID NUMBER>");
    }
    std::vector< Word > words;
    words.reserve(dictIt->second.size());
    const auto& dict = dictIt->second;
    std::copy(dict.cbegin(), dict.cend(), std::back_inserter(words));
    sortWords(words, order);
    std::for_each(words.cbegin(), words.cbegin() + number, PrintWord{ out });
  }

  void printFrequency(std::istream& in, std::ostream& out, const Dicts& dicts)
  {
    std::string dictName, wordName;
    in >> dictName >> wordName;
    auto dictIt = dicts.find(dictName);
    if (dictIt == dicts.cend())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    if (dictIt->second.find(wordName) == dictIt->second.cend())
    {
      throw std::runtime_error("<INVALID WORD>");
    }
    out << dictIt->second.at(wordName) << '\n';
  }

  void createWordRange(std::istream& in, Dicts& dicts)
  {
    std::string resultName, dictName;
    int freq1, freq2;
    in >> resultName >> dictName >> freq1 >> freq2;
    if (freq2 < freq1)
    {
      throw std::runtime_error("<INVALID INTERVAL>");
    }
    auto dictIt = dicts.find(dictName);
    if (dictIt == dicts.end() || dicts.find(resultName) != dicts.end())
    {
      throw std::runtime_error("<INVALID DICTIONARY>");
    }
    auto& result = dicts[resultName];
    const auto& dict = dictIt->second;

    for (const auto& word : dict)
    {
      if (word.second >= freq1 && word.second <= freq2)
      {
        result[word.first] = word.second;
      }
    }

    if (result.empty())
    {
      dicts.erase(resultName);
      throw std::runtime_error("<EMPTY INTERVAL>");
    }
  }

  void saveDictionaries(std::istream& in, const Dicts& dicts)
  {
    std::string fileName;
    in >> fileName;
    std::ofstream file(fileName);
    if (!file.is_open())
    {
      throw std::runtime_error("<INVALID FILE>");
    }
    file << dicts.size() << '\n';
    std::for_each(dicts.cbegin(), dicts.cend(), PrintDicts{ file });
  }

  void loadFile(const std::string& filename, Dicts& dicts)
  {
    std::ifstream file(filename);
    if (!file.is_open())
    {
      throw std::runtime_error("<INVALID FILE>");
    }
    size_t dictCount;
    file >> dictCount;
    for (size_t i = 0; i < dictCount; ++i)
    {
      auto namedDict = readNamedDict(file);
      dicts.insert(namedDict);
    }
  }

  void printHelp(std::ostream& out)
  {
    out << std::left;
    out << "Available commands:\n" << '\n';
    constexpr size_t cmdWidth = 50;
    constexpr size_t numWidth = 4;
    out << std::setw(numWidth) << "1." << std::setw(cmdWidth);
    out << "createdict <dict_name>" << "create a new dictionary\n";

    out << std::setw(numWidth) << "2." << std::setw(cmdWidth);
    out << "showdicts" << "shows all dictionary names\n";

    out << std::setw(numWidth) << "3." << std::setw(cmdWidth);
    out << "loadtext <dict_name> <file>" << "upload text from a file to the dictionary\n";

    out << std::setw(numWidth) << "4." << std::setw(cmdWidth);
    out << "union <result> <dict1> <dict2>" << "creates a new dictionary that combines the other two\n";

    out << std::setw(numWidth) << "5." << std::setw(cmdWidth);
    out << "intersect <result> <dict1> <dict2>" << "creates a new dictionary that intersects the other two\n";

    out << std::setw(numWidth) << "6." << std::setw(cmdWidth);
    out << "copy <result> <dict>" << "copies dictionary data\n";

    out << std::setw(numWidth) << "7." << std::setw(cmdWidth);
    out << "addword <dict_name> <word> <frequency>" << "adding a word to the dictionary\n";

    out << std::setw(numWidth) << "8." << std::setw(cmdWidth);
    out << "size <dict_name>" << "print the number of words in the dictionary\n";

    out << std::setw(numWidth) << "9." << std::setw(cmdWidth);
    out << "cleanword <dict_name> <word>" << "deleting a word from the dictionary\n";

    out << std::setw(numWidth) << "10." << std::setw(cmdWidth);
    out << "cleandict <dict_name>" << "deleting all words from the dictionary\n";

    out << std::setw(numWidth) << "11." << std::setw(cmdWidth);
    out << "top <dict_name> <number>" << "print the most frequently used words\n";

    out << std::setw(numWidth) << "12." << std::setw(cmdWidth);
    out << "rare <dict_name> <number>" << "print the least frequently used words\n";

    out << std::setw(numWidth) << "13." << std::setw(cmdWidth);
    out << "frequency <dict_name> <word>" << "print the word frequency\n";

    out << std::setw(numWidth) << "14." << std::setw(cmdWidth);
    out << "wordrange <result> <dict_name> <freq1> <freq2>";
    out << "creates a dictionary of words whose frequencies are in the range\n";

    out << std::setw(numWidth) << "15." << std::setw(cmdWidth);
    out << "save <file>" << "saves all dictionaries to a file\n";
  }
}
