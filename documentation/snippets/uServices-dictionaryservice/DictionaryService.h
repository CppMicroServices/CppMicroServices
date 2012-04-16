#ifndef DICTIONARYSERVICE_H
#define DICTIONARYSERVICE_H

#include <usServiceInterface.h>

#include <string>

/**
 * A simple service interface that defines a dictionary service.
 * A dictionary service simply verifies the existence of a word.
 **/
struct DictionaryService
{
  virtual ~DictionaryService() {}

  /**
   * Check for the existence of a word.
   * @param word the word to be checked.
   * @return true if the word is in the dictionary,
   *         false otherwise.
   **/
  virtual bool checkWord(const std::string& word) = 0;
};

US_DECLARE_SERVICE_INTERFACE(DictionaryService, "DictionaryService/1.0")

#endif // DICTIONARYSERVICE_H
