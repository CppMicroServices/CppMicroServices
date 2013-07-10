#ifndef IDICTIONARYSERVICE_H
#define IDICTIONARYSERVICE_H

#include <usServiceInterface.h>

#include <string>

/**
 * A simple service interface that defines a dictionary service.
 * A dictionary service simply verifies the existence of a word.
 **/
struct IDictionaryService
{
  virtual ~IDictionaryService();

  /**
   * Check for the existence of a word.
   * @param word the word to be checked.
   * @return true if the word is in the dictionary,
   *         false otherwise.
   **/
  virtual bool CheckWord(const std::string& word) = 0;
};

US_DECLARE_SERVICE_INTERFACE(IDictionaryService, "IDictionaryService/1.0")

#endif // DICTIONARYSERVICE_H
