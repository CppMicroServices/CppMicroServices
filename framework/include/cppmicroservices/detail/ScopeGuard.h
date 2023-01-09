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

#ifndef CPPMICROSERVICES_DETAIL_SCOPEGUARD_H
#define CPPMICROSERVICES_DETAIL_SCOPEGUARD_H

#include <functional>

namespace cppmicroservices
{

    namespace detail
    {

        /**
         * A simple, single use scope guard using RAII
         * to guarantee operations are executed when
         * the ScopeGuard object goes out of scope.
         */
        class ScopeGuard
        {
          public:
            template <typename Callable>
            ScopeGuard(Callable&& func) : f(func)
            {
            }

            ScopeGuard(ScopeGuard const&) = delete;
            ScopeGuard& operator=(ScopeGuard const&) = delete;
            ScopeGuard(ScopeGuard&&) = delete;
            ScopeGuard& operator=(ScopeGuard&&) = delete;

            ~ScopeGuard() noexcept
            {
                f(); // must not throw
            }

          private:
            std::function<void()> f;
        };

    } // namespace detail
} // namespace cppmicroservices
#endif
