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
#include "ISpellCheckService.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>
#include <usServiceTracker.h>
#include <usServiceTrackerCustomizer.h>

#include <map>
#include <cstring>
#include <memory>

US_USE_NAMESPACE

namespace {

/**
 * This class implements a module that implements a spell
 * checker service. The spell checker service uses all available
 * dictionary services to check for the existence of words in
 * a given sentence. This module uses a ServiceTracker to
 * monitors the dynamic availability of dictionary services,
 * and to aggregate all available dictionary services as they
 * arrive and depart. The spell checker service is only registered
 * if there are dictionary services available, thus the spell
 * checker service will appear and disappear as dictionary
 * services appear and disappear, respectively.
**/
class US_ABI_LOCAL Activator : public ModuleActivator, public ServiceTrackerCustomizer<IDictionaryService>
{

private:

  /**
   * A private inner class that implements a spell check service; see
   * ISpellCheckService for details of the service.
   */
  class SpellCheckImpl : public ISpellCheckService
  {

  private:

    typedef std::map<ServiceReference<IDictionaryService>, IDictionaryService*> RefToServiceType;
    RefToServiceType m_refToSvcMap;

  public:

    /**
     * Implements ISpellCheckService::Check(). Checks the given passage for
     * misspelled words.
     *
     * @param passage the passage to spell check.
     * @return A list of misspelled words.
     */
    std::vector<std::string> Check(const std::string& passage)
    {
      std::vector<std::string> errorList;

      // No misspelled words for an empty string.
      if (passage.empty())
      {
        return errorList;
      }

      // Tokenize the passage using spaces and punctuation.
      const char* delimiters = " ,.!?;:";
      char* passageCopy = new char[passage.size()+1];
      std::memcpy(passageCopy, passage.c_str(), passage.size()+1);
      char* pch = std::strtok(passageCopy, delimiters);

      {
        // Lock the m_refToSvcMap member using your favorite thread library here...
        // MutexLocker lock(&m_refToSvcMapMutex)

        // Loop through each word in the passage.
        while (pch)
        {
          std::string word(pch);

          bool correct = false;

          // Check each available dictionary for the current word.
          for (RefToServiceType::const_iterator i = m_refToSvcMap.begin();
               (!correct) && (i != m_refToSvcMap.end()); ++i)
          {
            IDictionaryService* dictionary = i->second;

            if (dictionary->CheckWord(word))
            {
              correct = true;
            }
          }

          // If the word is not correct, then add it
          // to the incorrect word list.
          if (!correct)
          {
            errorList.push_back(word);
          }

          pch = std::strtok(NULL, delimiters);
        }
      }

      delete[] passageCopy;

      return errorList;
    }

    std::size_t AddDictionary(const ServiceReference<IDictionaryService>& ref, IDictionaryService* dictionary)
    {
      // Lock the m_refToSvcMap member using your favorite thread library here...
      // MutexLocker lock(&m_refToSvcMapMutex)

      m_refToSvcMap.insert(std::make_pair(ref, dictionary));

      return m_refToSvcMap.size();
    }

    std::size_t RemoveDictionary(const ServiceReference<IDictionaryService>& ref)
    {
      // Lock the m_refToSvcMap member using your favorite thread library here...
      // MutexLocker lock(&m_refToSvcMapMutex)

      m_refToSvcMap.erase(ref);

      return m_refToSvcMap.size();
    }
  };

  virtual IDictionaryService* AddingService(const ServiceReference<IDictionaryService>& reference)
  {
    IDictionaryService* dictionary = m_context->GetService(reference);
    std::size_t count = m_spellCheckService->AddDictionary(reference, dictionary);
    if (!m_spellCheckReg && count > 1)
    {
      m_spellCheckReg = m_context->RegisterService<ISpellCheckService>(m_spellCheckService.get());
    }
    return dictionary;
  }

  virtual void ModifiedService(const ServiceReference<IDictionaryService>& /*reference*/,
                               IDictionaryService* /*service*/)
  {
    // do nothing
  }

  virtual void RemovedService(const ServiceReference<IDictionaryService>& reference,
                              IDictionaryService* /*service*/)
  {
    if (m_spellCheckService->RemoveDictionary(reference) < 2 && m_spellCheckReg)
    {
      m_spellCheckReg.Unregister();
      m_spellCheckReg = 0;
    }
  }

  std::auto_ptr<SpellCheckImpl> m_spellCheckService;
  ServiceRegistration<ISpellCheckService> m_spellCheckReg;

  ModuleContext* m_context;
  std::auto_ptr<ServiceTracker<IDictionaryService> > m_tracker;

public:

  Activator()
    : m_context(NULL)
  {}

  /**
   * Implements ModuleActivator::Load(). Registers an
   * instance of a dictionary service using the module context;
   * attaches properties to the service that can be queried
   * when performing a service look-up.
   *
   * @param context the context for the module.
   */
  void Load(ModuleContext* context)
  {
    m_context = context;

    m_spellCheckService.reset(new SpellCheckImpl);
    m_tracker.reset(new ServiceTracker<IDictionaryService>(context, this));
    m_tracker->Open();
  }

  /**
   * Implements ModuleActivator::Unload(). Does nothing since
   * the C++ Micro Services library will automatically unregister any registered services
   * and release any used services.
   *
   * @param context the context for the module.
   */
  void Unload(ModuleContext* /*context*/)
  {
    // NOTE: The service is automatically unregistered

    m_tracker->Close();
  }

};

}

US_EXPORT_MODULE_ACTIVATOR(Activator)
//![Activator]
