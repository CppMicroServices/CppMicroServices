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

//! [Activator]
#include "IDictionaryService.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>

#include <set>
#include <algorithm>
#include <memory>

US_USE_NAMESPACE

namespace {

/**
 * This class implements a module activator that uses the module
 * context to register a French language dictionary service
 * with the C++ Micro Services registry during static initialization
 * of the module. The dictionary service interface is
 * defined in Example 2 (dictionaryservice) and is implemented by a
 * nested class. This class is identical to the class in Example 2,
 * except that the dictionary contains French words.
 */
class US_ABI_LOCAL Activator : public ModuleActivator
{

private:

  /**
   * A private inner class that implements a dictionary service;
   * see DictionaryService for details of the service.
   */
  class DictionaryImpl : public IDictionaryService
  {
    // The set of words contained in the dictionary.
    std::set<std::string> m_dictionary;

  public:

    DictionaryImpl()
    {
      m_dictionary.insert("bienvenue");
      m_dictionary.insert("au");
      m_dictionary.insert("tutoriel");
      m_dictionary.insert("micro");
      m_dictionary.insert("services");
    }

    /**
     * Implements DictionaryService.checkWord(). Determines
     * if the passed in word is contained in the dictionary.
     * @param word the word to be checked.
     * @return true if the word is in the dictionary,
     *         false otherwise.
     **/
    bool CheckWord(const std::string& word)
    {
      std::string lword(word);
      std::transform(lword.begin(), lword.end(), lword.begin(), ::tolower);

      return m_dictionary.find(lword) != m_dictionary.end();
    }
  };

  std::auto_ptr<DictionaryImpl> m_dictionaryService;


public:

  /**
   * Implements ModuleActivator::Load(). Registers an
   * instance of a dictionary service using the module context;
   * attaches properties to the service that can be queried
   * when performing a service look-up.
   * @param context the context for the module.
   */
  void Load(ModuleContext* context)
  {
    m_dictionaryService.reset(new DictionaryImpl);
    ServiceProperties props;
    props["Language"] = std::string("French");
    context->RegisterService<IDictionaryService>(m_dictionaryService.get(), props);
  }

  /**
   * Implements ModuleActivator::Unload(). Does nothing since
   * the C++ Micro Services library will automatically unregister any registered services.
   * @param context the context for the module.
   */
  void Unload(ModuleContext* /*context*/)
  {
    // NOTE: The service is automatically unregistered
  }

};

}

US_EXPORT_MODULE_ACTIVATOR(Activator)
//![Activator]
