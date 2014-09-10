/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#ifndef ISPELLCHECKSERVICE_H
#define ISPELLCHECKSERVICE_H

//! [service]
#include <usServiceInterface.h>

#include <string>
#include <vector>

#ifdef US_BUILD_SHARED_LIBS
#ifdef Example_spellcheckservice_EXPORTS
  #define SPELLCHECKSERVICE_EXPORT US_ABI_EXPORT
#else
  #define SPELLCHECKSERVICE_EXPORT US_ABI_IMPORT
#endif
#else
  #define SPELLCHECKSERVICE_EXPORT US_ABI_EXPORT
#endif

/**
 * A simple service interface that defines a spell check service. A spell check
 * service checks the spelling of all words in a given passage. A passage is any
 * number of words separated by a space character and the following punctuation
 * marks: comma, period, exclamation mark, question mark, semi-colon, and colon.
 */
struct SPELLCHECKSERVICE_EXPORT ISpellCheckService
{
  // Out-of-line virtual desctructor for proper dynamic cast
  // support with older versions of gcc.
  virtual ~ISpellCheckService();

  /**
   * Checks a given passage for spelling errors. A passage is any number of
   * words separated by a space and any of the following punctuation marks:
   * comma (,), period (.), exclamation mark (!), question mark (?),
   * semi-colon (;), and colon(:).
   *
   * @param passage the passage to spell check.
   * @return A list of misspelled words.
   */
  virtual std::vector<std::string> Check(const std::string& passage) = 0;
};
//! [service]
//!
#endif // ISPELLCHECKSERVICE_H
