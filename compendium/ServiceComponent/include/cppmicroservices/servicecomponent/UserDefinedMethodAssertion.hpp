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

#ifndef UserDefinedMethodAssertion_hpp
#define UserDefinedMethodAssertion_hpp

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"

namespace cppmicroservices::service::component
{
    template <typename T>
    struct UserDefinedMethodAssertion
    {
        virtual ~UserDefinedMethodAssertion();
    };

    template <typename T>
    UserDefinedMethodAssertion<T>::~UserDefinedMethodAssertion()
    {
        static_assert(std::is_base_of_v<UserDefinedMethodAssertion<T>, T>);

        static_assert(std::is_void_v<decltype(std::declval<T>().Activate(std::shared_ptr<ComponentContext>()))>);

        static_assert(std::is_void_v<decltype(std::declval<T>().Deactivate(std::shared_ptr<ComponentContext>()))>);

        static_assert(
            std::is_void_v<decltype(std::declval<T>().Modified(std::shared_ptr<ComponentContext>(),
                                                               std::shared_ptr<cppmicroservices::AnyMap>()))>);
    }
} // namespace cppmicroservices::service::component
#endif
