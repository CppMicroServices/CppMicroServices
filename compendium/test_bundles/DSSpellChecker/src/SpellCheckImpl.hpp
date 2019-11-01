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

#ifndef SPELLCHECKIMPL_HPP
#define SPELLCHECKIMPL_HPP

#include "ISpellCheckService/ISpellCheckService.hpp"
#include "IDictionaryService/IDictionaryService.hpp"
#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;
namespace DSSpellCheck {

/**
 * A private class that implements a spell check service; see
 * ISpellCheckService for details of the service.
 */
class SpellCheckImpl : public test::ISpellCheckService
{

private:
  std::shared_ptr<test::IDictionaryService> mDictionary;

public:
  SpellCheckImpl(const std::shared_ptr<test::IDictionaryService>& dict) : mDictionary(dict) {}
  ~SpellCheckImpl() override = default;
  std::vector<std::string> Check(const std::string& passage) override;
};

}

#endif // SPELLCHECKIMPL_HPP
