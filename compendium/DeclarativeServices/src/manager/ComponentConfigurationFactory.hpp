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

#ifndef COMPONENTCONFIGURATIONFACTORY_HPP
#define COMPONENTCONFIGURATIONFACTORY_HPP

#include "ComponentConfigurationImpl.hpp"
#include "ConfigurationNotifier.hpp"
#include <memory>

namespace cppmicroservices
{
    namespace scrimpl
    {

        class ComponentConfigurationFactory
        {
          public:
            /**
             * Factory method to create the appropriate {@link ComponentConfigurationImpl}
             * object based on the Component' service scope
             */
            static std::shared_ptr<ComponentConfigurationImpl> CreateConfigurationManager(
                std::shared_ptr<const metadata::ComponentMetadata> compDesc,
                cppmicroservices::Bundle const& bundle,
                std::shared_ptr<ComponentRegistry> registry,
                std::shared_ptr<logservice::LogService> logger,
                std::shared_ptr<ConfigurationNotifier> configNotifier);
       };
    } // namespace scrimpl
} // namespace cppmicroservices

#endif /* COMPONENTCONFIGURATIONFACTORY_HPP */
