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
#include "ISpellCheckService.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usServiceTracker.h>

#include <iostream>
#include <cstring>

US_USE_NAMESPACE

namespace {

/**
 * This class implements a module that uses a spell checker
 * service to check the spelling of a passage. This module
 * is essentially identical to Example 5, in that it uses the
 * Service Tracker to monitor the dynamic availability of the
 * spell checker service. When loading this module, the thread
 * calling the Load() method is used to read passages from
 * standard input. You can stop spell checking passages by
 * entering an empty line, but to start spell checking again
 * you must un-load and then load the module again.
**/
class US_ABI_LOCAL Activator : public ModuleActivator
{

public:

  Activator()
   : m_context(NULL)
   , m_tracker(NULL)
  {}

  /**
   * Implements ModuleActivator::Load(). Creates a service
   * tracker object to monitor spell checker services. Enters
   * a spell check loop where it reads passages from standard
   * input and checks their spelling using the spell checker service.
   *
   * \note It is very bad practice to use the calling thread to perform a
   *       lengthy process like this; this is only done for the purpose of
   *       the tutorial.
   *
   * @param context the module context for this module.
   */
  void Load(ModuleContext *context)
  {
    m_context = context;

    // Create a service tracker to monitor spell check services.
    m_tracker = new ServiceTracker<ISpellCheckService>(m_context);
    m_tracker->Open();

    std::cout << "Enter a blank line to exit." << std::endl;

    // Loop endlessly until the user enters a blank line
    while (std::cin)
    {
      // Ask the user to enter a passage.
      std::cout << "Enter passage: ";

      std::string passage;
      std::getline(std::cin, passage);

      // Get the selected spell check service, if available.
      ISpellCheckService* checker = m_tracker->GetService();

      // If the user entered a blank line, then
      // exit the loop.
      if (passage.empty())
      {
        break;
      }
      // If there is no spell checker, then say so.
      else if (checker == NULL)
      {
        std::cout << "No spell checker available." << std::endl;
      }
      // Otherwise check passage and print misspelled words.
      else
      {
        std::vector<std::string> errors = checker->Check(passage);

        if (errors.empty())
        {
          std::cout << "Passage is correct." << std::endl;
        }
        else
        {
          std::cout << "Incorrect word(s):" << std::endl;
          for (std::size_t i = 0; i < errors.size(); ++i)
          {
            std::cout << "    " << errors[i] << std::endl;
          }
        }
      }
    }

    // This automatically closes the tracker
    delete m_tracker;
  }

  /**
   * Implements ModuleActivator::Unload(). Does nothing since
   * the C++ Micro Services library will automatically unget any used services.
   * @param context the context for the module.
   */
  void Unload(ModuleContext* /*context*/)
  {
  }

private:

  // Module context
  ModuleContext* m_context;

  // The service tracker
  ServiceTracker<ISpellCheckService>* m_tracker;
};

}

US_EXPORT_MODULE_ACTIVATOR(Activator)
//![Activator]
