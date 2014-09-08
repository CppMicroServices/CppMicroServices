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

#include <usModule.h>
#include <usModuleRegistry.h>
#include <usSharedLibrary.h>
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

#ifdef US_PLATFORM_WINDOWS
  static const std::string LIB_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
#else
  static const std::string LIB_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
#endif

std::vector<std::string> GetExampleModules()
{
  std::vector<std::string> names;
#ifdef US_BUILD_SHARED_LIBS
  names.push_back("eventlistener");
  names.push_back("dictionaryservice");
  names.push_back("frenchdictionary");
  names.push_back("dictionaryclient");
  names.push_back("dictionaryclient2");
  names.push_back("dictionaryclient3");
  names.push_back("spellcheckservice");
  names.push_back("spellcheckclient");
#endif
  return names;
}

int main(int /*argc*/, char** /*argv*/)
{
  char cmd[256];

  std::vector<std::string> availableModules = GetExampleModules();

  /* module path -> lib handle */
  std::map<std::string, SharedLibrary> libraryHandles;

  SharedLibrary sharedLib(LIB_PATH, "");

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
                << std::setw(15) << "l <id | name>" << " Load the module with id <id> or name <name>\n"
                << std::setw(15) << "u <id>" << " Unload the module with id <id>\n"
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
        Module* module = ModuleRegistry::GetModule(id);
        if (!module)
        {
          std::cout << "Error: unknown id" << std::endl;
        }
        else if (module->IsLoaded())
        {
          std::cout << "Info: module already loaded" << std::endl;
        }
        else
        {
          try
          {
            std::map<std::string, SharedLibrary>::iterator libIter =
                libraryHandles.find(module->GetLocation());
            if (libIter != libraryHandles.end())
            {
              libIter->second.Load();
            }
            else
            {
              // The module has been loaded previously due to a
              // linker dependency
              SharedLibrary libHandle(module->GetLocation());
              libHandle.Load();
              libraryHandles.insert(std::make_pair(libHandle.GetFilePath(), libHandle));
            }
          }
          catch (const std::exception& e)
          {
            std::cout << e.what() << std::endl;
          }
        }
      }
      else
      {
        Module* module = ModuleRegistry::GetModule(idOrName);
        if (!module)
        {
          try
          {
            std::map<std::string, SharedLibrary>::iterator libIter =
                libraryHandles.find(sharedLib.GetFilePath(idOrName));
            if (libIter != libraryHandles.end())
            {
              libIter->second.Load();
            }
            else
            {
              bool libFound = false;
              for (std::vector<std::string>::const_iterator availableModuleIter = availableModules.begin();
                   availableModuleIter != availableModules.end(); ++availableModuleIter)
              {
                if (*availableModuleIter == idOrName)
                {
                  libFound = true;
                }
              }
              if (!libFound)
              {
                std::cout << "Error: unknown example module" << std::endl;
              }
              else
              {
                SharedLibrary libHandle(LIB_PATH, idOrName);
                libHandle.Load();
                libraryHandles.insert(std::make_pair(libHandle.GetFilePath(), libHandle));
              }
            }

            std::vector<Module*> modules = ModuleRegistry::GetModules();
            for (std::vector<Module*>::const_iterator moduleIter = modules.begin();
                 moduleIter != modules.end(); ++moduleIter)
            {
              availableModules.erase(std::remove(availableModules.begin(), availableModules.end(), (*moduleIter)->GetName()),
                                     availableModules.end());
            }

          }
          catch (const std::exception& e)
          {
            std::cout << e.what() << std::endl;
          }
        }
        else if (!module->IsLoaded())
        {
          try
          {
            const std::string modulePath = module->GetLocation();
            std::map<std::string, SharedLibrary>::iterator libIter =
                libraryHandles.find(modulePath);
            if (libIter != libraryHandles.end())
            {
              libIter->second.Load();
            }
            else
            {
              SharedLibrary libHandle(LIB_PATH, idOrName);
              libHandle.Load();
              libraryHandles.insert(std::make_pair(libHandle.GetFilePath(), libHandle));
            }
          }
          catch (const std::exception& e)
          {
            std::cout << e.what() << std::endl;
          }
        }
        else if (module)
        {
          std::cout << "Info: module already loaded" << std::endl;
        }
      }
    }
    else if (strCmd.find("u ") != std::string::npos)
    {
      std::stringstream ss(strCmd);
      ss.ignore(2);

      long int id = -1;
      ss >> id;

      if (id == 1)
      {
        std::cout << "Info: Unloading not possible" << std::endl;
      }
      else
      {
        Module* const module = ModuleRegistry::GetModule(id);
        if (module)
        {
          std::map<std::string, SharedLibrary>::iterator libIter =
              libraryHandles.find(module->GetLocation());
          if (libIter == libraryHandles.end())
          {
            std::cout << "Info: Unloading not possible. The module was loaded by a dependent module." << std::endl;
          }
          else
          {
            try
            {
              libIter->second.Unload();

              // Check if it has really been unloaded
              if (module->IsLoaded())
              {
                std::cout << "Info: The module is still referenced by another loaded module. It will be unloaded when all dependent modules are unloaded." << std::endl;
              }
            }
            catch (const std::exception& e)
            {
              std::cout << e.what() << std::endl;
            }
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
      std::vector<Module*> modules = ModuleRegistry::GetModules();

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
        std::cout << std::setw(9) << ((*moduleIter)->IsLoaded() ? "LOADED" : "UNLOADED");
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
US_IMPORT_MODULE(dictionaryclient)
#endif
