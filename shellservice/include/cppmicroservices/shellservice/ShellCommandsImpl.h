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

#ifndef CPPMICROSERVICES_SHELLCOMMANDSIMPL_H
#define CPPMICROSERVICES_SHELLCOMMANDSIMPL_H

#include "cppmicroservices/GlobalConfig.h"

#include "ShellCommandInterface.h"

namespace cppmicroservices {

    class ShellCommandLs : public ShellCommandInterface {
    public:
        ShellCommandLs() {};
        virtual ~ShellCommandLs() {};
        virtual void doCommand(cppmicroservices::BundleContext bc, const std::vector<std::string> arguments);
        virtual void help();
    };
    
    class ShellCommandInstall : public ShellCommandInterface {
    public:
        ShellCommandInstall() {};
        virtual ~ShellCommandInstall() {};
        virtual void doCommand(cppmicroservices::BundleContext bc, const std::vector<std::string> arguments);
        virtual void help();
    };
    
    class ShellCommandUninstall : public ShellCommandInterface {
    public:
        ShellCommandUninstall() {};
        virtual ~ShellCommandUninstall() {};
        virtual void doCommand(cppmicroservices::BundleContext bc, const std::vector<std::string> arguments);
        virtual void help();
    };
    
    class ShellCommandStart : public ShellCommandInterface {
    public:
        ShellCommandStart() {};
        virtual ~ShellCommandStart() {};
        virtual void doCommand(cppmicroservices::BundleContext bc, const std::vector<std::string> arguments);
        virtual void help();
    };
    
    class ShellCommandStop : public ShellCommandInterface {
    public:
        ShellCommandStop() {};
        virtual ~ShellCommandStop() {};
        virtual void doCommand(cppmicroservices::BundleContext bc, const std::vector<std::string> arguments);
        virtual void help();
    };
    
}

#endif // CPPMICROSERVICES_SHELLCOMMANDSIMPL_H
