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
#ifndef ENGLISHDICTIONARY_HPP
#define ENGLISHDICTIONARY_HPP

#include <string>
#include <algorithm>
#include <set>

#include <IDictionaryService/IDictionaryService.hpp>

namespace EnglishDictionary {

class DictionaryImpl : public test::IDictionaryService
  {
  public:

    DictionaryImpl();
    ~DictionaryImpl() override = default;

    /**
     * Implements IDictionaryService::CheckWord(). Determines
     * if the passed in word is contained in the dictionary.
     * @param word the word to be checked.
     * @return true if the word is in the dictionary,
     *         false otherwise.
     **/
    bool CheckWord(const std::string& word) override;
  private:
    // The set of words contained in the dictionary.
    std::set<std::string> m_dictionary;
  };
}

#endif // ENGLISHDICTIONARY_HPP
