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

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"

#include "cppmicroservices/shellservice/ShellService.h"

namespace cppmicroservices {

class ShellActivator : public BundleActivator
{
public:

  void Start(BundleContext context)
  {
    m_ShellService.reset(new ShellService());
    context.RegisterService<ShellService>(m_ShellService);
  }

  void Stop(BundleContext)
  {

  }

private:
  std::shared_ptr<ShellService> m_ShellService;
};

}

US_EXPORT_BUNDLE_ACTIVATOR(cppmicroservices::ShellActivator)
