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
#include "FrenchDictionary.hpp"

namespace DSFrenchDictionary {

  DictionaryImpl::DictionaryImpl()
  {
    m_dictionary.insert("bienvenue");
    m_dictionary.insert("au");
    m_dictionary.insert("tutoriel");
    m_dictionary.insert("micro");
    m_dictionary.insert("services");
  }

  /**
   * Implements IDictionaryService::CheckWord(). Determines
   * if the passed in word is contained in the dictionary.
   * @param word the word to be checked.
   * @return true if the word is in the dictionary,
   *         false otherwise.
   **/
  bool DictionaryImpl::CheckWord(const std::string& word)
  {
    std::string lword(word);
    std::transform(lword.begin(), lword.end(), lword.begin(), ::tolower);
    return (m_dictionary.find(lword) != m_dictionary.end());
  }

}
