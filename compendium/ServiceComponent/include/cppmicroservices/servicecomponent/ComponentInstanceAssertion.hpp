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

#ifndef COMPONENTINSTANCEASSERTION_hpp
#define COMPONENTINSTANCEASSERTION_hpp

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

namespace cppmicroservices::service::component
{
    class ComponentInstanceAssertion
    {
      public:
        virtual ~ComponentInstanceAssertion() = default; // Add a virtual destructor

        virtual void Modified(std::shared_ptr<ComponentContext> const& context,
                              std::shared_ptr<cppmicroservices::AnyMap> const& configuration)
            = 0;
        virtual void Activate(std::shared_ptr<ComponentContext> const& context) = 0;
        virtual void Deactivate(std::shared_ptr<ComponentContext> const& context) = 0;
    };
} // namespace cppmicroservices::service::component
#endif