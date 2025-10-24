#include "commands.hpp"
#include <algorithm>
#include <fstream>
#include <random>
#include <set>
#include <functional>
#include <iterator>
#include <numeric>
#include "iofmtguard.hpp"

namespace
{
  using namespace kurbyko;

  struct DictionaryFinder
  {
    const std::string& name;

    DictionaryFinder(const std::string& n) : name(n) {}

    bool operator()(const std::pair<std::string, Dictionary>& pair) const
    {
      return pair.first == name;
    }
  };

  struct OptionsSetAccumulator
  {
    const std::string& correctAnswer;
    size_t options;

    OptionsSetAccumulator(const std::string& answer, size_t opt)
      : correctAnswer(answer), options(opt) {}

    std::set<std::string>* operator()(std::set<std::string>* set, const std::string& translation) const
    {
      if (translation != correctAnswer && set->size() < options)
      {
        set->insert(translation);
      }
      return set;
    }
  };

  struct OptionTransformer
  {
    const std::vector<std::string>& optionsVec;

    OptionTransformer(const std::vector<std::string>& vec) : optionsVec(vec) {}

    std::string operator()(const std::string& option) const
    {
      auto it = std::find(optionsVec.begin(), optionsVec.end(), option);
      size_t index = std::distance(optionsVec.begin(), it) + 1;
      return std::to_string(index) + ". " + option;
    }
  };

  struct MergeAccumulator
  {
    Dictionary& resultDict;

    MergeAccumulator(Dictionary& dict) : resultDict(dict) {}

    bool* operator()(bool* addedFlag, const std::pair<std::string, std::string>& pair) const
    {
      if (!resultDict.contains(pair.first) && resultDict.isTranslationUnique(pair.second))
      {
        resultDict.insert(pair.first, pair.second);
        *addedFlag = true;
      }
      return addedFlag;
    }
  };

  struct DiffAccumulator
  {
    Dictionary& resultDict;
    const std::unordered_map<std::string, std::string>& dataB;

    DiffAccumulator(Dictionary& dict, const std::unordered_map<std::string, std::string>& b)
      : resultDict(dict), dataB(b) {}

    bool* operator()(bool* addedFlag, const std::pair<std::string, std::string>& pair) const
    {
      if (dataB.find(pair.first) == dataB.end())
      {
        resultDict.insert(pair.first, pair.second);
        *addedFlag = true;
      }
      return addedFlag;
    }
  };

  struct IntersectAccumulator
  {
    Dictionary& resultDict;
    const std::unordered_map<std::string, std::string>& dataB;

    IntersectAccumulator(Dictionary& dict, const std::unordered_map<std::string, std::string>& b)
      : resultDict(dict), dataB(b) {}

    bool* operator()(bool* addedFlag, const std::pair<std::string, std::string>& pair) const
    {
      auto it = dataB.find(pair.first);
      if (it != dataB.end() && it->second == pair.second)
      {
        resultDict.insert(pair.first, pair.second);
        *addedFlag = true;
      }
      return addedFlag;
    }
  };

  struct QuizAccumulator
  {
    std::istream& in;
    std::ostream& out;
    const Dictionary& dict;
    size_t options;
    int& correct;
    int& incorrect;

    QuizAccumulator(std::istream& i, std::ostream& o, const Dictionary& d, size_t opt, int& corr, int& incorr)
      : in(i), out(o), dict(d), options(opt), correct(corr), incorrect(incorr) {}

    std::pair<int, int> operator()(std::pair<int, int> counts, const std::pair<std::string, std::string>& question) const
    {
      const std::string& correctWord = question.first;
      const std::string& correctAnswer = question.second;

      std::vector<std::string> allTranslations = dict.getAllTranslations();
      std::set<std::string> optionsSet;
      optionsSet.insert(correctAnswer);

      std::accumulate(allTranslations.begin(), allTranslations.end(), &optionsSet,
        OptionsSetAccumulator(correctAnswer, options));

      std::vector<std::string> optionsVec(optionsSet.begin(), optionsSet.end());

      out << "Translate: " << correctWord << "\n";

      std::transform(optionsVec.begin(), optionsVec.end(),
        std::ostream_iterator<std::string>(out, "\n"),
        OptionTransformer(optionsVec));

      out << "Your answer (number): ";
      int userChoice;
      in >> userChoice;

      if (userChoice > 0 && static_cast<size_t>(userChoice) <= optionsVec.size() &&
        optionsVec[static_cast<size_t>(userChoice) - 1] == correctAnswer)
      {
        out << "Correct!\n";
        counts.first++;
      }
      else
      {
        out << "Incorrect! Correct answer: " << correctAnswer << "\n";
        counts.second++;
      }
      out << "---\n";
      return counts;
    }
  };

  struct OutputPrinter
  {
    std::ostream& out;

    OutputPrinter(std::ostream& o) : out(o) {}

    void operator()(const std::string& str) const
    {
      out << str << "\n";
    }
  };

  struct IdentityTransformer
  {
    std::string operator()(const std::string& str) const
    {
      return str;
    }
  };
}

void kurbyko::printHelp()
{
  std::cout << "Command reference:\n";
  std::cout << "1. create <dict_name>\n";
  std::cout << "2. delete_dict <dict_name>\n";
  std::cout << "3. load <dict_name> <filename>\n";
  std::cout << "4. save <dict_name> <filename>\n";
  std::cout << "5. merge <source_dict> <target_dict> <result_dict>\n";
  std::cout << "6. diff <dict_a> <dict_b> <result_dict>\n";
  std::cout << "7. intersect <dict_a> <dict_b> <result_dict>\n";
  std::cout << "8. search <dict_name> <english_word>\n";
  std::cout << "9. add <dict_name> <english_word> <translation>\n";
  std::cout << "10. remove <dict_name> <english_word>\n";
  std::cout << "11. translate <dict_name> <english_word>\n";
  std::cout << "12. replace <dict_name> <english_word> <new_translation>\n";
  std::cout << "13. mask_search <dict_name> <mask>\n";
  std::cout << "14. quiz <dict_name> <questions_count> <options_count>\n";
}

void kurbyko::create(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  std::string name;
  in >> name;

  auto it = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(name));
  if (it != dicts.end())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  dicts[name] = Dictionary();
  out << "OK\n";
}

void kurbyko::delete_dict(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  std::string name;
  in >> name;

  auto it = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(name));
  if (it == dicts.end())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  dicts.erase(it);
  out << "OK\n";
}

void kurbyko::load(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  std::string name, filename;
  in >> name >> filename;

  Dictionary dict;
  if (!dict.loadFromFile(filename))
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  dicts[name] = std::move(dict);
  out << "OK\n";
}

void kurbyko::save(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  std::string name, filename;
  in >> name >> filename;

  auto it = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(name));
  if (it == dicts.end())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  if (!it->second.saveToFile(filename))
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  out << "OK\n";
}

void kurbyko::merge(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  std::string source, target, result;
  in >> source >> target >> result;

  auto sourceIt = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(source));
  auto targetIt = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(target));
  auto resultIt = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(result));

  if (sourceIt == dicts.end() || targetIt == dicts.end() || resultIt != dicts.end())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  Dictionary resultDict = targetIt->second;
  bool added = false;
  const auto& sourceData = sourceIt->second.getData();

  std::accumulate(sourceData.begin(), sourceData.end(), &added, MergeAccumulator(resultDict));

  if (!added)
  {
    out << "<NO CHANGES>\n";
    return;
  }

  dicts[result] = resultDict;
  out << "OK\n";
}

void kurbyko::diff(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  std::string dictA, dictB, result;
  in >> dictA >> dictB >> result;

  auto dictAIt = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(dictA));
  auto dictBIt = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(dictB));
  auto resultIt = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(result));

  if (dictAIt == dicts.end() || dictBIt == dicts.end() || resultIt != dicts.end())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  Dictionary resultDict;
  const auto& dataA = dictAIt->second.getData();
  const auto& dataB = dictBIt->second.getData();
  bool added = false;

  std::accumulate(dataA.begin(), dataA.end(), &added, DiffAccumulator(resultDict, dataB));

  if (!added)
  {
    out << "<EMPTY>\n";
    return;
  }

  dicts[result] = resultDict;
  out << "OK\n";
}

void kurbyko::intersect(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  std::string dictA, dictB, result;
  in >> dictA >> dictB >> result;

  auto dictAIt = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(dictA));
  auto dictBIt = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(dictB));
  auto resultIt = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(result));

  if (dictAIt == dicts.end() || dictBIt == dicts.end() || resultIt != dicts.end())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  Dictionary resultDict;
  const auto& dataA = dictAIt->second.getData();
  const auto& dataB = dictBIt->second.getData();
  bool added = false;

  std::accumulate(dataA.begin(), dataA.end(), &added, IntersectAccumulator(resultDict, dataB));

  if (!added)
  {
    out << "<EMPTY>\n";
    return;
  }

  dicts[result] = resultDict;
  out << "OK\n";
}

void kurbyko::search(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  std::string name, word;
  in >> name >> word;

  auto it = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(name));
  if (it == dicts.end())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  if (it->second.contains(word))
  {
    out << "YES\n";
  }
  else
  {
    out << "NO\n";
  }
}

void kurbyko::add(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  std::string name, english, translation;
  in >> name >> english >> translation;

  auto it = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(name));
  if (it == dicts.end())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  if (it->second.insert(english, translation))
  {
    out << "OK\n";
  }
  else
  {
    throw std::logic_error("<INVALID COMMAND>");
  }
}

void kurbyko::removeWord(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  std::string name, word;
  in >> name >> word;

  auto it = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(name));
  if (it == dicts.end())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  if (it->second.remove(word))
  {
    out << "OK\n";
  }
  else
  {
    throw std::logic_error("<INVALID COMMAND>");
  }
}

void kurbyko::translate(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  iofmtguard guard(out);
  std::string name, word;
  in >> name >> word;

  auto it = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(name));
  if (it == dicts.end())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  std::string translation = it->second.translate(word);
  if (translation.empty())
  {
    out << "<EMPTY>\n";
  }
  else
  {
    out << word << ": " << translation << "\n";
  }
}

void kurbyko::replace(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  std::string name, word, newTranslation;
  in >> name >> word >> newTranslation;

  auto it = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(name));
  if (it == dicts.end())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  if (it->second.replace(word, newTranslation))
  {
    out << "OK\n";
  }
  else
  {
    throw std::logic_error("<INVALID COMMAND>");
  }
}

void kurbyko::mask_search(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  iofmtguard guard(out);
  std::string name, pattern;
  in >> name >> pattern;

  auto it = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(name));
  if (it == dicts.end())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  auto results = it->second.maskSearch(pattern);
  if (results.empty())
  {
    out << "<EMPTY>\n";
  }
  else
  {
    std::transform(results.begin(), results.end(),
      std::ostream_iterator<std::string>(out, "\n"),
      IdentityTransformer());
    out << "Total matches: " << results.size() << "\n";
  }
}

void kurbyko::quiz(dictionaries& dicts, std::istream& in, std::ostream& out)
{
  iofmtguard guard(out);
  std::string name;
  int questions, options;
  in >> name >> questions >> options;

  if (questions <= 0 || options <= 1)
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  auto it = std::find_if(dicts.begin(), dicts.end(), DictionaryFinder(name));
  if (it == dicts.end())
  {
    throw std::logic_error("<INVALID COMMAND>");
  }

  const Dictionary& dict = it->second;
  if (dict.empty())
  {
    out << "<EMPTY>\n";
    return;
  }

  size_t availableWords = dict.size();
  size_t questionsCount = static_cast<size_t>(questions);

  if (availableWords < questionsCount)
  {
    out << "Warning: only " << availableWords << " words available, quiz will use all of them\n";
    questionsCount = availableWords;
  }

  auto quizWords = dict.getQuizOptions(static_cast<int>(questionsCount));
  if (quizWords.empty())
  {
    out << "<EMPTY>\n";
    return;
  }

  int correct = 0;
  int incorrect = 0;

  std::accumulate(quizWords.begin(), quizWords.end(),
    std::make_pair(correct, incorrect),
    QuizAccumulator(in, out, dict, static_cast<size_t>(options), correct, incorrect));

  out << "Correct: " << correct << "\n";
  out << "Incorrect: " << incorrect << "\n";
}
