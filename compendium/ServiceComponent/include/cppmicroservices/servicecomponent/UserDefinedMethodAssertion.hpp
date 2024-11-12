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
#include <type_traits>

namespace cppmicroservices::service::component
{
    // Helper template to check for the existence of a method
    template <typename, typename = std::void_t<>>
    struct has_activate_method : std::false_type
    {
    };

    template <typename T>
    struct has_activate_method<T,
                               std::void_t<decltype(std::declval<T>().Activate(std::shared_ptr<ComponentContext>()))>>
        : std::true_type
    {
    };

    template <typename, typename = std::void_t<>>
    struct has_deactivate_method : std::false_type
    {
    };

    template <typename T>
    struct has_deactivate_method<
        T,
        std::void_t<decltype(std::declval<T>().Deactivate(std::shared_ptr<ComponentContext>()))>> : std::true_type
    {
    };

    template <typename, typename = std::void_t<>>
    struct has_modified_method : std::false_type
    {
    };

    template <typename T>
    struct has_modified_method<
        T,
        std::void_t<decltype(std::declval<T>().Modified(std::shared_ptr<ComponentContext>(),
                                                        std::shared_ptr<cppmicroservices::AnyMap>()))>> : std::true_type
    {
    };

    template <typename T>
    struct UserDefinedMethodAssertion
    {
        virtual ~UserDefinedMethodAssertion();
    };

    template <typename T>
    UserDefinedMethodAssertion<T>::~UserDefinedMethodAssertion()
    {
        static_assert(std::is_base_of_v<UserDefinedMethodAssertion<T>, T>,
                      "T must derive from UserDefinedMethodAssertion<T>");

        static_assert(has_activate_method<T>::value,
                      "Error: An Activate method was not found with the appropriate signature: void "
                      "Activate(std::shared_ptr<ComponentContext> const&)");

        static_assert(has_deactivate_method<T>::value,
                      "Error: A Deactivate method was not found with the appropriate signature: void "
                      "Deactivate(std::shared_ptr<ComponentContext> const&)");

        static_assert(has_modified_method<T>::value,
                      "Error: A Modified method was not found with the appropriate signature: void "
                      "Modified(std::shared_ptr<ComponentContext> const&, "
                      "std::shared_ptr<cppmicroservices::AnyMap> const&)");
    }
} // namespace cppmicroservices::service::component
#endif
