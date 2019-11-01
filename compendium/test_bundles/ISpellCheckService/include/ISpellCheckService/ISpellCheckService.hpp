/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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
#ifndef _ISPELLCHECKSERVICE_HPP_
#define _ISPELLCHECKSERVICE_HPP_
#include "ISpellCheckService/ISpellCheckServiceExport.h"

#include <string>
#include <vector>

namespace test {

class US_ISpellCheckService_EXPORT ISpellCheckService
{
  public:
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

}
#endif // _ISPELLCHECKSERVICE_HPP_
