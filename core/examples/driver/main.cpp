/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#include <usFrameworkFactory.h>
#include <usFramework.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleImport.h>

#include "usCoreExamplesDriverConfig.h"

#if defined(US_PLATFORM_POSIX)
  #include <dlfcn.h>
#elif defined(US_PLATFORM_WINDOWS)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <strsafe.h>
#else
  #error Unsupported platform
#endif

#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <algorithm>

US_USE_NAMESPACE

#if !defined (US_BUILD_SHARED_LIBS)
static const std::string BUNDLE_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
#else
#ifdef US_PLATFORM_WINDOWS
static const std::string BUNDLE_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
#else
static const std::string BUNDLE_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
#endif
#endif  // US_BUILD_SHARED_LIBS

std::vector<std::string> GetExampleModules()
{
  std::vector<std::string> names;
  names.push_back("eventlistener");
  names.push_back("dictionaryservice");
  names.push_back("frenchdictionary");
  names.push_back("dictionaryclient");
  names.push_back("dictionaryclient2");
  names.push_back("dictionaryclient3");
  names.push_back("spellcheckservice");
  names.push_back("spellcheckclient");
  return names;
}

int main(int /*argc*/, char** /*argv*/)
{
  char cmd[256];

  FrameworkFactory factory;
  Framework* framework = factory.NewFramework(std::map<std::string, std::string>());
  framework->Start();
  ModuleContext* frameworkContext = framework->GetModuleContext();

  std::vector<std::string> availableModules = GetExampleModules();

  /* install all available bundles for this example */
  for (std::vector<std::string>::const_iterator iter = availableModules.begin();
      iter != availableModules.end(); ++iter)
  {
#if defined (US_BUILD_SHARED_LIBS)
      frameworkContext->InstallBundle(BUNDLE_PATH + PATH_SEPARATOR + LIB_PREFIX + (*iter) + LIB_EXT + "/" + (*iter));
#else
      frameworkContext->InstallBundle(BUNDLE_PATH + PATH_SEPARATOR + "usCoreExamplesDriver" + EXE_EXT + "/" + (*iter));
#endif
  }

  std::cout << "> ";
  while(std::cin.getline(cmd, sizeof(cmd)))
  {
    std::string strCmd(cmd);
    if (strCmd == "q")
    {
      break;
    }
    else if (strCmd == "h")
    {
      std::cout << std::left << std::setw(15) << "h" << " This help text\n"
                << std::setw(15) << "l <id | name>" << " Start the bundle with id <id> or name <name>\n"
                << std::setw(15) << "u <id>" << " Stop the bundle with id <id>\n"
                << std::setw(15) << "s" << " Print status information\n"
                << std::setw(15) << "q" << " Quit\n" << std::flush;
    }
    else if (strCmd.find("l ") != std::string::npos)
    {
      std::string idOrName;
      idOrName.assign(strCmd.begin()+2, strCmd.end());
      std::stringstream ss(idOrName);

      long int id = -1;
      ss >> id;
      if (id > 0)
      {
        Module* module = frameworkContext->GetModule(id);
        if (!module)
        {
          std::cout << "Error: unknown id" << std::endl;
        }
        else 
        {
          try
          {
            /* starting an already started bundle does nothing.
                There is no harm in doing it. */
            if (module->IsLoaded())
            {
              std::cout << "Info: module already started" << std::endl;
            }
            module->Start();
          }
          catch (const std::exception& e)
          {
            std::cout << e.what() << std::endl;
          }
        }
      }
      else
      {
        Module* module = frameworkContext->GetModule(idOrName);
        if (!module)
        {
          try
          {
              /* Installing a bundle can't be done by id since that is a
                    framework generated piece of information. */
#if defined (US_BUILD_SHARED_LIBS)
              module = frameworkContext->InstallBundle(BUNDLE_PATH + PATH_SEPARATOR + LIB_PREFIX + idOrName + LIB_EXT + "/" + idOrName);
#else
              module = frameworkContext->InstallBundle(BUNDLE_PATH + PATH_SEPARATOR + "usCoreExamplesDriver" + EXE_EXT + "/" + idOrName);
#endif
          }
          catch (const std::exception& e)
          {
            std::cout << e.what() << std::endl;
          }
        }
        
        if (module)
        {
          try
          {
            /* starting an already started bundle does nothing.
            There is no harm in doing it. */
            if (module->IsLoaded())
            {
              std::cout << "Info: module already started" << std::endl;
            }
            module->Start();
          }
          catch (const std::exception& e)
          {
            std::cout << e.what() << std::endl;
          }
        }
        else
        {
          std::cout << "Info: Unable to install module " << idOrName << std::endl;
        }
      }
    }
    else if (strCmd.find("u ") != std::string::npos)
    {
      std::stringstream ss(strCmd);
      ss.ignore(2);

      long int id = -1;
      ss >> id;
      // TODO: the "system bundle" is 0 in the OSGi spec. Change this once CppMicroServices is
      //    inline with the spec.
      if (id == 1)
      {
        std::cout << "Info: Unloading not possible" << std::endl;
      }
      else
      {
        Module* const module = frameworkContext->GetModule(id);
        if (module)
        {
          try
          {
            module->Stop();

            // Check if it has really been unloaded
            if (module->IsLoaded())
            {
              std::cout << "Info: The bundle is still referenced by another active bundle. It will be stopped when all dependent bundles are stopped." << std::endl;
            }
          }
          catch (const std::exception& e)
          {
            std::cout << e.what() << std::endl;
          }
        }
        else
        {
          std::cout << "Error: unknown id" << std::endl;
        }
      }
    }
    else if (strCmd == "s")
    {
      std::vector<Module*> modules = frameworkContext->GetModules();

      std::cout << std::left;

      std::cout << "Id | " << std::setw(20) << "Name" << " | " << std::setw(9) << "Status" << std::endl;
      std::cout << "-----------------------------------\n";

      for (std::vector<std::string>::const_iterator nameIter = availableModules.begin();
           nameIter != availableModules.end(); ++nameIter)
      {
        std::cout << " - | " << std::setw(20) << *nameIter << " | " << std::setw(9) << "-" << std::endl;
      }

      for (std::vector<Module*>::const_iterator moduleIter = modules.begin();
           moduleIter != modules.end(); ++moduleIter)
      {
        std::cout << std::right << std::setw(2) << (*moduleIter)->GetModuleId() << std::left << " | ";
        std::cout << std::setw(20) << (*moduleIter)->GetName() << " | ";
        std::cout << std::setw(9) << ((*moduleIter)->IsLoaded() ? "ACTIVE" : "RESOLVED");
        std::cout << std::endl;
      }
    }
    else
    {
      std::cout << "Unknown command: " << strCmd << " (type 'h' for help)" << std::endl;
    }
    std::cout << "> ";
  }

  return 0;
}

#ifndef US_BUILD_SHARED_LIBS
US_IMPORT_MODULE(CppMicroServices)
US_IMPORT_MODULE(eventlistener)
US_IMPORT_MODULE(dictionaryservice)
US_IMPORT_MODULE(spellcheckservice)
US_IMPORT_MODULE(frenchdictionary)
US_IMPORT_MODULE(dictionaryclient)
US_IMPORT_MODULE(dictionaryclient2)
US_IMPORT_MODULE(dictionaryclient3)
US_IMPORT_MODULE(spellcheckclient)
#endif
