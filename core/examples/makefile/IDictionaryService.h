#ifndef IDICTIONARYSERVICE_H
#define IDICTIONARYSERVICE_H

#include <usServiceInterface.h>

#include <string>

#ifdef BUNDLE_EXPORTS
  #define BUNDLE_EXPORT US_ABI_EXPORT
#else
  #define BUNDLE_EXPORT US_ABI_IMPORT
#endif

/**
 * A simple service interface that defines a dictionary service.
 * A dictionary service simply verifies the existence of a word.
 **/
struct BUNDLE_EXPORT IDictionaryService
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
#endif // DICTIONARYSERVICE_H
