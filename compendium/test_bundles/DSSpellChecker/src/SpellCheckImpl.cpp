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
#include "SpellCheckImpl.hpp"
#include "cppmicroservices/BundleContext.h"
#include <map>

US_MSVC_PUSH_DISABLE_WARNING(4996)

using namespace cppmicroservices;
namespace DSSpellCheck
{
    /**
     * Implements ISpellCheckService::Check(). Checks the given passage for
     * misspelled words.
     *
     * @param passage the passage to spell check.
     * @return A list of misspelled words.
     */
    std::vector<std::string>
    SpellCheckImpl::Check(std::string const& passage)
    {
        std::vector<std::string> errorList;

        // No misspelled words for an empty string.
        if (passage.empty())
        {
            return errorList;
        }

        // Tokenize the passage using spaces and punctuation.
        char const* delimiters = " ,.!?;:";
        char* passageCopy = new char[passage.size() + 1];
        std::memcpy(passageCopy, passage.c_str(), passage.size() + 1);
        char* pch = std::strtok(passageCopy, delimiters);

        {
            // Loop through each word in the passage.
            while (pch)
            {
                std::string word(pch);

                bool correct = false;

                if (mDictionary->CheckWord(word))
                {
                    correct = true;
                }

                // If the word is not correct, then add it
                // to the incorrect word list.
                if (!correct)
                {
                    errorList.push_back(word);
                }

                pch = std::strtok(nullptr, delimiters);
            }
        }

        delete[] passageCopy;

        return errorList;
    }
} // namespace DSSpellCheck

US_MSVC_POP_WARNING
