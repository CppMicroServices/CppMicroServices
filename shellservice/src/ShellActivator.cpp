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

#include <memory>

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"

#include "cppmicroservices/shellservice/ShellServiceImpl.h"
#include "cppmicroservices/shellservice/ShellCommandsImpl.h"

namespace cppmicroservices {

class ShellActivator : public BundleActivator
{
public:
  void Start(BundleContext context) override
  {
    context.RegisterService<ShellService>(std::make_shared<ShellServiceImpl>());
    context.RegisterService<ShellCommandInterface>(std::make_shared<ShellCommandLs>(), {{"commandName", Any(std::string("us-ls"))}});
    context.RegisterService<ShellCommandInterface>(std::make_shared<ShellCommandInstall>(), {{"commandName", Any(std::string("us-install"))}});
    context.RegisterService<ShellCommandInterface>(std::make_shared<ShellCommandUninstall>(), {{"commandName", Any(std::string("us-uninstall"))}});
    context.RegisterService<ShellCommandInterface>(std::make_shared<ShellCommandStart>(), {{"commandName", Any(std::string("us-start"))}});
    context.RegisterService<ShellCommandInterface>(std::make_shared<ShellCommandStop>(), {{"commandName", Any(std::string("us-stop"))}});
  }

  void Stop(BundleContext) override {}
};
}

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(cppmicroservices::ShellActivator)
