#ifndef DICTIONARY_HPP
#define DICTIONARY_HPP

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace kurbyko
{

  class Dictionary
  {
  public:
    Dictionary() = default;

    bool insert(const std::string& english, const std::string& translation);
    bool remove(const std::string& english);
    bool contains(const std::string& english) const;
    std::string translate(const std::string& english) const;
    bool replace(const std::string& english, const std::string& new_translation);

    std::vector< std::string > maskSearch(const std::string& pattern) const;
    std::vector< std::pair< std::string, std::string > > getQuizOptions(int count) const;

    size_t size() const;
    bool empty() const;
    void clear();

    const std::unordered_map< std::string, std::string >& getData() const;
    std::vector< std::string > getAllWords() const;
    std::vector< std::string > getAllTranslations() const;

    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;

    bool isTranslationUnique(const std::string& translation) const;

  private:
    std::unordered_map< std::string, std::string > data;
  };

}

#endif
