#include "dictionary.hpp"
#include <algorithm>
#include <fstream>
#include <random>
#include <regex>
#include <sstream>
#include <limits>
#include <iterator>
#include <functional>
#include <numeric>

namespace
{
  using namespace kurbyko;
  
  struct TranslationUniquenessChecker
  {
    const std::string& translation;

    TranslationUniquenessChecker(const std::string& trans) : translation(trans) {}

    bool operator()(const std::pair<std::string, std::string>& pair) const
    {
      return pair.second == translation;
    }
  };

  struct PatternBuilder
  {
    std::string* operator()(std::string* regexPattern, char c) const
    {
      switch (c)
      {
      case '*': *regexPattern += ".*"; break;
      case '?': *regexPattern += "."; break;
      case '[': *regexPattern += "["; break;
      case ']': *regexPattern += "]"; break;
      default:
        if (std::string(".^$()+{}|\\").find(c) != std::string::npos)
        {
          *regexPattern += '\\';
        }
        *regexPattern += c;
        break;
      }
      return regexPattern;
    }
  };

  struct MaskSearchAccumulator
  {
    const std::regex& re;
    std::vector<std::string>& result;

    MaskSearchAccumulator(const std::regex& regex, std::vector<std::string>& res)
      : re(regex), result(res) {}

    std::vector<std::string>* operator()(std::vector<std::string>* res,
      const std::pair<std::string, std::string>& pair) const
    {
      if (std::regex_match(pair.first, re))
      {
        res->push_back(pair.first + ": " + pair.second);
      }
      return res;
    }
  };

  struct WordAccumulator
  {
    std::vector<std::string>* operator()(std::vector<std::string>* words,
      const std::pair<std::string, std::string>& pair) const
    {
      words->push_back(pair.first);
      return words;
    }
  };

  struct TranslationAccumulator
  {
    std::vector<std::string>* operator()(std::vector<std::string>* translations,
      const std::pair<std::string, std::string>& pair) const
    {
      translations->push_back(pair.second);
      return translations;
    }
  };
}

struct LineProcessor
{
  Dictionary& dict;

  LineProcessor(Dictionary& d) : dict(d) {}

  Dictionary* operator()(Dictionary* dictPtr, const std::string& line) const
  {
    std::istringstream iss(line);
    std::string english, translation;

    if (std::getline(iss, english, ':') && std::getline(iss, translation))
    {
      english.erase(0, english.find_first_not_of(" \t"));
      english.erase(english.find_last_not_of(" \t") + 1);
      translation.erase(0, translation.find_first_not_of(" \t"));
      translation.erase(translation.find_last_not_of(" \t") + 1);

      if (!english.empty() && !translation.empty())
      {
        dict.insert(english, translation);
      }
    }
    return dictPtr;
  }
};

struct FileWriter
{
  std::ofstream* operator()(std::ofstream* file,
    const std::pair<std::string, std::string>& pair) const
  {
    *file << pair.first << ":" << pair.second << "\n";
    return file;
  }
};

struct WordPairTransformer
{
  const Dictionary& dict;

  WordPairTransformer(const Dictionary& d) : dict(d) {}

  std::pair<std::string, std::string> operator()(const std::string& word) const
  {
    return std::make_pair(word, dict.translate(word));
  }
};

namespace kurbyko
{

  bool Dictionary::insert(const std::string& english, const std::string& translation)
  {
    if (contains(english) || !isTranslationUnique(translation))
    {
      return false;
    }

    data[english] = translation;
    return true;
  }

  bool Dictionary::remove(const std::string& english)
  {
    return data.erase(english) > 0;
  }

  bool Dictionary::contains(const std::string& english) const
  {
    return data.find(english) != data.end();
  }

  std::string Dictionary::translate(const std::string& english) const
  {
    auto it = data.find(english);
    return it != data.end() ? it->second : "";
  }

  bool Dictionary::replace(const std::string& english, const std::string& new_translation)
  {
    if (!contains(english) || !isTranslationUnique(new_translation))
    {
      return false;
    }

    data[english] = new_translation;
    return true;
  }

  bool Dictionary::isTranslationUnique(const std::string& translation) const
  {
    return std::find_if(data.begin(), data.end(), TranslationUniquenessChecker(translation)) == data.end();
  }

  std::vector<std::string> Dictionary::maskSearch(const std::string& pattern) const
  {
    std::vector<std::string> result;

    std::string regexPattern = "^";
    std::accumulate(pattern.begin(), pattern.end(), &regexPattern, PatternBuilder());
    regexPattern += "$";

    std::regex re(regexPattern, std::regex::icase);
    std::accumulate(data.begin(), data.end(), &result, MaskSearchAccumulator(re, result));

    return result;
  }

  std::vector< std::pair< std::string, std::string > > Dictionary::getQuizOptions(int count) const
  {
    std::vector< std::pair< std::string, std::string > > result;
    std::vector< std::string > words = getAllWords();

    if (words.empty() || count <= 0)
    {
      return result;
    }

    if (count > words.size())
    {
      count = words.size();
    }

    result.resize(count);
    std::transform(words.begin(), words.begin() + count, result.begin(),
      WordPairTransformer(*this));

    return result;
  }

  size_t Dictionary::size() const
  {
    return data.size();
  }

  bool Dictionary::empty() const
  {
    return data.empty();
  }

  void Dictionary::clear()
  {
    data.clear();
  }

  const std::unordered_map< std::string, std::string >& Dictionary::getData() const
  {
    return data;
  }

  std::vector< std::string > Dictionary::getAllWords() const
  {
    std::vector< std::string > words;
    std::accumulate(data.begin(), data.end(), &words, WordAccumulator());
    return words;
  }

  std::vector< std::string > Dictionary::getAllTranslations() const
  {
    std::vector< std::string > translations;
    std::accumulate(data.begin(), data.end(), &translations, TranslationAccumulator());
    return translations;
  }

  bool Dictionary::loadFromFile(const std::string& filename)
  {
    std::ifstream file(filename);
    if (!file.is_open())
    {
      return false;
    }

    clear();

    std::vector<std::string> lines;
    std::copy(std::istream_iterator<std::string>(file),
      std::istream_iterator<std::string>(),
      std::back_inserter(lines));

    std::accumulate(lines.begin(), lines.end(), this, LineProcessor(*this));

    return true;
  }

  bool Dictionary::saveToFile(const std::string& filename) const
  {
    std::ofstream file(filename);
    if (!file.is_open())
    {
      return false;
    }

    std::accumulate(data.begin(), data.end(), &file, FileWriter());

    return true;
  }

}
