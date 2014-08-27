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

#include <usModuleActivator.h>
#include <usModuleInitialization.h>

US_USE_NAMESPACE

struct MyStaticModuleActivator : public ModuleActivator
{
  void Load(ModuleContext* /*context*/)
  {
    std::cout << "Hello from a static module." << std::endl;
  }

  void Unload(ModuleContext* /*context*/) {}
};

US_EXPORT_MODULE_ACTIVATOR(MyStaticModuleActivator)

US_INITIALIZE_MODULE
